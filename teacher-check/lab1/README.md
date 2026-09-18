# Lab1 教师检查摘要

Lab1 实现启动汇编、M 态到 S 态切换、PMP 授权、UART 轮询输出、最小 `printf` 和个性化 banner。验收以单核 QEMU 为准。

## 主要结果

* `images/lab1-terminal-run.png`：QEMU 运行截图，显示启动命令、banner、边界自检和校验和。
* `images/lab1-verification.png`：验收结果汇总图，显示字节匹配、重复启动和零异常。
* `images/lab1-startup-sequence.png`：启动路径图，标出 `_entry`、PMP、`mret`、UART 和 `main`。
* `images/lab1-git-timeline.png`：Lab 标签时间线。

## 文字证据

* `docs/lab1-build-and-test.txt`：构建、形状检查、回归测试和 QEMU 结果。
* `docs/lab1-qemu-output.txt`：串口期望/实测输出。
* `docs/lab1-qemu-real-run.txt`：同一次运行的原始 stdout 与字节数说明。
* `docs/lab1-git-proof.txt`：提交、标签和远端指针核验。
* `docs/lab1-design-notes.md`：设计决策、参数和两个自测用例。
* `docs/lab1-summary.md`：一页式客观结果摘要。
