# GitHub 配置任务 — 交接纸条

> **生成**:2026-05-21,phase-1.5 会话末段
> **任务性质**:独立运维任务(跟 JS8CALL-CN 设计无关),单开会话执行
> **核心目的**:把 git 身份从 mrwenmin 切到 bg7ipb(双账号共存),把 JS8Call-CN 项目成果配置到 bg7ipb 的 GitHub 仓库,**确保后续重置电脑后能恢复**
> **重要**:本纸条本身就是"重置电脑后照着重做"的说明书,请保留

---

## 1. 已确认现状(本机诊断结果)

| 项 | 值 |
|---|---|
| 操作系统 | macOS,home = `/Users/wenyoungstar` |
| 连 GitHub 方式 | **SSH key**(`~/.ssh/id_ed25519` + `.pub`),当前挂在 **mrwenmin** 账号 |
| SSH key 数量 | 只有一对 ed25519 |
| GitHub CLI(gh) | **未安装** |
| git 全局署名 | name = `mrwenmin` / email = `mrwenmin@hotmail.com` |
| 项目 docs 位置 | `/Users/wenyoungstar/JS8Call-improved/docs/discipline` |
| `JS8Call-improved` 是否已 git init | **未知**(home 不是 git repo;子目录待新会话用 `cd` 进去后 `git status` 确认) |

## 2. 用户已拍板的决定

1. **账号**:bg7ipb 已注册;JS8Call 全部分支已 fork 到 bg7ipb(**public fork**)
2. **SSH 方案**:**路 B 双账号共存**(mrwenmin + bg7ipb 各一把 key + `~/.ssh/config` 用 Host alias 区分)
3. **新仓库范围**:整个 JS8Call-CN 项目(但见 §3 架构待澄清)
4. **提交署名**:换成 bg7ipb —— **建议用 per-repo 局部配置**(`git config --local`),不动 global 的 mrwenmin,避免影响这台机器上其他项目
5. **场景约束**:写代码前会重置电脑 → 配置要可恢复,工作成果必须先上云

## 3. 架构已定:选项 B(用户 2026-05-21 拍板)

**双仓库架构**:
- **代码**:继续用 bg7ipb 下已 fork 的 **public 仓库**(将来向上游 JS8Call 发 PR,符合"上游优先"工作模式)。本次配置**不改代码仓库内容**。
- **docs**:`docs/discipline` 单独放一个**新建 private repo**(内部纪律 + 决策文档不公开)。本次配置的 push 目标 = 这个 private docs repo。

**依据**:代码走 PR 路线必须 public;docs 是内部决策文档该 private;两者分开最干净,不破坏已 fork 的东西。

**架构 B 的落地细节(新会话开局先定)**:docs 当前物理上在 `JS8Call-improved/docs/discipline`,**嵌在代码项目目录树里**。要让它成为独立 private repo,有两种做法:
- **(B-1,推荐)**:把 `docs/discipline` 抽出来放到独立目录(如 `/Users/wenyoungstar/js8call-cn-docs`),单独 git init + 推到 private docs repo。简单直接。
- (B-2):git submodule / subtree 机制,更复杂,小白不推荐。

新会话开局让用户在 B-1 / B-2 间确认(默认 B-1)。

## 4. 安全红线(新会话 Claude 必须守)

- **不代劳创建账号**(已完成,bg7ipb 已注册)
- **SSH 私钥 / token 的值绝不进对话**;public key 由用户自己复制到 GitHub 网页
- **生成 SSH key、网页加 key、网页建 repo = 用户自己操作**,Claude 只给指令和说明
- **push origin = Tier 3 操作**,push 前 Claude 必须二次复述确认(做什么 / 推到哪个仓库 / 影响)
- **署名用 per-repo 配置**(`git config --local`),不动 global,避免污染 mrwenmin 的其他项目
- 所有 git 命令走 paste-bridge:Claude 写命令 → 用户在 zsh 跑 → 粘回完整 stdout
- git 命令默认加 `--no-pager`

## 5. 配置步骤大纲(新会话展开每步细节 + 小白解释)

**阶段一:SSH 双账号共存**
1. 为 bg7ipb 生成第二把 SSH key(ed25519,文件名区分如 `id_ed25519_bg7ipb`)—— 用户终端自己生成
2. 用户把 bg7ipb 的 public key 加到 bg7ipb GitHub 账号(网页 Settings → SSH keys)
3. 配 `~/.ssh/config`,用 Host alias 区分:
   - `Host github-mrwenmin` → 用旧 key
   - `Host github-bg7ipb` → 用新 key
4. 验证:`ssh -T git@github-bg7ipb` 应回 "Hi bg7ipb!"(旧的 `ssh -T git@github.com` 仍回 mrwenmin)

**阶段二:建 private docs repo + 抽出 docs(架构 B / B-1)**
5. 用户在 bg7ipb 网页建一个 private repo(如 `js8call-cn-docs`)
6. 把 `JS8Call-improved/docs/discipline` 抽到独立目录(如 `/Users/wenyoungstar/js8call-cn-docs`),`git init`
7. 配 remote 用 bg7ipb alias(`git@github-bg7ipb:bg7ipb/js8call-cn-docs.git`)
8. per-repo 署名:在 docs repo 里 `git config --local user.name bg7ipb` + `git config --local user.email <bg7ipb 邮箱>`(不动 global)
9. 把 phase-1.5 归档文件(HANDOFF v2 / DRAFT v3 / STATE_SNAPSHOT seq=4,5 / PARKING_LOT 更新 / CHEATSHEET)放进 docs repo,`git add` + commit(`[loc][docs]` 前缀)
10. **首次 push 到 private docs repo(Tier 3,二次确认后)**

**阶段二补充:代码 fork 的 bg7ipb 访问(本次可不做)**
- 用户已说"代码重置后能 re-clone,不急",故本次只做 docs;代码 fork 的本地 remote/署名配置留待重置后处理

**阶段三:验证重置恢复路径**
11. 记录重置电脑后的恢复步骤:重新生成/导入 SSH key + `git clone` 命令 + ssh config 重建

## 6. 新会话怎么开

1. 新建会话(这是运维任务,跟设计 Project 关系不大,独立会话即可)
2. 上传本纸条 `GITHUB_SETUP_HANDOFF.md`
3. 第一条消息:
   ```
   按这份 GitHub 配置纸条给我小白教程。架构已定 B(代码 public fork + docs 单独 private repo),开局先帮我确认 §3 的 B-1/B-2 落地细节(默认 B-1),然后一步一步带我做,每步告诉我为什么 + 跑完该看到什么。所有 git 命令我在终端跑完粘输出给你。
   ```

## 7. 待办附件清单(新会话需要的归档文件)

phase-1.5 归档文件已在上个会话生成,确保这些在 `JS8Call-improved/docs/discipline/`(或手边可上传):
- `HANDOFF.md`(v2)
- `PHASE1_DESIGN_DRAFT.md`(v3)
- `STATE_SNAPSHOT.md`(含 seq=1..5;或 seq4 + seq5 两个待并入文件)
- `CHEATSHEET.md`(v1.1)
- `PARKING_LOT.md`(待追加 PARK-019/020/021)

这些就是"现在的工作"要保护上云的核心内容。旧文件按用户意思先不管。
