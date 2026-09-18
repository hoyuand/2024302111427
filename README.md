# os实践A-杜浩源-2024302111427

本仓库用于“操作系统实践”课程的全过程归档。代码基线仅来自课程发放的 `baselines/2024302111427-kernel`；公开版 `xv6-riscv` 只用于理解操作系统机制，不向课程基线复制实现。教师验收材料统一放在 [`teacher-check/`](teacher-check/)；每个实验独立使用 `labN/images/` 和 `labN/docs/`，便于快速检查和期末报告汇总。

GitHub 远端为 <https://github.com/hoyuand/os-A---2024302111427>。GitHub 仓库名只接受 ASCII 字符，创建时会自动移除中文字符；课程项目全称以本页标题和仓库描述中的 `os实践A-杜浩源-2024302111427` 为准。

## 教师检查入口

| 实验 | 代码状态 | 直接材料 | 归档标签 |
|---|---|---|---|
| Lab0 | 阅读、架构图、环境自检 | [`teacher-check/lab0/`](teacher-check/lab0/) | [`lab0`](https://github.com/hoyuand/os-A---2024302111427/tree/lab0) |
| Lab1 | 启动、UART、printf、回归测试 | [`teacher-check/lab1/`](teacher-check/lab1/) | [`lab1`](https://github.com/hoyuand/os-A---2024302111427/tree/lab1) |
| Lab2–Lab7 | 按同一模板追加 | 对应 `teacher-check/labN/` | 对应 `labN` |

每个实验材料集中在对应目录：`images/` 放运行截图和图示，`docs/` 放结果说明、原始输出与 Git 记录。

## Lab0 阅读与剖析

Lab0 不计分，但必须通过，且会在 Lab1 现场验收时随机抽查。材料已整理到 [`teacher-check/lab0/`](teacher-check/lab0/)：

- `images/01-echo-control-flow.png`：`echo hi` 从键盘输入到退出回收的全系统控制流参考图。
- `images/02-exec-state-snapshot.png`：`exec` 完成、echo 首条用户指令尚未执行时的进程表、Sv39 页表与文件引用快照。
- `images/03-timer-interrupt-journey.png`：用户态时钟中断经 `uservec`、`usertrap`、`yield`、调度器到 `sret` 的时序图。
- `docs/lab0-reading-notes.md`：源码依据、关键不变式、自查题答案与现场问答提纲。
- `docs/environment-check.md`：WSL2 和课程工具链自检记录。
- `docs/manual-capture-guide.md`：软件图纸导出与验收截图步骤。

三张图保留 Mermaid 源文件，PNG 用于查看和报告插图。

## 每次实验完成后的固定流程

```bash
git add .
git commit -m "第n次实验：完成代码、测试和报告证据"
git tag labN
git push origin main
git push origin labN
git ls-remote --heads --tags origin refs/heads/main refs/tags/labN
```

标签依次使用 `lab0`、`lab1`、`lab2`……，不使用日期或临时标签。本轮材料完成后创建标签并推送。如需提交课程归档包，可执行：

```bash
git archive --format=zip -o 提交-labN-2024302111427.zip labN
```

## 基线保护

课程预置文件是接口边界，不应擅自修改。至少包括 `Makefile`、`kernel/kernel.ld`、`kernel/riscv.h`、`kernel/course_sid.h`；其余发放文件也应先核对说明书再改。开发前先做设计笔记，代码实验应自行设计两个测试用例，并保留功能测试、回归测试和现场问答所需证据。

