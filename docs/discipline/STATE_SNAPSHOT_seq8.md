## seq=8

**1. 时间戳**:2026-05-22T<HH:MM:SS>+08:00  *(本会话操作时间;User 提交前调整为精确时间)*

**2. 序号**:8

**3. 当前阶段名**:phase-1.6 续 — Route ② CN 中继转发协议层 spec。本 seq 双核心:**(核心 1)基线纠正(master→v3.0.1)+ re-home**;**(核心 2)dependency #2 位预算锁定 —— 中继控制帧 spec**。design 任务(进 spec)。

**4. 状态摘要**:

- **基线纠正(本 seq 核心 1)**:实测发现 HEAD 在 **master**(非 v3.0.1),seq=6 文档 commit(`c9cfbd6e`)落在 master,且 `v3.0.1` tag **非 HEAD 祖先** —— 与 PARK-022「base = v3.0.1」直接冲突。**User 拍板:base 必须用 v3.0.1。** diff 查证:master↔v3.0.1 仅差 7 文件,其中**唯一沾我方 anchor 的源码 = `processCommandActivity.cpp`,且差异仅在 `" CQ"` 分支(logCallActivity/logHeardGraph),与中继转发无关**。**结论:V1–V10 + bit[53] + 50/28-bit wire 事实全部对 v3.0.1 成立,零重验**;唯一代价 = §10 中继 anchor 行号重 pin(见 6.B)。已 re-home 工作线到 v3.0.1-based 分支 `cn-dev`。
- **dependency #2 锁定(本 seq 核心 2)**:中继模型 = **M2 多跳**(User 否决 M1 单跳);载体 = **中继控制帧 `bit[8..10]=5`**(保留区取一,与 ARQ 控制帧 =4 并列);**52-bit** payload(自纠:非 53,bit[53] 是 anti-APRS 锁)拆 **原始呼号 28 + 目标哈希 16 + msg-ID 5 + TTL 3**;跳模型 = **TTL + 逐节点去重 + 反向路径学习**,位预算随跳数恒定。
- **docs 碎片化(本 seq 暴露)**:git `docs/discipline/` 仅含 seq6;CHEATSHEET/DISCIPLINE/DRAFT/HANDOFF/seq4/5/7 仅在 Claude Project,未入 git。完整 backfill 待"更大范围(B)"决定。

**5. 本段动作**(实际执行;均 paste-bridge):

1. **EXEC 只读核对**(公理 2;User 提出 —— 涉数据判断 User 易错):`git describe --tags` → `lib/2.6-70-gc9cfbd6e`(非 v3.0.1);`packCompoundFrame` 注释 `[3][50][11],[5][3]=72` + `intToBits(packed_callsign,50)`(**50-bit callsign 字段坐实**);`packDirectedMessage` 注释 `[3][28][28][5],[2][6]=72` + `intToBits(packed_from/to,28)`(**28-bit/呼号坐实**);`unpackCompoundMessage` 两 grid/cmd 分支均 gate `extra<nmaxgrid`(**bit[53] 锁复证**)。
2. **基线诊断**:`status -sb` = on master[ahead 1];HEAD commit = 我方 seq6 doc(留占位 `<按你 §5.3 前缀>`);`tag -l` = lib/2.5 / lib/2.6 / **v3.0.1 存在**;`merge-base --is-ancestor v3.0.1 HEAD` = **NOT**;CMake VERSION = 0.0.0(版本靠 build 时 git describe 注入)。→ **User 拍板 base=v3.0.1**。
3. **分叉量化**:`diff --shortstat v3.0.1 HEAD` = 7 files / 172+ / 5-;anchor 文件 numstat:`Varicode.cpp`/`.h`、`DecodedText.cpp`、`processRxActivity.cpp`、`processDecodeEvent.cpp` 全 **IDENTICAL**;`processCommandActivity.cpp` = **16+/1-**;实质 diff = 仅 `" CQ"` 分支(L737)。`mainwindow.cpp`(V4)/`initializeDummyData.cpp`(V7 旁证)**不在 7 文件内** → 亦对 v3.0.1 有效。merge-base = `05b439fd`(audio notifications dialog)。
4. **§10 中继 anchor 在 v3.0.1 重 pin**(grep v3.0.1 树):见 6.B。
5. **锁 dependency #2 中继控制帧 spec**(待办 2,含 53→52 自纠):见 6.C / 6.D。
6. **re-home**:`ls-tree master -- docs/discipline/` = 仅 seq6;建 `cn-dev` off v3.0.1,携 seq6 + 本 seq8 落盘(本 snapshot 即随此 commit)。

**6. 决策与依据**:

### 6.A 基线纠正(取代 PARK-022 的执行假设)

