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

**上下文映射（Context Map）**：
项目里全部限界上下文、以及它们之间**连线（关系）**的全局视图。首先是一种**认识**（先如实描述现状——有哪些上下文、哪两个之间有通信、每条连线什么关系），而非一上来就设计或画 UML。
_判据（评审武器）_：指着任意一条连线问「这是什么关系？」——答不上来，或答「就直接调 / 共用一张表」，多半是一条**没被治理的连线**，两个模型正在互相污染（Evans: they tend to bleed into each other）。
_Avoid_：把它当成部署拓扑图或 UML 类图。
> 出处（原文）：*Identify each model in play on the project and define its bounded context. […] Describe the points of contact between the models, outlining explicit translation for any communication […]. Map the existing terrain. Take up transformations later.* — DDD Reference, "Context Map"

**上游 / 下游（Upstream / Downstream）**：
两个上下文之间**影响力的不对称**：上游变了，下游得跟着改；下游怎么折腾，上游不受影响。一句话——「谁能不管谁的死活」。
_歧义澄清_：与开发先后顺序、网络位置、代码量**都无关**。判清谁上谁下，才谈得上下游该用哪种应对模式（遵奉者？防腐层？客户/供应商？）。

**防腐层（Anticorruption Layer, ACL）**：
下游为**自保**而建的一层隔离/翻译层，把上游系统的功能**以「自己的领域模型」的措辞**提供出来，挡住上游烂模型的概念/词汇渗进自己的领域。用在上游**不**配合、你只能防御的时候。
_判据_：意图是「主动隔离 + 翻译」，不是性能优化，也不是求上游改模型。
_C++_：就是[[仓储]]那套**端口/适配器**的又一次化身——端口在领域层（用你自己的措辞），适配器是**全程序唯一认识外部模型的地方**。把 `SqlOrderRepository` 挡的「数据库」换成「另一个上下文的模型」即得。见 `code/lesson8_check.cc`、[C++ 写法](reference/context-map-patterns.html)。
_对照（同处境的另一条路）_：**遵奉者（Conformist）**——下游放弃自己的理想模型、盲从上游，换取集成的极大简化。认命 vs 自保，是上下游关系里最易混的一对。全部九种关系模式见 [关系模式速查表](reference/context-map-patterns.html)。
> 出处（原文）：*As a downstream client, create an isolating layer to provide your system with functionality of the upstream system in terms of your own domain model. […] Internally, the layer translates in one or both directions as necessary between the two models.* — DDD Reference, "Anticorruption Layer"

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

**领域服务（Domain Service）**：
领域中一段**重要的过程或变换**，但**不自然属于**任何一个实体或值对象时，作为模型里独立声明的服务操作；名字进入统一语言，通常**无状态**。
_判据_：硬塞进某个实体会扭曲其定义，或逼你造出无意义的人造对象 → 领域服务。**默认仍是聚合根方法**——领域服务是例外出口，不是默认口袋。
_对照_：**应用服务**只做用例编排（取 / 调 / 存 / 发），**不含业务规则**；名字带 Service 不决定它是哪一种——看里面有没有业务 if。
_Avoid_：① `Wallet::transfer_to(other)`（过程挂错对象）；② 把币种/余额/满减规则写进应用服务（贫血领域）；③ 什么逻辑都往领域服务里扔。
_C++_：无成员的函数对象或自由函数即可；更好的形状是应用服务取聚合、交给领域服务、再 save——领域服务尽量不自己拿仓储（对持久化无知，可零基础设施测）。见 `code/lesson9_check.cc`、[C++ 写法](reference/cpp-tactical-patterns.html)。
> 出处（原文）：*Sometimes, it just isn’t a thing. […] When a significant process or transformation in the domain is not a natural responsibility of an entity or value object, add an operation to the model as a standalone interface declared as a service. […] Give the service a name, which also becomes part of the ubiquitous language.* — DDD Reference, "Services"
