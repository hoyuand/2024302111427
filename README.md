# os实践A-杜浩源-2024302111427

本仓库用于“操作系统实践”课程的全过程归档。代码基线仅来自课程发放的 `baselines/2024302111427-kernel`；公开版 `xv6-riscv` 只用于理解操作系统机制，不向课程基线复制实现。教师验收材料统一放在 [`teacher-check/`](teacher-check/)；每个实验独立使用 `labN/images/` 和 `labN/docs/`，便于快速检查和期末报告汇总。

GitHub 远端为 <https://github.com/hoyuand/os-A---2024302111427>。GitHub 仓库名只接受 ASCII 字符，创建时会自动移除中文字符；课程项目全称以本页标题和仓库描述中的 `os实践A-杜浩源-2024302111427` 为准。

## 教师检查入口

| 实验 | 代码状态 | 直接材料 | 归档标签 |
|---|---|---|---|
| Lab0 | 阅读、架构图、环境自检 | [`teacher-check/lab0/`](teacher-check/lab0/) | [`lab0`](https://github.com/hoyuand/os-A---2024302111427/tree/lab0) |
| Lab1 | 启动、UART、printf、回归测试 | [`teacher-check/lab1/`](teacher-check/lab1/) | [`lab1`](https://github.com/hoyuand/os-A---2024302111427/tree/lab1) |
| Lab2–Lab7 | 按同一模板追加 | 对应 `teacher-check/labN/` | 对应 `labN` |

每个 `labN` 目录约定如下：

* `images/`：真实运行截图优先；必要的黑白架构图保留 SVG、PNG 和可编辑源文件。
* `docs/`：客观简要说明、测试结果、串口/功能输出、Git 归档证明。

## Lab0 阅读与剖析

Lab0 不计分，但必须通过，且会在 Lab1 现场验收时随机抽查。材料已整理到 [`teacher-check/lab0/`](teacher-check/lab0/)：

- `images/01-echo-control-flow.*`：`echo hi` 从键盘输入到退出回收的全系统控制流参考图。
- `images/02-exec-state-snapshot.*`：`exec` 完成、echo 首条用户指令尚未执行时的进程表、Sv39 页表与文件引用快照。
- `images/03-timer-interrupt-journey.*`：用户态时钟中断经 `uservec`、`usertrap`、`yield`、调度器到 `sret` 的时序图。
- `docs/lab0-reading-notes.md`：源码依据、关键不变式、自查题答案与现场问答提纲。
- `docs/environment-check.md`：WSL2 和课程工具链自检记录。
- `docs/manual-capture-guide.md`：软件图纸导出与验收截图步骤。

三张图使用常用的 Mermaid 流程图软件绘制，`.mmd` 是可编辑源文件，`.svg` 和 `.png` 是提交与展示版本。实验说明书要求本人理解并亲手绘制，但没有限定纸笔；使用软件绘图时仍须逐条核对并能在现场解释所有箭头和自主思考。

## 每次实验完成后的固定流程

```bash
git add .
git commit -m "第n次实验：完成代码、测试和报告证据"
git tag labN
git push origin main
git push origin labN
git ls-remote --heads --tags origin refs/heads/main refs/tags/labN
```

标签依次使用 `lab0`、`lab1`、`lab2`……，不使用日期或临时标签。通常只有在本轮全部材料完成后才创建标签，标签创建后不移动、不强制更新。本次 Lab1 是对上一轮已完成实验的教师检查目录补齐，因此会在最终提交后做一次性标签校正；从后续 Lab 开始严格按一次创建执行。如需提交课程归档包，可执行：

```bash
git archive --format=zip -o 提交-labN-2024302111427.zip labN
```

## 基线保护

课程预置文件是接口边界，不应擅自修改。至少包括 `Makefile`、`kernel/kernel.ld`、`kernel/riscv.h`、`kernel/course_sid.h`；其余发放文件也应先核对说明书再改。开发前先做设计笔记，代码实验应自行设计两个测试用例，并保留功能测试、回归测试和现场问答所需证据。

## 报告证据规范

每轮至少保留一张真实运行截图（终端中显示实验命令、关键输出和 PASS 结果），并同时保留原始文本记录，避免图片压缩造成信息丢失。黑白架构图只作为解释材料，不能替代真实运行证据。简要说明使用客观句式，注明环境、命令、结果、提交号和标签；不写过程性宣传语。
