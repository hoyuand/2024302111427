# Lab0 软件图纸与截图指南

课程原文要求三张图“自己理解并亲手绘制”，没有限定纸笔。任课教师允许软件绘图时，可直接提交本目录 `teacher-check/lab0/images/` 中的 Mermaid 图纸；现场验收前必须逐条检查并能脱离图稿说明因果关系。

## 一 图纸文件

每张图提供三种格式：

- `.mmd`：Mermaid 可编辑源文件，证明图纸不是一张不可修改的成品图片。
- `.svg`：矢量展示与打印版本，任意缩放仍保持清晰。
- `.png`：适合直接插入实验报告或提交平台。

对应关系：

```text
teacher-check/lab0/images/01-echo-control-flow.*
teacher-check/lab0/images/02-exec-state-snapshot.*
teacher-check/lab0/images/03-timer-interrupt-journey.*
```

如需自行修改，打开 <https://mermaid.live/>，粘贴 `.mmd` 内容并编辑。修改后同时更新 `.svg` 和 `.png`，避免源文件与图片不一致。

## 二 验收前逐图复核

### 图一：echo 全系统控制流

- 能沿箭头口述 `read -> 阻塞 -> UART唤醒 -> fork -> exec -> write -> exit -> wait回收`。
- 能区分用户栈、进程内核栈和 CPU 调度器栈。
- 能解释 U/S 态切换、M 态为何不进入本次运行链路，以及 `cons.lock`、`p.lock`、`uart_tx_lock` 的作用。
- 能解释三条个人思考，而不只是照读文字。

### 图二：exec 后状态快照

- 能说明快照时刻为何 echo 已是新地址空间，但 PID、parent、ofile 没变。
- 能从 L2/L1/L0 走到低地址四页、TRAPFRAME 和 TRAMPOLINE。
- 能解释 guard page 的 `U=0`、空 heap 的边界和标准输出引用链。
- 能解释 `ref=9` 的场景假设：init、shell、echo 各有三个控制台 fd。

### 图三：时钟中断旅程

- 能说明硬件保存什么、`uservec` 还要保存什么，以及硬件为何不能直接换页表和栈。
- 能说明 `tickslock`、`p.lock` 各自在哪一段持有。
- 能说明 `swtch` 的双向交接、`prepare_return/userret` 和 `sret` 的恢复顺序。
- 能解释三条个人思考，尤其是同址 TRAMPOLINE 和跨 `swtch` 持锁。

## 三 获取环境自检截图

1. 打开 Ubuntu（WSL2）终端。
2. 执行：

```bash
cd /mnt/c/Users/27226/Documents/ChatGPT/OS实践
python3 tools/preflight.py
```

3. 调整窗口高度，使标题、命令和结论同时可见。
4. 按 `Win + Shift + S`，框选完整终端窗口。
5. 保存为 `teacher-check/lab0/images/01-preflight.png`。

## 四 获取 Git 与标签截图

执行：

```bash
git log --oneline --decorate --graph --all
git status
git tag --list
git remote -v
```

将命令和结果完整截入一张图，保存为 `teacher-check/lab0/images/02-git-proof.png`。截图中应能看到 `lab0`、clean 工作区和 `origin`。

## 五 获取 GitHub 截图

1. 打开 <https://github.com/hoyuand/os-A---2024302111427>。
2. 截取仓库首页，确保文件列表包含 `teacher-check`、`kernel`、`README.md`，保存为 `teacher-check/lab0/images/03-github-repository.png`。
3. 打开 Tags 页面，确认 `lab0` 存在，保存为 `teacher-check/lab0/images/04-github-lab0-tag.png`。

GitHub 会移除仓库名中的中文字符，因此 URL 中显示 `os-A---2024302111427`；课程项目全称保留在 README 标题和仓库描述中。

## 六 截图补入后的最终提交

```bash
git add .
git commit -m "第0次实验：补充验收截图"
git tag -f lab0
git push origin main
git push origin lab0 --force
```

最终 `lab0` 标签应指向包含全部图纸和截图的提交。
