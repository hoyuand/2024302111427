# 教师验收材料

本目录集中放置各实验的教师验收材料。每个已完成 Lab 使用相同的结构：

* `README.md`：该实验的短索引，列出目录层次和每个文件用途；
* `images/`：真实运行截图及黑白实验图示；展示图为 PNG，流程图保留同名 Mermaid 源文件；
* `docs/`：一篇中文主报告，合并要求摘要、设计与实现、实测结果、图示说明和归档信息。

详细实现要求对照仍集中保存在 [`course-config/`](../course-config/)，不与面向教师的主报告重复。

## 已完成实验

* [Lab0：课程阅读、环境自检与架构图](lab0/) — 主报告：[lab0-experiment-report.md](lab0/docs/lab0-experiment-report.md)
* [Lab1：启动、UART、printf 与 QEMU 验证](lab1/) — 主报告：[lab1-experiment-report.md](lab1/docs/lab1-experiment-report.md)
* [Lab2：陷入、系统调用、中断驱动控制台与 Shell](lab2/) — 主报告：[lab2-experiment-report.md](lab2/docs/lab2-experiment-report.md)

查看顺序：先看该 Lab 的 README 了解文件位置，再阅读唯一主报告；PNG 图和真实终端截图已嵌入报告，源图文件留在 `images/` 供复查和修改。
