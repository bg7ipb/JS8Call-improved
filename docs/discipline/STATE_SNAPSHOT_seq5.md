## seq=5

**1. 时间戳**:2026-05-21T<HH:MM:SS>+08:00  *(本会话操作时间;User 提交前调整为精确时间)*

**2. 序号**:5

**3. 当前阶段名**:phase-1.5 — 议题 4(I18N 协议层 ARQ + 最大帧数限定)拍板

**4. 状态摘要**:

- 议题 4 全部 8 个 sub-question 收口
- I18N 消息层引入选择性重传 ARQ,协议层完整 spec
- **核心决策**:8 帧上限 / 首帧序号字段复用做总帧数广播 / (ε+β') 复合重传请求载体(piggyback 进 ILC payload header 优先 + 控制帧 fallback)/ Selective-repeat 重传策略 / 三参数超时机制 / ARQ 救回 QSO 算"成功交换"
- 新增 V10 验证项(控制帧 bit[8..10]∈{4-7} V3.0.1 兼容性,推理 PASS,phase-2 启动前 EXEC sanity check)
- `PHASE1_DESIGN_DRAFT.md` 已出 v3(新增 §4.4 ARQ 协议层 + §4.2/§5.1/§7/§9 多处修订)
- 新增 PARK-020(ARQ 超时/重试具体数值)+ PARK-021(fallback 控制帧 payload 布局)

**5. 本段动作**(实际执行清单):

1. User 提议议题 4(I18N 丢帧重发机制 + 最大帧数限定),本质属 phase-1.5 §4.2 范围(SESSION-OPEN manifest "用户补充清单"位置)
2. Claude frame 议题边界(协议层 ARQ / 帧编码语义 / 状态机交互 IN;UI 实现细节 / LDPC 改动 OUT),User 校对无歪
3. Claude 关键协议发现:总帧数广播 dependency —— bit[3..4] 位置标记 4 状态不含"总帧数",末帧丢失则接收侧无法判定丢失帧数
4. Sub-question wire pattern 组拍板:
   - Q1 最大帧数 = 8(物理上限,User 拍板)
   - Q2 总帧数广播 = 首帧序号字段复用做"总帧数-1"(User 拍板)
   - Q3 重传请求帧载体 = 首轮 Claude 建议 (β) 独立控制帧,User push back "前提不能影响通信的速度";第二轮 Claude 重新搜索设计空间,提出 (β')/(ε)/(ε+β') 三候选 + 通俗解释;User 最终拍板 (ε+β') 复合方案
   - Q4 丢帧检测 = 推断到末帧位置后等 N 秒(User 拍板)
5. Sub-question 机制细节组拍板(User "全部按建议走"):
   - Q5 = Selective-repeat
   - Q6 = phase-1 锁三参数机制,具体数值 PARK-020
   - Q7 = ARQ 救回 QSO 算"成功交换"
   - Q8 = 重发请求自身超时重发(非递归),复用 N_retry
6. Q3 follow-up:fallback 控制帧 payload 布局选 (n) PARK-021 phase-2 实施前细化
7. V10 验证项基于已有 anchor 推理 PASS(`Varicode.cpp:1494-1525` unpackCompoundMessage 把 bit[3..52] 整体当 callsign 解码,不识别"语言 ID"字段)
8. 出 `PHASE1_DESIGN_DRAFT.md` v3 + 本 snapshot seq=5

**6. 决策与依据**:

### 6.A 协议字段层

| 决策 | 依据 |
|---|---|
| 最大帧数 = 8(协议物理上限) | User 拍板 Q1=(i);bit[5..7] 序号 3 bit 即上限,议题 4 不再缩 |
| 首帧 bit[5..7] 复用做"总帧数-1" | User 拍板 Q2=(b);首帧序号必定=0,该 3 bit 在首帧位置无信息冗余;接收侧获知总帧数提早 N-1 帧周期(N=总帧数);末帧丢失场景下首帧仍能告知总数;首帧+末帧都丢则整体放弃(承认设计边界) |
| 单帧(bit[3..4]=00)bit[5..7] 固定为 0 | 单帧 = 1 帧,"总帧数-1"=0,语义一致 |
| 中/末帧(bit[3..4]=10/11)bit[5..7] = 消息序号 | 保持普通含义;末帧序号+1 作为总帧数冗余备份 |
| bit[8..10] 保留区 4..7 取 4 做"控制帧"标识 | User 拍板 Q3=(ε+β')复合;协议层与 codec 解耦;5..7 仍保留供未来语言扩展或其他控制语义 |

### 6.B ARQ 机制层

| 决策 | 依据 |
|---|---|
| 重传请求载体 = (ε+β') 复合:Piggyback 优先 + 控制帧 fallback | User 拍板 Q3;双向对话场景(HAM 中文通联 99% 主流)零额外开销;单方监听场景退化到 +1 TX 周期;期望值最优 |
| ILC payload bit[19] = ARQ_FLAG | 议题 4 协议契约;phase-2 codec 必须遵守;纯数据帧 -1 bit overhead(64→63 汉字/消息,可接受);含 NACK 帧 -9 bit overhead |
| bit[20..27] = NACK bitmap(若 ARQ_FLAG=1) | 8 bit 对应 8 帧最大,MSB=SEQ 0;Selective-repeat 一次列出所有 missing seq |
| Fallback 触发条件 = T_piggyback 秒内无 piggyback 机会 | (ε+β') 退化路径;T_piggyback 具体数值 PARK-020 |
| 丢帧检测 = 推断到末帧位置后等 T_detect 秒 | User 拍板 Q4=(i);两种推断路径:收末帧反推 / 收首帧从总帧数算 |
| 重传策略 = Selective-repeat | Claude 建议,User confirm;JS8 慢速半双工场景下 Stop-and-wait 浪费链路 90%+ 空闲,Go-back-N 重发成本高;Selective-repeat 是事实唯一选择 |
| 超时机制 = phase-1 锁 5 参数,具体数值 PARK-020 | Claude 建议,User confirm;5 参数:T_single / N_retry / T_total / T_piggyback / T_detect;phase-1 锁机制语义,phase-2 实测基于 JS8 各 mode (Normal/Fast/Turbo) 时序拍数值 |
| 重传请求帧自身丢失复用 N_retry 机制 | Claude 建议,User confirm;避免递归 ARQ(无限层级),避免被动等死(单次发就完丢);判定"请求帧已丢" = T_single 内无响应 |
| ARQ 救回的 QSO 算"I18N 帧成功交换" | Claude 建议,User confirm;符合 §5.1 状态机本意(衡量"能力 + 通信稳定");ARQ 救回 = 链路质量一般但能力存在,HAM 通信常态;避免状态机过度严苛 |

### 6.C 跟其他章节交互

| 影响章节 | 修订内容 |
|---|---|
| §4.2 单帧布局 | bit[5..7] 描述补"上下文相关"(首帧时复用);bit[8..10] 增加值 4=控制帧;bit[19] ARQ_FLAG 占位;容量描述从 64 改为 ~62-63 汉字 |
| §4.3 HB bits3 | 不变(HB 语义独立于 ARQ) |
| §4.4 ARQ 协议层 | **新增整节**,完整 spec |
| §5.1 缓存状态机 | 附加规则末追加"ARQ 救回算成功交换" |
| §7 ILC codec | 追加"ILC 必须识别 bit[19] ARQ_FLAG";phase-2 设计清单增 ARQ_FLAG 跳过逻辑 |
| §9 验证矩阵 | 新增 V10 控制帧载体 V3.0.1 兼容性(推理 PASS) |
| §10 上游 PR | PR 内容追加"ARQ 协议层"作为 phase-1 spec 一部分 |
| 附录 B | unpackCompoundMessage 注释追加"V10 推理依据" |

**7. 未解决问题**(明确未决,与"下一步"区分):

- T_single / N_retry / T_total / T_piggyback / T_detect 具体数值 → PARK-020,phase-2 实测拍
- Fallback 控制帧 bit[19..71] payload 内部字段布局(子类型 / 目标消息标识 / NACK bitmap / CRC 等) → PARK-021
- V10 phase-2 启动前 EXEC sanity check(精确验证 V3.0.1 对 bit[8..10]∈{4,5,6,7} 不做语义特殊处理,仅做 callsign 字段解码)
- ARQ 状态机与发送方 UI 的交互细节(进度显示 / 失败提示)phase-2 实施时定
- ARQ 失败的整体超时后 UI 提示用户的措辞 / 行为 phase-2 实施时定

**8. 风险与挂起项**:

- **PARK-020(新增)**:ARQ 超时/重试参数具体数值。涉及 T_single / N_retry / T_total / T_piggyback / T_detect 五项,phase-2 实测拍。需考虑 JS8 各 mode(Normal/Fast/Turbo)时序差异
- **PARK-021(新增)**:Fallback 控制帧(bit[8..10]=4 时)bit[19..71] 控制 payload 内部字段布局
- 既有 PARK-010~019 不变

**9. 下一步候选**:

- **本会话内**:若 budget 允许,继续推 §4.1 余下挂起项(默认缓存档位 PARK-016 / 英文能力询问帧格式 PARK-017 / 模式 A 双方互证 CN 能力具体协议步骤 / V5 形式划掉)
- **本会话外**:User 决定是否 commit phase-1.5 整套(snapshot seq=4 + seq=5 + DRAFT v3 + PARKING_LOT 更新 + HANDOFF 清理)
- **phase-2 启动条件**:phase-1 草案最终批准 + phase-1.5 议题澄清完成 + V10 EXEC sanity check 完成 + User 显式宣告进入 phase-2
- **phase-2 开局首动作建议**(同 seq=4):中文 HAM 真实通联语料字频统计(PARK-018)— codec tier 设计的前置数据基础

**10. 验证回执**:

本 seq 无 EXEC 命令(纯设计讨论)。所有决策依据来自:

- 上下文中的 `PHASE1_DESIGN_DRAFT.md` v2 / `STATE_SNAPSHOT seq=1/2/3/4` / `HANDOFF.md` / `CHEATSHEET.md v1.1`
- User 在本会话内的拍板回复
- Claude 基于 V3.0.1 协议结构(§4.2 / §4.3 / 附录 B 已 cite 的源码 anchor)的协议层 dependency 推理

**V10 推理依据**(无 EXEC,基于现有 anchor):

- `Varicode.cpp:1494-1525` `unpackCompoundMessage` 把 Compound 帧的 bit[3..52] 整体作为 callsign 字段解码(base37 编码)
- V3.0.1 不知道 bit[8..10] 是"语言 ID 字段",这是 I18N 协议层后赋予的语义
- 因此 bit[8..10] 取 0..7 任何值,V3.0.1 都是把它们当 callsign 字段的某 3 bit 解码 —— 都解出"乱码 callsign"形状
- 不存在 V3.0.1 对 bit[8..10] ∈ {4,5,6,7} 做"特殊处理"的语义路径
- 风险点:phase-2 启动前用 EXEC 实测确认(构造一个 bit[8..10]=4 的帧,观察 V3.0.1 解码行为 → 应跟 bit[8..10]=0..3 完全等价,band activity 显示乱码 callsign,不触发任何 spot/autoreply)

无新增源码 anchor。所有验证 anchor 已在 PHASE1_DESIGN_DRAFT v3 附录 B 列出。

**11. 作废标记**:

不适用。本 seq 不覆盖任何既有 seq 决策。

**关联修订**:本 seq 触发对 `PHASE1_DESIGN_DRAFT.md` 从 v2 到 v3 的更新(新版已出),主要修订点:

- §4.2 bit[5..7] / bit[8..10] / bit[19..52] 描述修订 + 容量公式修订
- 新增 §4.4 ARQ 协议层(整节)
- §5.1 附加规则追加 ARQ 救回 QSO 算成功交换
- §7 phase-2 设计清单关联追加
- §9 验证矩阵追加 V10
- §10 PR 内容追加 ARQ
- 附录 B `Varicode.cpp:1494-1525` 注释追加 V10 推理依据
- 文档头部"修订历史"追加 phase-1.5 议题 4 收尾条目
