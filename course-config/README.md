# 课程配置与个人参数

本目录保存不属于内核实现、但用于理解和复现实验的课程资料：

* `course-requirements.md`：课程提交、环境和验收说明的整理记录；
* `student-parameters.txt`：本人的学号、协议版本、栈大小和 Lab 参数。
* `lab0-v2-requirements.md`：新版 Lab0 说明书与 `doc/lab0/` 验收材料的对照；
* `lab1-v2-requirements.md`：新版 Lab1 启动、串口、边界测试和归档要求的对照。
* `lab2-requirements.md`：Lab2 陷入、系统调用、控制台、测试与归档要求的对照。
* `lab3-v3.1-requirements.md`：Lab3 v2/V3.1 物理页、SV39、sbrk、fork/exec 与回归要求。
* `dump_pagetable-ABI.md`、`dump-callpoint-ABI.md`：Lab3 页表观测输出和集成点约定。
* `lab4-v3-requirements.md`：Lab4 V3/v2 上下文切换、阻塞唤醒、个人策略与测试口径。
* `lab4-design-notes.md`：实现前设计笔记，记录数据结构、不变式、锁序和回滚路径。
* `lab4-sources.sha256`：Lab4 输入说明书和增量包的字节数与 SHA-256。

内核编译时使用的 `course_sid.h` 仍位于 [`code/kernel/`](../code/kernel/)，以保持课程骨架的编译接口不变。
