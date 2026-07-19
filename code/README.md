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

# 第 6 课：领域事件（值对象 + variant）/ 根记录事件 / 忘记 publish 会无声丢副作用
c++ -std=c++20 -Wall -Wextra -o /tmp/lesson6_check lesson6_check.cc && /tmp/lesson6_check \
  && echo "ALL CLAIMS VERIFIED"

# 第 7 课：三层 + 端口/适配器 / 应用服务在零 DB 下可测 / 双写裂缝（发件箱动机）
c++ -std=c++20 -Wall -Wextra -o /tmp/lesson7_check lesson7_check.cc && /tmp/lesson7_check \
  && echo "ALL CLAIMS VERIFIED"

# 第 8 课：防腐层（ACL）作为端口/适配器 / 翻译上游烂模型 / 遵奉者路线的污染泄漏
c++ -std=c++20 -Wall -Wextra -o /tmp/lesson8_check lesson8_check.cc && /tmp/lesson8_check \
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
- `lesson6_check.cc` — 验证 [第 6 课](../lessons/0006-domain-event.html) 的全部断言：
  领域事件是值对象（`= default` 相等、可自由拷贝，与实体相反）、聚合根在 `confirm` 里
  **和状态改变一起**记下 `OrderConfirmed`（携 id 与快照，不携引用）、事件集用 `std::variant` + `std::visit` 派发、
  以及最终一致性的时序——`confirm` 时库存纹丝不动，`publish` 之后才 `-1`。
  同样有一条**用绿色断言证明的坏事**：应用层若忘了 `publish`（丢弃 `pull_events()` 的结果），
  库存扣减无声蒸发，`assert(inv.units == 99)` 通过——引出发件箱（outbox）的必要性。
- `lesson7_check.cc` — 验证 [第 7 课](../lessons/0007-repository-layered-architecture.html) 的全部断言：
  三层（领域/应用/基础设施）堆在一个 TU 里，依赖只朝里指；`OrderRepository`/`EventPublisher` 是领域层的纯虚端口，
  `InMemoryOrderRepository`/`RecordingPublisher` 是基础设施层适配器；`ConfirmOrderService` 只依赖抽象，
  于是**零数据库、零真实总线**即可测通（依赖倒置的可测性红利）、存取原样往返（仓储的集合错觉）。
  又一条**用绿色断言证明的坏事**：`save` 与 `publish` 是两次独立写入，先发布后 `save` 崩溃 →
  下游收到了事件、但重载订单竟是「未确认」，`assert(!confirmed())` 通过——双写裂缝，引出发件箱（outbox）。
- `lesson8_check.cc` — 验证 [第 8 课](../lessons/0008-context-map-anticorruption-layer.html) 的全部断言：
  **防腐层（ACL）就是第 7 课那个端口/适配器结构的又一次化身**，只是适配器翻译的对象从「数据库」换成
  「另一个上下文（老旧 CRM）的烂模型」。`ConsigneeDirectory` 是领域层端口（用物流自己的措辞），
  `CrmConsigneeDirectory` 是防腐层适配器，且是**全程序里唯一认识 `CrmContactRecord` 的地方**。
  断言：翻译把带空格的名字 trim 干净、把哨兵 `""` 译成 honest 的 `std::nullopt`、上游查不到时返回 `nullopt`。
  又一条**用绿色断言证明的坏事**：若走遵奉者（Conformist）的路直接传 `CrmContactRecord`，
  哨兵 `""` 与没文档的 `status_flag=9` 就在原始记录里明晃晃存在（`assert(raw.ctry_cd == "")` 通过）——
  而防腐层的产物里它俩一个变成诚实缺席、一个根本不在模型里。同样输入，污染被挡在边界。

已验证：Apple clang 17.0.0 / arm64 / `-std=c++20`，零告警通过、断言全绿。

## 教训

第 4 课的代码首次编译就抓到两个错（`assert` 宏吃不下花括号初始化列表、漏 `#include <type_traits>`）。
给 C++ 工程师发不能编译的 C++ 会很难看——**新增代码示例一律先过编译器再进课程**。
