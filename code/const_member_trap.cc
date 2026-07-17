#include <string>
#include <vector>
#include <algorithm>

class BadMoney {
 public:
  BadMoney(long long c, std::string cur) : cents_(c), currency_(std::move(cur)) {}
 private:
  const long long   cents_;
  const std::string currency_;
};

int main() {
  BadMoney a{100, "CNY"};
  BadMoney b{200, "CNY"};
  a = b;                       // <-- looks harmless
  return 0;
}
