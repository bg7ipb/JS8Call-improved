## seq=4

**1. 时间戳**:2026-05-21T<HH:MM:SS>+08:00  *(本会话操作时间;User 提交前调整为精确时间)*

**2. 序号**:4

**3. 当前阶段名**:phase-1.5 — phase-1 挂起项与未考虑细节澄清

**4. 状态摘要**:

- phase-1.5 三议题全部收口
- **议题 1**(缓存 TTL 档位):拍板完成,3 档状态机 + 滚动 24h 升级窗口 + 全档续期 TTL,详见 §6.A
- **议题 2**(应急词 Tier 锁定):关闭,确认 PARK-011 挂 phase-2,详见 §6.B
- **议题 3**(上游 PR 节奏):主动挂起,phase-2 实施推进时或之后再议,详见 §6.C
- `PHASE1_DESIGN_DRAFT.md` 已出新版反映议题 1 终稿、议题 2 关闭注释、议题 3 状态从"议题待澄清"改为"主动挂起"
- `HANDOFF.md §2.5` 三议题全部消化,phase-2 启动前 HANDOFF 可清理

**5. 本段动作**(实际执行清单):

1. SESSION-OPEN manifest 合规,phase-1.5 任务类型 = design,推进原始目的字段非空
2. Claude 复述对 phase-1 已定稿的理解(核心机制 / 三红线 / 关键不变量 / 已放弃方案 / V1-V9 验证状态),User 确认无歪
3. 议题 1 推进:
   - Claude frame 决策(用陈旧记忆赌远端能力没变 / 档数 vs 操作负担)
   - 摆出上 session 两套候选(5 档自选 / 4 档自动升级)+ 6 个 sub-question
   - 三轮 sub-detail 拍板(状态机框架 / 升级条件 / TTL 续期语义 / 缓存粒度 / 手动控制)
   - Claude push back 协议层不存在 software version 字段,PARK-019 挂起
   - Claude push back 用户答 (c)(d) 之间的逻辑矛盾,User 选 (c) 优先 → (d) 改为"30d 从最后一次活动起算"
   - Claude 提出 edge case (1) 集中爆发型 peer 窗口冻结问题,User 选方案 A(滚动 24h 窗口)
   - 议题 1 终稿出 phase1.5_topic1_cache_ttl.md 暂存件
4. 议题 2 推进:
   - User 主动询问"下个议题是不是有必要做"(精确的纪律检查)
   - Claude 分析议题 2 是否有协议层 dependency,结论:无,纯 codec 内部
   - User 选 (α) 关闭并提供关键外部约束依据:"全球应急通信必须英文"
5. 议题 3 推进:
   - User 决定"上游 PR 先挂起不急"
6. 收敛归档:
   - 出 `PHASE1_DESIGN_DRAFT.md` 新版
   - 出本 snapshot seq=4(本文件)

**6. 决策与依据**:

### 6.A 议题 1:缓存 TTL 档位拍板

**最终设计**:状态机自动升级 + 全档续期 TTL。**3 档**,无永久 pin,无 per-peer 手动控制。

| 状态 | 入档条件 | TTL | 升级触发 | TTL 续期触发 | 过期行为 |
|---|---|---|---|---|---|
| 0(默认) | 所有未入缓存的 peer | — | 收到 peer HB,bits3 ∈ {1,2,3} → 入状态 1 | — | N/A(不在缓存) |
| 1(首见外语 HB) | 进入触发 ← | 1 h | 完成"双向 CN QSO"(我发一帧 CN + 收到 ACK,且收到对方至少一帧 CN)→ 升状态 2 | 同档内收到同语言 bits3 的 HB | TTL 到 → 完全清除,退状态 0 |
| 2(完成首次外语 QSO) | 进入触发 ← | 12 h | **滚动 24h 窗口**:任意时刻往前看 24h,累计 ≥ 3 次 I18N 帧成功交换,且总跨度 ≥ 30 min → 升状态 3 | 同档内任一 I18N 帧成功交换 | TTL 到 → 完全清除,退状态 0 |
| 3(持续稳定) | 进入触发 ← | 30 d | (顶档,无更高状态) | 同档内任一 I18N 帧成功交换(等价"30d 从最后一次活动起算") | TTL 到 → 完全清除,退状态 0 |

**附加规则**:
- bits3 = 0(EN-only)的 HB **不入缓存** —— 默认即 EN
- 缓存粒度 = callsign-only(software version 字段需求 → PARK-019)
- UI 只暴露"清除全部语言缓存"总开关

**决策依据**:

