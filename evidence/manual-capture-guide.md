# Lab0 手绘与截图操作指南

实验说明书要求三张图由本人理解并亲手绘制。数字图已覆盖验收点，但最终证据必须补上本人手绘照片。

## 一 手绘三张图

准备三张 A4 横向白纸，黑色笔画主流程，蓝色笔写栈和特权级，红色笔写锁和个人思考。每张纸右下角手写：

```text
杜浩源 2024302111427 Lab0
```

不要逐像素临摹。先读 `notes/lab0-reading-notes.md`，再参考三张 PNG，用自己的话重画并能口述箭头。

### 图一检查项

- 从 Shell 的 `read` 阻塞开始，画出 UART 中断唤醒。
- 覆盖 `fork -> exec -> write -> exit -> wait 回收`。
- 在关键节点旁写 U/S 态、用户栈/进程内核栈/调度器栈。
- 标出 `cons.lock`、`p->lock`、`wait_lock`，以及 UART 的睡眠锁。
- 至少写一条自己的疑问或边界思考。

### 图二检查项

- 明确截面是 `exec` 已提交新页表、echo 尚未执行首条用户指令。
- 画 init、shell、echo 的 state、parent、pagetable、sz、ofile。
- 画三级页表索引与六个关键虚拟页，写出 PTE 权限。
- 画 `ofile -> struct file -> inode -> devsw[1] -> consolewrite`。
- 至少写一条自己的疑问或边界思考。

### 图三检查项

- 从 `scause=0x8000000000000005` 开始。
- 覆盖硬件 CSR、`uservec` 保存现场、切内核栈/页表、`usertrap`、`clockintr`、`yield`、`swtch`、scheduler、`prepare_return`、`userret`、`sret`。
- 标清 `tickslock` 与 `p->lock` 的获取和释放。
- 至少写一条自己的疑问或边界思考。

## 二 拍摄手绘证据

1. 把纸平放在光线均匀的桌面，避免灯光反射。
2. 手机相机切换到最高分辨率，镜头与纸面保持平行，让四个纸角都进入画面。
3. 每张图拍一张完整照片；若文字仍不清晰，再为密集区域各拍一张近照。
4. 检查放大后函数名、权限位、箭头和姓名学号都能读清。
5. 按以下名称放入仓库：

```text
evidence/handdrawn/01-echo-control-flow.jpg
evidence/handdrawn/02-exec-state-snapshot.jpg
evidence/handdrawn/03-timer-interrupt-journey.jpg
```

## 三 获取环境自检截图

1. 打开 Ubuntu（WSL2）终端。
2. 执行：

```bash
cd ~/os实践A-杜浩源-2024302111427
python3 tools/preflight.py
```

3. 将终端窗口拉高，保证从标题 `== osLab 开课前环境自检 ==` 到最后一行 `结论:环境自检通过` 全部同时可见。
4. 按 `Win + Shift + S`，选择“矩形截图”，框住终端标题栏、命令和完整输出。
5. 保存为 `evidence/screenshots/01-preflight.png`。截图必须显示命令和完整输出，不要只截最后一行。

## 四 获取 Git 与标签截图

在同一终端执行：

```bash
git log --oneline --decorate --graph --all
git status
git tag --list
git remote -v
```

将命令和结果完整截入一张图，保存为 `evidence/screenshots/02-git-proof.png`。应能看到 `lab0` 指向本次提交、工作区 clean，以及 GitHub 的 origin 地址。

## 五 获取 GitHub 仓库截图

1. 浏览器打开本项目 GitHub 页面。
2. 页面上方应清晰显示仓库名 `os实践A-杜浩源-2024302111427`。
3. 文件列表应能看到 `figures`、`notes`、`evidence` 和 `kernel`。
4. 打开仓库的 Tags 页面，确认 `lab0` 存在。
5. 分别截图仓库首页和 Tags 页面，保存为：

```text
evidence/screenshots/03-github-repository.png
evidence/screenshots/04-github-lab0-tag.png
```

## 六 补交前最终检查

照片与截图加入后执行：

```bash
git add .
git commit -m "第0次实验：补充手绘与验收截图"
git tag -f lab0
git push origin main
git push origin lab0 --force
```

这里移动 `lab0` 标签是因为证据文件是在初次提交后由本人补入；最终标签必须指向包含完整证据的提交。

