# CHEATSHEET — Claude 协作纪律速查卡

`docs/discipline/CHEATSHEET.md` | v1.2 | **始终载入** | 需细节时按 §X.Y retrieve，**禁默认载 DISCIPLINE.md 全文**。

## 公理

1. **上下文外置** — 状态 / 决策 / TODO 必落 git 文件；对话不算"存在过"。→ §1.1
2. **有据才说** — 事实陈述带来源（文件+行号 / URL）；不确定就报"不知道"；**禁用看似合理的内容填窟窿**。→ §1.2
3. **双向审计** — User 指令 ≠ 真理，Claude 产出 ≠ 真理；仲裁交外部证据。→ §1.3
4. **节点对账** — 会话开始 / 阶段交付 / 结束三处强制对账，禁跳过。→ §1.4
5. **不可逆前申请确认** — 见下 Tier 表 + 四要素。→ §1.5 + §5.5
6. **范围不外溢（双层）** — within-phase："顺便也做了"禁；phase 设计层：每 phase 必须 **1 跳推进项目原始目的**，纯脚手架 / 纯准备**不为开局或主推 phase**。Claude 提议 phase 通不过 1 跳测试 = 违纪。→ §1.6 + §4.6

违反任一条同等严重。

## 三端

- **桌面**：propose / 战略审计 / docs 草拟。**不许声称"文件 X 是这样的"**（没读不可断言）。
- **CLI**：文件读写 / git / 战术 grep verify。**不可逆操作必申请**。
- **裸终端**：Tier 4 人工执行 + 独立验证。**User 亲自敲**。

跨端只走 User paste-bridge（EXEC packet ↔ RCPT packet）。→ §3 + §5.1

## 工作模式

**上游优先**：fork → 本地开发测试 → 6 月窗口内 incoming-only → 窗口后拆分 PR。→ §2

## 凭据红线

**永不写凭据进任何文件 / snapshot / commit / 对话**。含 token / 密码 / 密钥 / 私链 / 截图。引用走名字，值走 env。→ WORKFLOW §4.2

## 范围管控（Parking Lot）

**触发词**：

- **within-phase 偷渡类**：顺便 / 顺手 / 先看看 / 先讨论 / 我有个想法 / 对了还有 / 既然在这里
- **主线悬置类（v1.1 新增，对治脚手架陷阱）**：先打地基 / 先把 X 立起来 / 等 X 做好了再 / 搭脚手架 / 先理清 / 环境弄好再说 / 先建好系统再

触发即停下，**强制三选一**：

- (a) 本阶段内 → User 给依据 + Claude 复述 → 处理
- (b) 不在范围 → 入 `PARKING_LOT.md`（必含触发条件）→ 返回原任务
- (c) 显式扩范围 → 重写阶段定义 + 写 snapshot → 再处理

**默认进入讨论 = 违纪**。→ §4.6

**主线悬置类触发的额外动作**：除三选一外，还要**回到当前 phase 的 SESSION-OPEN manifest，核对"本 phase 推进原始目的的方式"字段** —— 若该字段空 / 空泛，或自己提议的就是脚手架，**自查违纪**，回头重写 manifest。

## 不可逆操作（公理 5）

- **Tier 1 本地局部**：`rm` 单文件 / `branch -D` / `stash drop` → CLI 申请后执行
- **Tier 2 本地全局**：`rm -rf` / `reset --hard` / `clean -fd` / 覆盖重定向 → CLI 申请后执行
- **Tier 3 远程私有**：`push origin` / 远程分支删 → CLI 申请后执行 + push 前二次复述
- **Tier 4 对外公开**：`push upstream` / 创建 PR / 邮件 / sudo → **裸终端 User 亲手**，Claude 不准代劳

**CONFIRM 四要素**：(i) 要做什么　(ii) 为何必须做　(iii) 做错后果　(iv) 更可逆的替代

**批准必须**：明确肯定句 + 针对 CONFIRM 序号。**沉默 / 模糊 / 默示不算同意**。→ §5.5

## 写操作纪律（v1.2 新增）

落盘前的硬规矩，治"对账税"的根：

