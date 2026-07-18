// Verifies every C++ claim in lesson 7 (Repository & Layered Architecture).
//   c++ -std=c++20 -Wall -Wextra -o /tmp/lesson7_check lesson7_check.cc && /tmp/lesson7_check
// Apple clang 17 / arm64: zero warnings, all assertions green.
//
// The whole file is ONE translation unit, but read it as three layers stacked
// top-to-bottom. Every dependency arrow points DOWNWARD in this file and INWARD
// toward the domain: infrastructure and application know the domain; the domain
// knows neither. That direction is the entire lesson.

#include <cassert>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

// ============================================================================
// DOMAIN LAYER  — the innermost circle. Depends on NOTHING below it. Notice the
// includes this file needs: not one database or framework header lives here.
// ============================================================================

struct OrderId    { std::uint64_t v; bool operator==(const OrderId&)    const = default; };
struct CustomerId { std::uint64_t v; bool operator==(const CustomerId&) const = default; };

struct OrderConfirmed {
  OrderId    order;
  CustomerId customer;
  long long  total_cents;
  bool operator==(const OrderConfirmed&) const = default;
};
using OrderEvent = std::variant<OrderConfirmed>;

class Order {  // aggregate root (slimmed from lessons 5–6)
 public:
  Order(OrderId id, CustomerId c) : id_(id), customer_(c) {}
  OrderId   id()          const { return id_; }
  long long total_cents() const { return total_; }
  bool      confirmed()   const { return confirmed_; }

  void add_line(long long cents) {
    if (confirmed_) throw std::domain_error{"order already confirmed"};
    total_ += cents;
  }
  void confirm() {
    if (confirmed_) throw std::domain_error{"order already confirmed"};
    confirmed_ = true;
    events_.push_back(OrderConfirmed{id_, customer_, total_});
  }
  std::vector<OrderEvent> pull_events() { return std::exchange(events_, {}); }

 private:
  OrderId    id_;
  CustomerId customer_;
  long long  total_ = 0;
  bool       confirmed_ = false;
  std::vector<OrderEvent> events_;
};

// The PORT, declared IN the domain layer: an abstract interface. Domain and
// application code depend only on this, never on how an order is stored. There
// is ONE repository per aggregate ROOT — no OrderLineRepository, because lines
// live inside the Order aggregate and are reached only through the root.
class OrderRepository {
 public:
  virtual ~OrderRepository() = default;
  virtual std::optional<Order> by_id(OrderId) const = 0;  // "collection illusion"
  virtual void save(const Order&) = 0;
};

// Publishing is infrastructure too. Another port; the real message bus lives
// outside, implementing this.
class EventPublisher {
 public:
  virtual ~EventPublisher() = default;
  virtual void publish(const OrderEvent&) = 0;
};

// ============================================================================
// APPLICATION LAYER  — thin orchestration. Holds NO business rules. It wires the
// ports together and runs the load -> act -> save -> publish dance. It names only
// the ABSTRACTIONS (OrderRepository&, EventPublisher&), never a concrete class.
// ============================================================================

class ConfirmOrderService {
 public:
  ConfirmOrderService(OrderRepository& repo, EventPublisher& bus)
      : repo_(repo), bus_(bus) {}

  void execute(OrderId id) {
    Order order = repo_.by_id(id).value();     // 1. load the whole aggregate
    order.confirm();                           // 2. run the domain rule (in Order)
    repo_.save(order);                         // 3. persist the new state...
    for (const auto& e : order.pull_events())  // 4. ...then announce the facts
      bus_.publish(e);
  }

 private:
  OrderRepository& repo_;
  EventPublisher&  bus_;
};

// ============================================================================
// INFRASTRUCTURE LAYER  — the outermost circle. It DEPENDS ON the domain (it
// implements the domain's ports). In a real system this is the only place that
// includes <pqxx>, <sqlite3>, a Kafka client, etc. Swapping it out never touches
// the two layers above.
// ============================================================================

class InMemoryOrderRepository : public OrderRepository {
 public:
  std::optional<Order> by_id(OrderId id) const override {
    auto it = store_.find(id.v);
    if (it == store_.end()) return std::nullopt;
    return it->second;
  }
  void save(const Order& o) override { store_.insert_or_assign(o.id().v, o); }
 private:
  std::unordered_map<std::uint64_t, Order> store_;
};

class RecordingPublisher : public EventPublisher {  // a test double for the bus
 public:
  void publish(const OrderEvent& e) override { published.push_back(e); }
  std::vector<OrderEvent> published;
};

int main() {
  // The COMPOSITION ROOT: the one spot that names concrete implementations and
  // plugs them into the ports. Everything else spoke only in abstractions.
  InMemoryOrderRepository repo;
  RecordingPublisher      bus;

  Order seed{OrderId{1}, CustomerId{7}};
  seed.add_line(300);
  repo.save(seed);

  // (1) The application service runs with ZERO database and ZERO real bus — we
  //     handed it fakes. That is the payoff of pointing dependencies inward:
  //     the domain and application layers are testable in complete isolation.
  ConfirmOrderService svc{repo, bus};
  svc.execute(OrderId{1});

  // (2) Repository = the illusion of an in-memory collection: what we saved, we
  //     load back — as a whole aggregate, through its root. No SQL in sight.
  Order reloaded = repo.by_id(OrderId{1}).value();
  assert(reloaded.confirmed());
  assert(reloaded.total_cents() == 300);

  // (3) The orchestration lived in the application layer; the fact reached the bus.
  assert(bus.published.size() == 1);
  assert(std::get<OrderConfirmed>(bus.published[0]).order == OrderId{1});

  // (4) Prove a BAD thing with a GREEN assertion — the dual-write hazard, and the
  //     reason lesson 6's "outbox" exists. save() and publish() are TWO separate
  //     writes (DB, then bus). No ordering is safe: here we announce FIRST, then
  //     the save crashes before commit. Downstream was told a fact that never
  //     became durable — and nothing surfaced the split.
  struct CrashingRepo : OrderRepository {
    std::optional<Order> cur;
    std::optional<Order> by_id(OrderId) const override { return cur; }
    void save(const Order&) override { throw std::runtime_error{"crash before commit"}; }
  };
  CrashingRepo       crepo;
  RecordingPublisher bus2;
  crepo.cur = Order{OrderId{2}, CustomerId{9}};   // "stored" as UN-confirmed

  Order loaded = crepo.by_id(OrderId{2}).value();
  loaded.confirm();
  for (const auto& e : loaded.pull_events()) bus2.publish(e);   // announce first...
  bool saved = true;
  try { crepo.save(loaded); } catch (const std::runtime_error&) { saved = false; }  // ...crash

  assert(!saved);
  assert(bus2.published.size() == 1);                       // downstream WAS notified
  assert(!crepo.by_id(OrderId{2}).value().confirmed());     // yet the store never confirmed
  // => a subscriber acted on OrderConfirmed for an order that, on reload, isn't
  //    even confirmed. The fix is an OUTBOX: write the event INTO the same
  //    transaction as the state change, then deliver it reliably afterwards.

  return 0;
}
