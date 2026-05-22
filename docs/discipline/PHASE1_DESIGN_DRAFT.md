# JS8CALL-CN — V3.0.1+ 中文支持设计方案(phase-1 草案)

> **基础**:JS8Call V3.0.1(`origin/release/3.0.1`)
> **状态**:协议层方案收敛 / §9 V1/V2/V3/V4/V6/V7/V8/V9 + bit[53] trick **全部 PASS** / V10 推理 PASS / V5 信息性可省 / 实施留 phase-2 / **§4.5 中继转发协议层(Route ②)spec 收口**
> **已锁定决策**:红线 #3 软化 / PSK Reporter 残余实际为 0 / 不切 whitening polynomial / 兼容路径方案 / I18N ARQ 协议层完整 spec / **中继转发协议层(Route ②)完整 spec(§4.5)**
> **修订历史**:
> - 2026-05-21 phase-1 续接会话 —— §5.1 TTL 表、§10 PR 策略弱化为"候选,议题待澄清"(详见 HANDOFF §2.5)
> - 2026-05-21 phase-1.5 议题 1/2/3 收尾 —— §5.1 终稿(3 档状态机定型);议题 2 关闭确认 PARK-011 挂 phase-2;§10 改为"phase-1.5 主动挂起"
> - **2026-05-21 phase-1.5 议题 4 收尾** —— 新增 §4.4 ARQ 协议层(8 帧上限 / 首帧序号复用做总帧数广播 / (ε+β') 复合重传请求载体 / Selective-repeat / 三参数超时机制);§4.2 bit[5..7] 描述追加首帧复用语义 + ILC payload 内部 ARQ header 布局;§5.1 备注 ARQ 救回 QSO 算"成功交换";§7 phase-2 设计清单追加 ILC ARQ_FLAG 识别;§9 验证矩阵追加 V10;新增 PARK-020/021。详细决议见 STATE_SNAPSHOT seq=5。
> - **2026-05-22 phase-1.6 收尾** —— Route ② CN↔CN 中继转发协议层 spec 收口,DRAFT 升 **v4**(在真 v3 上手术式 10 处增改,其余字节不动):①修订历史本条 ②§4.2 登记 bit[8..10]=5=中继控制帧 ③§8.2 CN↔CN 中继从挂起→已 spec ④§9 V10 注记覆盖 =5 ⑤附录 B 追加 v3.0.1 中继 anchor ⑥新增 §4.5 中继转发协议层整章 ⑦头部状态/已锁定决策标注 Route ② ⑧§4.2 容量段追加中继 payload 口径 ⑨§7 phase-1 锁定追加中继帧 codec 旁路 ⑩附录 A 追加中继/控制帧兼容注记。详细决议见 STATE_SNAPSHOT seq=8/9。

## 1. 目标

在不破坏 V3.0.1 任何现有功能、与 V3.0.1 用户**同频道共存**的前提下,给 JS8Call 加中文(及后续日韩)原生通信能力。V3.0.1 用户**不需要升级**。

## 2. 红线与残余风险

| # | 红线/风险 | 性质 | 保护机制 | 验证 |
|---|---|---|---|---|
| 1 | APRS-IS 污染 | **硬,不可破** | 三层防御(§3 + 附录 A) | ✅ V1+V2+V3+V4+bit[53] |
| 2 | V3.0.1 auto-response 触发(ACK/RELAY/INFO/STATUS/MSG TO/AGN 等) | **硬,不可破** | 三层防御:① FrameType=001 → 不填 directed_;② `isDirectedMessage()`=false → 整段 directed 分支跳过;③ `isCommandAutoreply` 走 `directed_cmds.contains(cmd)` guard,空 cmd 不命中 | ✅ V1+V8 |
| 3 | V3.0.1 band activity 显示乱码 callsign | **软,接受** | 协议层不可消除;纯视觉,无主动行为 | (无需验证) |
| 4 | V3.0.1 经 PSK Reporter 上报乱码 callsign | **残余,V3.0.1 实测为 0** | `processRxActivity.cpp:92` 的 `!d.isCompound` guard 阻断 m_rxCallQueue 入队 | ✅ V2 FULL PASS |

## 3. 核心机制 — 兼容路径

**不阻止 V3.0.1 解码**(不切 whitening、不改 LDPC),而是利用 V3.0.1 协议中**已存在的"含 callsign 但非 directed message"中性入口** —— FrameType=001 (FrameCompound)。

```
V3.0.1 收到 I18N 帧:
  LDPC 解出 → dispatch 链 (FastData → Data → Heartbeat → Compound → Directed)
  → tryUnpackHeartbeat 退出(非 HB type)
  → tryUnpackCompound 命中 FrameCompound 分支
      · 填 compound_(乱码 callsign 形状)
      · 不填 directed_
      · 返回 true,dispatch 终止
  → 不进 tryUnpackDirected
  → processRxActivity 的 spotting guard `!d.isCompound` 失败 → logCallActivity(cd,true) 不调
  → m_rxCallQueue 无入队 → spotReport/pskLogReport 不调
  → m_messageBuffer[].compound 缓冲,90 秒后静默 remove
  → 仅 band activity 显示一行乱码 callsign(可接受)
```

**V3.0.1 实证**(`JS8_Mode/DecodedText.cpp:198-220`):

```cpp
bool DecodedText::tryUnpackCompound(QString const &m) {
    ...
    if (type == Varicode::FrameCompound) {
        message_ = compound_ % ": ";       // 不动 directed_
    } else if (type == Varicode::FrameCompoundDirected) {
        directed_ = {"<....>", compound_}; // 我们绝不走此分支
    }
    return true;
}
```

## 4. Wire 协议规范

### 4.1 TransmissionType

复用 V3.0.1 现有的 `JS8CallFirst | JS8CallLast = 0b011`(值 3),作为 I18N 帧的载体。V3.0.1 视为"合法单帧完整消息",不拼接多帧 buffer。

**V3.0.1 实证**(V7 PASS):
- `Varicode.h:33-38` enum 是位掩码定义(JS8Call=0 / First=1 / Last=2 / Data=4),3 = First|Last 是合法 bit 组合
- 全仓库无 `bits == 3` 精确数值匹配,所有检查都是单 bit 按位 AND
- `initializeDummyData.cpp:218` 中 V3.0.1 自身就用 `JS8CallFirst | JS8CallLast` 构造代表性单帧测试数据 —— 这是上游本意用法

### 4.2 单帧 72-bit payload 布局

**V3.0.1 Compound 帧物理结构**(`Varicode::packCompoundFrame`):

```
[3 bits FrameType] [50 bits callsign] [11 bits packed_11] [5 bits packed_5] [3 bits bits3]
 bits 0..2          bits 3..52         bits 53..63          bits 64..68       bits 69..71

其中 extra/num = (packed_11 << 5) | packed_5  (16 bit 复合字段)
```

**I18N 帧映射到此结构**:

| Payload bits | I18N 含义 | V3.0.1 视角 |
|---|---|---|
| `bit[0..2]` | FrameType **固定 = 001** | FrameCompound(✅ V9 PASS)<br>⚠️ 绝不能 = 010(FrameCompoundDirected,会填 directed_,红线 #2 破) |
| `bit[3..4]` | 位置标记(2 bit):00=单帧 / 01=首 / 10=中 / 11=末 | callsign 字段一部分,V3.0.1 视为 base37 callsign 比特 |
| `bit[5..7]` | **依位置标记上下文相关**(议题 4 终稿):<br>· `bit[3..4]=01`(首帧)→ **总帧数 - 1**(0..7 对应 1..8 帧)<br>· `bit[3..4]=00`(单帧)→ 总帧数 - 1 = 0(固定值)<br>· `bit[3..4]=10/11`(中/末帧)→ 消息序号(0..7)<br>末帧序号 + 1 = 总帧数(冗余,首帧丢失时反推) | 同上 |
| `bit[8..10]` | **语言 ID / 帧类型(议题 4 扩展)**:0=EN保留 / 1=CN / 2=JA / 3=KO / **4=控制帧(ARQ fallback)** / **5=中继控制帧(Route ②,见 §4.5)** / 6-7 保留 | 同上 |
| `bit[11..18]` | CRC-8(8 bit)覆盖所有非 CRC 比特(64 bit:bit[0..10] + bit[19..71]) | 同上 |
| `bit[19..52]` | ILC 内容 part1(34 bit)— **议题 4 修订**:首 1 bit 为 ARQ_FLAG,详见 §4.4.2 | 同上 |
| `bit[53]` | **anti-APRS lock = 1** | packed_11 的 MSB = num bit 15 → 强制 extra ≥ 32768 > nmaxgrid (32767)<br>→ unpackCompoundMessage 不 append grid/cmd(✅ bit[53] trick PASS) |
| `bit[54..71]` | ILC 内容 part2(18 bit) | packed_11 lower 10 + packed_5 + bits3,V3.0.1 在 Compound 分支不消费 bits3 |

**容量**(议题 4 后修订):
- ILC payload 物理 52 bit(=34+18)
- 议题 4 引入 ARQ_FLAG(bit[19],1 bit overhead)→ 纯数据帧有效 ILC = 51 bit
- 含 NACK 帧 ILC = 43 bit(扣 1 bit FLAG + 8 bit NACK bitmap)
- 8 帧消息理论容量约 408 bit(若一帧含 NACK 则 ~400 bit)
- 按 ~6.5 bit/字 估算:8 帧消息约 62-63 汉字(原 64,议题 4 损失 ~1 字)
- **中继控制帧(bit[8..10]=5)**:52 bit 全作路由 payload(原始呼号 28 / 目标哈希 16 / msg-ID 5 / TTL 3),不分 ILC 内容 / 不带 ARQ_FLAG —— 详见 §4.5

### 4.3 HB bits3 = 语言能力广播

V3.0.1 HB(FrameType=000)帧的 `bits3` 字段在 isAlt=0 时是 deprecated legacy(hbs map 8 条全是 `"HB"`,显示层对 bits3 无感知 — 已验证)。复用为语言能力:

| bits3(isAlt=0 时) | 语言能力 |
|---|---|
| 0 | EN-only(默认,V3.0.1 自然落点) |
| 1 | CN |
| 2 | JA |
| 3 | KO |
| 4..7 | 保留 |

**关键约束**:语言 ID **仅在 isAlt=0 的 HB 帧中有效**。当 isAlt=1 时(CQ-style HB),bits3 索引 cqs map(活的,8 条不同 CQ 字符串),**不能挪用作语言 ID**。

**V3.0.1 实证**(`Varicode.cpp:1361-1413`,V6 PASS):
- 正常 HB(isAlt=0):`cqNumber = hbs.key(type, 0)` → 因 hbs 所有值都 = "HB",QMap.key 返回最小匹配 key = **0** → V3.0.1 永远写 bits3 = 0
- CQ-style HB(isAlt=1):`cqNumber = cqs.key(type, 0)` → 可能写 1..7(cqs 索引)
- frame 由 `packCompoundFrame(callsign, FrameHeartbeat, packed_extra, cqNumber)` 构造,`cqNumber` 即 bits3 字段

**约定**:bits3 表示**编译期能力**(codebook 决定),不是当前 UI 偏好。能力稳定 → 缓存有意义。

### 4.4 ARQ 协议层(议题 4 终稿)

I18N 消息层引入选择性重传 ARQ,补齐 V3.0.1 英文 JSC "一帧丢就丢"的鲁棒性短板。

#### 4.4.1 消息分帧与总帧数广播

**最大帧数 = 8**(协议物理上限,bit[5..7] 序号 3 bit 即上限,议题 4 锁定不再缩)。

**总帧数广播策略 = 首帧序号字段复用**。详见 §4.2 bit[5..7] 描述。

接收侧获知总帧数的路径:

- 收到首帧 → 立即获知(bit[5..7] 即"总帧数-1")
- 首帧丢失但收到末帧 → 反推总帧数 = 末帧序号 + 1
- 首帧 + 末帧都丢失 → 整体超时放弃(无法 ARQ,设计承认)

#### 4.4.2 重传请求载体 — (ε+β') 复合策略

**主路径 — Piggyback NACK 进 ILC payload header**:

接收方判定丢帧后,等自己下一次自然要发的 I18N 数据帧机会,**在该帧 ILC payload header 里夹带 NACK bitmap**,与正常通信内容共用一个 TX 周期 —— **零额外开销**。

ILC payload 内部布局:

```
bit[19]        = ARQ_FLAG (0=纯数据帧,1=本帧含 NACK header)
若 ARQ_FLAG = 1:
  bit[20..27]  = NACK bitmap (MSB=SEQ 0, LSB=SEQ 7;每 bit 表示对应 SEQ 是否未收到)
  bit[28..52] + bit[54..71] = ILC 数据 (43 bit)
若 ARQ_FLAG = 0:
  bit[20..52] + bit[54..71] = ILC 数据 (51 bit)
```

代价:每帧 ILC payload 永久 -1 bit;含 NACK 帧额外 -8 bit。8 帧消息容量损失 ~1 汉字(从 64 降至 ~63)。

**Fallback — bit[8..10]=4 独立控制帧**:

适用单方监听场景(接收方无自然回话机会)。

- 接收方判定丢帧后,等 T_piggyback 秒(PARK-020,phase-2 实测拍)
- 期内无自然 piggyback 机会 → 发独立控制帧
- 帧形态:bit[8..10] = 4(语言 ID 保留区 4..7 取 4 做"控制帧"标识,5..7 仍保留)
- bit[19..71] 控制 payload 字段布局(子类型 / 目标消息标识 / NACK bitmap / CRC 等)**→ PARK-021,phase-2 实施前细化**

#### 4.4.3 丢帧检测时机

接收侧在**推断到末帧位置后**等 T_detect 秒(PARK-020),若未收齐则进入 ARQ 流程。

"推断到末帧位置"的两种路径:
- 收到末帧(bit[3..4]=11)→ 末帧序号 + 1 = 总帧数,核对已收 SEQ bitmap
- 收到首帧(bit[3..4]=01)→ 首帧 bit[5..7]+1 = 总帧数,从首帧到达时间起等 T_detect

#### 4.4.4 重传策略 — Selective-repeat

- 接收侧维护已收 SEQ bitmap(8 bit)
- 发送 NACK 时,标记所有未收 SEQ
- 发送侧收到 NACK 后,**只重发被标记的 SEQ**(不重发已收到的)

#### 4.4.5 超时与重试机制

phase-1 锁定**三参数机制**,具体数值 PARK-020,phase-2 实测拍:

| 参数 | 语义 | 初步候选(phase-2 调) |
|---|---|---|
| T_single | 单次 ARQ 请求超时(发出 NACK 后等待重传响应) | 60s(约 2 个 TX 周期机会响应) |
| N_retry | 重试次数上限(单次失败后再发请求的次数) | 3 次 |
| T_total | 整体放弃超时(从首帧接收开始算) | 180s |
| T_piggyback | Fallback 触发等待(等 piggyback 机会的时长) | 30s |
| T_detect | 末帧位置后判定丢帧的等待 | 30s |

#### 4.4.6 重传请求帧自身丢失

**复用 N_retry 机制(非递归)**。接收方判定"发出的请求帧已丢"的依据 = T_single 内未收到任何重传响应。不引入第二套超时系统。

#### 4.4.7 跟 §5.1 缓存状态机的交互

- **ARQ 救回**的 QSO 算"I18N 帧成功交换",参与 §5.1 状态 1→2 / 状态 2→3 升级判定
- **ARQ 彻底失败**的 QSO 不算(消息没成,状态机不被触发,无需特殊处理)

## 4.5 中继转发协议层(Route ②,phase-1.6 收口)

JS8CALL-CN 节点间(**CN↔CN**)多跳中继转发,补"弱信号 / 远距直连不可达"场景。**走 FrameCompound 兼容路径,绝不借 V3.0.1 RELAY cmd**(RELAY ∈ V3.0.1 directed_cmds,破红线 #2)—— 中继控制帧与内容帧一样对 V3.0.1 表现为乱码 callsign(V10),不触发 spot / autoreply。

> **范围**:本节 = Route ②(CN↔CN)。Step 2(让未改的 V3.0.1 转发 CN)仍挂起 / Route ① 长线轻量 PR(PARK-023);其"信封/隧道"路径(Route ③)已**永久技术否决**(效率不可接受),不再讨论。

### 4.5.1 中继模型 = M2 多跳

- **M2(多跳)**:消息可经多个中间 JS8CALL-CN 节点逐跳转发。
- **M1(单跳)已否决**:单跳无意义;弱信号 / 远距正需多个中间节点接力。

### 4.5.2 中继控制帧 — 载体与位预算

- **载体 = 中继控制帧,`bit[8..10] = 5`**(保留区 5..7 取一;与 §4.4 ARQ 控制帧 `=4` 并列;V10 覆盖 `=5` 的 V3.0.1 兼容性)。
- **payload = 52 bit**(非 53):`bit[53]` = anti-APRS 锁(必 =1)落在 `bit[19..71]` 区内吃 1 bit;可用 = `bit[19..52]`(34)+ `bit[54..71]`(18)= **52**。

52-bit payload 拆分:

| 字段 | 位宽 | 说明 |
|---|---|---|
| 原始发送方呼号 | 28 | 字面(对标 §4.2 packDirectedMessage 标准呼号 28-bit packCallsign),可显示 |
| 目标呼号哈希 | 16 | 收件方"是否给我"判定;假阳无害(多显一条)→ 哈希选型 PARK-027 |
| msg-ID | 5 | `(原始呼号, msg-ID)` 作去重键;0..31 |
| TTL 剩余跳数 | 3 | 0..7;**默认初值 3**;字段留 7 备短波传播不确定性 |
| **合计** | **52** | 整帧 CRC-8(`bit[11..18]`)已覆盖此区,无需内嵌 CRC |

控制帧结构:`bit[0..2]=001` / `bit[3..4]=00`(单帧)/ `bit[5..7]=0` / `bit[8..10]=5` / `bit[11..18]=CRC-8` / `bit[19..52]+bit[54..71]=52-bit payload` / `bit[53]=1` 锁。

### 4.5.3 跳模型 — M2 防爆 + 防环

| 项 | 决策 | 依据 |
|---|---|---|
| 路径**不上线缆** | 只带 TTL,不带累积呼号路径 | 携带路径 = 逐跳 28-bit 爆炸;TTL 计数器位预算随跳数恒定 |
| 防环 / 防无限转发 | TTL 每跳 −1 到 0 即丢 + 每节点 `(原始呼号, msg-ID)` 去重缓存 | 对标 V3.0.1 `m_aprsRelayDedupCache` 模式 |
| ACK/ARQ 回程 | 反向路径学习(节点记"从邻居 Y 听到 msg-ID X"),路径活在节点状态非线缆 | 见 §4.5.5 |
| max-hop 真实约束 | = 信道占空比(每跳重广播占 airtime),**非位预算** | TTL 几乎免费;默认 3 按信道礼仪,字段留 7 |

### 4.5.4 内容帧↔控制帧绑定 = C1 原子突发

- **决策**:中继消息整组原子收发(中继控制帧 + N 个内容帧一起);组内绑定 = JS8 原生 offset 归组(`m_messageBuffer[cd.offset]`,`processDecodeEvent.cpp:439`,v3.0.1↔master IDENTICAL)+ `bit[8..10]` 标签(5=控制 / 1=内容);msg-ID 专司跨跳去重。
- **内容帧零新增 bit**(中文容量全保);控制帧丢失被同一套 §4.5.5 / §4.4 ARQ 兜。
- **否决 C2**(每内容帧 msg-ID 标签):M2 下中继节点拿裸内容帧无法路由,C2"独立重发"好处买不到、却每帧付 bit → 净亏;退为逃生舱 **PARK-029**。

### 4.5.5 ACK/ARQ 回程 = Arch II 逐跳 store-and-forward + SNR 退避

- **架构**:逐跳 store-and-forward —— 每中继缓存(有界,策略 PARK-026)已转内容、本地答下游 NACK、cache miss 则往上游升级;多跳 ARQ = 每跳跑一遍已锁的 §4.4 ARQ,零新协议。
- **响应者选择**(广播无定向):SNR 加权退避(听下游 NACK 越强 = 越近末跳 → 退避越短 → 自然选出末跳应答)+ 听到即抑制 + msg-ID 去重;SNR 取自 decode(`cd.snr`)。显式寻址因控制帧 52-bit 满、不带中继方呼号而排除。
- **残留(接受)**:退避撞车偶发双重发,靠抑制 + 去重兜成有界冗余 —— 无定向广播介质固有税。
- **依据**:JS8 慢速,端到端多跳重传延迟不可接受 → 逐跳本地恢复;对齐 V3.0.1 store-and-forward Inbox(`addCommandToStorage("STORE")`)。

### 4.5.6 认证 = 开放信任 / CRC-8 only(dependency ⑥)

- **本版不做防伪造**,仅 CRC-8 防错码;开放信任模型。
- **依据**:控制帧 52/52 满,无 bit 放签名 / MAC;HAM 加密 / 签名受限、数字模式普遍不做;对齐 V3.0.1(其中继亦无防伪造);upstream 把公钥校验列 Future-Work → 认证随之 **PARK-030**。

### 4.5.7 关联 PARKING(均 phase-2)

PARK-026 中继缓存策略(大小 / 寿命 / 逐出)/ PARK-027 目标呼号 16-bit 哈希函数选型 + 碰撞容忍 / PARK-028 控制帧丢失硬化 / PARK-029 C1 逃生舱 / PARK-030 中继认证 / 防伪造。

## 5. 语言能力发现 — 三层机制

| 层 | 来源 | 触发 | 用途 |
|---|---|---|---|
| L1 | HB bits3 广播 | 被动,每个 HB 周期 | 主要发现机制 |
| L2 | I18N 帧头 bit[8..10] | 主动,每个 I18N 帧自描述 | 容错冗余 |
| L3 | callsign → 语言缓存 | 历史积累 | 跨会话快速判定 |

### 5.1 缓存状态机 — phase-1.5 终稿

**框架**:状态机自动升级 + 全档续期 TTL。**3 档**,无永久 pin,无 per-peer 手动控制。

| 状态 | 入档条件 | TTL | 升级触发 | TTL 续期触发 | 过期行为 |
|---|---|---|---|---|---|
| **0**(默认) | 所有未入缓存的 peer | — | 收到 peer HB,bits3 ∈ {1,2,3} → 入状态 1 | — | N/A(不在缓存) |
| **1**(首见外语 HB) | 进入触发 ← | 1 h | 完成"双向 CN QSO"(我发一帧 CN + 收到 ACK,且收到对方至少一帧 CN)→ 升状态 2 | 同档内收到同语言 bits3 的 HB | TTL 到 → 完全清除,退状态 0 |
| **2**(完成首次外语 QSO) | 进入触发 ← | 12 h | **滚动 24h 窗口**:任意时刻往前看 24h,累计 ≥ 3 次 I18N 帧成功交换,且总跨度 ≥ 30 min → 升状态 3 | 同档内任一 I18N 帧成功交换 | TTL 到 → 完全清除,退状态 0 |
| **3**(持续稳定) | 进入触发 ← | 30 d | (顶档,无更高状态) | 同档内任一 I18N 帧成功交换(等价"30d 从最后一次活动起算") | TTL 到 → 完全清除,退状态 0 |

**附加规则**:

- bits3 = 0(EN-only)的 HB **不入缓存** —— 默认即 EN,无需显式标记
- 缓存粒度 = **callsign-only**(software version 字段需求 → PARK-019,phase-2 codec 设计时审字段预算)
- UI 只暴露"清除全部语言缓存"总开关,无 per-peer 手动控制
- **ARQ 救回的 QSO 算"I18N 帧成功交换"**,参与状态 1→2 / 状态 2→3 升级判定(议题 4 拍板)

**保留 edge case**(已识别,均符合"准永久续期" + "档位语义自然结果",非 bug):

- 状态 1 长期挂载:peer 持续发 CN HB 但从不发起 I18N → 状态 1 永久续期
- 状态 2 低频 peer:不满足"24h 滚动内 3 次 + 跨 30 min"的低频用户永久挂状态 2,12h TTL 续期。符合状态 3"持续稳定 24h+"的语义
- 状态 3 长期挂载:活跃 peer 在状态 3 持续续期 30d → 实际"准永久",这就是"无永久 pin"设计的工作机制

**决策依据**:详见 STATE_SNAPSHOT seq=4(议题 1)+ seq=5(议题 4 ARQ 关联)。

## 6. QSO 流程

### 6.1 5 步英文 ritual(完全等同 V3.0.1)

```
1. CQ      "CQ CQ CQ DE BG1ABC"
2. 应答    "BG1ABC DE BG2DEF"
3. SNR1   "BG2DEF -15"
4. SNR2   "BG1ABC R-12"
5. 73/QSL "73 SK"
```

这 5 帧**绝不发 I18N=3** —— 全英文,V3.0.1 完全兼容。

### 6.2 中文 I18N 启动条件 — 两种模式,用户可选

**模式 A(严格档,默认)** — 适合陌生 peer / 第一次接触:
- 必须完成标准 5 步英文 ritual(同 V3.0.1)
- 5 步完成后查缓存:双方互证 CN 能力 → ESTABLISHED_SAME_LANG → 允许 CN
- 否则 → ESTABLISHED_EN_ONLY,UI 禁中文

**模式 B(信任档,熟人快捷)** — 适合已确认过的 buddy:
- L3 缓存命中 + 标记为 CN + TTL 有效 → 直接允许 CN 输入,**跳过 ritual**
- 缓存未命中 / 过期 → 自动退回模式 A

**UI 设置**:
- 全局默认模式(出厂建议:模式 A)
- per-peer 覆盖(单个熟人/陌生人单独切)

## 7. 中文 Codec — ILC(详细设计挂起至 phase-2)

**phase-1 锁定**:

- **位置**:新建独立目录 `JS8_I18N/`,镜像 `JS8_JSC/` 结构与接口(`compress`/`decompress`)
- **原则**:多层 codebook 按字频分层(候选起点 8/64/512/4096+)
- **隔离**:V3.0.1 英文 JSC 路径完全不动
- **议题 4 追加**:ILC payload 解析需识别 bit[19] = ARQ_FLAG;若为 1,跳过 bit[20..27] NACK bitmap,从 bit[28] 开始解码 ILC 数据。ARQ_FLAG 是协议层定义的契约,codec 必须遵守
- **Route ② 追加**:中继控制帧(`bit[8..10]=5`)是协议层路由元数据,**不经 ILC codec**;codec 只处理内容帧(`bit[8..10]=1`)

**phase-2 设计**(进 PARKING_LOT):

- 具体 tier 大小、前缀码、内容分配
- **应急词处理机制**(PARK-011)—— phase-1.5 已评审并确认 PARK 触发条件正确。**理由:全球应急通信必须英文**,中文 codec 不在协议层提供应急词专门处理,phase-2 codec 内部按字频自然落点(若需 tier 0 锁定属 codec 内部决策,无协议层 dependency)
- 版本演进 / append-only 预留区机制
- Codebook 文件格式 / 分发 / 热更新
- **议题 4 关联**:ILC payload 内部 ARQ_FLAG 头部识别 / NACK bitmap 跳过逻辑

## 8. 不实现 / 挂起

### 8.1 phase-1 明确不做

- whitening polynomial 切换 / 改 LDPC 矩阵(本方案不需要)

### 8.2 挂起待定(进 PARKING_LOT,phase-2 或之后决定)

- ~~CN↔CN 中继~~ → **已 spec(Route ②,见 §4.5)**,自 phase-1.6 起移出本挂起表
- CN 经 V3.0.1 中继(**Step 2**;V3.0.1 不识别 I18N)—— **仍挂起** / Route ① 长线轻量 PR(PARK-023);其信封/隧道路径(Route ③)已永久技术否决
- 跨语言互通(CN↔JA 等)

### 8.3 强制降级

target 缓存为 EN-only 时,UI 禁中文,自动退英文。

## 9. V3.0.1 验证矩阵

| # | 验证项 | 影响 | 状态 | V3.0.1 源码 anchor |
|---|---|---|---|---|
| V1 | tryUnpackCompound(FrameType=FrameCompound) 不填 directed_ | 方案命根子 | ✅ PASS | `JS8_Mode/DecodedText.cpp:198-220` |
| V2 | spotting 路径在 isCompound==true 时跳过 | 残余 PSK 污染范围 | ✅ FULL PASS | `JS8_Mainwindow/processRxActivity.cpp:92` guard |
| V3 | unpackGrid(value > nbasegrid) 返回 "" | bit[53]=1 trick 支撑 | ✅ PASS | `JS8_Main/Varicode.cpp:1157-1163` |
| V4 | APRS-IS 上报 grid.length() < 4 早退 | bit[53] 二级 backup | ✅ PASS | `JS8_UI/mainwindow.cpp:2422`(`spotAprsGrid` 内)|
| bit[53] trick | bit[53]=1 → extra > nmaxgrid → 无 grid/cmd append | Compound 路径不放出乱码 grid | ✅ PASS | `JS8_Main/Varicode.cpp:1494-1525`(`unpackCompoundMessage`)|
| V6 | packHeartbeatMessage 实际写 bits3 = 0 | §4.3 映射定稿 | ✅ PASS | `JS8_Main/Varicode.cpp:1361-1413` |
| V7 | TransmissionType=3 = `JS8CallFirst\|JS8CallLast` 合法单帧 | I18N 载体合法性 | ✅ PASS | `JS8_Main/Varicode.h:33-38` + `JS8_Mainwindow/initializeDummyData.cpp:218` |
| V8 | buffered_cmds 触发条件依赖 `directed_cmds.contains` | 多帧 buffer 不被打乱;红线 #2 第三层防御(顺带) | ✅ PASS | `JS8_Main/Varicode.cpp:128, 1239-1242` |
| V9 | FrameCompound enum = 1 | wire pattern 锁定 | ✅ PASS | `JS8_Main/Varicode.h:51` |
| **V10** | **bit[8..10] ∈ {4,5,6,7} 时 V3.0.1 不做语义特殊处理**(把整个 bit[3..52] 当 callsign 解码,不识别"语言 ID"字段),控制帧载体 V3.0.1 兼容(含 **=5 中继控制帧 / Route ②**) | **议题 4 控制帧 fallback 兼容性** | 🟡 **推理 PASS**(基于已验证的 V3.0.1 bit[3..52] 整体当 callsign 解码 + Varicode.cpp:1494-1525 unpackCompoundMessage 行为;phase-2 启动前 EXEC sanity check,**须含 =5**) | `JS8_Main/Varicode.cpp:1494-1525`(已 cite,议题 4 推理基于此) |
| V5 | pskLogReport 无 callsign 格式校验 | (V2 后已无需验证) | 可省 | `JS8_UI/mainwindow.cpp:2444+` |

## 10. 上游 PR 策略 — 候选方案,phase-1.5 主动挂起

> ⚠ **此节为候选方案。phase-1.5 主动挂起,phase-2 实施推进时或之后再议。** 上 session 记录的 2 次 PR 节奏无 EXEC 证据,且 PR 时机本质上依赖 phase-2 实施进度与早期测试反馈,在 phase-1.5 阶段无法实质决策。

下方为上 session 记录的候选(2 次 PR 节奏),仅作记忆参考:

| PR | 时机 | 内容 |
|---|---|---|
| 第 1 次 | phase-2 实施方案确定 + 实现完成时 | 完整 phase-1 协议规范 + ILC codec + UI 集成 + QSO 状态机扩展 + **ARQ 协议层**(整包) |
| 第 2 次 | 第 1 次 PR 落地后,定向邀约测试满 2 个月,带真实使用反馈 | 根据测试发现的协议/codec/ARQ 问题做迭代 |

第 1 次 PR 之前**不主动接触上游**(避免半成品干扰 review)—— 此条同属待定。

---

## 附录 A. 防御深度图

```
我们的 I18N 帧 → V3.0.1
  ↓
红线 #1(APRS-IS)三层防御:
  Layer 1(V1+V2): isCompound guard 阻断 m_rxCallQueue
    → spotReport / pskLogReport 不调
  Layer 2(bit[53]=1 trick): extra > nmaxgrid
    → Compound unpack 不 append grid/cmd → grid 字段为空
  Layer 3(V4 spotAprsGrid): grid.length() < 4 → APRS-IS 不上报
  ↓
红线 #2(auto-response)三层防御:
  Layer 1(V1): tryUnpackCompound 不填 directed_
  Layer 2(V2): isDirectedMessage() = false → directed 处理分支跳过
  Layer 3(V8): isCommandAutoreply / isCommandBuffered 走 directed_cmds.contains
    → 空 cmd 不命中已知 directed cmd → 即使误进也不触发
  ↓
APRS-IS / 全球 HAM 观察网 / V3.0.1 自动行为 全部不被触发
```

> **中继 / 控制帧注记**:中继控制帧(`bit[8..10]=5`,Route ②)与 ARQ 控制帧(`=4`)同走 FrameCompound 兼容路径 —— V3.0.1 把整个 `bit[3..52]` 当乱码 callsign 解(V10),不触发 spot / autoreply。两类控制帧均受上述红线 #1 / #2 三层防御覆盖。

## 附录 B. V3.0.1 关键源码 anchor 速查

(本会话 phase-1 / phase-1.5 验证过的位置,phase-2 实施 / 上游 PR 时回查)

```
JS8_Main/Varicode.h:33-38           TransmissionType enum(位掩码;V7 PASS:值 3 = First|Last 合法单帧)
JS8_Main/Varicode.h:48-58           FrameType enum 定义
JS8_Main/Varicode.h:51              FrameCompound = 1 (= 001 binary,我们的 FrameType)
JS8_Main/Varicode.cpp:125-135       autoreply_cmds / buffered_cmds / snr_cmds / checksum_cmds 集合
JS8_Main/Varicode.cpp:128           buffered_cmds = {5,9,10,11,12,13,15,24}(V8 PASS)
JS8_Main/Varicode.cpp:214-216       nbasegrid=32400 / nusergrid=32410 / nmaxgrid=32767
JS8_Main/Varicode.cpp:287-294       cqs map(活的,8 条不同 CQ 字符串,禁挪用)
JS8_Main/Varicode.cpp:292-295       hbs deprecation comment "deprecated as of 2.2"
JS8_Main/Varicode.cpp:296-304       hbs map(8 条全 "HB",可挪用做语言能力)
JS8_Main/Varicode.cpp:1157-1163     unpackGrid(value > nbasegrid → return "",V3 PASS)
JS8_Main/Varicode.cpp:1239-1242     isCommandBuffered(要求 directed_cmds.contains,我们的 compound 帧不命中,V8 PASS)
JS8_Main/Varicode.cpp:1253-1256     isCommandAutoreply(同款 guard,红线 #2 第三层防御)
JS8_Main/Varicode.cpp:1361-1413     packHeartbeatMessage(V6 PASS:正常 HB bits3 恒 0,CQ-style 写 cqs 索引)
JS8_Main/Varicode.cpp:1414-1432     unpackHeartbeatMessage(HB 解包,带 bit 15 mask)
JS8_Main/Varicode.cpp:1494-1525     unpackCompoundMessage(关键!extra > nmaxgrid → 无 grid/cmd,bit[53] trick PASS;V10 控制帧载体兼容性推理依据)
JS8_Main/Varicode.cpp:1525-1555     packCompoundFrame(72-bit 物理 wire layout)
JS8_Mode/DecodedText.cpp:153-188    tryUnpackHeartbeat
JS8_Mode/DecodedText.cpp:198-220    tryUnpackCompound(命根子!FrameCompound 分支不填 directed_,V1 PASS)
JS8_Mode/DecodedText.cpp:232-258    tryUnpackDirected(本设计绝不走此路径)
JS8_Mode/DecodedText.h:34           isCompound() { return !compound_.isEmpty(); }
JS8_Mainwindow/processDecodeEvent.cpp:335    `!decodedtext.isCompound()` 早退 guard
JS8_Mainwindow/processDecodeEvent.cpp:359    `decodedtext.isCompound()` 进 compound 分支
JS8_Mainwindow/processDecodeEvent.cpp:439    `m_messageBuffer[cd.offset].compound.append(cd)`
JS8_Mainwindow/processDecodeEvent.cpp:469    isCommandBuffered 检查点(被 directed_cmds.contains 守住)
JS8_Mainwindow/processRxActivity.cpp:92      关键 spotting guard `!d.isCompound`(V2 PASS)
JS8_Mainwindow/processRxActivity.cpp:100     `logCallActivity(cd, true)` 入 m_rxCallQueue 的唯一活跃点
JS8_Mainwindow/processBufferedActivity.cpp   缓冲池清理(不触发任何 spot/log)
JS8_Mainwindow/initializeDummyData.cpp:218   V3.0.1 自身用 First|Last 表示单帧完整消息(V7 旁证)
JS8_UI/mainwindow.cpp:2359-2369     spotReport(无 grid 检查,但被 V2 阻断)
JS8_UI/mainwindow.cpp:2418-2450     spotAprsGrid(有 grid.length() < 4 检查,APRS-IS 专用,V4 PASS)
JS8_UI/mainwindow.cpp:2444+         pskLogReport 定义
JS8_UI/mainwindow.cpp:6597-6598     m_rxCallQueue 出队循环(唯一调 spotReport + pskLogReport 处)

# —— 中继/转发 anchor(@ v3.0.1,seq=8 §6.B 重 pin;Route ② 承重) ——
# 唯一与 master 差异 = L737 " CQ" 分支(logCallActivity/logHeardGraph),非中继;L737 前同行号,后 master 偏高,以下以 v3.0.1 为准
JS8_Mainwindow/processCommandActivity.cpp:326,328   relayPath autoreply swap
JS8_Mainwindow/processCommandActivity.cpp:441        relay handler parseRelayPathCallsigns(relayPath join 457;ACK reply 459)
JS8_Mainwindow/processCommandActivity.cpp:599,600,610,621,625
                                    MSG TO: 存储路径(parseRelayPath 599 / relayPath 600 / cd.relayPath 610 / addCommandToStorage("STORE") 621 / ACK 625)
JS8_Mainwindow/processCommandActivity.cpp:560-590,773-794
                                    APRS dedup m_aprsRelayDedupCache(cd.relayPath="APRS" 579 / 788)—— M2 去重缓存对标模式
JS8_Mainwindow/processCommandActivity.cpp:858,978,1045   QUERY 系 replyPath(relayPath split)
```
