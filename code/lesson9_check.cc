// Verifies every C++ claim in lesson 9 (Domain Service).
//   c++ -std=c++20 -Wall -Wextra -o /tmp/lesson9_check lesson9_check.cc && /tmp/lesson9_check
// Apple clang 17 / arm64: zero warnings, all assertions green.
//
// Domain Service = a significant domain process that is NOT a natural
// responsibility of any one entity or value object. It lives in the DOMAIN
// layer (business rules), not the application layer (orchestration).

#include <cassert>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

// ============================================================================
// DOMAIN LAYER
// ============================================================================

struct WalletId { std::uint64_t v; bool operator==(const WalletId&) const = default; };

class Money {  // value object — from lesson 4
 public:
  Money(long long cents, std::string ccy) : cents_(cents), currency_(std::move(ccy)) {
    if (cents_ < 0) throw std::invalid_argument{"negative money"};
  }
  bool operator==(const Money&) const = default;
  long long         cents()    const { return cents_; }
  const std::string& currency() const { return currency_; }
  Money plus(const Money& o) const {
    if (currency_ != o.currency_) throw std::invalid_argument{"currency mismatch"};
    return Money{cents_ + o.cents_, currency_};
  }
  Money minus(const Money& o) const {
    if (currency_ != o.currency_) throw std::invalid_argument{"currency mismatch"};
    if (cents_ < o.cents_) throw std::domain_error{"insufficient funds"};
    return Money{cents_ - o.cents_, currency_};
  }
 private:
  long long   cents_;
  std::string currency_;
};

class Wallet {  // aggregate root — knows ONLY its own balance invariant
 public:
  Wallet(WalletId id, Money balance) : id_(id), balance_(std::move(balance)) {}
  WalletId     id()      const { return id_; }
  const Money& balance() const { return balance_; }

  void debit(const Money& amount)  { balance_ = balance_.minus(amount); }
  void credit(const Money& amount) { balance_ = balance_.plus(amount); }

  // BAD shape kept for the green-assert proof below: a wallet should not own
  // "transfer to another wallet". That process spans two aggregates.
  void transfer_to_BAD(Wallet& other, const Money& amount) {
    debit(amount);
    other.credit(amount);
  }

 private:
  WalletId id_;
  Money    balance_;
};

// Domain Service: named in the ubiquitous language, holds the process that
// doesn't belong on either Wallet. Stateless — no fields of its own.
struct TransferFunds {
  void operator()(Wallet& from, Wallet& to, const Money& amount) const {
    if (from.id() == to.id())
      throw std::domain_error{"cannot transfer to self"};
    if (from.balance().currency() != to.balance().currency())
      throw std::domain_error{"currency mismatch across wallets"};
    if (from.balance().currency() != amount.currency())
      throw std::domain_error{"amount currency mismatch"};
    // Each wallet still guards its OWN invariant (debit rejects overdraft).
    from.debit(amount);
    to.credit(amount);
  }
};

class WalletRepository {
 public:
  virtual ~WalletRepository() = default;
  virtual std::optional<Wallet> by_id(WalletId) const = 0;
  virtual void save(const Wallet&) = 0;
};

// ============================================================================
// APPLICATION LAYER — thin orchestration. NO business rules.
// ============================================================================

class TransferFundsAppService {
 public:
  TransferFundsAppService(WalletRepository& repo) : repo_(repo) {}

  void execute(WalletId from_id, WalletId to_id, Money amount) {
    Wallet from = repo_.by_id(from_id).value();
    Wallet to   = repo_.by_id(to_id).value();
    TransferFunds{}(from, to, amount);   // domain process lives HERE (domain)
    repo_.save(from);
    repo_.save(to);
  }

 private:
  WalletRepository& repo_;
};

// BAD twin: same orchestration shape, but the RULES leaked into the app layer.
// This is the anemic-domain smell the lesson drills.
class AnemicTransferAppService {
 public:
  explicit AnemicTransferAppService(WalletRepository& repo) : repo_(repo) {}

