# HANDOFF — phase-1.6 → phase-2(实施)

> **生成时间**:2026-05-22,phase-1.6 收尾
> **修订**:相对 v2 全面更新 —— phase-1.6 议题(Route ② CN↔CN 中继转发协议层)spec **收口**;DRAFT 升 **v4**;snapshot 至 **seq=9**;基线纠正 master→**v3.0.1** + re-home 工作线 `cn-dev`
> **目的**:为 phase-2(实施)提供衔接锚点,避免设计跑偏 / 重复讨论已决项 / 重新引入污染
> **本会话产出**:Route ② 全 dependency 收口(详见 STATE_SNAPSHOT seq=6/7/8/9)+ DRAFT v4 + PARKING 026~030 + 本 HANDOFF v3
> **下次会话使命**:**phase-2 实施** —— ILC codec 详细设计 + 中继/ARQ 实现 + UI 集成 + 上游 sync setup;**启动前先跑 V10 EXEC sanity check(含 bit[8..10]=5)**

---

## 1. 当前状态(指针)

| 文档 | 状态 | 路径 |
|---|---|---|
| 设计草案 | **v4** 定稿,新增 §4.5 中继转发协议层(Route ②) | `docs/discipline/PHASE1_DESIGN_DRAFT.md`(约 458 行) |
| 状态快照 | **seq=9** 已落 `cn-dev`;seq=10(phase-1.6 收尾)待 append | `docs/discipline/STATE_SNAPSHOT.md` |
| 纪律 cheatsheet | v1.1(本会话未动) | `docs/discipline/CHEATSHEET.md` |
| 挂起清单 | 累积至 **PARK-030** | `docs/discipline/PARKING_LOT.md` |
| 本交接文档 | **v3**,当前文件 | `docs/discipline/HANDOFF.md` |

**基线**:base = **v3.0.1**;工作线 = `cn-dev`(off v3.0.1);docs backfill re-home commit = `05d377ba`(15 files,`[loc][docs]`)。

**DRAFT §9 验证矩阵**:V1/V2/V3/V4/V6/V7/V8/V9 + bit[53] trick **全 PASS**;**V10**(控制帧载体,含 =5 中继控制帧)推理 PASS,**phase-2 启动前 EXEC sanity check 须含 =5**;V5 可省。

---

## 2. 已决定不做 / 不要重提(防止下次会话 Claude 重新提议)

| 已放弃 / 否决方案 | 理由 |
|---|---|
| **Route ③ 信封/隧道(把 CN 编码成 ASCII 信封过英文中继)** | **永久技术否决**:效率结构性劣化(英文调优 codec + 失词典增益 + 双重编码,变差方向必然),弱信号模式下逐跳累加不可接受。**未来 session 不得再提**(seq=7 拍死) |
| **M1 单跳中继** | 否决:单跳无意义,弱信号/远距需多个中间节点接力 → 定 M2 多跳 |
| **C2 每内容帧 msg-ID 标签** | 否决:M2 下中继节点拿裸内容帧无法路由,C2 好处买不到却每帧付 bit → 净亏。退逃生舱 PARK-029 |
| 切换 whitening polynomial / 改 LDPC | 兼容路径方案更优(seq=3 C-1 作废 seq=2 G) |
| hbs + cqs 联用扩展到 6 bit 语言 ID | cqs 是活的(`Varicode.cpp:287-294`),禁挪用 |
| 在 phase-1 / phase-1.5 / phase-1.6 做 codebook 详细设计 | 显式挂起到 phase-2 |
| 中文应急词 Tier 锁定(议题 2) | 全球应急通信必须英文;PARK-011 phase-2 codec 内部按字频自然落点 |
| 跨语言互通(CN↔JA) | 显式挂起到 phase-2 之后 |
| 引用 2.5.2 时代的方案文档 | 用户显式要求,避免污染;不要请求或重读 |

> **注**:CN↔CN 中继(Route ②)已 spec(DRAFT §4.5),**不再是放弃项**。仍挂起的是 **Step 2**(让未改的 V3.0.1 转发 CN)—— 见 PARKING / DRAFT §8.2。

---

## 3. PARKING_LOT 累积清单