| 层 | 内容 |
|---|---|
| 决策(User 权威) | base **必须** v3.0.1;放弃 master 线 |
| 依据 | 原始目标"与 released v3.0.1 用户同频共存、不需升级"(DRAFT §1)→ 红线/共存分析对象应是用户实际在跑的 v3.0.1,非未发布 master;`describe`/`merge-base` 实证 HEAD ≠ v3.0.1 |
| PARK-022 定性 | **决策对(base=v3.0.1)、执行错(活儿落 master)**;本 seq 纠正执行,PARK-022 改为"已决 + 执行已纠正" |
| 影响范围 | 红线/wire anchor **零重验**(5/6 anchor 文件 IDENTICAL;唯一分叉文件差异与中继无关);仅 §10 中继 anchor 行号重 pin |

### 6.B §10 中继/转发 anchor 重 pin(v3.0.1 准确行号,**取代 seq=6 §10 的 master-read 数字**)

`JS8_Mainwindow/processCommandActivity.cpp` @ v3.0.1:

| anchor | v3.0.1 行 |
|---|---|
| relayPath autoreply swap | 326 / 328 |
| relay handler `parseRelayPathCallsigns` | 441(relayPath join 457;ACK reply 459) |
| MSG TO: 存储路径 | parseRelayPath 599 / relayPath 600 / cd.relayPath 610 / `addCommandToStorage("STORE")` **621** / ACK 625 |
| APRS dedup(`m_aprsRelayDedupCache`) | 560–590(一处)/ 773–794(二处);`cd.relayPath="APRS"` 579 / 788 |
| QUERY 系 replyPath(relayPath split) | 858 / 978 / 1045 |

> 唯一与 master 差异 = L737 `" CQ"` 分支(logCallActivity/logHeardGraph),非中继。L737 之前 anchor 两树同行号;之后 master 因多 15 行而偏高,**以本表 v3.0.1 为准**。Route ② 承重的"decode-reencode 反证"在 v3.0.1 完好(parseRelayPath → relayPath join → 沿路径 ACK 俱在;re-encode 走 IDENTICAL 的 `Varicode.cpp` buildMessageFrames/packDataMessage)。

### 6.C 中继控制帧 spec(dependency #2 锁定;含 53→52 自纠)

**中继模型 = M2 多跳**(User 否决 M1 单跳;依据:单跳无意义,弱信号/远距需多中间节点中继)。

**载体 = 中继控制帧,`bit[8..10] = 5`**(保留区 5..7 取一;与议题 4 ARQ 控制帧 =4 并列;V10 已覆盖 `bit[8..10]∈{5,6,7}` 的 V3.0.1 兼容性 —— 整 `bit[3..52]` 当 callsign 解,不识别"语言 ID"字段)。

**自纠(公理 2,Claude 产出≠真理)**:控制帧 payload **= 52 bit,非 53**。`bit[53]` = anti-APRS 锁(必须 =1)落在 `bit[19..71]` 区内,吃 1 bit。可用 = `bit[19..52]`(34)+ `bit[54..71]`(18)= **52**。(HANDOFF §4.2.1 与本 session 前述"53"皆此口径错,本 seq 改 52。)

payload 52-bit 拆分:

| 字段 | 位宽 | 说明 |
|---|---|---|
| 原始发送方呼号 | 28 | 字面(对标 packDirectedMessage 标准呼号 28-bit packCallsign),可显示 |
| 目标呼号哈希 | 16 | 收件方"是否给我"判定;假阳无害(多显一条) |
| msg-ID | 5 | `(原始呼号, msg-ID)` 作去重键;0..31 |
| TTL 剩余跳数 | 3 | 0..7;**默认初值 3**;字段留 7 备短波传播不确定性 |
| **合计** | **52** | 整帧 CRC-8(`bit[11..18]`)已覆盖此区,无需内嵌 CRC |

控制帧结构:`bit[0..2]=001` / `bit[3..4]=00`(单帧)/ `bit[5..7]=0` / `bit[8..10]=5` / `bit[11..18]=CRC-8` / `bit[19..52]+bit[54..71]=52-bit payload` / `bit[53]=1` 锁。

### 6.D 跳模型(M2 防爆 + 防环)

| 项 | 决策 | 依据 |
|---|---|---|
| 路径**不上线缆** | 只带 TTL,不带累积呼号路径 | 携带路径 = 逐跳 28-bit 爆炸;TTL 计数器位预算随跳数恒定 |
| 防环/防无限转发 | TTL 每跳 −1 到 0 即丢 + 每节点 `(原始呼号, msg-ID)` 去重缓存 | 对标 v3.0.1 `m_aprsRelayDedupCache` 模式 |
| ACK/ARQ 回程 | 反向路径学习(节点记"从邻居 Y 听到 msg-ID X"),路径活在节点状态非线缆 | 与议题 4 ARQ 交织,细节待下关 |
| max-hop 真实约束 | = 信道占空比(每跳重广播占 airtime),**非位预算** | TTL 几乎免费;默认 3 按信道礼仪,字段留 7 |

