# BOOTSTRAP

`docs/discipline/BOOTSTRAP.md` | 环境登记 + 健康检查脚本 + 兼容矩阵

**用途**：§4.4 环境与依赖核查协议的物理载体。本文件是项目专属的，每个 fork 项目独立维护。

**规则**：

- 凭据严禁写入（见 WORKFLOW §4.2）；
- 工具链版本声明以上游 build 系统可识别的配置文件为权威来源（见 §4.4）；
- 跨架构 / 跨机切换前后必须更新；
- 健康检查跑过后将原文输出录入下一条 snapshot 的验证回执字段。

---

## 1. 环境登记

| 字段 | 值 |
|---|---|
| OS + 版本 | <e.g., macOS 14.5 / Ubuntu 22.04> |
| CPU 架构 | <x86_64 / arm64>（`uname -m` 查） |
| Shell | <bash 5.x / zsh 5.x> |
| 包管理器 | <brew / apt / pip / npm / cargo / ...> |
| 当前设备 ID | <自定义, 用于跨机区分> |
| 最后更新 | <YYYY-MM-DD> |

---

## 2. 项目工具链版本声明

权威来源优先级：(1) build 系统可识别配置文件（`.python-version` / `requires-python` / `engines` / `CMakeLists.txt`...）（2) 上游官方文档 (3) 上游 CI 配置。

| 工具链 | 上游声明 | 本地实测版本 | 状态 | 备注 |
|---|---|---|---|---|
| 语言: <e.g., Python> | <e.g., >=3.10,<3.12> | <e.g., 3.10.13> | ✓ 范围内 | - |
| 框架: <e.g., Qt> | <e.g., 5.15.x> | <e.g., 5.15.10> | ✓ | - |
| build 工具: <e.g., CMake> | <e.g., >=3.20> | <e.g., 3.27> | ✓ | - |

**实测判定通过的小版本超出**（按 §4.4 流程通过的）：

| 工具链 | 上游声明 | 本地版本 | 验证 snapshot seq | 备注 |
|---|---|---|---|---|
| - | - | - | - | - |

---

## 3. 健康检查脚本 / 指令清单

**目的**：用于验证环境就绪 + 检测兼容性变化。每次 bootstrap / 同步 / 跨架构切换后必跑，**输出原文录入 snapshot**。

```bash
# 示例（请按项目实际填充）
# Step 1: 关键依赖能 import / require
# python -c "import <pkg1>, <pkg2>; print('imports OK')"

# Step 2: 关键命令能跑
# <lang> --version

# Step 3: 项目能 build
# <build command>

# Step 4: smoke test
# <test command>
```

**预期成功标志**：所有步骤退出码 0；关键输出符合预期模式。

---

## 4. 跨平台兼容矩阵

| OS / Arch | 状态 | 最近验证 | snapshot seq | 已知问题 / Workaround |
|---|---|---|---|---|
| macOS arm64 | 待验证 | - | - | - |
| macOS x86_64 | 待验证 | - | - | - |
| Linux x86_64 | 待验证 | - | - | - |
| Linux arm64 | 待验证 | - | - | - |

---

## 5. 架构特定陷阱

**Mac Intel ↔ Apple Silicon**：
- `uname -m` 必查
- Homebrew 路径动态取：`brew --prefix`（不硬写 `/usr/local` 或 `/opt/homebrew`）
- Docker 镜像区分 `linux/amd64` vs `linux/arm64`
- Rosetta 2 转译能跑但不可信，仅过渡用
- 某些 Python wheel 仅 x86_64 可用，arm64 需源码编译

**Linux 发行版差异**：
- 包名不同（apt / dnf / apk）
- glibc 版本差异可能导致预编译二进制不兼容

**容器内外**：
- 容器 user / permission / network 与宿主机不同
- 容器架构与宿主架构可能不一致

---

## 6. 新机首次接手清单

1. clone 仓库到新机
2. 走第 1 节填写本机环境登记
3. 走第 2 节核对工具链版本（走 §4.4 实测判定流程）
4. 跑第 3 节健康检查脚本，输出录入下一条 snapshot 验证回执
5. 更新第 4 节兼容矩阵（含验证时间 + snapshot seq）
6. 全部通过后才进入恢复协议（§4.3）

---

## 7. 凭据管理（不写入此文件）

参考 WORKFLOW §4.2：

- 任何 token / 密码 / 密钥 / 私链不进 git；
- 走环境变量 + 本地 `.env`（加入 `.gitignore`）+ 密码管理器；
- 本文件**只引用凭据的名字与用途**（如"需要 `GITHUB_TOKEN` 才能 push"）。
