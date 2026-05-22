## seq=6

**1. 时间戳**:2026-05-22T<HH:MM:SS>+08:00  *(本会话操作时间;User 提交前调整为精确时间)*

**2. 序号**:6

**3. 当前阶段名**:phase-1.6 — 中继和转发 I18N:**原版机制盘点(只读) + 载体路线决策**。本 seq 为 research/analysis + 战略方向,**未写 CN spec**。

**4. 状态摘要**:

- **基线对齐(对应 PARK-022,本 seq 决议)**:开发基线 = **`v3.0.1` tag**(非 master)。`origin` = `github.com/bg7ipb/JS8Call-improved`(个人 fork);**upstream = `github.com/JS8Call-improved/JS8Call-improved`(org)**。3.0.2/3.0.3 = upstream 未来迭代,**incoming-only 跟随**。
- **重大事实校正**:`JS8Call-improved` **现在就是官方 JS8Call**(原作者 Jordan/KN4CRD 已归档原仓库、加入 improved 团队;Chris/AC9KH 从 2.3.1 接手;2.5.0 起正式叫回 JS8Call)。**v3.0.1 = 2026-05-17 发布的当前正式版**。不存在"vanilla vs fork"分裂;共存对象 = 跑官方 v3.0.1(及向后兼容的全部 JS8 台)的用户。
- **原版"中继和转发"功能已盘点完整**(upstream 术语,见 §6.B),经官方 User Guide 交叉印证。
- **载体路线决策(本 seq 核心,见 §6.E)**:**主推 Route ②**(phase-1 的 FrameCompound 载体,自给自足、零上游改动、先能跑);**Route ① 的"上游留 codec 位"作长线轻量 PR,短期不期望落地**。
- **术语约定(User 定)**:不特别指明时,**"中继和转发"统称**:relay `>` + MSG/MSG TO:/QUERY MSGS/QUERY MSG 收件箱 + 群组投递 + HB 自动投递。
- **四级审计原则框架**:User 本 seq 显式**挂起**(PARK-024);本 seq 分析用 §5.5 四要素 + 公理 2/3 作工作框架。

**5. 本段动作**(实际执行清单,均走 paste-bridge,全只读):

1. R1–R5:在 master 上定位命令词表/集合 → 发现 **master ≠ 3.0.1(已分叉)**;origin = bg7ipb fork,但本地已有 `v3.0.1` tag + `origin/release/3.0.1` branch。User 拍板**对齐 v3.0.1**(PARK-022)。
2. 读 `v3.0.1` 的 `directed_cmds` map + 各命令集合(命令层与 master 字节级一致,行号相同)→ 纠正 HANDOFF §4.2.2 的错断言(见 §6.C)。
3. R5–R6:盘点 v3.0.1 中继和转发功能全貌(§6.B),读 relay handler 实体 + 校验和入口。
4. 读官方一手资料:js8call.com(= JS8Call-improved 站点)、upstream README(项目史/版本史)、User Guide(功能语义 + **Future Work 路线图**)。发现 upstream 自己计划 SRARQ / store-forward routing / automatic relay routing / 公钥校验(与我方 phase-1.5 ARQ + 本 phase 重叠,见 §6.D / §8)。
5. User 把 phase-1.6 **战略重定义**(§4.6 (c)):由"自建 CN 中继 + 调研 V3.0.1 转发 CN"改为"**分析能否把官方中继和转发扩成中英双语并 PR 上游**"。
6. R6–R10:读编码/校验边界、directed 帧构造、Data 帧结构、FrameType 枚举、RX 分发链 → 完成 Route ① 的可行性/复杂度分析(§6.D/§6.E)。
7. **得出决定性反证**:relay 是"解码-重编码",**旧台无法中继 CN(必毁),两路皆然**;directed 路径会让旧台主动发乱码污染。据此 User 拍板 **Route ② 主推 + Route ① 长线**(§6.E)。
8. 出本快照 seq=6 落盘(防漂移)。**本 seq 无 commit 之外的写操作;DRAFT 暂不出 v4**(尚无 CN spec)。

**6. 决策与依据**:

### 6.A 基线与上游(PARK-022 决议)

| 决策 | 依据 |
|---|---|
| 开发基线 = `v3.0.1` tag,放弃 master 增量 | User 拍板;master 持续变动无法做稳定 base;v3.0.1 = 当前官方正式版(2026-05-17) |
| upstream = `JS8Call-improved/JS8Call-improved`(org),origin = bg7ipb fork | User 提供;`remote -v` + tag/branch 列表证实 |
| 跟随 3.0.x:incoming-only,窗口内不外溢 | 符合 CHEATSHEET 工作模式"上游优先";几个月后才考虑并入上游 |
| 红线验证对象 = v3.0.1(命令层已证与 master 同源) | `git grep v3.0.1` 命令表/集合行号与内容和 master 一致 |

