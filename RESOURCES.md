# DDD Resources

## Knowledge

- [Domain-Driven Design Reference — Eric Evans](reference/DDD_Reference_2015-03.pdf) ★ 本工作区的权威基准
  Evans 本人整理的全部 DDD 模式的权威简明定义（59 页，CC BY 4.0），含蓝皮书未收录的 3 个模式。**副本已存入工作区** `reference/DDD_Reference_2015-03.pdf`。Use for: 任何术语/模式的权威定义，词汇表的对齐基准——引用定义前先查它。
  在线：[PDF 原址](https://www.domainlanguage.com/wp-content/uploads/2016/05/DDD_Reference_2015-03.pdf) · [落地页](https://www.domainlanguage.com/ddd/reference/)
- [Book: _Domain-Driven Design: Tackling Complexity in the Heart of Software_ — Eric Evans（蓝皮书，2003）](https://www.amazon.com/Domain-Driven-Design-Tackling-Complexity-Software/dp/0321125215)
  DDD 的开山之作，思想最完整但阅读门槛高。Use for: 统一语言、模型驱动设计的原始论证；进阶后精读。
- [Book: _Domain-Driven Design Distilled_ — Vaughn Vernon](https://www.oreilly.com/library/view/domain-driven-design-distilled/9780134434964/)
  最薄的一本（约 170 页），战略设计讲得最清楚。Use for: 零基础的第一本书；限界上下文、上下文映射、事件风暴入门。
- [Book: _Learning Domain-Driven Design_ — Vlad Khononov（O'Reilly, 2021）](https://www.oreilly.com/library/view/learning-domain-driven-design/9781098100124/)
  现代、易读，把战略/战术与架构决策（含何时不用 DDD）连接得最好。Use for: 系统性通读的主教材；子域类型判断。
- [Book: _Implementing Domain-Driven Design_ — Vaughn Vernon（红皮书）](https://www.amazon.com/Implementing-Domain-Driven-Design-Vaughn-Vernon/dp/0321834577)
  战术模式实现细节最全（示例为 Java，需翻译成 C++）。Use for: 聚合设计四原则、仓储、领域事件的实现参考。
- [Martin Fowler bliki: Ubiquitous Language](https://martinfowler.com/bliki/UbiquitousLanguage.html) / [Bounded Context](https://martinfowler.com/bliki/BoundedContext.html) / [DDD Aggregate](https://martinfowler.com/bliki/DDD_Aggregate.html)
  单个概念的短小权威解释，适合课程内引用。Use for: 每课的延伸阅读。
- [GitHub: awesome-ddd（heynickc）](https://github.com/heynickc/awesome-ddd)
  社区维护的 DDD/CQRS/ES 资源索引。Use for: 按需检索特定主题的文章与示例项目。

## Wisdom (Communities)

- [Virtual DDD](https://virtualddd.com/)
  最活跃的国际 DDD 社区，免费线上 meetup + Discord 异步讨论，Kenny Baas-Schwegler 等实践者主持。Use for: 建模问题求教、看真实案例讨论。
- [DDD-CQRS-ES Discord](https://github.com/ddd-cqrs-es/community)
  聚焦 DDD/CQRS/ES 的老牌聊天社区（由 Slack 迁移至 Discord）。Use for: 战术实现层面的具体问题。
- [dddcommunity.org](https://www.dddcommunity.org/)
  Evans 参与的官方社区站，资料与会议信息。Use for: 一手资料与大会演讲索引。

## Gaps

- C++ 语境下的 DDD 实践资料稀缺（主流示例是 Java/C#）；课程需自行把模式翻译成现代 C++ 惯用法，翻译结果沉淀到 reference/。
- 中文高质量一手资源待考察（张逸《解构领域驱动设计》、ThoughtWorks 系列文章），下次会话核实后再收录。
