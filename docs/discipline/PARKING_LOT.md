# PARKING_LOT

`docs/discipline/PARKING_LOT.md` | append-only | 按 DISCIPLINE.md §4.6 维护

**字段要求**：每条至少含 — 时间戳与登记人、内容描述（具体）、来源（snapshot seq）、类型、**触发条件**（缺此 = 废条目）、优先级、关闭标记。

**重新激活协议**：满足触发条件 → 新阶段定义中显式纳入 → 本文件中标记"已激活，见阶段 N" → 写一条 snapshot 记录激活动作（§4.6 末段）。

---

## PARK-001：pre-commit hook 装配（凭据扫描）

- **时间戳 / 登记人**：2026-05-21 / Claude（桌面端）+ User 同意
- **内容描述**：装 pre-commit 框架，配置凭据扫描 hook，覆盖 WORKFLOW §4.2 推荐的常见凭据字面模式（`sk-` / `ghp_` / `xoxb-` / RSA private key block 等），作为本项目 push 前的安全护栏
- **来源**：snapshot seq=1
- **类型**：工程基建（safety net）
- **触发条件**：最迟在第一次准备 `git push` 到 `origin` 之前必装（预计 phase-1 末或 phase-2 初）
- **优先级**：中（早装早安心，但不卡 phase-1 推进）
- **关闭标记**：未关闭

---

## PARK-002：Apple Silicon（arm64）兼容矩阵第二行验证

- **时间戳 / 登记人**：2026-05-21 / Claude（桌面端）+ User 同意
- **内容描述**：User 后续会切到 Apple Silicon 开发机同步开发，届时需在 arm64 上跑完整 BOOTSTRAP 健康检查 + build，把 BOOTSTRAP §4 兼容矩阵第二行从"待验证"填实
- **来源**：snapshot seq=1
- **类型**：跨平台兼容验证
- **触发条件**：User 实际拿到 / 切到 Apple Silicon 机器，且 phase-1（干净构建）在 Intel 上已通过
- **优先级**：低（取决于切机时间，且不卡当前推进）
- **关闭标记**：未关闭

---

## PARK-003：`/opt/pmk/env/global/bin` PATH 来源待查

- **时间戳 / 登记人**：2026-05-21 / Claude（桌面端，触发于本机 PATH 探测）
- **内容描述**：本机 PATH 包含 `/opt/pmk/env/global/bin`，Claude 不认识 "pmk"，User 也不清楚来源。可能是某工作工具 / IDE 装的 / 残留 / 别的。对本项目无直接影响，但属于环境清晰原则的小缺口
- **来源**：snapshot seq=1（验证回执 B）
- **类型**：环境清晰度
- **触发条件**：phase-1 干净构建前可顺手查清（`ls /opt/pmk/` + `which` 看 PATH 内常用工具是否走 pmk）；或 User 主动想起来源时
- **优先级**：低
- **关闭标记**：未关闭

---

## PARK-004：JS8Call-improved 与原始 KN4CRD JS8Call 仓的回流关系

- **时间戳 / 登记人**：2026-05-21 / Claude（桌面端）+ User 同意
- **内容描述**：README 自述 JS8Call-improved 的目的是把改动推回原作者 KN4CRD，但实际：(a) KN4CRD 仓 `github.com/js8call/js8call` 的活跃度未知；(b) 是否接受外部贡献未知；(c) JS8Call-improved 历史改动有多少已实际回流未知。会影响本项目 6 月无回流窗口结束后的回流策略 — 到底回流到 improved 还是再尝试 KN4CRD，或两边都给
- **来源**：snapshot seq=1
- **类型**：战略 / 回流路径
- **触发条件**：6 月无回流窗口结束前（预计 phase-N 启动回流准备时），需要梳理战略；或 phase-1 完成后 User 主动决定时
- **优先级**：中（战略层，当前不卡进度）
- **关闭标记**：未关闭


---

## PARK-005：`whitening_processor.h` 实现细节（固定 vs 可配置）

