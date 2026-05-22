# HANDOFF — phase-1.5 → 下次会话(phase-1.6 中继转发 I18N)

> **生成时间**:2026-05-21,phase-1.5 收尾
> **修订**:相对 v1 全面更新 —— phase-1.5 全部议题 closed;DRAFT 升级 v3;snapshot 至 seq=5;新增 phase-1.6 议题"中继转发 I18N"frame + 用户两步方案初步评估
> **目的**:为下次会话 phase-1.6 提供衔接锚点,避免设计跑偏 / 重复讨论已决项 / 重新引入污染
> **本会话产出**:phase-1.5 全 4 议题收口(详见 STATE_SNAPSHOT seq=4/5)+ DRAFT v3 + 本 HANDOFF v2
> **下次会话使命**:讨论 phase-1.6 议题 —— CN 经 V3.0.1 中继(两步方案)

---

## 1. 当前状态(指针)

| 文档 | 状态 | 路径 |
|---|---|---|
| 设计草案 | **v3** 定稿,新增 §4.4 ARQ 协议层 | `docs/discipline/PHASE1_DESIGN_DRAFT.md`(议题 4 后约 350 行) |
| 状态快照 | seq=5 已 ready,待 User append | `docs/discipline/STATE_SNAPSHOT.md` + 本次 outputs/STATE_SNAPSHOT_seq4.md + STATE_SNAPSHOT_seq5.md |
| 纪律 cheatsheet | v1.1(本会话未动) | `docs/discipline/CHEATSHEET.md` |
| 本交接文档 | **v2**,当前文件 | `docs/discipline/HANDOFF.md` |

DRAFT §9 验证矩阵:V1/V2/V3/V4/V6/V7/V8/V9 + bit[53] trick **全 PASS**;V10(议题 4 控制帧载体)推理 PASS,phase-2 启动前 EXEC sanity check;V5 可省。

---

## 2. 已决定不做 / 不要重提(防止下次会话 Claude 重新提议)

| 已放弃方案 | 放弃理由 |
|---|---|
| 切换 whitening polynomial / 改 LDPC | 兼容路径方案更优,改动小、上游可接受(seq=3 C-1 作废 seq=2 G 方案) |
| hbs + cqs 联用扩展到 6 bit 语言 ID | cqs 是活的(`Varicode.cpp:287-294`),禁挪用 |
| 在 phase-1 / phase-1.5 阶段做 codebook 详细设计 | 用户显式挂起到 phase-2 |
| 中文应急词 Tier 锁定(议题 2) | **全球应急通信必须英文**,中文 codec 无需协议层提供应急词处理;PARK-011 phase-2 codec 内部按字频自然落点 |
| 跨语言互通(CN↔JA) | 用户显式挂起到 phase-2 之后 |
| 引用 2.5.2 时代的方案文档 | 用户显式要求,避免污染。下次会话也不要请求或重读 |

> **注意**:原 v1 §2 含 "CN 经 V3.0.1 中继"作为已放弃项,**phase-1.6 议题 4 将重新讨论,故从本表移除**。

---

## 3. PARKING_LOT 累积清单(待 User 决定如何合并)

phase-1 续接会话产生:
| ID | 内容 | 触发条件 / phase |
|---|---|---|
| PARK-010 | ILC codec 详细设计:tier 大小、前缀码、内容分配 | phase-2 启动 |
| PARK-011 | 应急词处理机制 | **议题 2 已评审 closed**:phase-2 codec 内部按字频自然落点(无协议层 dependency) |
| PARK-012 | Codebook 版本演进 / append-only 预留区机制 | phase-2 |
| PARK-013 | Codebook 文件格式 / 分发 / 热更新 | phase-2 |
| PARK-014 | CN 经 V3.0.1 中继 | **phase-1.6 议题 5,不再"挂起到 phase-2 之后"** |
| PARK-015 | 跨语言互通(CN↔JA 等) | phase-2+ 视需求 |
| PARK-016 | 默认缓存档位(候选 30 分钟 / 24 小时) | **议题 1 已 closed**:状态机已定 3 档 + 默认状态 0;本项实质失效 |
| PARK-017 | 英文能力询问帧格式(复用 `INFO?` 还是新 cmd code) | phase-2 实施 |
| PARK-018 | 中文 HAM 真实通联语料字频统计 | phase-2 早期(codec 设计前置) |

