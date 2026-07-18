# DDD Glossary

本工作区的规范用语。术语只有在用户通过测验或练习证明理解后才收录；所有课程、参考文档、学习记录一律沿用此处的叫法。定义对齐 Eric Evans 的 [DDD Reference](https://www.domainlanguage.com/wp-content/uploads/2016/05/DDD_Reference_2015-03.pdf)（本地副本：`reference/DDD_Reference_2015-03.pdf`）。

## 战略设计

**统一语言（Ubiquitous Language）**：
一门围绕领域模型构建、由团队全体成员在**一个限界上下文内**共用的语言，它把团队的一切活动与软件连接起来——因此必须出现在代码的命名里。
_Avoid_：通用语言、领域词汇表、术语表（后者只是词汇清单，不含「代码里也这么叫」的强制含义）
_歧义澄清_：本工作区中「统一」永远指**限界上下文内**统一，不是全公司统一。全公司统一是 DDD 明确否定的目标。
> 出处（原文）：*A language structured around the domain model and used by all team members within a bounded context to connect all the activities of the team with the software.* — DDD Reference, "Definitions"

**限界上下文（Bounded Context）**：
一条边界的描述——通常是一个子系统或某个特定团队的工作范围——在这条边界内，某个特定的模型有定义、且适用。属**解空间**：它是你的设计选择。
_Avoid_：模块、微服务、子系统、包（前三者是部署/组织概念，与上下文可能重合但不等价）
_判据_：边界必须有牙齿——体现在团队组织、代码库、数据库 schema 上。共享同一套表与同一批类的两个「上下文」，只是文档里的两个词。
> 出处（原文）：*A description of a boundary (typically a subsystem, or the work of a particular team) within which a particular model is defined and applicable.* — DDD Reference, "Definitions"

**子域（Subdomain）**：
业务本身的一块内聚领域。属**问题空间**：它客观存在，你去发现它，而不是设计它。
_歧义澄清_：子域 ≠ 限界上下文。子域是老天给的，限界上下文是你画的。理想情况下二者对齐，但**不必严格 1:1**；一个上下文横跨多个子域通常是该拆的信号。

**核心子域（Core Domain）**：
你靠它赢得竞争的那块子域——复杂**且**是差异化来源，因而买不到。设计精力、最强的人、全套 DDD 都优先投在这里。
_判据_：客户会因为这块做得好而选我们吗？
_歧义澄清_：核心 ≠ 技术难。规则复杂但客户看不见的（如供应商对账），是支撑子域。
> 出处（原文）：*Make the core small. Apply top talent to the core domain, and recruit accordingly.* — DDD Reference, "Core Domain"

**支撑子域（Supporting Subdomain）**：
你家特有、市面买不到，但不产生竞争优势的子域。正确姿态是「够用就行」——CRUD 或事务脚本，不建聚合。
_出处_：三分法中的这一档由 Vaughn Vernon 在 *DDD Distilled* 中补齐；Evans 原书只明确了核心与通用两极。

**通用子域（Generic Subdomain）**：
人人都一样、已有成熟方案与公认模型的子域（鉴权、发票、短信）。正确姿态是买，不是造。
_关键理由_：Evans 要求避免把核心开发者派去做这些任务——**因为他们从中学不到领域知识**。亏的不是人月，是本可积累的业务理解。
> 出处（原文）：*avoid assigning your core developers to the tasks (because they will gain little domain knowledge from them). Also consider off-the-shelf solutions or published models.* — DDD Reference, "Generic Subdomains"

## 战术设计

在**单个限界上下文内部**工作。C++ 落地写法见 [战术模式速查表](reference/cpp-tactical-patterns.html)。

**实体（Entity）**：
由**标识**（而非属性值）定义、贯穿生命周期而保持同一性的对象——同一张订单改了地址，还是那张订单。
_判据_：属性全相同的两个，是不是同一个东西？答「否」就是实体。
_Avoid_：把「带 id 的结构体」当判据（id 是实现手段，不是理由）；`= default` 的 `operator==`（会退化成值对象——头号事故）。
_C++_：`operator==` 只比 id；根实体通常 `= delete` 默认拷贝（默认拷贝会造出两个 id 相同的「同一个」）。
> 出处（原文）：*When an object is distinguished by its identity, rather than its attributes, make this primary to its definition in the model. Keep the class definition simple and focused on life cycle continuity and identity.* — DDD Reference, "Entities"

**值对象（Value Object）**：
由属性值定义、**无概念标识、不可变**的对象——金额、地址、时段。战术建模的**默认选项**。
_判据_：你只关心它的属性和逻辑（而非「是哪一个」）→ 值对象。
_歧义澄清（本工作区重点）_：**没有哪个概念「永远」是值对象或「永远」是实体——分类是限界上下文内的判断，不是概念固有的属性。** 配送上下文里的「时段」是值对象；排班上下文里的「周二上午这个班次」是实体。钞票在钱包里是值对象，在央行冠字号追踪里是实体。同理，「我要查它 / 拿它排序」是**存储与检索**的需求，不构成标识，不该反过来决定领域模型（值对象照样能查、能排，给它 `operator<=>` 即可）。
_C++_：`operator==(...) const = default`（逐成员比）；不可变靠**无 setter**，**不靠 `const` 成员**（const 成员会删掉拷贝赋值）。
> 出处（原文）：*When you care only about the attributes and logic of an element of the model, classify it as a value object. […] Treat the value object as immutable.* — DDD Reference, "Value Objects"

**聚合（Aggregate）**：
一簇实体和值对象构成的**一致性单元**，外部只能通过唯一的「根」实体访问和修改。一句话：**聚合边界 = 一致性边界 = 事务边界**。
_判据_：哪些东西必须**同时**正确？问得出这样一条不变量，才画这条线；问不出，就别画。
_核心原则_：一个事务只改一个聚合；跨聚合**只用 id 引用**；边界外走**最终一致性**（Vaughn Vernon, *Effective Aggregate Design*）。
_C++_：按值持有 = 圈内，按 id 持有 = 圈外——边界在类型里看得见。绝不开放非 `const` 的内部引用（调用方会绕过根破坏不变量，且悄无声息）。
> 出处（原文）：*Cluster the entities and value objects into aggregates and define boundaries around each. Choose one entity to be the root of each aggregate, and allow external objects to hold references to the root only […]. Define properties and invariants for the aggregate as a whole and give enforcement responsibility to the root […]. Use the same aggregate boundaries to govern transactions and distribution.* — DDD Reference, "Aggregates"

**领域事件（Domain Event）**：
领域中一件**已经发生**、领域专家关心的**事实**——`OrderConfirmed`。过去式、不可变，由聚合根产生，携带涉及实体的 **id 与快照**（不携引用）。跨聚合的协作靠它异步传递。
_判据_：领域专家会说「这件事很重要，我要知道它发生了」吗？
_Avoid_：**命令**（`ConfirmOrder` 是祈使、可被拒；`OrderConfirmed` 是既成事实——一字之差是分水岭）；系统/UI 事件（反映软件自身活动，不在领域里）；给每个 setter 都发事件。
_C++_：它就是个**值对象**（`= default` 相等、可自由拷贝）；一个聚合的事件集用 `std::variant`（封闭和类型）+ `std::visit` 派发。携 id 不携引用——放指针 = 开后门反向篡改聚合。
> 出处（原文）：*Something happened that domain experts care about.* / *Model information about activity in the domain as a series of discrete events. Represent each event as a domain object.* / *Domain events are ordinarily immutable, as they are a record of something in the past.* — DDD Reference, "Domain Events"（标 `*`：蓝皮书之后补入模式语言的三个模式之一）