### 6.B 原版"中继和转发"功能盘点(upstream 自己的术语;锚点见 §10)

| 功能(upstream 名) | cmd | 干什么 | 关键机制 |
|---|---|---|---|
| **Relay** `>` | 5 | 多跳转发 directed 消息 | `parseRelayPathCallsigns` → `relayPath=calls.join('>')`;沿路径回 ACK;**转发 payload 可嵌套命令再 dispatch**;受 `relay_off()` 开关;在 `buffered_cmds`,**不在** `autoreply_cmds` |
| **MSG TO:** | 10 | 在中间台持久化存消息给某呼号 | `addCommandToStorage("STORE")` → SQLite Inbox(`$.type='STORE'`,`$.params.TO`=收件呼号);回 ACK |
| **MSG** | 9 | 投递完整消息给本机用户 | 弹 `SelfDestructMessageBox` 展示 |
| **QUERY MSGS** | 12 | "你那有没有存给我的" | 查 Inbox 未读,回最老未投递的 ID |
| **QUERY MSG [ID]** | (借 11/12 体系) | 取回指定 ID 完整消息 | — |
| **QUERY CALL** | 13 | 探测能否直达某呼号 | 沿 relayPath |
| **Inbox**(实体) | — | SQLite 持久化库 | `fetchForCall`/`countUnreadForCallsign`/`getNextGroupMessageIdForCallsign`/`markGroupMsgDeliveredForCallsign`;type 恒 `"STORE"` |
| **群组消息** | 借 Inbox | @GROUP 暂存 + 逐成员投递并标记 | 按 `(group,callsign)` 记未读 |
| **HB 自动投递** | — | 听到目标 HB → 播报"我有 MSG <id> 给你" | `HEARTBEAT SNR ... MSG <id>` → 目标发 `QUERY MSG <id>`(User Guide 确认) |
| **APRS 集成** | 借 MSG/MSG TO: | `@APRSIS` 当群呼;入站 APRS 转存本机 | `m_aprsRelayDedupCache`;`relayPath="APRS"`。**踩红线 #1,CN 设计不得喂它** |

> upstream User Guide 对 relay 的定性:**不是同/异频原样转发射频(那叫中继台),而是"消息转发系统"——生成含原文+校验和+回程路径的新消息重发**(法理 FCC 97.219)。

### 6.C 公理 2 纠错(纠正 HANDOFF §4.2.2)

| 原断言(错) | 真相(v3.0.1 源码) |
|---|---|
| "autoreply_cmds 含 cmd 30 = RELAY-likes" | **cmd 30 = `" AGN?"`(repeat message),与中继无关**;RELAY = **cmd 5 `">"`**;5 在 `buffered_cmds`、**不在** `autoreply_cmds` |
| "compound 收到只进 buffer,无转发路径"(半验) | m_messageBuffer 确含 compound;但"无转发"未坐实,本 seq 转向 Route 决策后不再追 |

### 6.D 架构发现(Route ① 可行性分析的支撑,均已坐实)

| 发现 | 依据 |
|---|---|
| 中继/转发**路由 + 校验和编码中立**(全在解码后 QString 层操作) | relay handler `processCommandActivity.cpp:414` 全程 QString 正则/split/join;`checksum16`(`Varicode.cpp:482`)对文本字节算 CRC-16/KERMIT,`:2288` 在编码**前**以文本追加 |
| **唯一语言相关环节 = codec**(正文→Data 帧) | `buildMessageFrames`(`:2023`)→ `packDataMessage`(`:1888`)→ `JSC::compress`(`:1849`,英文 (s,c)-Dense Code) |
| **directed 命令帧 72-bit 满栈,无空位** | `packDirectedMessage`(`:1605-1715`):`[3 type=FrameDirected][28 from][28 to][5 cmd],[2 portable][6 num]=72`;cmd 5-bit 的 0..31 全占满 |
| **Data 帧旗标位满**:`bit[0]=isData`、`bit[1]=compressed(JSC/Huffman)`,无空旗标 | `unpackDataMessage`(`:1912`);加旗标位即移位破旧解码(向后不兼容) |
| **FrameType 3-bit 饱和**:0–3 具体类型,4–7 全归 data(高位=data,低位被丢) | `Varicode.h` FrameType 枚举 + 注释 `1XX=data` |
| **分类由 itype(`bits_`)驱动,非 payload FrameType**;全部 unpacker 拒绝 → `FrameUnknown` → 惰性(不触发中继/autoreply) | `DecodedText.h:60-62` unpackStrategies + 构造函数;各 tryUnpack 首行 gate `bits_ & JS8CallData` |

### 6.E 载体路线决策(本 seq 核心)