phase-1.5 新增:
| ID | 内容 | 触发条件 / phase |
|---|---|---|
| PARK-019 | HB / I18N 协议增加 software version 字段 | phase-2 codec 设计时审字段预算 |
| PARK-020 | ARQ 超时/重试 5 参数具体数值(T_single / N_retry / T_total / T_piggyback / T_detect)| phase-2 实测拍 |
| PARK-021 | Fallback 控制帧(bit[8..10]=4)bit[19..71] payload 内部字段布局 | phase-2 实施前细化 |

seq=2 时代的 **PARK-005~009**(关于 whitening polynomial 切换的硬假设)已被 phase-1 兼容路径方案取代,**全部作废**。

---

## 4. 下次会话 phase-1.6 议题 —— 中继转发 I18N

### 4.1 用户给定的两步设想

**Step 1**:JS8CALL_CN 1.0.0 发出的中文消息只能由 JS8CALL_CN 1.0.0 转发(英文信息保持原版,不影响)。
**Step 2**:下一个版本 JS8CALL_CN 1.0.1 发出的中文消息,JS8CALL 3.0.1 也能转发(尽可能实现),先看可行性,不做具体方案细节。

### 4.2 Claude 初步评估(供下次会话开局 frame,**非定论**,User 校对后再深入)

> **评估框架声明**:本评估使用 CHEATSHEET 中的纪律,具体是:**公理 2(有据才说)+ 公理 3(双向审计)+ 公理 6(范围不外溢)+ §5.5(CONFIRM 四要素:要做什么/为何必须做/做错后果/更可逆替代)**。User 提到的"四级审计原则"我未在 CHEATSHEET/WORKFLOW 已载入部分中找到精确定义,下次会话开局请 User 澄清,如有差异本评估按 User 框架重做。

#### 4.2.1 Step 1 评估:CN 1.0.0 间互相转发

**可行性**:可行。但工作量是议题 4 量级或以上。

**关键协议层 dependency**(下次会话需要逐项 spec):

1. **中继帧载体形式**
   - ❌ 复用 V3.0.1 RELAY cmd —— RELAY 在 V3.0.1 directed_cmds 集合中,破红线 #2
   - 候选 (i):新 I18N 控制帧子类型(bit[8..10] 保留区 5..7 取一个,比如 5 = "中继帧",跟议题 4 的 bit[8..10]=4 控制帧并列)
   - 候选 (ii):复用议题 4 控制帧 mechanism,只是 sub-type 不同(扩展 PARK-021 控制帧 payload 布局,加 sub-type 字段)

2. **原始发送方 callsign 携带** —— **重大 dependency**
   - I18N 帧 bit[3..52] 已被全用(位置标记/序号/语言 ID/CRC/ILC),**没有空间放原始 callsign**
   - 选项 (i):中继帧用多帧分段格式,某一帧专门承载原始 callsign
   - 选项 (ii):重新审视 §4.2 字段预算,挪 ILC 容量给原始 callsign(损失中文容量)
   - 选项 (iii):用控制帧 mechanism(bit[8..10]=5),控制帧 payload bit[19..71]=53 bit 中分配给原始 callsign

3. **中继路径发现**
   - 静态配置 vs 动态发现?
   - HB bits3 是否需扩展"中继能力"标记?(但 bits3 已被语言 ID 占)

4. **Hop 控制(防无限转发)**
   - TTL 字段位置?协议层需留 bit
   - 或静态最大 hop=1(只允一跳,极简)

5. **跟 ARQ(议题 4)交互**
   - 中继帧本身丢了如何处理?
   - 接收端 ARQ 请求发给原始发送方还是中继方?
   - 中继方收到 ARQ 请求是否要中转?

