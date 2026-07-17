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

（第 4 课起填充：实体、值对象、聚合……）
