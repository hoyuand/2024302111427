# Lab0 文档与素材索引

先读 [`docs/lab0-experiment-report.md`](docs/lab0-experiment-report.md)：这是教师查阅的中文主报告，集中说明要求、源码分析、验证边界、图片和归档。目录层次如下：

```text
doc/lab0/
├── README.md                         本索引
├── docs/
│   └── lab0-experiment-report.md     唯一中文主报告：要求、源码分析、结果、自查与证据边界
└── images/
    ├── 01-echo-control-flow.png      echo hi 生命周期图
    ├── 01-echo-control-flow.mmd      上图可编辑 Mermaid 源
    ├── 02-exec-state-snapshot.png    exec 后进程、地址空间和文件状态图
    ├── 02-exec-state-snapshot.mmd    上图可编辑 Mermaid 源
    ├── 03-timer-interrupt-journey.png 时钟中断与上下文切换时序图
    ├── 03-timer-interrupt-journey.mmd 上图可编辑 Mermaid 源
    └── lab0-terminal-run.png         环境预检真实终端截图（不是 xv6 运行日志）
```

三张图的 PNG 是展示版本、`.mmd` 是可编辑源文件；环境检查终端截图保留真实预检输出。V2 要求对照见 [`course-config/lab0-v2-requirements.md`](../../course-config/lab0-v2-requirements.md)。教师查阅主报告即可；如需复绘图示，使用对应 `.mmd` 文件。
