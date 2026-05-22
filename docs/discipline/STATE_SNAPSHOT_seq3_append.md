
---

## seq=3

**1. 时间戳**:2026-05-21T<HH:MM:SS>+08:00  *(实际时间)*

**2. 序号**:3

**3. 当前阶段名**:phase-1 — JS8CALL-CN V3.0.1+ 中文支持设计 — 协议层定稿

**4. 状态摘要**:

- phase-1 协议层方案**正式定稿**,落盘 `docs/discipline/PHASE1_DESIGN_DRAFT.md`(273 行)
- §9 验证矩阵 9 条:V1/V2/V3/V4/V6/V7/V8/V9 + bit[53] trick **全部 PASS**;V5 信息性,V2 后无需验证,标"可省"
- 方案建立在 V3.0.1 **FrameType=001 (FrameCompound) 兼容路径**上,V3.0.1 用户无需升级
- 红线 #1(APRS-IS)三层防御 / 红线 #2(auto-response)三层防御 / 红线 #3 软化(实测残余 0)
- ILC codec 详细设计、中继、跨语言互通、应急词处理 — 全部挂起至 phase-2

**5. 本段动作**(实际执行清单):

1. V1 验证:`DecodedText.cpp:198-220` `tryUnpackCompound` FrameCompound 分支不填 `directed_`
2. cqs map 仍然 active 验证(排除其作为语言 ID 第二载体)
3. V3.0.1 HB unpack / display 路径地形 grep
4. V2 验证:`processRxActivity.cpp:92` `!d.isCompound` guard 阻断 m_rxCallQueue 入队 + 补遗 `processBufferedActivity.cpp` 全文 + `processRxActivity.cpp:1-110` 确认 PSK 残余 = 0
5. V3 验证:`Varicode.cpp:1157-1163` `unpackGrid(value > nbasegrid → return "")`
6. V4 验证:`spotAprsGrid` 内 `grid.length() < 4` 早退 + bit[53] trick 验证(`unpackCompoundMessage` extra > nmaxgrid 不 append grid/cmd)
7. V9 验证:`Varicode.h:51` `FrameCompound = 1`
8. V6 验证:`Varicode.cpp:1361-1413` `packHeartbeatMessage`,正常 HB `bits3` 恒 0
9. V7 验证:`Varicode.h:33-38` TransmissionType 位掩码 enum + `initializeDummyData.cpp:218` V3.0.1 自身用 `First|Last` 旁证
10. V8 验证:`Varicode.cpp:128, 1239-1242` `buffered_cmds` + `isCommandBuffered`,顺带发现 `isCommandAutoreply` 走同款 `directed_cmds.contains` guard → 红线 #2 第三层防御
11. PHASE1_DESIGN_DRAFT.md 4 次迭代:初稿 → 去 2.5.2 污染 → 用户审计修订 + 7 处污染审计修订 → V6/V7/V8 PASS + anchor 补丁定稿

**6. 决策与依据**:

| 决策 | 依据 |
|---|---|
| 不切 whitening polynomial / 不改 LDPC,改用 FrameType=001 兼容路径 | 借用 V3.0.1 已有"含 callsign 但非 directed"中性入口,改动小、上游可接受 |
| 红线 #3 软化(乱码 callsign 显示可接受) | User 显式确认 |
| HB bits3 映射 `0=EN, 1=CN, 2=JA, 3=KO`,限 isAlt=0 | V6 PASS:V3.0.1 正常 HB 永远写 bits3=0;CQ-style HB(isAlt=1)走 cqs |
| §6.2 双模式(严格档 / 信任档,UI 全局 + per-peer 可选) | User 显式要求:熟人快捷 + 陌生人安全两者并存 |
| Codec 详细设计 / 中继 / 跨语言 / 应急词 挂起到 phase-2 之后 | User 显式要求挂起,避免跑飞 |
| 上游 PR 2 次节奏(phase-2 实施完成 → 定向邀约测试 2 个月后) | User 显式要求 |
| PR 1 之前不主动接触上游 | 避免半成品干扰 review |
| 不修订 CHEATSHEET / DISCIPLINE / WORKFLOW | 本会话纪律工作良好,无暴露新结构性 bug |

**7. 未解决问题**(明确未决,与"下一步"区分):

- V5 (pskLogReport 无 callsign 格式校验)未单独验证,V2 后已无需,但形式上未划掉
- phase-2 codec 详细设计待启动(tier 大小、前缀码、内容分配、应急词处理、版本管理)
- 默认缓存档位(候选 30 分钟 或 24 小时)phase-2 时定稿
- 英文能力询问帧格式未定(候选:复用 `INFO?` cmd 扩展,或新增专用 cmd code)
- 中文 HAM 真实通联语料字频统计未开始
- 本会话末段 User 提议讨论"挂起项和没考虑到的细节",受 budget 限制可能转交下次会话(取决于本次落盘后剩余 budget)

**8. 风险与挂起项**:见 `docs/discipline/PARKING_LOT.md`(本会话后待更新项目:codec 详细设计 / 中继 / 跨语言 / 应急词处理 / 缓存档位默认值 / 能力询问帧格式)

**9. 下一步候选**:

- **若 budget 充足**:User 在本会话内讨论挂起项 / 未考虑细节,Claude 现场对接
- **若 budget 不足**:本会话先收,下次会话用 SESSION-OPEN manifest 加载 CHEATSHEET v1.1 + 本 snapshot seq=1/2/3 + PHASE1_DESIGN_DRAFT.md,继续设计细节优化
- **phase-2 启动条件**:phase-1 草案最终批准(本 snapshot 已 commit)+ User 显式宣告进入 phase-2

**10. 验证回执**:

本段验证主要走 paste-bridge,完整 stdout 见本次会话对话历史。关键 V# 验证结论及 V3.0.1 源码位置已全部 cite 进 `PHASE1_DESIGN_DRAFT.md` §9 验证矩阵的"V3.0.1 源码 anchor"字段 + 附录 B anchor 速查表,不在此重复。

关键事实摘要(完整证据见 PHASE1_DESIGN_DRAFT.md):

- `Varicode.cpp:214-216`:`nbasegrid=32400 / nusergrid=32410 / nmaxgrid=32767`
- `Varicode.cpp:128`:`buffered_cmds = {5,9,10,11,12,13,15,24}`(同 2.5.2)
- `Varicode.cpp:125`:`autoreply_cmds = {0,2,3,4,6,9,10,11,12,13,14,16,30}`(新发现)
- `Varicode.h:33-38`:TransmissionType 位掩码 enum,值 1/2/4 → 3=First|Last 合法组合
- `Varicode.h:51`:`FrameCompound = 1`(锁定 wire bit pattern 001)
- `Varicode.cpp:296-304`:hbs map 8 条全 "HB"(已知,本会话再确认)
- `Varicode.cpp:287-294`:cqs map 活的,8 条不同 CQ 字符串(已知,本会话再确认)
- `DecodedText.cpp:198-220`:`tryUnpackCompound` FrameCompound 分支不填 directed_
- `processRxActivity.cpp:92`:`!d.isCompound` guard 仍现役,阻断 spotting
- `processBufferedActivity.cpp` 全文:不调任何 logCallActivity / pskLogReport

**11. 作废标记**:不适用