**决定性反证(公理 3,据 §6.D + R6 handler 结构)**:relay 是**解码→重构文本→重编码**重发(`reply="%1 *DE* %2".arg(text)` → buildMessageFrames 重编码),**不是原样转发**。因此:

- **旧台(pre-CN)无法正确中继 CN**:收 CN Data 帧 → 英文 JSC 解出**乱码** → 重编码 → **转发乱码**,消息在第一跳即毁。**两条路都是如此** —— 只有 CN-aware(3.0.2+)台能中继 CN。
- 若 CN 走官方 directed `>`:旧台命中 relay handler → **主动发 ACK/转发乱码(占频污染)**,比 FrameCompound 重。
- 若 CN 走 phase-1 FrameCompound:旧台**纯惰性**(只 band activity 显乱码,红线 #3 软,不发射)。

| 决策 | 依据(§5.5 四要素工作框架) |
|---|---|
| **主推 Route ②**:CN 中继和转发建在 **phase-1 FrameCompound 载体**上,自成体系 | **要做什么**:CN-aware 台间的中继和转发。**为何**:零上游改动、立即可做、对旧台污染最轻(惰性)、与 phase-1 架构(含已设计 ARQ)一脉相承。**做错后果**:需自解 dependency #2(载体位预算)。**更可逆替代**:Route ① |
| **Route ① 降为长线**:仅把"上游在 relay/data 路径留一个向后安全的 codec/语言选择位"作为**轻量 PR**长线推动,**短期不期望落地** | directed/data 路径满栈+主动解析,无便宜钩子;且旧台本就不能中继 CN,Route ① 相对 ② 的增益缩水为"CN 台间复用官方语法 + 路线图对齐"。最干净切入点 = 搭 upstream"待替换 data 帧格式"的便车请其预留 codec 位 |

**对 phase-1 红线的影响**:Route ② 沿用 FrameCompound 载体 → phase-1 红线 #1/#2 模型**继续成立**;不引入 directed 路径的主动污染问题。

**7. 未解决问题**(明确未决):

- **Route ② 的核心待解 = dependency #2**:FrameCompound 的 `bit[3..52]` 已占满,**没空间放 relay path + 原始发送方呼号**。这是下一步 CN 中继 spec 必须先攻克的设计问题(候选:多帧分段 / 重审 ILC 容量 / 控制帧 bit[8..10] 保留区 5..7)。
- Route ① 长线 PR 的**具体形态**:需进一步了解 upstream"待替换 data 帧格式"的计划(其 packDataMessage 标注 deprecated-since-2.2)。
- **四级审计原则**框架定义(PARK-024,挂起)。
- **未读验证**(本 seq 结论不依赖,但走 spec 前应补):`buildMessageFrames` 全体(坐实"重编码")、`processDecodeEvent` 对 `FrameUnknown` 的落地(丢弃 vs 显示)、itype 值空间、`packDataMessage` 精确位布局。

**8. 风险与挂起项**:

- **PARK-022(本 seq 决议为"已决")**:CN 开发线 git base = v3.0.1 + incoming-only 跟随 upstream。剩 setup 机械动作(建 dev 分支 / `git remote add upstream` / 定 sync 节奏)→ phase-2 实施启动时做。
- **PARK-023(新增)**:Route ① 上游"留 codec 位"轻量 PR(长线,低预期)。触发:upstream 公布 data 帧格式替换计划时,或我方 Route ② 稳定后。
- **PARK-024(新增)**:"四级审计原则"框架定义待 User 补(本 seq 挂起;暂用 §5.5 四要素)。
- **upstream Future-Work 重叠(关键外部情报)**:upstream 计划 SRARQ(撞 phase-1.5 议题 4 ARQ)、store-forward routing / automatic relay routing(撞本 phase)、公钥校验(撞 §4.2.1 dep #6)。**不重开已 closed 项,仅记录**;影响"自建 vs 对齐"长线策略,并入 PARK-023 一并考量。
- 既有 **PARK-010~021 不变**。

**9. 下一步候选**:

- **本会话外 / 下次会话**:进 Route ② 的 CN 中继和转发 **spec 设计**,**第一关 = dependency #2**(载体位预算)。届时两道闸门(复述无歪 + 框架,框架已 PARK)按需处理。
- 长线:择机起草 Route ① 的英文上游 proposal(PARK-023)。
- **commit 节点**:本 seq + 之前未 commit 的 phase-1.5 整套(seq=4/5 + DRAFT v3 + PARKING_LOT + HANDOFF v2)可一并 commit。

**10. 验证回执**(本 seq 建立的 v3.0.1 源码 + 一手文档锚点;phase-2/spec 回查):

源码(`v3.0.1`,仓库 JS8Call-improved):
```
JS8_Main/Varicode.cpp:48-124     directed_cmds map（>=5 relay / AGN?=30 / MSG=9 / MSG TO:=10 / QUERY=11 / QUERY MSGS=12 / QUERY CALL=13 …）
JS8_Main/Varicode.cpp:125        autoreply_cmds = {0,2,3,4,6,9,10,11,12,13,14,16,30}
JS8_Main/Varicode.cpp:128        buffered_cmds = {5,9,10,11,12,13,15,24}
JS8_Main/Varicode.cpp:131/134    snr_cmds={25,29} / checksum_cmds={{5,16},{9,16},{10,16},{11,16},{12,16},{13,16},{15,0},{24,16}}
JS8_Main/Varicode.cpp:482        checksum16（CRC-16/KERMIT，对文本字节，3 字符 varicode 结果）
JS8_Main/Varicode.cpp:1183/1207  packCmd / unpackCmd（cmd 5-bit）
JS8_Main/Varicode.cpp:1605-1715  packDirectedMessage（[3][28][28][5],[2][6]=72 满栈）
JS8_Main/Varicode.cpp:1849       JSC::compress（英文 (s,c)-dense）
JS8_Main/Varicode.cpp:1888/1912  packDataMessage / unpackDataMessage（bit[0]=isData,bit[1]=compressed；注 deprecated-since-2.2）
JS8_Main/Varicode.cpp:2133/2156/2288  buildMessageFrames 调 packDirectedMessage / packDataMessage / 对 line 施 checksum16（编码前）
JS8_Main/Varicode.h              FrameType enum（HB=0/Compound=1/CompoundDirected=2/Directed=3/Data=4[10X]/DataCompressed=6[11X]/Unknown=255）；TransmissionType（JS8Call=0/First=1/Last=2/Data=4）
JS8_Mode/DecodedText.h:60-62     unpackStrategies = {FastData,Data,Heartbeat,Compound,Directed}
JS8_Mode/DecodedText.cpp:81-107  构造函数（逐策略试，全不中→FrameUnknown）；gate = bits_ & JS8CallData
JS8_Mode/DecodedText.cpp:109/131/153/198/232  tryUnpackFastData/Data/Heartbeat/Compound/Directed
JS8_Mainwindow/processCommandActivity.cpp:414  relay handler（解码-重编码；递归 cmd 入 m_rxCommandQueue；STORE_RELAY_MSGS_TO_INBOX 编译开关）
JS8_Mainwindow/processCommandActivity.cpp:528/745/972/1042  MSG TO: / MSG / QUERY MSGS / QUERY CALL
JS8_Mainwindow/processCommandActivity.cpp:324-328/542/560/750  relayPath autoreply swap / APRS relay 路径 / m_aprsRelayDedupCache
JS8_Main/Inbox.h, Inbox.cpp      class Inbox（SQLite；type "STORE"）
```
一手文档:
```
js8call.com（= JS8Call-improved 站点）：v3.0.1 发布 2026-05-17；功能名"store-and-forward / relayed messages / message forwarding / APRS-iGate"
README：JS8Call-improved 现为官方源；Jordan 归档原仓库并加入；AC9KH 自 2.3.1 founded，2.4.0(2025-11-03)；2.5.0 起改回 JS8Call
User Guide：Inbox/Relay 命令语义；relay=新消息转发(FCC 97.219)；Future Work=SRARQ / store-forward routing / automatic relay routing / 公钥校验 / DTN；(s,c)-Dense Code 26 万条目（jsc.h/jsc.cpp/jsc_map.cpp）
```

**11. 作废标记**:

- **作废**:上一轮"上游保留**全新空闲 FrameType 值**作钩子"的设想 —— R9/R10 证 FrameType 3-bit 饱和、分类由 itype 驱动,无此空槽。
- **作废**:"FrameType **5/7 灰区**作钩子"的设想 —— 分类不在该层判别。
- **作废纠正**:HANDOFF §4.2.2 "cmd 30 = RELAY-likes" → 改为 cmd 30=AGN?、RELAY=cmd 5。
- **作废**:phase-1.6 原 **Step 2**(V3.0.1 不改即可转发 CN)—— 已证死路(我方 FrameCompound 帧不进 directed;且 relay 解码-重编码会毁 CN),被本 seq Route 决策取代。
- 原 phase-1.6 Step 1("自建 CN-only 中继")被本 seq 明确为 **Route ②** 并继续。

**关联修订**:本 seq **暂不触发 DRAFT v4**(尚无 CN relay/forward spec)。待 Route ② 实际 spec 完成后出 v4,届时修订:§8.2(CN 中继从"挂起"→"Route ② 设计中")+ 红线章节(记录 directed-vs-FrameCompound 载体分析及为何选 FrameCompound)+ 新增中继和转发协议层章节。HANDOFF 待 phase-1.6 实际收尾(spec 出稿)时再更新为 v3。
