# os实践A-杜浩源-2024302111427

本仓库用于“操作系统实践”课程的全过程归档。代码基线仅来自课程发放的 `baselines/2024302111427-kernel`；公开版 `xv6-riscv` 只用于理解操作系统机制，不向课程基线复制实现。

GitHub 远端为 <https://github.com/hoyuand/os-A---2024302111427>。GitHub 仓库名只接受 ASCII 字符，创建时会自动移除中文字符；课程项目全称以本页标题和仓库描述中的 `os实践A-杜浩源-2024302111427` 为准。

## Lab0 阅读与剖析

Lab0 不计分，但必须通过，且会在 Lab1 现场验收时随机抽查。本目录已整理：

- `figures/01-echo-control-flow.*`：`echo hi` 从键盘输入到退出回收的全系统控制流参考图。
- `figures/02-exec-state-snapshot.*`：`exec` 完成、echo 首条用户指令尚未执行时的进程表、Sv39 页表与文件引用快照。
- `figures/03-timer-interrupt-journey.*`：用户态时钟中断经 `uservec`、`usertrap`、`yield`、调度器到 `sret` 的时序图。
- `notes/lab0-reading-notes.md`：源码依据、关键不变式、自查题答案与现场问答提纲。
- `notes/course-requirements.md`：根据《05-学生须知》整理的全学期执行规则。
- `evidence/environment-check.md`：WSL2 和课程工具链自检记录。
- `evidence/manual-capture-guide.md`：软件图纸导出与验收截图步骤。

三张图使用常用的 Mermaid 流程图软件绘制，`.mmd` 是可编辑源文件，`.svg` 和 `.png` 是提交与展示版本。实验说明书要求本人理解并亲手绘制，但没有限定纸笔；使用软件绘图时仍须逐条核对并能在现场解释所有箭头和自主思考。

## 每次实验完成后的固定流程

```bash
git add .
git commit -m "第n次实验：<本次完成内容>"
git tag labN
git push origin main
git push origin labN
```

本次标签为 `lab0`，后续依次为 `lab1`、`lab2`。如需提交课程归档包，可执行：

```bash
git archive --format=zip -o 提交-labN-2024302111427.zip labN
```

## 基线保护

课程预置文件是接口边界，不应擅自修改。至少包括 `Makefile`、`kernel/kernel.ld`、`kernel/riscv.h`、`kernel/course_sid.h`；其余发放文件也应先核对说明书再改。开发前先做设计笔记，代码实验应自行设计两个测试用例，并保留功能测试、回归测试和现场问答所需证据。
