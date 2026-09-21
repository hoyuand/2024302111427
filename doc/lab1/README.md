# Lab1 · 启动与串口输出

Lab1 完成启动汇编、M 态到 S 态切换、PMP 授权、UART 轮询输出、最小 `printf` 和个性化 banner。验收环境为 QEMU `virt` 单核。

## 图片

* `images/lab1-terminal-run.png`：真实 Windows Terminal 截图，包含 QEMU 命令和实际 stdout；
* `images/lab1-startup-sequence.png`：启动路径图，说明 `_entry`、PMP、`mret`、`start`、`consoleinit`、UART 和 `main` 的关系。

启动路径图的 Mermaid 源文件为 `images/lab1-startup-sequence.mmd`。

## 文档

* `docs/lab1-result.md`：本轮目标、命令和结果摘要；
* `docs/lab1-qemu-output.txt`：期望串口输出及字节数；
* `docs/lab1-design-notes.md`：启动、UART、printf 和两个自测用例的设计说明；
* `docs/lab1-requirement-matrix.md`：新版说明书的分阶段任务、边界测试和归档要求对照；
* `docs/lab1-git-proof.txt`：提交、标签和远端指针记录；
* [`course-config/lab1-v2-requirements.md`](../../course-config/lab1-v2-requirements.md)：新版说明书要求总表。
