// Verifies every C++ claim in lesson 6 (Domain Events).
//   c++ -std=c++20 -Wall -Wextra -o /tmp/lesson6_check lesson6_check.cc && /tmp/lesson6_check
// Apple clang 17 / arm64: zero warnings, all assertions green.

#include <cassert>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

// --- Ids and Money: value objects from earlier lessons, trimmed ---
struct OrderId    { std::uint64_t v; bool operator==(const OrderId&)    const = default; };
struct CustomerId { std::uint64_t v; bool operator==(const CustomerId&) const = default; };
struct Money      { std::int64_t cents; bool operator==(const Money&)   const = default; };

using TimePoint = std::chrono::system_clock::time_point;

// --- Domain events: each names a PAST fact. Past tense, immutable, value equality.
//     Note it carries the IDENTITY of entities involved (Evans), never a pointer. ---
struct OrderConfirmed {
  OrderId    order;
  CustomerId customer;   // "identity of entities involved" — held BY ID
  Money      total;      // a snapshot of the fact, not a live handle
  TimePoint  at;
  bool operator==(const OrderConfirmed&) const = default;  // correct here (a value)
};
struct OrderCancelled {
  OrderId   order;
  TimePoint at;
  bool operator==(const OrderCancelled&) const = default;
};

// The CLOSED set of facts this aggregate can emit. A sum type: the C++-idiomatic
// way to say "one of these, and the compiler knows every one." (Java: an interface
// hierarchy — open, and no exhaustiveness checking.)
using OrderEvent = std::variant<OrderConfirmed, OrderCancelled>;

// --- The aggregate root raises events as part of the state change ---
class Order {
 public:
  Order(OrderId id, CustomerId customer) : id_(id), customer_(customer) {}

  void add_line(Money subtotal) {
    if (confirmed_) throw std::domain_error{"order already confirmed"};
    total_.cents += subtotal.cents;
  }

  // The fact and the state change are produced together, in one operation.
  void confirm(TimePoint now) {
    if (confirmed_) throw std::domain_error{"order already confirmed"};
    confirmed_ = true;
    events_.push_back(OrderConfirmed{id_, customer_, total_, now});
  }

  Money total()     const { return total_; }
  bool  confirmed() const { return confirmed_; }

  // The application layer pulls the recorded facts and publishes them AFTER the
  // aggregate's transaction commits. Pulling clears the buffer: released once.
  std::vector<OrderEvent> pull_events() { return std::exchange(events_, {}); }

 private:
  OrderId    id_;
  CustomerId customer_;         // another aggregate, held BY ID (lesson 5)
  Money      total_{0};
  bool       confirmed_ = false;
  std::vector<OrderEvent> events_;
};

// A DIFFERENT aggregate / subsystem. The publisher (Order) has no idea it exists.
struct Inventory { int units = 100; };

// Application-layer dispatch: route each fact to whoever cares. std::visit forces
// us to handle the variant; adding a new event type surfaces here at compile time.
void publish(const OrderEvent& e, Inventory& inv, int& sms_sent) {
  std::visit([&](const auto& ev) {
    (void)ev;
    using T = std::decay_t<decltype(ev)>;
    if constexpr (std::is_same_v<T, OrderConfirmed>) {
      inv.units -= 1;   // deduct stock — a separate aggregate, updated later
      sms_sent  += 1;   // notify — a second, independent subscriber
    }
    // OrderCancelled: no subscriber here. That's fine; events don't command.
  }, e);
}

int main() {
  const TimePoint t0 = std::chrono::system_clock::now();

  // (1) A domain event is a VALUE OBJECT: immutable, compared by value.
  //     `= default` is CORRECT here — the exact opposite of Order, an entity,
  //     where `= default` on == would be the headline bug (lessons 4 & 5).
  OrderConfirmed a{OrderId{1}, CustomerId{7}, Money{300}, t0};
  OrderConfirmed b = a;                                     // copying a fact is cheap
  assert(a == b);                                           // equal member-by-member
  static_assert(std::is_copy_assignable_v<OrderConfirmed>); // a value: freely copyable

  // (2) The root produces the fact AS PART OF the state change — not before.
  Order o{OrderId{1}, CustomerId{7}};
  o.add_line(Money{120});
  o.add_line(Money{180});
  assert(o.pull_events().empty());                          // nothing happened yet
  o.confirm(t0);
  assert(o.confirmed());
  auto events = o.pull_events();
  assert(events.size() == 1);
  assert(std::holds_alternative<OrderConfirmed>(events[0]));
  assert(std::get<OrderConfirmed>(events[0]).total    == Money{300});
  assert(std::get<OrderConfirmed>(events[0]).customer == CustomerId{7});

  // (3) A fact happens ONCE. Re-confirming is rejected; no duplicate fact is minted.
  bool threw = false;
  try { o.confirm(t0); } catch (const std::domain_error&) { threw = true; }
  assert(threw);
  assert(o.pull_events().empty());

  // (4) EVENTUAL CONSISTENCY: effects in OTHER aggregates happen AFTER the order's
  //     transaction, when the app publishes the pulled facts. confirm() touches
  //     nothing outside the order.
  Order o2{OrderId{2}, CustomerId{9}};
  o2.add_line(Money{500});
  Inventory inv;
  int sms = 0;
  o2.confirm(t0);
  assert(inv.units == 100 && sms == 0);                     // confirm changed nothing outside
  for (const auto& e : o2.pull_events()) publish(e, inv, sms);
  assert(inv.units == 99);                                  // stock deducted — later, decoupled
  assert(sms == 1);                                         // SMS fired too; Order never knew

  // (5) Prove a BAD thing with a GREEN assertion: if the app FORGETS to publish
  //     (drops the pulled events on the floor), the downstream effects silently
  //     never happen. No error, no warning. This is precisely why real systems
  //     need an outbox — a forward hook, not this lesson.
  Order o3{OrderId{3}, CustomerId{9}};
  o3.add_line(Money{80});
  o3.confirm(t0);
  o3.pull_events();                                         // pulled... and discarded (the bug)
  assert(inv.units == 99);                                  // still 99: the deduction vanished

  return 0;
}
