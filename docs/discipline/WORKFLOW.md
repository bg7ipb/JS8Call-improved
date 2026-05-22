# WORKFLOW —— 工程流程实践参考

`docs/discipline/WORKFLOW.md` | v1.0 | **参考文档** | **不参与 Claude 默认载入** | User 需要查时打开

**定位说明**：本文件容纳"行业工程实践"（git / branch / commit / PR / 凭据隔离 / 上游同步等），与 Claude 协作纪律分离管理。这些内容属于 industry best practices，可参考既有 reference（如 Conventional Commits 规范、Git Flow 等），本文件给出当前项目的具体落地约定。

**与 DISCIPLINE.md 的关系**：

- DISCIPLINE.md 是 Claude 协作纪律的主线（公理 + 多端协同 + 上下文 + 状态管理 + scope）；
- WORKFLOW.md 是本项目工程实践（git / fork / PR 等）；
- DISCIPLINE.md §5.5 是不可逆操作框架；本文件 §5 是本项目的具体 Tier 映射。

---

## §4.2 凭据隔离

**用途**：snapshot 与文档在 git 公开仓库可见，任何凭据写入都是泄露。本节是 §4.1 SNAPSHOT 机制的安全护栏。

### 禁入清单

永不进入 repo / docs / snapshot / commit message：

- API token、密码、私钥、SSH key 内容；
- 数据库连接串（含密码部分）；
- 内部网络链接（含未公开的内网地址、内部文档链接）；
- 截图中的 token / cookie / session id；
- 任何可被 GitHub secret scanning 检测到的凭据模式。

### 正确引用方式

- snapshot 与文档里**只写凭据名字与用途**，不写值：例如"需要 `GITHUB_TOKEN` 才能 push"，不写真实 token 串；
- 凭据本身存放：环境变量（`.env`，加入 `.gitignore`）、系统 keychain、密码管理器；
- 跨设备同步凭据：**走密码管理器，不走 git**。

### 预防机制（强烈建议）

- 配置 pre-commit hook，扫描 commit 内容的常见凭据字面模式（`sk-`、`ghp_`、`xoxb-`、RSA private key block 等）；
- 提交前用 `git diff --staged` 自审，不许"反正没敏感信息"就跳过。

### Claude 端禁令

- **桌面端**：草拟 snapshot 或文档时，凡涉及凭据**只能写引用名**；即便 User 在对话中粘贴了真实值，桌面端也不许将其写入文档草稿；
- **CLI 端**：检测到 staged 内容含可疑凭据模式时，**必须停下报告**，禁止提交。

### 事故处理（如已泄露）

1. 立刻轮换凭据（旋转 token、改密码）；
2. 用 `git filter-repo` 或 BFG 清除 git 历史；
3. force push（少数允许 `--force` 的场景，**必须走公理 5 + 本文件 §5 Tier 4 申请**）；
4. 写一条 snapshot 记录事故与处理过程（不写凭据本身）。

---

## §5.2 上游同步协议

**用途**：在 6 个月无回流窗口内，保持本地 fork 持续吸纳上游变更；窗口结束后切换到回流准备模式。

### 两阶段模式

- **窗口内（incoming-only）**：方向为 upstream → local，单向；目标是始终保持"随时可回流"的质量基线；
- **窗口后（outgoing 准备）**：方向为 local → upstream；一次性整理 + 持续维护。

### 触发条件（混合策略，任一命中即触发评估）

- **时间**：每周固定一次同步评估；
- **事件**：上游发布新 tag / release，24h 内触发；
- **量**：上游 main 累计 >20 commits 未同步时；
- **依赖**：上游引入新依赖 / 改 build 系统 / 改架构相关代码时**立即触发**；
- **手动**：User 主动判断需要时。

**评估 ≠ 必同步**。触发后先 `git fetch` + 看 changelog，再决定是否同步：

- 上游引入破坏性变更 → 评估对本地的影响，可能推迟；
- 上游变更与本阶段范围严重冲突 → 进 PARKING_LOT；
- 改动微不足道 → 跳过。

**跳过同步必须在 snapshot 中记录跳过原因**。