| 决策点 | 依据 |
|---|---|
| 框架 = 状态机自动升级,3 档(无永久 pin) | User 拍板,简化工作量;状态 3 持续续期机制已弥补"无永久"的语义缺口(活跃熟人在状态 3 持续续期 30d,实际"准永久") |
| TTL 续期 = 同档证据再现即续期(全档统一) | User 拍板;语义一致,实现简单(一条规则) |
| 状态 2 升级用滚动 24h 窗口而非固定窗口 | 消除"集中爆发型 peer 窗口冻结永卡状态 2"的 edge case(Claude push back,User 同意);实现复杂度增量可控(每 peer 一个 ≤24 条时间戳列表) |
| 状态 1→2 要求"双向 CN QSO" | User 拍板;避免单方向解码成功误判对方完整能力 |
| 状态 2→3 条件"3 次 + 跨 30 min" | User 拍板;3 次防瞬时冒泡,30 min 跨度防同一连发被刷计数 |
| EN-only 不入缓存 | User 拍板;默认即 EN,无需显式标记;节省存储 |
| 缓存粒度 = callsign-only | 协议层 HB / I18N 帧字段预算均已占满,无可用 bit 广播 software version;30d 自然 TTL 已起到周期性强制重训作用;version 字段 → PARK-019 |
| 无 per-peer 手动控制(方案 α) | User 拍板;状态机自动管理是主线,人工介入在 phase-2 实测反馈后再考虑 |

### 6.B 议题 2:应急词 Tier 锁定关闭

**结论**:议题 2 关闭,**确认 PARK-011(应急词处理机制)正确挂在 phase-2**。

**关键外部约束**(User 提供):**全球应急通信必须英文**。

**协议层无 dependency**:
- I18N 帧 header bit[3..18] 已按 §4.2 分配满,无空间放"紧急标记位"
- 帧类型 FrameType=001 (FrameCompound) 已锁定,不新增 emergency 帧类型
- HAM 通信物理链路无优先级抢占语义,V3.0.1 也无此功能

phase-2 codec 设计时,应急词若需 tier 0 锁定属 codec 内部决策(不涉及帧格式),无协议层 dependency。但鉴于"全球应急通信必须英文"的法规约束,中文 codec **不需要在 tier 0 锁定中文应急词** —— 按字频自然落点即可。

### 6.C 议题 3:上游 PR 节奏主动挂起

**结论**:议题 3 **主动挂起,phase-2 实施推进时或之后再议**。

**依据**:
- PR 时机本质依赖 phase-2 实施进度与早期测试反馈
- phase-1.5 阶段无法实质决策(信息不足,决策面太大)
- 主动挂起 ≠ 待澄清:从"议题待澄清"升级为"明确推迟到 phase-2",PHASE1_DESIGN_DRAFT §10 状态相应修订

**7. 未解决问题**(明确未决,与"下一步"区分):

- phase-2 codec 详细设计待启动(tier 大小、前缀码、内容分配、应急词处理、版本管理)
- 英文能力询问帧格式未定(候选:复用 `INFO?` cmd 扩展,或新增专用 cmd code)→ PARK-017
- 中文 HAM 真实通联语料字频统计未开始 → PARK-018
- 缓存状态机的"清除全部语言缓存"UI 入口位置 phase-2 实施时定
- 状态机活动时间戳列表的持久化策略(进程内存 / SQLite / 文件)phase-2 实施时定
- 上游 PR 节奏 phase-2 推进时再议(本会话主动挂起)

**8. 风险与挂起项**:

- 新增 **PARK-019**:HB / I18N 协议增加 software version 字段(phase-2 codec 设计时审字段预算)
- 既有 PARK-010~018 不变(详见 `HANDOFF.md §3` PARKING_LOT 建议追加项)
- HANDOFF.md §2.5 三议题全部消化,phase-2 启动前可清理 §2.5 整节

**9. 下一步候选**:

- **本会话外**:User 决定本会话内是否 commit phase-1.5 整套(snapshot seq=4 + 新版 DRAFT + PARKING_LOT 更新 + HANDOFF 清理)
- **phase-2 启动条件**:phase-1 草案最终批准(已完成)+ phase-1.5 议题澄清完成(本 seq=4)+ User 显式宣告进入 phase-2
- **phase-2 开局首动作建议**:中文 HAM 真实通联语料字频统计(PARK-018)— codec tier 设计的前置数据基础

**10. 验证回执**:

本 seq 无 EXEC 命令(纯设计讨论会话)。所有决策依据来自:
- 上下文中 `PHASE1_DESIGN_DRAFT.md` v1 / `STATE_SNAPSHOT seq=1/2/3` / `HANDOFF.md` / `CHEATSHEET.md v1.1`
- User 在本会话内的拍板回复
- Claude 基于 V3.0.1 协议结构(§4.2 / §4.3 / 附录 B 已 cite 的源码 anchor)做出的协议层 dependency 推理

无新增源码 anchor。所有验证 anchor 已在 PHASE1_DESIGN_DRAFT 附录 B 列出。

**11. 作废标记**:

不适用。本 seq 不覆盖任何既有 seq 决策。

**关联修订**:本 seq 触发对 `PHASE1_DESIGN_DRAFT.md` 的更新(新版已出),修订点:
- §5.1 从"候选方案,议题待澄清"改为本 seq §6.A 终稿
- §7 phase-2 设计清单"应急词处理机制"加注释,反映本 seq §6.B 关闭依据
- §10 标题与警告横幅从"议题待澄清"改为"phase-1.5 主动挂起"
- 文档头部"修订历史"追加 phase-1.5 收尾记录条目
