## seq=7

**1. 时间戳**：2026-05-22T<HH:MM:SS>+08:00  *(本会话操作时间；User 提交前调整为精确时间)*

**2. 序号**：7

**3. 当前阶段名**：phase-1.6 续 — **Route ③「信封/隧道」可行性评估与永久否决**。本 seq 为 research/feasibility，**未写 CN spec**。

**4. 状态摘要**：

- **seq=6 维持不变**：Route ② 主推（phase-1 FrameCompound 自给自足 CN 中继）、Route ① 降为长线轻量 PR（上游"留 codec 位"，PARK-023）。
- **Route ③（信封/隧道，本 session 新提）经评估后永久否决、彻底关门**（效率不可接受，User 决定）。
- 下一步仍是 seq=6 §9 既定：进 Route ② 的 CN 中继和转发 spec，第一关 = dependency #2。

**5. 本段动作**（feasibility 讨论 + 只读核查；harness 已造未运行）：

1. **User 提 Route ③**：把中文编码成"只用 JS8 合法字符集（大写 ASCII+数字+符号+Latin-1）的 ASCII 信封串"，当普通英文消息走官方现有中继。合法字符集文本的"解码-重编码"无损往返 → **旧台也能转发 CN**（只当无意义英文转发）；中继路由复用现成 `>` ASCII 呼号路径；零上游改动。
2. **Claude 初评**：原理看好。关键 upside = **绕过 seq=6 决定性反证**（旧台不能中继 CN —— 因为信封是合法字符集的无损往返，非 CN 专用编码）+ **绕开 dependency #2**（路由不进我方帧，用现成 `>`）+ 零上游。主要代价 = 效率。
3. **linchpin 锁定**：每个中继跳 = codec「文本→帧→文本」一次往返 + 已知 `*DE* <呼号>` 注释；证 codec 对信封字符集逐字节无损往返即证隧道。
4. **只读核查（Phase 0 / 0.5）**：确认无 C++ 单测框架（media/tests/ 仅 .wav）；拿到 `packDataMessage`/`unpackDataMessage` 等精确签名、JSC 接口、CMake build 结构（见 §10）。产出测试 harness `linchpin_roundtrip.cpp`（逐字符探测安全字母表 + 整串测大小写/空白）。
5. **User 判定**：实测/落地工程量大 → 进而 **技术否决**（效率结构性劣化，弱信号下逐跳累加不可接受）→ **永久关门，不留条件门**。harness **未运行**。

**6. 决策与依据**：

**决策：Route ③ 信封/隧道 = 永久技术否决。** 三层如实标注（防未来 session 误用估算）：

| 层 | 内容 |
|---|---|
| 决策（User 权威） | 效率不可接受，永久否决，彻底关门 |
| 证据（标注真实级别） | 效率"2–5x、逐跳累加"= **Claude 推算，非实测**；linchpin（无损往返）harness **设计未运行** |
| 稳健性（为何否决不受估算误差影响） | 效率惩罚是**结构性**的：中文经"英文调优 codec + 失词典增益 + 双重编码"，变差**方向必然**（仅量级是估计）；弱信号模式下哪怕不大的惩罚，**逐跳累加**也足以否决 |

**对 seq=6 的影响**：无。Route ② 仍主推，Route ① 仍长线 PR，dependency #2 仍是下一关。

**7. 未解决问题**（继承 seq=6 §7，不变）：

- **Route ② 的核心待解 = dependency #2**：FrameCompound `bit[3..52]` 已满，没空间放 relay path + 原始发送方呼号。下一步 CN 中继 spec 第一关。
- **PARK-024**：四级审计原则框架定义待 User 补（仍挂起）。
- 走 spec 前应补读（seq=6 §7 列）：buildMessageFrames 全体、processDecodeEvent 对 FrameUnknown 落地、itype 值空间、packDataMessage 精确位布局。

**8. 风险与挂起项**：

- **【已否决，不要重提】Route ③ 信封/隧道**：把 CN 编码成 ASCII 信封过英文中继 —— **永久技术否决（效率不可接受）**。未来 session **不得再提**这条思路（它表面上很诱人：似乎一举解决旧台中继 + dependency #2 + 零上游；但效率代价对弱信号模式是 dealbreaker，已 User 拍死）。
- **作废上一轮口头给的"PARK-025 暂缓至下个大版本"框架** —— 那是误判（写成非技术否决），以本 §8 第 1 条为准。
- **资产**：`linchpin_roundtrip.cpp`（本 session 造，在 outputs，仅用已确认的 `Varicode::packDataMessage`/`unpackDataMessage`）。Route ③ 既永久否决，此 harness **仅作历史，无需进仓库**（User 可弃）。
- **PARK-022（已决）/ PARK-023（Route ① 长线 PR）/ PARK-024（挂起）/ PARK-010~021 全部不变。**

**9. 下一步候选**：

- **下次会话**：进 Route ② 的 CN 中继和转发 **spec 设计**，第一关 = dependency #2（载体位预算：候选 多帧分段 / 重审 ILC 容量 / 控制帧 bit[8..10] 保留区 5..7）。
- **commit 节点**：本 seq=7 可单独 commit，或与未 commit 的历史（seq=4/5/6 + DRAFT v3 + HANDOFF v2）一并提交。

**10. 验证回执**（本 session 建立的 v3.0.1 源码 anchor；后续回查）：

源码（`v3.0.1`，仓库 JS8Call-improved）：
```
JS8_Main/Varicode.h:188   static QString packDataMessage(QString const &text, int *n)
JS8_Main/Varicode.h:189   static QString unpackDataMessage(QString const &text)
JS8_Main/Varicode.h:192   static QString unpackFastDataMessage(QString const &text)
JS8_Main/Varicode.h:195   buildMessageFrames(QString const &mycall, QString const &mygrid, ...)
JS8_JSC/JSC.h             compress(QString)→QList<CodewordPair>；decompress(Codeword)→QString；
                          size=262144（2^18，印证 User Guide ~26万）；map/list/prefix 为 static const（无需运行时加载）；(C)2018 KN4CRD
JS8_JSC/                  源文件：JSC.cpp / JSC_checker.cpp / JSC_list.cpp / JSC_map.cpp
CMakeLists.txt:167        qt_add_executable(${TARGET} ...) —— 单一可执行目标，无独立 lib target
CMakeLists.txt:251-254    JSC 四源纳入主目标
CMakeLists.txt:304        JS8_Main/Varicode.cpp 纳入主目标
media/tests/              仅 .wav 解码素材 + README，无 C++ 单测框架（不可搭车）
```

**11. 作废标记**：

- **作废**：上一轮口头给的"PARK-025 Route ③ 暂缓至下个大版本"框架 —— 改为**永久技术否决**（B 已定）。若已贴入仓库，本 seq 取代之。
- **作废**：Route ③ 探索期 Claude "原理可行、值得实测"的倾向 —— 经 User 否决，效率结构性劣化使其不值得追，linchpin 不再跑。

**关联修订**：本 seq **不触发 DRAFT/HANDOFF 修订**（无 spec 变化；Route ② 仍是 seq=6 既定主推）。HANDOFF 待 phase-1.6 实际收尾（CN 中继 spec 出稿）时更新。