### 同步执行流程

**前置**：当前阶段已声明允许做同步（或本阶段就是同步阶段）。

**评估阶段**：

1. `git fetch upstream`（未配置过 upstream remote 的，先 `git remote add upstream <url>`）；
2. `git log upstream/main --oneline ^upstream-sync`：查看新增 commits；
3. 阅读上游 CHANGELOG / release notes / breaking changes；
4. 比对工具链与依赖变化（见 BOOTSTRAP §2）；
5. 用 `git merge --no-commit --no-ff upstream/main` 试探冲突点，然后 `--abort`；
6. **决策**：同步 / 推迟 / 进 PARKING_LOT，写 snapshot。

**若决定同步**：

7. 更新 upstream-sync 分支：`git checkout upstream-sync && git merge --ff-only upstream/main`（**只允许 fast-forward**）；
8. 同步进 local 主线：在 main 分支上 `git merge upstream-sync`（或 rebase）；
9. 冲突解决（见下节）；
10. 重跑健康检查 + 兼容矩阵（BOOTSTRAP §3 / §4）；
11. 更新 BOOTSTRAP（若工具链 / 依赖变化）；
12. commit + push（push 走 §5 Tier 3 申请）；
13. 写 snapshot：记录同步动作 + 结果 + 健康检查输出。

### 冲突处理

**禁止 Claude 端自行解决冲突**。

**标准动作**：

1. CLI 检测到冲突 → 立即 `git merge --abort`（或 `git rebase --abort`）；
2. CLI 在 RCPT packet 中列出冲突文件 + 冲突点摘要 + `git diff` 原文；
3. Desktop 评估：
   - **小且明确**：冲突点在阶段范围内，桌面端给出冲突解决方案（EXEC packet），CLI 按方案手动解决；
   - **大或不明确**：进 PARKING_LOT，等下个评估子阶段；
   - **跨阶段范围**：必须显式开新阶段处理（公理 6）；
4. 不许 Claude 任一端用"应该这样合并吧"的猜测解决冲突。

### 工具链 / 依赖变化的处理

若上游同步引入工具链或依赖版本变化：

- 走 DISCIPLINE §4.4 "项目工具链版本绑定" + "兼容性实测判定流程"；
- **跨主版本** → 拒绝同步本次，进 PARKING_LOT 开专项升级阶段；
- **小版本超出** → 走实测判定，通过则记录，不通过则推迟。

### 窗口后回流准备

6 个月窗口结束后启动。**前置**：本地 main 已通过测试发布且稳定；上游仍接受外部贡献。

**步骤**：

1. 同步上游到最新（一次彻底同步）；
2. 走本文件 §5.4 PR 准备纪律，拆分本地改动为多个 PR-ready 分支；
3. 与上游 maintainer 协商节奏（**一次给一个 PR**，等 review 完再给下一个）；
4. 跟随 reviewer 反馈迭代；
5. 维护本地 fork 的同步。

---

## §5.3 分支与改动分类纪律

**用途**：让"拟上行"与"仅本地"两类改动从第一天起就物理隔离，避免回流时 diff 爆炸。

### 分支模型

- **`upstream-sync`**：永远反映 upstream/main 的**纯净镜像**。仅 `git merge --ff-only upstream/main` 更新，**禁止直接 commit**。
- **`main`**（或上游惯用名 `master` / `dev` / `trunk` 等）：本地集成主线。
- **`feature/<topic>`**：单 feature 工作分支，从 main 切出，完成后合回 main。
- **`pr-prep/<topic>`**：回流准备分支，窗口结束后从 upstream-sync 切出，cherry-pick `[ups]` commits 上去。

**沿用上游分支命名**：若上游用 `master` / `develop` 等惯例，本地的"main"对应名**沿用上游名字**，不擅自改名。

### 改动分类：拟上行 vs 仅本地

每个 commit **必须**显式标记类型，**前缀放在 commit subject 开头**：

- **`[ups]`**：拟上行（destined for upstream）。符合上游质量基准、目录约定、风格规范；
- **`[loc]`**：仅本地（downstream-only）。专为本地需求，不打算回流。