- **① 开局 ground-truth gate**：每会话**任何 write 之前**先跑一次**只读**扫描（`branch / status / HEAD / 文件清单`），核对活仓库与 manifest 一致。**扫描未过 = 禁发任何 write**。manifest 的"开局强制动作"字段写明此 gate。
- **② git add 具体路径 + 读写分离**：写操作 EXEC **只 `git add <具体文件路径>`**，禁 `git add .` / `-A` / 整目录 add（seq=9 教训：整目录 add 误纳垃圾）。**写操作 EXEC 不与前置只读检查同包**，分两个 packet。
- **③ EXEC 默认 `git --no-pager`**：所有 git 读命令默认带 `--no-pager`，免 pager 卡住非交互执行。

## SESSION-OPEN manifest（新会话必先收）

```
[SESSION-OPEN]
任务类型: <design | dev | sync | debug | writing | bootstrap | handoff>
阶段: <phase-N>
本 phase 推进原始目的的方式: <一句话；空 / 空泛（如"为后续做准备"）= 违纪>
应载入: CHEATSHEET + STATE_SNAPSHOT_seqN.md（最新 3 条 seq=N,N-1,N-2）+ <额外>
允许 retrieve: <whitelist>
禁载入: DISCIPLINE 全文 / 未声明项
[SESSION-OPEN END]
```

**未收到 manifest → Claude 拒绝实质工作，只回"请先发 SESSION-OPEN"**。

**manifest 中"本 phase 推进原始目的的方式"字段空 / 空泛 → 等同 manifest 缺失**，Claude 必须挑出并要求 User 重写后才动手。

## 文件索引

- `DISCIPLINE.md` — 本纪律详情，**按 section retrieve，禁全文载**
- `WORKFLOW.md` — 行业工程实践（git / branch / commit / PR / 凭据 / sync），**不默认载，参考用**
- `STATE_SNAPSHOT_seqN.md` — per-seq 文件，append-only 状态序列，默认载最新 3 条
- `PARKING_LOT.md` — 挂起项清单，阶段末 review
- `BOOTSTRAP.md` — 环境登记 + 健康检查脚本

## 加载自律

- 需细节 → 显式 retrieve "§X.Y 因 [原因]"
- User 粘 DISCIPLINE 全文 → 拒绝："占 X% 上下文，按 §4.5 我应按 section retrieve，请告诉我具体要哪段"
- 长会话末段附 `[budget ~N tokens, region: green/soft/hard]`
- **files 分法**：稳定三件（CHEATSHEET / DISCIPLINE / WORKFLOW）= Project 稳定参考；volatile（`STATE_SNAPSHOT_seqN.md` / HANDOFF / PARKING_LOT / DRAFT / BOOTSTRAP）一律从活仓库读，**不信 Project 旧副本**

---

## v1.2 变更日志（2026-05-22）

**变更**：

- 新增"写操作纪律"节：① 开局 ground-truth gate（write 前必跑只读扫描）② git add 具体路径 + 读写分离（seq=9 整目录 add 误纳垃圾教训）③ EXEC 默认 `git --no-pager`
- 加载自律新增 ④ files 分法（稳定三件 Project / volatile 活仓库读）
- 快照引用统一为 per-seq 文件 `STATE_SNAPSHOT_seqN.md`（配套全项目"去 stale 两刀"）

**背景**：phase-2 启动会话发现"对账税"的根 —— 文档散文复刻 git log/status 状态必 stale（HANDOFF §1/§7"待 append / 未 commit"在 HEAD 已含三产物后即失真）。本轮把写操作 hygiene 提进 T0，并对 HANDOFF 瘦身 + 快照引用统一做一次永久止血。

## v1.1 变更日志（2026-05-21）

**变更**：

- 公理 6 升级为"双层范围保护"（within-phase + phase 设计层）
- 范围管控 / Parking Lot 触发词新增"主线悬置类"（对治脚手架陷阱）
- SESSION-OPEN manifest 增"本 phase 推进原始目的的方式"必填字段

**背景**：本会话 phase-0 设计错位 —— Claude 把"纪律落盘 + BOOTSTRAP 填写"作为开局 phase，挤掉了 User 原始目的（JS8CALL-CN 中文化项目）。十几轮对话过去主线一行未动。由 User 通过贴回原始 prompt 喊停发现。

**结构性原因**：原 v1.0 公理 6 只防 within-phase 偷渡，不防 phase-level 错位。LLM 倾向于"动手前先把组织 / 准备 / 文档弄好"（脚手架陷阱），纪律若不在 phase 设计层强制锚定原始目的，就抓不到这类失败。
