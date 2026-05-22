## seq=9

**1. 时间戳**:2026-05-22T<HH:MM:SS>+08:00  *(本会话操作时间;User 提交前调整为精确时间)*

**2. 序号**:9

**3. 当前阶段名**:phase-1.6 续 — Route ② CN 中继转发协议层 spec **收口**。design 任务。本 seq 完成中继 spec 全部 dependency 决议,触发 DRAFT v3→v4。

**4. 状态摘要**:

- **基线**:续在 `cn-dev`(off v3.0.1);seq=8 的纠正(base=v3.0.1 / §10 重 pin / 控制帧 53→52)继续有效。
- **核心(本 seq)**:中继转发协议层 spec **收口**。HANDOFF §4.2.1 的 6 个 dependency 全部决议 + 绑定 + 认证拍板。**多跳中继 M2 继续**(中途考虑挂起,User 否决)。DRAFT 升 **v4**。
- **re-home 善后纪实**:上轮 `git add docs/discipline/`(整目录)误纳全部未跟踪文档 → **User 追认 B(全套 discipline 文档 backfill 到 cn-dev)**;清除 3 个垃圾(`seq6 2.md`/`seq7 2.md` macOS 副本 + `phase1.5_topic1_cache_ttl.md` 过期暂存);amend 成 `[loc][docs]` 前缀。最终 re-home commit = **`05d377ba`(15 files)**。

**5. 本段动作**(实际执行):

1. 接 seq=8 dependency #2 已锁的中继模型 M2 + 控制帧 bit[8..10]=5 + 52-bit 拆分,推进绑定与回程。
2. **绑定拍板 = C1 原子突发**(offset 归组 + bit[8..10] 标签,内容帧零新增 bit)。评估并否决 C2(每内容帧 msgID 标签):M2 下中继节点拿裸内容帧无法路由,C2 的"独立重发"好处买不到,白付 bit。
3. **考虑挂起 M2**(改做 standalone store-forward / A′ 关 1.6 / B 仅信箱)→ User 判定"都一样,relay 本质=store-forward 链",**否决挂起,M2 继续**。
4. **ACK/ARQ 回程 = Arch II 逐跳 store-and-forward**:每跳缓存已转内容、本地答下游 NACK、cache miss 则往上游升级;多跳 ARQ = 每跳跑一遍已锁的 §4.4 ARQ。
5. **响应者选择(广播无定向)= SNR 加权退避 + 听到即抑制 + msgID 去重**:听 NACK 越强(越近 C)退避越短,自然选出末跳应答;显式寻址因控制帧 52-bit 满、不带中继方呼号而排除。
6. **认证(dep⑥)拍板 = 开放信任 / CRC-8 only**:控制帧满 bit + HAM 生态(加密/签名受限、数字模式普遍不做)+ 对齐 V3.0.1 无防伪造现状 + upstream 把公钥校验列 Future-Work → 认证 PARK 到未来。
7. re-home 善后(B 追认 + 清垃圾 + amend `[loc][docs]`,`05d377ba`)。
8. 出 DRAFT v4 + 本 snapshot。

**6. 决策与依据**:

### 6.A 绑定 = C1 原子突发

| 项 | 内容 |
|---|---|
| 决策 | 中继消息整组原子收发(控制帧 + 内容帧一起);组内绑定 = JS8 原生 offset 归组(`m_messageBuffer[cd.offset]`,processDecodeEvent.cpp:439,v3.0.1↔master IDENTICAL)+ bit[8..10] 标签(5=控制/1=内容);msgID 专司跨跳去重 |
| 依据 | M2 下控制帧本就必须随内容过每跳(路由刚需),原子突发非额外成本;内容帧零新增 bit(中文容量全保);控制帧丢失被同一套 ARQ 兜 |
| 否决 C2 | 每帧 msgID 标签:M2 下裸内容帧无路由不可转发,C2 省帧好处买不到、却每帧付 bit → 净亏。退为逃生舱 PARK |

### 6.B M2 继续(否决挂起)

| 项 | 内容 |
|---|---|
| 决策(User 权威) | 不挂起多跳中继,继续完成 M2 |
| 依据 | "relay = store-and-forward 一跳跳串"——挂起省不下复杂度;且 Arch II 逐跳 store-forward 正是中继的实现机制,二者本就统一 |
| seq=8 影响 | M2 锁定继续有效,**不反转**;上轮"挂起/A′/B"记为 considered-then-declined |

### 6.C ACK/ARQ 回程 = Arch II 逐跳 store-and-forward + SNR 退避

| 项 | 内容 |
|---|---|
| 架构 | 逐跳 store-and-forward:每中继缓存(有界)已转内容、本地答下游 NACK、miss 升级上游;多跳 ARQ = 每跳 §4.4 串联,零新协议 |
| 响应者选择 | SNR 加权退避(听 C 越强退避越短→末跳先答)+ 听到即抑制 + msgID 去重;SNR 取自 decode(`cd.snr`) |
| 残留(接受) | 退避撞车偶发双重发,靠抑制+去重兜成有界冗余 —— 无定向广播介质固有税 |
| 依据 | JS8 慢速,端到端多跳重传延迟不可接受;逐跳本地恢复;对齐 V3.0.1 store-and-forward Inbox(`addCommandToStorage("STORE")`) |