6. **认证 / 防伪造**
   - phase-1.6 极简(不做,仅依赖 CRC-8 防错码)vs 加签名(需协议层字段)

**CONFIRM 四要素应用**:

| 要素 | Step 1 |
|---|---|
| 要做什么 | 给 CN 1.0.0 间增加中继转发能力,V3.0.1 不受影响 |
| 为何必须做 | 弱信号 / 远距离场景,直连不可达时通过中间节点中继 |
| 做错后果 | 红线 #2 破(若误用 V3.0.1 RELAY)/ 中继环路 / 带宽放大攻击(无 TTL 时)/ 跟 ARQ 协议冲突 |
| 更可逆替代 | (a)不做中继,接受弱信号无解;(b)仅 Step 1 静态配置 + hop=1,功能有限但实施风险低 |

#### 4.2.2 Step 2 评估:V3.0.1 中继 CN 1.0.1 消息

**可行性**:**极度悲观,我倾向不可行**。

**核心 obstacle**(基于已验证 V3.0.1 anchor):

1. V3.0.1 RELAY 走 directed message 路径(autoreply_cmds 含 cmd 30 = RELAY-likes);directed 路径需要解析合法 callsign;我们的 I18N 帧 callsign 是 base37 "乱码"(设计意图,见红线 #2 防御)—— V3.0.1 不会把它识别为 directed message,不会触发 RELAY 行为
2. V3.0.1 收到 FrameCompound 后只放进 `m_messageBuffer[].compound`,90s 静默 remove —— **没有"转发"逻辑路径**
3. 修改 V3.0.1 = 违反原始约束(同频道共存,V3.0.1 用户不需升级)
4. 让 I18N 帧"看起来像 directed" = 破红线 #2

**唯一可能的路径(完全猜测,需 EXEC 探查)**:V3.0.1 是否存在"非 directed 转发"机制?比如某种"广播 callsign"语义(base37 全 0 或特殊值)是否会触发自动复述?这个我**没有 anchor 支撑**,只是没排除的可能。

**CONFIRM 四要素应用**:

| 要素 | Step 2 |
|---|---|
| 要做什么 | 让 V3.0.1 不修改就能转发我们 CN 1.0.1 的消息 |
| 为何必须做 | 用户基数 —— V3.0.1 节点远多于 CN 节点,如能借力则中继网络密度大 |
| 做错后果 | 若强行尝试可能触碰红线 #2;若可行性调研不严谨导致幻想性 spec,phase-2 实施时撞墙 |
| 更可逆替代 | 把 Step 2 限定为"可行性调研"任务(EXEC 探查 V3.0.1 是否有未知转发机制),调研不通过则永久放弃 |

#### 4.2.3 我的初步建议(供下次会话校对)

**phase-1.6 主推 Step 1**(议题 4 量级,需若干轮 sub-question 拍板),**Step 2 限定为 phase-1.6 末尾的小型可行性调研**,不开 spec。如果调研无果,Step 2 进 PARK 永久放弃;如果调研有果,phase-1.7 再开 spec。

**phase-1.6 SESSION-OPEN 时,User 先校对**:
- 我的"评估框架声明"是否符合 User "四级审计原则"
- Step 1 的 6 项 dependency 是否完整(我可能漏)
- Step 2 限定为"可行性调研"是否接受,还是要平等深入 spec

---

## 5. 下次会话 SESSION-OPEN manifest 模板

(User 复制到下次会话第一条消息,按需调整)

```
[SESSION-OPEN]
任务类型: design
阶段: phase-1.6(中继转发 I18N 协议层)
本 phase 推进原始目的的方式: 为 JS8CALL-CN 协议层增加中继转发能力 Step 1(CN 1.0.0 间互相转发);
                              Step 2(V3.0.1 兼容中继)限定为本 phase 末尾小型可行性调研。
                              讨论清单见 HANDOFF §4.1 用户给定两步设想 + §4.2 Claude 初步评估校对。
应载入: CHEATSHEET v1.1 + STATE_SNAPSHOT seq=1..5(若 budget 紧可只载 seq=3..5)+ PHASE1_DESIGN_DRAFT.md v3 + HANDOFF.md v2
允许 retrieve: DISCIPLINE 按 § retrieve / WORKFLOW 按需(§5.3 commit 前缀,§4.2 凭据红线)
禁载入: DISCIPLINE / WORKFLOW 全文 / 任何 2.5.2 时代外部参考文档 / phase-1 / phase-1.5 已 closed 议题的重复讨论
[SESSION-OPEN END]
```

---

## 6. 下次会话开局推荐动作

1. User 发 SESSION-OPEN manifest(上模板,按需改字段)
2. **User 校对 HANDOFF §4.2 评估框架**:确认是 §5.5 四要素,还是另有"四级审计原则"具体定义
3. Claude 复述对 phase-1 / phase-1.5 已定稿的理解(核心机制 + ARQ 协议层 + 缓存状态机),User 确认无歪
4. 进 phase-1.6 议题 5 —— 先 frame Step 1 完整 dependency 清单(基于 §4.2.1 的 6 项,User 校对补漏),再逐项 sub-question 拍板
5. Step 1 完成后,phase-1.6 末尾做 Step 2 可行性调研(EXEC 探查 V3.0.1 是否有未知转发机制)
6. 每完成一步 append snapshot 子段(seq=6 = phase-1.6 整体一条,按 phase-1.5 模式)
7. phase-1.6 收尾时如修订了 PHASE1_DESIGN_DRAFT,出 v4

---

## 7. 注意事项 / 红线给下次会话 Claude

- **不要重提 §2 的已放弃方案**
- **§4.2 评估是初步且基于现有 anchor 推理**,不构成 phase-1.6 决议;User 校对后再认真深入
- **红线 #1(APRS-IS)/ #2(auto-response)在中继议题里硬,不可破** —— 中继帧自己也必须走 FrameCompound 兼容路径,不可借用 V3.0.1 RELAY cmd
- **挂起项讨论严格按 CHEATSHEET §4.6 范围管控**
- **`git add` / `commit` 节点**:本会话末段未 commit。下次会话开局前,User 可决定先 commit phase-1.5 整套(snapshot seq=4 + seq=5 + DRAFT v3 + PARKING_LOT 改动 + HANDOFF v2)再开始
- **每条 EXEC 命令仍走 paste-bridge**(Claude 写命令 → User zsh 跑 → 粘回完整 stdout)
- **EXEC 命令默认加 `git --no-pager`**
- **本会话暂存件 `phase1.5_topic1_cache_ttl.md` 内容已并入 seq=4,可归档/删除**

---

## 8. 关键不变量(下次会话不要轻易改的硬约束)

| 不变量 | 来源 |
|---|---|
| 红线 #1(APRS-IS)、#2(auto-response)硬,不可破 | 项目根目标 |
| 红线 #3(band activity 乱码)软,接受 | phase-1 续接会话 |
| FrameType=001(FrameCompound)是 I18N 帧固定值 | V1 + V9 + bit[53] trick 三层验证支撑 |
| `JS8_JSC/` 不动,新建 `JS8_I18N/` | 用户原始约束 |
| HB bits3 语言 ID 仅 isAlt=0 有效 | V6 实证 |
| `bit[53] = 1` anti-APRS lock | bit[53] trick PASS |
| **bit[8..10] = 0..3 数据帧(EN/CN/JA/KO),4 = ARQ 控制帧,5..7 保留** | 议题 4 终稿;phase-1.6 若加中继控制帧,从 5..7 中拿 |
| **bit[19] = ARQ_FLAG,bit[20..27] = NACK bitmap(条件)** | 议题 4 ILC payload 内部协议契约 |
| **首帧(bit[3..4]=01)的 bit[5..7] = 总帧数 - 1** | 议题 4 总帧数广播策略 |
| **缓存状态机 3 档(无永久 pin),全档续期 TTL,bits3=0 不入缓存,callsign-only 粒度** | 议题 1 终稿 |
| **应急词处理 = phase-2 codec 内部决策(无协议层支持)** | 议题 2 关闭依据 |