phase-1 / phase-1.5 既有:**PARK-010~021**(其中 011/016 已 closed,见 v2 §3)。
phase-1.6 新增(详见 PARKING_LOT.md;源 seq=9 §8):

| ID | 内容 | 触发 / phase |
|---|---|---|
| PARK-026 | 中继缓存策略(大小/寿命/逐出) | phase-2 实施 |
| PARK-027 | 目标呼号 16-bit 哈希选型 + 碰撞容忍 | phase-2 |
| PARK-028 | 控制帧丢失硬化(现接受"似末跳直发") | 现场实测丢失频发 |
| PARK-029 | C1 逃生舱(每内容帧 msg-ID 标签) | 现场 offset 碰撞 / 掉队帧频发 |
| PARK-030 | 中继认证 / 防伪造(现 = 开放信任) | upstream 公钥校验落地 |

> **作废提醒**:PARK-025(Route ③ 暂缓框架)已于 seq=7 作废;PARK-005~009(whitening 硬假设)早 phase-1 作废。**PARK-024**(四级审计原则框架定义)仍挂起待 User 补。

---

## 4. phase-2 议题清单 + 启动条件

### 4.1 启动条件(不变 + 强化)

1. phase-1 草案(**v4**)最终批准
2. **V10 EXEC sanity check —— 必须含 `bit[8..10]=5`**(控制帧载体在 v3.0.1 真机/真码确认不触发语义特殊处理)
3. 中文 HAM 真实通联语料字频统计(PARK-018,codec 设计前置)

### 4.2 上游 sync setup(承 PARK-022 剩余,phase-2 启动时做)

- `git remote add upstream` + sync 节奏(CHEATSHEET §2:6 月窗口内 incoming-only)
- master 上孤立的 `c9cfbd6e`(seq=6 doc commit,落错基线)是否清理
- `cn-dev` 分支名 vs **WORKFLOW §5.3** 命名对齐

### 4.3 实施清单(spec → code)

- **ILC codec**:tier 大小 / 前缀码 / 内容分配(PARK-010)、版本演进 append-only(012)、文件格式/分发/热更新(013);新建 `JS8_I18N/` 镜像 `JS8_JSC/`
- **中继实现**:M2 控制帧(`bit[8..10]=5`,52-bit payload)+ 跳模型(TTL/去重/反向路径学习)+ C1 原子突发绑定 + Arch II 逐跳 store-and-forward 回程 + SNR 退避;缓存策略(026)、哈希选型(027)
- **ARQ 实现**:§4.4 选择性重传 + 5 参数实测拍数(PARK-020)、fallback 控制帧 payload 布局(PARK-021)
- **认证**:开放信任 / CRC-8 only;随 upstream 公钥校验对齐(PARK-030)
- **UI 集成 + QSO 状态机扩展**(缓存 3 档状态机 §5.1)
- **上游 PR 策略**:DRAFT §10 仍为候选 / 主动挂起,依实施进度与早期测试反馈再议

---

## 5. phase-2 SESSION-OPEN manifest 模板

```
[SESSION-OPEN]
任务类型: dev
阶段: phase-2(实施 — codec + 中继/ARQ + UI + sync setup)
本 phase 推进原始目的的方式: 把 phase-1 v4 协议层 spec 落成可运行实现,
                              先 ILC codec + 中继/ARQ 核心,再 UI 集成。
                              启动前跑 V10 EXEC sanity check(含 bit[8..10]=5)。
应载入: CHEATSHEET v1.1 + STATE_SNAPSHOT seq=8/9/10 + PHASE1_DESIGN_DRAFT.md v4 + HANDOFF.md v3
允许 retrieve: DISCIPLINE 按 § / WORKFLOW §5.3(commit 前缀)/ §4.2(凭据红线)/ §2(sync)
禁载入: DISCIPLINE / WORKFLOW 全文 / 2.5.2 时代外部文档 / Route ③ 任何材料 / 已 closed 议题重复讨论
[SESSION-OPEN END]
```

---

## 6. 下次会话开局推荐动作

