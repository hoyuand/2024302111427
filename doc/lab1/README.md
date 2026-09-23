# Lab1：启动与串口输出

本目录采用“单篇中文报告 + 图片”的结构。教师查阅实验要求、实现、测试结果和归档信息，请直接阅读 [`docs/lab1-experiment-report.md`](docs/lab1-experiment-report.md)。

## 目录索引

```text
doc/lab1/
├── README.md
├── docs/
│   └── lab1-experiment-report.md   # 要求摘要、实现说明、实测结果、插图与归档记录
└── images/
    ├── lab1-terminal-run.png       # QEMU 真实终端截图
    ├── lab1-startup-sequence.png   # 黑白启动流程图，在主报告内嵌入
    └── lab1-startup-sequence.mmd   # 流程图可编辑 Mermaid 源文件
```

## 查验命令

在仓库根目录执行：

```bash
make -C code
python3 code/check_expect.py 2024302111427 code/expect_banner.txt
python3 -m unittest discover -s code/tests -p "test_*.py" -v
python3 code/tests/verify_lab1_qemu.py
```

说明书逐条要求映射见 [`course-config/lab1-v2-requirements.md`](../../course-config/lab1-v2-requirements.md)。
