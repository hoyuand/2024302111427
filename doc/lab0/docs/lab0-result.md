# Lab0 结果

## 目标

完成课程代码阅读、环境自检，并整理 `echo` 控制流、`exec` 状态快照和时钟中断旅程三张图。三张图均用黑白 Mermaid 绘制并导出为单一 PNG 展示格式，同时保留 `.mmd` 源文件。

## 查验

在仓库根目录执行：

```bash
python3 code/tools/preflight.py
```

环境输出记录见 `environment-check.md`，对应终端截图为 `../images/lab0-terminal-run.png`。

## 结果

工具链和 QEMU 可用性已记录；图纸资产检查输出见 `lab0-artifact-check.txt`，阅读依据和现场说明见 `lab0-reading-notes.md`。新版说明书要求的 `doc/lab0/` 目录、三张图、三类指定思考题和图纸批注已逐项对照：图 1 标注 fork 双返回与 `a0`，图 2 标注 TRAMPOLINE 同址映射，图 3 标注独立调度器栈。

按照课程竞赛认定，Lab 实验验证环节可免除；因此本目录保留真实环境预检截图和静态资产检查作为留痕，并明确区分“环境预检”和“xv6 运行自检”，不把未取得的 xv6 `-d int` 日志写成实测结果。现场若要求复核，可按 `lab0-reading-notes.md` 的源码链路逐项口述。

归档标签：`lab0`。
