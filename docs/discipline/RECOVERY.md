# 重置电脑后恢复清单 — GitHub 双账号 + JS8Call-CN docs

> 环境:macOS,本机用户 `wenyoungstar`  ·  GitHub:mrwenmin(旧)+ bg7ipb(新)
> 最后更新:2026-05-21
>
> ⚠️ **把这份清单在「电脑之外」也存一份**(密码管理器 / 个人云 / 发给自己邮箱)。
> docs 仓库是 private,重置后必须先重建 SSH 才能 clone;若这份说明只躺在仓库里,
> 重置后你将够不着它。

---

## 0. 现状速查:东西都在哪

| 内容 | 位置 | 重置后处理 |
|---|---|---|
| 代码 | bg7ipb 名下的 **public fork**(JS8Call 相关) | re-clone 即可,不急 |
| 文档(本套纪律/设计) | **private** 仓库 `bg7ipb/js8call-cn-docs`,分支 `main` | 见 §2 |
| 旧账号 | mrwenmin(`mrwenmin@hotmail.com`) | global 署名仍归它 |
| 新账号 | bg7ipb(`bg7ipb@hotmail.com`) | 仅在 docs 仓库做 per-repo 署名 |

两个账号各一把 SSH key,用 `~/.ssh/config` 的 Host 别名区分。

---

## 1. 重建 SSH 双账号

> 私钥随硬盘一起没了。**不要备份/搬运私钥**,直接重新生成、把新公钥加回 GitHub 最干净。

### 1.1 生成两把 key
```bash
ssh-keygen -t ed25519 -C "mrwenmin-macbook" -f ~/.ssh/id_ed25519
ssh-keygen -t ed25519 -C "bg7ipb-macbook"  -f ~/.ssh/id_ed25519_bg7ipb
```
(passphrase 留空或自设;自设的话记进密码管理器。)

### 1.2 把公钥加回对应账号
GitHub 网页 → 头像 → Settings → SSH and GPG keys → New SSH key。**注意别加错账号**:
```bash
cat ~/.ssh/id_ed25519.pub          # 这把 → 加到 mrwenmin 账号
cat ~/.ssh/id_ed25519_bg7ipb.pub   # 这把 → 加到 bg7ipb 账号
```
顺手把两个账号里失效的旧 key 删掉。

### 1.3 重建 ~/.ssh/config
```bash
cat > ~/.ssh/config << 'EOF'
# 默认 / mrwenmin(旧账号)—— 直接连 github.com 走这把
Host github.com github-mrwenmin
  HostName github.com
  User git
  IdentityFile ~/.ssh/id_ed25519
  IdentitiesOnly yes

# bg7ipb(新账号)—— 用 github-bg7ipb 别名连,走新钥匙
Host github-bg7ipb
  HostName github.com
  User git
  IdentityFile ~/.ssh/id_ed25519_bg7ipb
  IdentitiesOnly yes
EOF
chmod 600 ~/.ssh/config
```

### 1.4 验证
```bash
ssh -T git@github.com        # 应回:Hi mrwenmin!
ssh -T git@github-bg7ipb     # 应回:Hi bg7ipb!
```
（"does not provide shell access" 是正常成功提示,不是报错。）

---

## 2. 取回 docs 仓库

### 2.1 clone(用 bg7ipb 别名)
```bash
git clone git@github-bg7ipb:bg7ipb/js8call-cn-docs.git ~/js8call-cn-docs
cd ~/js8call-cn-docs
```

### 2.2 重设 per-repo 署名(关键!clone 不会带回 --local 配置)
```bash
git config --local user.name  bg7ipb
git config --local user.email bg7ipb@hotmail.com
```
> 邮箱不要加引号,直接写,避免 macOS 智能标点把直引号变成弯引号。

### 2.3 验证
```bash
git config --local --list                       # user.name/email 应是 bg7ipb 的
git --no-pager log --oneline -5                  # 应看到历史提交
git --no-pager show -s --format='Author: %an <%ae>'   # 作者应为 bg7ipb <bg7ipb@hotmail.com>
```

---

## 3.(需要时)取回代码 fork
```bash
git clone git@github-bg7ipb:bg7ipb/<你的-fork-仓库名>.git ~/JS8Call-improved
```
代码走 PR 路线时记得配 upstream;本地署名按需用 `git config --local` 单独设(同 §2.2)。

---

## 4. 日常推送 docs
```bash
cd ~/js8call-cn-docs
git add .
git commit -m "[loc][docs] 说明"
git push
```

---

## 备忘 / 待办
- `PARKING_LOT.md` 当前不在 docs 仓库里,定位到后补入并 push。
- `.DS_Store` 已被 `.gitignore` 屏蔽,无需处理。
- 原 `JS8Call-improved/docs/discipline/` 在搬出后只剩一个 `.DS_Store`,可留可清。
