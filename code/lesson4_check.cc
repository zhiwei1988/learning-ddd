// Verifies every C++ claim made in lesson 0004 actually holds under clang -std=c++20.
#include <cassert>
#include <compare>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// ============ Value Object ============
class Money {
 public:
  Money(long long cents, std::string currency)
      : cents_(cents), currency_(std::move(currency)) {}

  // Equality by value — the whole point of a value object.
  bool operator==(const Money&) const = default;

  // Side-effect-free: returns a new value, mutates nothing.
  Money plus(const Money& other) const {
    if (currency_ != other.currency_) throw std::invalid_argument{"currency mismatch"};
    return Money{cents_ + other.cents_, currency_};
  }

  long long cents() const { return cents_; }

 private:
  long long   cents_;
  std::string currency_;
};

// ============ Entity ============
using OrderId = std::string;

class Order {
 public:
  explicit Order(OrderId id) : id_(std::move(id)) {}

  // Equality by identity ONLY. Attributes deliberately ignored.
  bool operator==(const Order& other) const { return id_ == other.id_; }

  void add_line(const Money& amount) { lines_.push_back(amount); }

 private:
  OrderId            id_;
  std::vector<Money> lines_;
};

// ============ The const-member trap ============
class BadMoney {
 public:
  BadMoney(long long c, std::string cur) : cents_(c), currency_(std::move(cur)) {}
  bool operator==(const BadMoney&) const = default;

 private:
  const long long   cents_;      // "immutable", right?
  const std::string currency_;
};

int main() {
  // --- Claim 1: two value objects with equal attributes ARE equal.
  Money a{10000, "CNY"};
  Money b{10000, "CNY"};
  assert(a == b);

  // --- Claim 2: defaulted operator== compares every member.
  assert((!(a == Money{10000, "USD"})));

  // --- Claim 3: value objects copy freely and independently (no aliasing).
  Money c = a;
  Money d = c.plus(Money{500, "CNY"});
  assert((a == Money{10000, "CNY"}));   // original untouched
  assert(d.cents() == 10500);

  // --- Claim 4: two entities with identical attributes are NOT equal.
  Order o1{"ORD-1"};
  Order o2{"ORD-2"};
  o1.add_line(Money{10000, "CNY"});
  o2.add_line(Money{10000, "CNY"});
  assert(!(o1 == o2));                // same attributes, different things

  // --- Claim 5: an entity stays itself as its attributes change.
  Order o1_later = o1;
  o1_later.add_line(Money{999, "CNY"});
  assert(o1 == o1_later);             // attributes differ, identity holds

  // --- Claim 6: value objects are usable in containers by value.
  std::vector<Money> wallet{a, b, d};
  assert(wallet.size() == 3);

  // --- Claim 7: BadMoney is still constructible and comparable...
  BadMoney bm{1, "CNY"};
  BadMoney bm2{1, "CNY"};
  assert(bm == bm2);

  // --- Claim 8 (THE POINT): const members delete copy-assignment.
  static_assert(std::is_copy_assignable_v<Money>,      "Money must be assignable");
  static_assert(std::is_move_assignable_v<Money>,      "Money must be movable");
  static_assert(!std::is_copy_assignable_v<BadMoney>,  "const members kill assignment");
  static_assert(!std::is_move_assignable_v<BadMoney>,  "const members kill move");

  // Money can live in a vector that reallocates/sorts; BadMoney fights you.
  static_assert(std::is_copy_constructible_v<BadMoney>, "still copy-constructible though");

  return 0;
}