**禁止混合**：一个 commit 不许同时含 `[ups]` 与 `[loc]` 改动。需两类变更则**拆成两个 commits**。

**子类型**（可选，沿用 Conventional Commits 习惯）：`[ups][feat]` / `[ups][fix]` / `[ups][refactor]` / `[ups][docs]` / `[loc][config]` / `[loc][branding]` 等。

### 目录隔离

**拟上行代码**：放在上游已有的自然目录，**严格遵循上游目录结构与命名习惯**。禁止整理性重构。

**仅本地代码**：选择优先级：

1. **优先**：沿用上游已有的扩展位置 —— `contrib/`、`plugins/`、`extensions/`、`addons/` 等；
2. **次选**：在适当层级新增 `downstream/` 子目录 —— `src/downstream/`、`lib/downstream/`、`docs/downstream/`；
3. **避免**：跨上游目录散布下游代码。

**强制纪律**：

- 修改上游已有源文件：默认视为 `[ups]`，除非显式标注 `[loc]` 并说明为何不可回流；
- 新增文件：放在上游目录 → `[ups]`；放在 `downstream/` 或扩展目录 → `[loc]`；
- **拟上行代码禁止 import 仅本地代码**，否则回流时缺依赖。

### 历史可追溯

定期（建议每周或每阶段结束）跑统计：

```bash
git log --since="<phase start>" --pretty=format:"%h %s" | grep -E '^\w+ \[ups\]'
git log --since="<phase start>" --pretty=format:"%h %s" | grep -E '^\w+ \[loc\]'
```

把统计结果进 snapshot，掌握当前累计的待回流 commits 量。

---

## §5.4 提交消息与 PR 准备纪律

**用途**：让每个 commit 在 6 个月窗口后仍能作为可独立提交的 PR 候选，不需要回头重写历史。

**语言约定**：拟上行的 commit message 与 PR 描述**用英文**（面向国际 reviewer）；仅本地的可用中文或英文，建议团队统一选其一。

### Commit message 结构

```
[ups|loc][type] <subject ≤72 chars>

<body, wrapped at 72 chars per line>

<footer>
```

**字段说明**：

- **前缀**：`[ups]` / `[loc]` + 可选子类型，见 §5.3；
- **subject**：imperative mood（"Add support for X"，不写 "Added"），≤72 chars，不带句号；
- **body**：说明 (i) what 改了什么 (ii) why 为何要改 (iii) how 关键设计选择 (iv) impact 兼容性/性能影响 (v) testing 如何验证；
- **footer**：关联 issue、breaking change 标记、Co-authored-by 等。

**示例（拟上行）**：

```
[ups][fix] Handle null config in TaskDispatcher init

The previous initialization assumed config.scheduler was always
present, causing NPE when the optional scheduler field was omitted
from user-provided configs.

This patch:
- adds a null check before accessing config.scheduler
- falls back to DEFAULT_SCHEDULER_CONFIG when absent
- emits a warning log at startup if fallback was used

Impact: backward compatible.

Testing: added unit test test_dispatcher_null_scheduler_config; ran
upstream test suite (all 247 tests pass).

Refs: upstream#1234
```

**示例（仅本地）**：

```
[loc][config] Bump max-worker-count to 32 for local prod env

Default upstream value is 8, which underutilizes our 32-core prod
hosts. This is environment-specific and not suitable for upstream.

Impact: only affects deployments using local config preset.

Testing: smoke tested in staging cluster (3x 32-core nodes).
```

### 内容五要素

每个 commit 必须能回答：

1. **What**：改了什么（具体的文件 / 函数 / 行为变化）；
2. **Why**：为何要改（问题背景 / 需求来源）；
3. **How**：关键实现选择（若非显然）；
4. **Impact**：兼容性、性能、行为变化；
5. **Testing**：如何验证（单元 / 集成 / 手动）。

### 禁止的 commit 风格

- "Fix bug" / "Update code" / "WIP" 类无信息 subject；
- "Minor changes" 但 diff 跨多文件且语义复杂；
- subject 详细但 body 完全为空（trivial 改动如 typo 例外）；
- 凑数 commit；
- 多个无关改动塞进一个 commit；
- 暴露凭据的 commit（违反 §4.2）。

