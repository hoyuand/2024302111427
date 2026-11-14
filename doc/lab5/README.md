# Lab5 写时复制与 exec 参数栈

主报告：[lab5-experiment-report.md](docs/lab5-experiment-report.md)。说明书对照与实现前设计分别见 [要求对照](../../course-config/lab5-v3-requirements.md) 和 [设计笔记](../../course-config/lab5-design-notes.md)。

```text
doc/lab5/
├── README.md
├── docs/lab5-experiment-report.md
└── images/
    ├── lab5-cow-snapshots.png / .mmd
    ├── lab5-exec-stack.png / .mmd
    └── lab5-cow-path.png / .mmd
```

| 文件 | 用途 |
|---|---|
| docs/lab5-experiment-report.md | 唯一中文主报告，含设计、真实结果、错误路径与归档 |
| images/lab5-cow-snapshots.png、lab5-cow-snapshots.mmd | 同一数据页的fork、先写复制、后写独占恢复 |
| images/lab5-exec-stack.png、lab5-exec-stack.mmd | echo hi 参数字符串、指针、哨兵和16字节对齐 |
| images/lab5-cow-path.png、lab5-cow-path.mmd | 用户缺页与内核copyout共用的处理和失败路径 |

三张PNG是机制说明图，非运行截图；自编渲染源为 code/tools/render_lab5_diagrams.py，Mermaid 文件保留等价流程源。真实运行依据主报告中的UART节选和可重复执行的QEMU测试。

仓库根目录在WSL2执行：

```bash
make -C code clean && make -C code -j2
python3 -m unittest discover -s code/tests -p 'test_*.py' -v
python3 code/tests/verify_lab5_qemu.py
python3 code/tests/verify_lab4_qemu.py
git diff --check
```

Lab5测试默认覆盖官方三程序、自编cowtest/cowoom/vmtest及Lab3错误和OOM路径，UART原始输出写入忽略的code/lab5-qemu.log；不要将日志文字或机制图当成未经执行的截图证据。其余Lab0–Lab4查验命令在根README和主报告中。
