# Lab0 V2 要求对照

本文件把《lab0-实验说明书-学生版-V2》中的交付要求映射到仓库路径，便于复核。Lab0 不要求新增内核代码，重点是源码阅读、三张图纸和自校验留痕。

## 交付物

| 说明书要求 | 仓库位置 | 对照结果 |
|---|---|---|
| `echo hi` 完整控制流图 | [`doc/lab0/images/01-echo-control-flow.png`](../doc/lab0/images/01-echo-control-flow.png) | 已覆盖输入、阻塞唤醒、`fork`、`exec`、`write`、`exit`、`wait` 和提示符恢复 |
| `exec` 瞬间数据结构快照 | [`doc/lab0/images/02-exec-state-snapshot.png`](../doc/lab0/images/02-exec-state-snapshot.png) | 已覆盖进程表、Sv39 映射、PTE 权限、guard、trapframe、trampoline 和 `ofile[0..2]` |
| 时钟中断与上下文切换时序图 | [`doc/lab0/images/03-timer-interrupt-journey.png`](../doc/lab0/images/03-timer-interrupt-journey.png) | 已覆盖 `scause`、`uservec`、`usertrap`、`yield`、`swtch`、调度器和 `sret` 闭环 |
| 图纸导出格式 | `doc/lab0/images/*.png`，同名 `.mmd` 为可编辑源 | 展示图片统一使用 PNG，流程图源文件可复绘 |
| 自主分析批注 | 三张 `.mmd` 中的“个人思考”节点 | 每张至少 1 处，合计不少于 3 处 |
| 自检留痕 | [`doc/lab0/images/lab0-terminal-run.png`](../doc/lab0/images/lab0-terminal-run.png)、[`doc/lab0/docs/lab0-artifact-check.txt`](../doc/lab0/docs/lab0-artifact-check.txt) | 保留真实环境预检截图和自动化资产检查输出 |

## 关键阅读依据

源码链路和截面数值记录在 [`doc/lab0/docs/lab0-reading-notes.md`](../doc/lab0/docs/lab0-reading-notes.md)。其中明确记录用户态/内核态、用户栈/进程内核栈/调度器栈、`TRAPFRAME`、`TRAMPOLINE`、guard page、文件描述符继承和中断返回路径。

说明书允许在后续实验中通过插桩或 `-d int` 日志进行现场自检。本仓库按课程竞赛认定免去 Lab 实验验证环节的安排，保留静态源码复核、图纸批注和真实环境预检证据，不把未实际取得的 xv6 日志冒充为运行结果。