**7. 未解决问题**(明确未决,与"下一步"区分):

- **下一关 spec(本 seq 后第一关)= 内容帧↔控制帧绑定**:内容帧(`bit[8..10]=1`)不带 msg-ID,N 个内容帧如何与中继控制帧关联?
- **ACK/ARQ 回程细节**(承 HANDOFF §4.2.1 第 5 项):反向路径学习的节点状态结构 / ARQ 请求发原始方还是中继方 / 中继方是否中转 ARQ。
- **目标哈希算法 + 碰撞容忍**:16-bit 哈希函数选型(phase-2);假阳行为已定为"多显无害"。
- **msg-ID 5-bit 是否够**:去重窗口内 32 ID/origin;若紧,目标哈希让 1 bit 给 msg-ID(→6)。
- **docs 碎片化**:re-home 后 git 仅 seq6 + seq8;其余文档(CHEATSHEET/DISCIPLINE/DRAFT/HANDOFF/seq4/5/7)仅在 Project,未入 git → 待"更大范围(B)"决定是否全量 backfill。
- **PARK-024 四级审计原则**框架定义(仍挂;本 seq 续用 §5.5 四要素工作框架)。

**8. 风险与挂起项**:

- **【已否决,不重提】Route ③ 信封/隧道**(承 seq=7 §8):永久技术否决,未来 session 不得再提。
- **PARK-022**:改为「**已决 + 执行已纠正**」—— base=v3.0.1,工作线 = `cn-dev`(off v3.0.1)。剩 setup(`git remote add upstream` / sync 节奏 / master 上孤立的 `c9cfbd6e` 是否清理)→ phase-2 启动时做。
- **PARK-023(Route ① 长线 PR)/ PARK-024(四级审计,挂)/ PARK-010~021** 全部不变。
- **资产**:`linchpin_roundtrip.cpp`(seq=7,Route ③ 否决后仅历史,可弃)。

**9. 下一步候选**:

- **下关**:内容帧↔控制帧绑定 + ACK/ARQ 回程(承 §7)。
- **commit 节点**:本 seq=8 随 re-home commit 落 `cn-dev`。
- **更大范围(B)**:User 待定是否把全套 discipline 文档 backfill 进 git(本 seq 暴露碎片化)。

**10. 验证回执**(本 seq 建立/纠正的 v3.0.1 anchor;base=v3.0.1):

```
基线:HEAD 原在 master(describe=lib/2.6-70-gc9cfbd6e);v3.0.1 tag 存在但非 HEAD 祖先;merge-base=05b439fd
分叉:diff v3.0.1↔HEAD = 7 files;anchor 文件仅 processCommandActivity.cpp(16+/1-,差异=L737 " CQ" 分支)
wire(v3.0.1,IDENTICAL 文件,直接有效):
  Varicode.cpp packCompoundFrame    注释 [3][50][11],[5][3]=72;packAlphaNumeric50;intToBits(callsign,50)
  Varicode.cpp packDirectedMessage  注释 [3][28][28][5],[2][6]=72;packCallsign;intToBits(from/to,28)
  Varicode.cpp unpackCompoundMessage grid/cmd 两分支均 gate extra<nmaxgrid(bit[53] 锁)
中继 anchor 重 pin(v3.0.1):见 6.B
re-home:cn-dev off v3.0.1;docs/discipline/ = seq6(自 master 取)+ seq8
```

**11. 作废标记**:

- **作废**:seq=6 §10 的 `processCommandActivity.cpp:414/528/745/972/1042` 等 **master-read 行号** —— 以本 seq 6.B 的 v3.0.1 行号为准。
- **作废**:seq=6 §6.A / PARK-022 的隐含执行状态"开发线已在 v3.0.1" —— 实为 master,本 seq 纠正并 re-home 到 `cn-dev`。
- **作废**:本 session 前述 + HANDOFF §4.2.1 的"控制帧 payload **53 bit**" —— 改 **52 bit**(bit[53] 是 anti-APRS 锁,落在区内吃 1 bit)。
- **作废**:dependency #2 候选中"**M1 单跳**" —— User 否决,定 M2 多跳。

**关联修订**:本 seq 锁定首段实际 CN relay spec(中继控制帧载体 + 位预算 + 跳模型),但 relay spec 未完整(内容帧绑定 / ACK 回程未定)。按项目"DRAFT 待 spec 完整后出 v4"惯例,**暂不出 v4**;待 relay spec 收口(下关之后)再出 v4,届时新增"中继转发协议层"章节 + 修订 §8.2(CN 中继从挂起 → Route ② 设计中)+ §4.2 登记 `bit[8..10]=5`。HANDOFF 待 phase-1.6 收尾出 v3。另:**关键不变量表(HANDOFF §8)应追加 `bit[8..10]=5 = 中继控制帧`**(下次 HANDOFF 更新时并入)。
