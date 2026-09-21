# Lab0 · 课程阅读与环境自检

Lab0 完成 xv6 代码导读、实验环境自检和三张操作系统机制图，作为 Lab1 现场检查的前置材料。

## 图片

* `images/lab0-terminal-run.png`：WSL2 环境预检的真实终端截图；
* `images/01-echo-control-flow.png`：`echo hi` 从输入、阻塞唤醒到退出回收的控制流；
* `images/02-exec-state-snapshot.png`：`exec` 完成后的进程表、Sv39 页表和 stdout 引用快照；
* `images/03-timer-interrupt-journey.png`：用户态时钟中断经过 trap、调度和返回用户态的时序图。

三张机制图的可编辑 Mermaid 源文件与 PNG 同名，位于 `images/`。

## 文档

* `docs/lab0-result.md`：本轮目标、命令和结果；
* `docs/lab0-reading-notes.md`：源码依据、自查题和现场说明要点；
* `docs/environment-check.md`：工具链、QEMU 和 WSL2 记录；
* `docs/lab0-artifact-check.txt`：实际运行的图纸与目录资产检查输出；
* [`course-config/lab0-v2-requirements.md`](../../course-config/lab0-v2-requirements.md)：新版说明书要求对照表。