- **时间戳 / 登记人**：2026-05-21 / Claude（桌面端，phase-1 决策时引出）
- **内容描述**：首选方案 G 依赖"切换 whitening polynomial"。需 view `JS8_Mode/whitening_processor.h` 及对应 .cpp（如有）确定：(a) 当前 whitening 是固定 polynomial 还是已支持多 seed？(b) 若固定，改成可配置工程量多大？(c) 若已可配置，接口如何使用？
- **来源**：snapshot seq=2
- **类型**：协议层技术验证
- **触发条件**：phase-1 第二轮（下次会话）开始时，验证首选方案可行性
- **优先级**：高（挡 design doc 落定）
- **关闭标记**：未关闭

---

## PARK-006：Decoder.cpp LDPC 失败路径行为验证

- **时间戳 / 登记人**：2026-05-21 / Claude（桌面端）
- **内容描述**：G 方案核心假设是"V3.0.1 收到 LDPC 失败帧 → 静默丢"。需 view `JS8_Mode/Decoder.cpp` 验证：(a) LDPC 失败真的 silent drop，还是有半解码 / 截断显示？(b) LDPC 失败的帧是否仍被记入"听到过这个频率有信号"类活动表（可能违红线 #3）？(c) LDPC 解码尝试次数 / 软判决阈值
- **来源**：snapshot seq=2
- **类型**：协议层技术验证
- **触发条件**：phase-1 第二轮开始时
- **优先级**：高（挡 design doc 落定；若假设破灭，G 方案需重审）
- **关闭标记**：未关闭

---

## PARK-007：新 submode 增加 vs 现有 submode 内切换

- **时间戳 / 登记人**：2026-05-21 / Claude（桌面端）
- **内容描述**：G 方案选项：(a) 新增 "Chinese" submode（与 Normal/Fast/Turbo/Slow 并列），用不同 whitening；(b) 在现有 submode 内增"语言"参数，whitening 随参数变。哪个对上游 merge 更友好？(a) 模块化强但帧选择 UI 要改；(b) 隐蔽但跨切换状态复杂
- **来源**：snapshot seq=2
- **类型**：设计 / 上游接受度
- **触发条件**：design doc 草案写到 "frame 调度路径" 章节时
- **优先级**：中
- **关闭标记**：未关闭

---

## PARK-008：中文 codebook 字符频率源选择

- **时间戳 / 登记人**：2026-05-21 / Claude（桌面端）
- **内容描述**：codebook 多层（8-64-512-4096）需排序字符。频率源候选：(a) 现代汉语语料（BCC / CCL / SUBTLEX-CH 等）；(b) GB2312 频率统计；(c) HAM 通联习语自建语料；(d) 混合（常用字 + HAM 高频词如 "73" / "QSO" / "CQ"）。选 (d) 最贴本场景但要做语料工程
- **来源**：snapshot seq=2
- **类型**：数据 / 工程
- **触发条件**：design doc 进入"codebook 构造方法论"章节时；不挡其他工作
- **优先级**：中
- **关闭标记**：未关闭

---

## PARK-009：whitening 变化在弱信号 -22dB 极限下的 SNR 容限验证

- **时间戳 / 登记人**：2026-05-21 / Claude（桌面端）
- **内容描述**：§3.3 #2 弱信号稳定性目标 = 英文版 80%。需数学 / 仿真验证：换 whitening polynomial **不会**显著降低 LDPC 抗错能力（理论上 whitening 是 invertible，LDPC 在线性域，不应影响 —— 但需严谨验证）。路径：(a) 数学推导；(b) 仿真（GNU Radio / Octave / Python 跑 -22dB AWGN 信道）
- **来源**：snapshot seq=2
- **类型**：数学验证 / 信号处理仿真
- **触发条件**：G 方案在 PARK-005 / PARK-006 验证通过后，作为最后一道质量门
- **优先级**：中-高（决定首选方案最终落地）
- **关闭标记**：未关闭
