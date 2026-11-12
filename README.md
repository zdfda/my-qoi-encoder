
点绿色按钮 `Commit changes`。

**完成。** 刷新仓库页就能看到 README 渲染出来了。

---

### ⚠️ 文件大小提示

`4.bmp`（35MB）、`11.bmp`（42MB）、`12.bmp`（1.76MB）如果都上传，仓库会很大。**建议只上传 `12.bmp`（最小）作为测试样例**，`4.bmp` 和 `11.bmp` 在 README 里用文字描述就行。或者：

- 把大图先压缩成 `.zip` 再上传；
- 或者干脆不上传大图，只上传代码 + README + `1.png` + `12.bmp`。

---

### 方法二：用 git 命令（如果以后想更新代码）

```bash
cd ~/qoi

# 初始化
git init

# 添加文件（注意：大图可选）
git add README.md my_qoi_pass.c my_qoi_pull.c 1.png 12.bmp

# 提交
git commit -m "feat: ZCJ1 lossless BMP compression in C99"

# 关联远程仓库（换成你的用户名）
git remote add origin https://github.com/你的用户名/ZCJ1-bmp-compression.git

# 推送
git branch -M main
git push -u origin main