1. User 发 SESSION-OPEN manifest(上模板)
2. **先跑 V10 EXEC sanity check(含 =5)** —— 这是 phase-1 唯一遗留的"推理 PASS"项,实施前必须实证
3. Claude 复述对 phase-1 v4 已定稿的理解(兼容路径 + ARQ + 缓存状态机 + **Route ② 中继**),User 确认无歪
4. PARK-022 剩余 sync setup(remote / 节奏 / 孤立 commit / 分支名对齐)
5. 进实施:codec 与中继可并行,但 codec 是 ARQ/中继内容帧的前置
6. 每完成一块 append snapshot 子段(seq=11 起)

---

## 7. 注意事项 / 红线给下次会话 Claude

- **不要重提 §2 的已放弃/否决方案**(尤其 Route ③ 信封隧道、M1、C2)
- **红线 #1(APRS-IS)/ #2(auto-response)在中继议题里仍硬,不可破** —— 中继控制帧(=5)与 ARQ 控制帧(=4)都走 FrameCompound 兼容路径,不可借 V3.0.1 RELAY cmd
- **挂起项讨论严格按 CHEATSHEET §4.6 范围管控**(含主线悬置类触发词:"先打地基/先搭脚手架"等 → 回核 manifest"推进原始目的"字段)
- **每条 EXEC 命令走 paste-bridge**;默认加 `git --no-pager`;**写操作 EXEC 不与前置检查批处理,`git add` 用具体文件路径**(seq=9 教训:整目录 add 误纳垃圾)
- **commit 节点**:phase-1.6 三产物(DRAFT v4 + PARKING 026~030 + HANDOFF v3 + seq=10)未 commit。落 `cn-dev`,`[loc][docs]` 前缀(WORKFLOW §5.3)。Tier 表内属本地,CLI 申请后执行
- **PARKING_LOT.md 落盘**:026~030 为 append-delta,粘入前核 ID 无碰撞

---

## 8. 关键不变量(下次会话不要轻易改的硬约束)

| 不变量 | 来源 |
|---|---|
| 红线 #1(APRS-IS)、#2(auto-response)硬,不可破 | 项目根目标 |
| 红线 #3(band activity 乱码)软,接受 | phase-1 续接会话 |
| FrameType=001(FrameCompound)是 I18N 帧固定值 | V1 + V9 + bit[53] trick |
| `JS8_JSC/` 不动,新建 `JS8_I18N/` | 用户原始约束 |
| HB bits3 语言 ID 仅 isAlt=0 有效 | V6 实证 |
| `bit[53] = 1` anti-APRS lock | bit[53] trick PASS |
| **bit[8..10] = 0..3 数据帧(EN/CN/JA/KO),4 = ARQ 控制帧,`5 = 中继控制帧`(Route ②),6..7 保留** | 议题 4 终稿 + seq=8/9(中继) |
| **中继模型 = M2 多跳;TTL(3 bit,默认 3)+ `(原始呼号,msg-ID)` 去重 + 反向路径学习;路径不上线缆** | seq=8 §6.C/D |
| **中继控制帧 payload = 52 bit**(原始呼号 28 / 目标哈希 16 / msg-ID 5 / TTL 3);非 53(bit[53] 是 anti-APRS 锁) | seq=8 §6.C 自纠 |
| **绑定 = C1 原子突发**(offset 归组 `processDecodeEvent.cpp:439` + bit[8..10] 标签);内容帧零新增 bit | seq=9 §6.A |
| **回程 = Arch II 逐跳 store-and-forward + SNR 加权退避(`cd.snr`)** | seq=9 §6.C |
| **中继认证 = 开放信任 / CRC-8 only**(防伪造 PARK-030,随 upstream 公钥校验对齐) | seq=9 §6.D |
| **bit[19] = ARQ_FLAG,bit[20..27] = NACK bitmap(条件)** | 议题 4 ILC payload 契约 |
| **首帧(bit[3..4]=01)的 bit[5..7] = 总帧数 - 1** | 议题 4 总帧数广播 |
| **缓存状态机 3 档(无永久 pin),全档续期 TTL,bits3=0 不入缓存,callsign-only 粒度** | 议题 1 终稿 |
| **应急词处理 = phase-2 codec 内部决策(无协议层支持)** | 议题 2 关闭依据 |
| **base = v3.0.1;工作线 `cn-dev`** | PARK-022 + seq=8 纠正 |