  void execute(WalletId from_id, WalletId to_id, Money amount) {
    Wallet from = repo_.by_id(from_id).value();
    Wallet to   = repo_.by_id(to_id).value();
    // Business rules in the application layer — the domain objects are bags.
    if (from.id() == to_id) throw std::domain_error{"cannot transfer to self"};
    if (from.balance().currency() != to.balance().currency())
      throw std::domain_error{"currency mismatch across wallets"};
    if (from.balance().cents() < amount.cents())
      throw std::domain_error{"insufficient funds"};
    from.debit(amount);
    to.credit(amount);
    repo_.save(from);
    repo_.save(to);
  }

 private:
  WalletRepository& repo_;
};

// ============================================================================
// INFRASTRUCTURE
// ============================================================================

class InMemoryWalletRepository : public WalletRepository {
 public:
  std::optional<Wallet> by_id(WalletId id) const override {
    auto it = store_.find(id.v);
    if (it == store_.end()) return std::nullopt;
    return it->second;
  }
  void save(const Wallet& w) override { store_.insert_or_assign(w.id().v, w); }
 private:
  std::unordered_map<std::uint64_t, Wallet> store_;
};

int main() {
  InMemoryWalletRepository repo;
  repo.save(Wallet{WalletId{1}, Money{1000, "CNY"}});
  repo.save(Wallet{WalletId{2}, Money{100,  "CNY"}});

  // (1) Happy path: domain service + thin app service.
  TransferFundsAppService app{repo};
  app.execute(WalletId{1}, WalletId{2}, Money{300, "CNY"});
  assert((repo.by_id(WalletId{1})->balance() == Money{700, "CNY"}));
  assert((repo.by_id(WalletId{2})->balance() == Money{400, "CNY"}));

  // (2) Domain service rejects overdraft — rule lives with TransferFunds/Wallet,
  //     not in the application layer. App service just propagates the exception.
  bool rejected = false;
  try {
    app.execute(WalletId{1}, WalletId{2}, Money{99999, "CNY"});
  } catch (const std::domain_error&) {
    rejected = true;
  }
  assert(rejected);
  assert((repo.by_id(WalletId{1})->balance() == Money{700, "CNY"}));  // unchanged

  // (3) Pure domain call works with NO repository — domain services are
  //     testable without infrastructure (same inward-dependency payoff as L7).
  Wallet a{WalletId{10}, Money{500, "CNY"}};
  Wallet b{WalletId{11}, Money{0,   "CNY"}};
  TransferFunds{}(a, b, Money{200, "CNY"});
  assert((a.balance() == Money{300, "CNY"}));
  assert((b.balance() == Money{200, "CNY"}));

  // (4) GREEN assertion of a BAD thing — putting "transfer" on Wallet itself.
  //     It "works", but the process now pretends to be a natural responsibility
  //     of one wallet. The other wallet is a passive victim. Evans: forcing the
  //     process onto an entity distorts the model.
  Wallet x{WalletId{20}, Money{100, "CNY"}};
  Wallet y{WalletId{21}, Money{0,   "CNY"}};
  x.transfer_to_BAD(y, Money{40, "CNY"});
  assert((x.balance() == Money{60, "CNY"}));
  assert((y.balance() == Money{40, "CNY"}));  // "works" — and that is the trap

  // (5) GREEN assertion of the OTHER bad thing — anemic application service.
  //     Same numbers move, but grep the file: the currency/self/overdraft checks
  //     live in AnemicTransferAppService, not in the domain. The Wallet is a bag.
  repo.save(Wallet{WalletId{30}, Money{1000, "CNY"}});
  repo.save(Wallet{WalletId{31}, Money{0,    "CNY"}});
  AnemicTransferAppService anemic{repo};
  anemic.execute(WalletId{30}, WalletId{31}, Money{250, "CNY"});
  assert((repo.by_id(WalletId{30})->balance() == Money{750, "CNY"}));
  assert((repo.by_id(WalletId{31})->balance() == Money{250, "CNY"}));  // moves — and that is the smell

  return 0;
}
