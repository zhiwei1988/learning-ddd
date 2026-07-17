// Verifies every claim made in lessons/0005-aggregate.html
//   c++ -std=c++20 -Wall -Wextra -o /tmp/lesson5_check lesson5_check.cc && /tmp/lesson5_check
//
// The invariant under test: an order's total must never exceed its credit limit.
// Only the aggregate root can be trusted to hold that line.

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// ---------- Value objects (lesson 4) ----------

class Money {
 public:
  Money(std::int64_t cents, std::string ccy)
      : cents_(cents), currency_(std::move(ccy)) {}

  bool operator==(const Money&) const = default;
  auto operator<=>(const Money&) const = default;

  Money plus(const Money& other) const {
    if (currency_ != other.currency_)
      throw std::invalid_argument{"currency mismatch"};
    return Money{cents_ + other.cents_, currency_};
  }
  Money times(int n) const { return Money{cents_ * n, currency_}; }

  std::int64_t cents() const { return cents_; }

 private:
  std::int64_t cents_;
  std::string currency_;
};

struct Sku {
  std::string value;
  bool operator==(const Sku&) const = default;
};

// Identity types. Distinct types so the compiler rejects passing an OrderId
// where a CustomerId belongs.
struct OrderId {
  std::string value;
  bool operator==(const OrderId&) const = default;
};
struct CustomerId {
  std::string value;
  bool operator==(const CustomerId&) const = default;
};

// ---------- Aggregate internals ----------

// An entity, but a LOCAL one: its identity only means anything inside the
// order that owns it. Nobody outside ever holds an OrderLine.
class OrderLine {
 public:
  OrderLine(Sku sku, int qty, Money unit_price)
      : sku_(std::move(sku)), qty_(qty), unit_price_(std::move(unit_price)) {
    if (qty <= 0) throw std::invalid_argument{"qty must be positive"};
  }

  Money subtotal() const { return unit_price_.times(qty_); }
  const Sku& sku() const { return sku_; }
  int qty() const { return qty_; }

 private:
  Sku   sku_;
  int   qty_;
  Money unit_price_;
};

// ---------- Aggregate root ----------

class Order {
 public:
  Order(OrderId id, CustomerId customer, Money credit_limit)
      : id_(std::move(id)),
        customer_id_(std::move(customer)),
        credit_limit_(std::move(credit_limit)) {}

  // Identity only — an entity (lesson 4).
  bool operator==(const Order& other) const { return id_ == other.id_; }

  // The ONLY way in. The invariant is checked here, on the whole aggregate,
  // before any state changes. Nothing outside can reach past this door.
  void add_line(Sku sku, int qty, Money unit_price) {
    OrderLine candidate{std::move(sku), qty, std::move(unit_price)};
    const Money would_be = total().plus(candidate.subtotal());
    if (credit_limit_ < would_be)
      throw std::domain_error{"order total would exceed credit limit"};
    lines_.push_back(std::move(candidate));
  }

  Money total() const {
    Money sum{0, "CNY"};
    for (const auto& l : lines_) sum = sum.plus(l.subtotal());
    return sum;
  }

  // Read-only window. Handing out a non-const reference here would let callers
  // push_back straight past add_line — see leaked_lines() below.
  const std::vector<OrderLine>& lines() const { return lines_; }

  const CustomerId& customer_id() const { return customer_id_; }

  // The trap, kept here only so the test can demonstrate it. Never ship this.
  std::vector<OrderLine>& leaked_lines() { return lines_; }

 private:
  OrderId                id_;
  CustomerId             customer_id_;  // another aggregate, held BY ID
  Money                  credit_limit_;
  std::vector<OrderLine> lines_;        // parts, owned BY VALUE
};

// Claim: holding another aggregate by id keeps Order a standalone class —
// it does not need Customer's definition to compile.
static_assert(std::is_same_v<decltype(std::declval<const Order&>().customer_id()),
                             const CustomerId&>);

int main() {
  const Money limit{10'000, "CNY"};  // 100.00

  // 1. Root enforces the invariant on the way in.
  Order o{OrderId{"O-1"}, CustomerId{"C-9"}, limit};
  o.add_line(Sku{"APPLE"}, 2, Money{3'000, "CNY"});  // 60.00
  assert(o.total() == Money(6'000, "CNY"));
  assert(o.lines().size() == 1);

  // 2. A line that would breach the limit is rejected, and — critically —
  //    the aggregate is left untouched. No half-applied change.
  bool threw = false;
  try {
    o.add_line(Sku{"STEAK"}, 1, Money{5'000, "CNY"});  // 60 + 50 > 100
  } catch (const std::domain_error&) {
    threw = true;
  }
  assert(threw);
  assert(o.total() == Money(6'000, "CNY"));
  assert(o.lines().size() == 1);

  // 3. Exactly at the limit is fine: the rule is "must not exceed".
  o.add_line(Sku{"MILK"}, 2, Money{2'000, "CNY"});  // 60 + 40 == 100
  assert(o.total() == limit);

  // 4. THE LEAK. A non-const accessor lets outside code bypass the root
  //    entirely. This compiles, runs, and silently breaks the invariant.
  o.leaked_lines().push_back(OrderLine{Sku{"GOLD"}, 1, Money{999'999, "CNY"}});
  assert(limit < o.total());  // invariant violated, and nothing complained

  // 5. Identity equality survives all attribute changes (lesson 4 holds).
  const Order same_id{OrderId{"O-1"}, CustomerId{"C-OTHER"}, Money{1, "CNY"}};
  assert(o == same_id);

  return 0;
}