### Squash 与 rebase 策略

**窗口内（development）**：

- feature 分支内可自由 rebase / squash 整理；
- 合并到 main 时：`git merge --no-ff` 保留分支记号，或 squash merge；
- **main 上的 commit 一旦 push，禁止 rewrite history**。

**窗口后（回流准备）**：

- 从 main 上把 `[ups]` commits cherry-pick 到 `pr-prep/<topic>` 分支；
- 在 pr-prep 分支上 rebase / squash 整理；
- commit message **再过一遍**，必要时改写以适合上游 reviewer 阅读。

### PR 准备清单

**前置检查**：

- PR 候选分支已 rebase 到 upstream/main 最新；
- 跑上游测试套件，全过；
- 跑上游 lint / format / 类型检查，全过；
- commit history 清晰，每个 commit 自洽；
- 无凭据、无 `[loc]` commits 串入；
- commit message 已按 PR-ready 标准更新。

**PR 描述模板**：

```
## Context
<问题背景, 上游 maintainer 不一定熟悉, 多介绍一些>

## Motivation
<为何这是上游应该接受的改动>

## Approach
<关键设计选择>

## Alternatives Considered
<评估过但未采用的方案>

## Testing
<手动测试 + 自动化测试>

## Breaking Changes
<是否有破坏性变更, 影响哪些下游>

## Checklist
- [ ] Rebased on latest upstream/main
- [ ] Tests pass locally
- [ ] Lint / format / type-check pass
- [ ] commit messages follow upstream conventions
- [ ] Documentation updated if needed
```

### 回流节奏

**默认拆分 PR**：一次一个 PR，等 reviewer 反馈完成再交下一个。

**禁止 mega-PR**：一次性 push 所有改动几乎必死。

**例外**（允许稍大的 PR）：

- 多个改动是同一逻辑特性的不可分割部分；
- 上游 maintainer 显式要求合并提交；
- 安全修复需一次性给出。

例外必须在 PR 描述中解释为何不拆。

---

## §5 不可逆操作 Tier 映射（本项目具体）

**说明**：DISCIPLINE §5.5 定义了 Tier 1-4 框架与 CONFIRM 四要素；本节给出本项目 fork-based / 开源 workflow 下的具体 Tier 映射。

| 等级 | 本项目具体动作 | 执行人 |
|---|---|---|
| **Tier 1** 本地局部 | `rm` 单文件 / `git branch -D feature/x` / `git stash drop` / `git checkout` 覆盖未暂存改动 | CLI（CONFIRM 后执行） |
| **Tier 2** 本地全局 | `rm -rf` / `git reset --hard` / `git clean -fd` / `> file` 覆盖重定向 / 删工作目录 | CLI（CONFIRM 后执行） |
| **Tier 3** 远程私有 | `git push origin <任何分支>` / `git push --force origin` / 远程分支 / tag 删除 / origin 仓库设置变更 | CLI（CONFIRM 后执行 + push 前再复述） |
| **Tier 4** 对外公开 | `git push upstream` / 创建上游 PR / 关闭 / 合并上游 PR / 上游 Issue 操作 / 邮件发送 / `sudo` 系统级 / 删生产数据 | **Bare Terminal，User 亲手**，Claude 不准代劳 |

### 特殊场景

**推到 origin/main**：标准 Tier 3 但**加强复述**（本地主线易被外部 fetch 复制）。

**推到 origin/upstream-sync**：Tier 3（镜像分支，偶有 force-update 风险）。

**推到任何别人也在用的共享分支**：升级到 Tier 4（即便仍在 origin，也涉及他人）。

**Tier 4 流程回顾**：

1. CLI 写出 CONFIRM 申请（含完整命令）；
2. User 评估，给出文字确认；
3. **User 切换到 Bare Terminal**，执行命令；
4. User 把命令 + 输出**原文**粘贴回 Claude（作为 RCPT 的一部分）；
5. CLI / Desktop 端基于实际输出更新 snapshot。