### 6.D 认证 = 开放信任 / CRC-8 only(dep⑥)

| 项 | 内容 |
|---|---|
| 决策 | 本版不做防伪造,仅 CRC-8 防错码;开放信任模型 |
| 依据 | 控制帧 52/52 满,无 bit 放签名/MAC;HAM 加密/签名受限;对齐 V3.0.1(其中继亦无防伪造);upstream 公钥校验列 Future-Work → 认证随之 PARK |

**7. 未解决问题**(明确未决):

- 内容帧↔控制帧的跨突发绑定:**已被 C1 原子突发解决,无残留**(整组同发,offset 归组)。
- 真正未决 = 下列 PARK + PARK-024(四级审计原则,仍挂)+ dep⑥ 认证随 upstream 公钥校验的对齐(future)。
- 走 phase-2 前应补的只读验证(承 seq=6 §7,未变):buildMessageFrames 全体、processDecodeEvent 对 FrameUnknown 落地、itype 值空间、packDataMessage 精确位布局;以及 V10 的 phase-2 启动前 EXEC sanity check(含 bit[8..10]=5)。

**8. 风险与挂起项**:

- **【已否决,不重提】Route ③ 信封/隧道**(承 seq=7/8):永久技术否决。
- **PARK-022**:已决 + 执行已纠正(base=v3.0.1,工作线 `cn-dev`)。剩 `git remote add upstream` / sync 节奏 / master 孤立 `c9cfbd6e` 处理 / `cn-dev` 分支名 vs WORKFLOW §5.3 命名对齐 → phase-2 启动时做。
- **新增 PARK(编号自 026 起;025 已于 seq=7 作废;落盘请对照 `PARKING_LOT.md` 确认无碰撞)**:
  - PARK-026:中继缓存策略(大小 / 寿命 / 逐出)— phase-2 实施
  - PARK-027:target 呼号 16-bit 哈希函数选型 + 碰撞容忍 — phase-2
  - PARK-028:控制帧丢失硬化(现接受降级为"似末跳直发")— 触发 = 现场实测丢失频发
  - PARK-029:C1 逃生舱(每内容帧 msgID 标签)— 触发 = 现场 offset 碰撞 / 掉队帧频发
  - PARK-030:中继认证 / 防伪造(开放信任 now)— 触发 = upstream 公钥校验落地(撞 dep⑥)
- **PARK-023(Route ① 长线)/ PARK-024(四级审计,挂)/ PARK-010~021** 不变。
- **跟进动作**:`PARKING_LOT.md` 需更新(并入 026~030);DRAFT 升 v4(本 seq 出);HANDOFF 待 phase-1.6 收尾出 v3。

**9. 下一步候选**:

- 中继 spec 已收口 → **phase-1.6 接近收尾**。剩机械收尾:DRAFT v4 commit(`cn-dev`,`[loc][docs]`)+ `PARKING_LOT.md` 更新 + HANDOFF v3(phase-1.6 交接)。
- phase-2 启动条件不变:phase-1 草案最终批准 + V10 EXEC sanity check(含 =5)+ 语料字频(PARK-018)。

**10. 验证回执**:

```
本 seq 无新 EXEC（纯设计 + 上轮 re-home 善后）。
源码 anchor 续用 seq=8 §6.B（中继 @ v3.0.1）/ §10。
re-home 最终 commit = 05d377ba（cn-dev, 15 files, [loc][docs]）；原 8504667e 经 amend 取代（reflog 可溯）。
绑定依据 anchor：processDecodeEvent.cpp:439 m_messageBuffer[cd.offset]（v3.0.1↔master IDENTICAL）。
SNR 可得性：cd.snr（processCommandActivity " CQ" 分支 diff 中可见 cd.snr = d.snr）。
```

**11. 作废标记**:

- **作废**:seq=8 §4 "re-home 后 git 仅 seq6+seq8" —— B 已追认,全套 discipline 文档已 backfill(commit `05d377ba`,15 files)。
- **作废**:本 session 上轮"挂起 M2 / A′ 关 1.6 / B 仅信箱"提议 —— User 否决,M2 继续。
- **重申**:Route ③ 永久否决,不重提。

**关联修订**:本 seq 触发 **DRAFT v3→v4**:新增"§4.5 中继转发协议层(Route ②)"整章 + §4.2 登记 bit[8..10]=5 = 中继控制帧 + §8.2 更新(CN 中继从挂起→Route ② 已 spec)+ §9 注 V10 覆盖 bit[8..10]=5 + 附录 B 追加 v3.0.1 中继 anchor(seq=8 §6.B)+ 修订历史。HANDOFF 待 phase-1.6 收尾出 v3。**教训沉淀**(§4.7 精神):上轮把 `status` 检查与 `git add` 批在一个 EXEC、且 `add` 用整目录,导致误纳 —— 今后写操作 EXEC 不与前置检查批处理,`add` 用具体文件路径。
