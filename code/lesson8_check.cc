// Lesson 8 — Anticorruption Layer (ACL) as a port/adapter, in modern C++.
//
// Two bounded contexts. Ours is Shipping (downstream). Upstream is a legacy CRM
// whose model is weak: stringly-typed, sentinel values, its own vocabulary. We
// REFUSE to let that model leak across our boundary. The ACL is the isolating
// layer that talks to the CRM and translates INTO our own domain model.
//
// Evans, DDD Reference p.34: "As a downstream client, create an isolating layer
// to provide your system with functionality of the upstream system in terms of
// YOUR OWN domain model. ... Internally, the layer translates ... between the
// two models."
//
// This file compiles clean under:  c++ -std=c++20 -Wall -Wextra
// It is the SAME port/adapter shape as lesson 7's OrderRepository — only now the
// adapter translates ANOTHER CONTEXT'S MODEL, not database rows.

#include <cassert>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

// ============================================================================
// UPSTREAM — the legacy CRM's model. Someone else's ugly world. We do not own
// it, cannot change it, and must never let it dictate our model.
// ============================================================================
struct CrmContactRecord {
  std::string full_nm;     // combined name, may arrive padded with spaces
  std::string ctry_cd;     // ISO-2 country, but "" is a SENTINEL for "unknown"
  int         status_flag; // 0=inactive, 1=active, 9=undocumented legacy junk
};

class LegacyCrm {  // stands in for the foreign system's existing interface
 public:
  std::optional<CrmContactRecord> lookup(int contact_id) const {
    if (contact_id == 1) return CrmContactRecord{"  Ada Lovelace ", "GB", 1};
    if (contact_id == 2) return CrmContactRecord{"Grace Hopper", "", 9}; // junk
    return std::nullopt;
  }
};

// ============================================================================
// DOWNSTREAM DOMAIN — our Shipping context, in OUR ubiquitous language.
// None of this knows the CRM exists. A Consignee is always valid once built.
// ============================================================================
struct CountryCode {                       // a value object: valid-by-construction
  std::string iso2;
  explicit CountryCode(std::string code) : iso2(std::move(code)) {
    if (iso2.size() != 2) throw std::invalid_argument("country must be ISO-2");
  }
};

class Consignee {                          // "收件人" — Shipping's own concept
 public:
  Consignee(std::string name, std::optional<CountryCode> country)
      : name_(std::move(name)), country_(std::move(country)) {
    if (name_.empty())                     // an invariant the CRM never enforced
      throw std::invalid_argument("consignee needs a name");
  }
  const std::string&                name()    const { return name_; }
  const std::optional<CountryCode>& country() const { return country_; }

 private:
  std::string                name_;
  std::optional<CountryCode> country_;      // "unknown" is a first-class absence,
};                                          // NOT a magic "" the way the CRM does it

// The PORT — Shipping declares what it needs, phrased entirely in its own terms.
// (Same role as lesson 7's OrderRepository: an abstract interface owned by us.)
class ConsigneeDirectory {
 public:
  virtual ~ConsigneeDirectory() = default;
  virtual std::optional<Consignee> find(int consignee_id) const = 0;
};

// ============================================================================
// THE ANTICORRUPTION LAYER — an adapter. This is the ONLY place in the whole
// program that names CrmContactRecord. The corruption stops here.
// ============================================================================
namespace {
std::string trim(std::string s) {
  const auto b = s.find_first_not_of(' ');
  const auto e = s.find_last_not_of(' ');
  return b == std::string::npos ? std::string{} : s.substr(b, e - b + 1);
}
}  // namespace

class CrmConsigneeDirectory : public ConsigneeDirectory {   // the ACL adapter
 public:
  explicit CrmConsigneeDirectory(const LegacyCrm& crm) : crm_(crm) {}

  std::optional<Consignee> find(int id) const override {
    auto rec = crm_.lookup(id);            // talk to the foreign system as-is
    if (!rec) return std::nullopt;
    return translate(*rec);                // <-- anticorruption happens here
  }

 private:
  // The translation. Foreign vocabulary + sentinels -> our clean model.
  static Consignee translate(const CrmContactRecord& r) {
    std::optional<CountryCode> country;
    if (!r.ctry_cd.empty())                // sentinel "" -> honest std::nullopt
      country = CountryCode{r.ctry_cd};
    // r.status_flag (incl. the undocumented 9) is simply NOT part of our model,
    // so it can never leak in and confuse Shipping logic.
    return Consignee{trim(r.full_nm), country};
  }

  const LegacyCrm& crm_;
};

int main() {
  LegacyCrm crm;
  CrmConsigneeDirectory acl{crm};          // wire the ACL to the foreign system

  // --- 1. The ACL translates the foreign model into OUR clean model ---
  auto ada = acl.find(1);
  assert(ada.has_value());
  assert(ada->name() == "Ada Lovelace");        // padding trimmed at the boundary
  assert(ada->country().has_value());
  assert(ada->country()->iso2 == "GB");

  // --- 2. The CRM's sentinel "" becomes an honest optional absence ---
  auto grace = acl.find(2);
  assert(grace.has_value());
  assert(grace->name() == "Grace Hopper");
  assert(!grace->country().has_value());        // "" did NOT leak in as a country

  // --- 3. Missing upstream -> honest nullopt, no exception, no junk ---
  assert(!acl.find(99).has_value());

  // --- 4. GREEN ASSERTION PROVING A BAD THING: the Conformist leak ---
  // If Shipping had "slavishly adhered to the upstream model" (Conformist,
  // p.33) and passed CrmContactRecord around directly, the CRM's sentinel and
  // its undocumented status_flag=9 would flow straight into Shipping's logic.
  // We prove the leak exists in the RAW record, and that the ACL erased it.
  CrmContactRecord raw = crm.lookup(2).value();
  assert(raw.ctry_cd == "");     // conformist path: "" masquerades as a country
  assert(raw.status_flag == 9);  // conformist path: undocumented junk rides along
  // The ACL path carries neither: a clean optional, and no status_flag field at
  // all. Same input, corruption stopped at the boundary.
  assert(!grace->country().has_value());

  return 0;
}
