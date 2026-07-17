# 课程代码的验证脚本

课程里出现的每一段 C++ 都必须真的编译过。这里放的是可复跑的验证脚本——
课程改了代码，就回来重跑一遍。

## 跑法

```sh
# 第 4 课：值对象 / 实体 / const 成员陷阱，共 8 条断言
c++ -std=c++20 -Wall -Wextra -o /tmp/lesson4_check lesson4_check.cc && /tmp/lesson4_check \
  && echo "ALL CLAIMS VERIFIED"

# 第 4 课：const 成员会删除拷贝赋值——这个「应当编译失败」
c++ -std=c++20 -fsyntax-only const_member_trap.cc   # 预期：error, copy assignment implicitly deleted

# 第 5 课：聚合根守不变量 / 泄漏 getter 会破坏它，共 9 条断言
c++ -std=c++20 -Wall -Wextra -o /tmp/lesson5_check lesson5_check.cc && /tmp/lesson5_check \
  && echo "ALL CLAIMS VERIFIED"
```

## 文件

- `lesson4_check.cc` — 验证 [第 4 课](../lessons/0004-entity-vs-value-object.html) 与
  [C++ 战术模式速查表](../reference/cpp-tactical-patterns.html) 的全部断言：
  值对象按值相等、拷贝互不影响、实体按 id 相等（属性变了仍是它）、
  以及 4 条 `static_assert` 证明 const 成员会删除拷贝/移动赋值。
- `const_member_trap.cc` — **预期编译失败**。用来抓取课程里引用的那段真实 clang 报错。
- `lesson5_check.cc` — 验证 [第 5 课](../lessons/0005-aggregate.html) 的全部断言：
  聚合根在 `add_line` 里守住「总额 ≤ 信用额度」、超额的行被拒绝**且聚合毫发无损**（无半应用状态）、
  边界值（正好等于额度）放行、跨聚合按 `CustomerId` 引用而不需要 `Customer` 的定义。
  最关键的一条是**用绿色断言证明一件坏事**：非 const 的 `leaked_lines()` 确实能绕过根把总额搞爆，
  `assert(limit < o.total())` 通过——不变量沦为空话，而编译器与运行时全程沉默。

## 环境

已验证：Apple clang 17.0.0 / arm64 / `-std=c++20`，零告警通过、断言全绿。

## 教训

第 4 课的代码首次编译就抓到两个错（`assert` 宏吃不下花括号初始化列表、漏 `#include <type_traits>`）。
给 C++ 工程师发不能编译的 C++ 会很难看——**新增代码示例一律先过编译器再进课程**。
