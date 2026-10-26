# Lab4：进程上下文切换、阻塞唤醒与步长调度

本轮依据 Lab4 学生说明书 V3（2026-10-09）和 `lab4-v2-20261009.zip`，在 Lab3 的 SV39、fork/exec 与 UART 基线上实现 `swtch`、每进程锁、sleep/wakeup、`pause`/调度观测接口和学号策略 `LAB4_POLICY_ID=7` 的步长调度。主验证为 QEMU `virt`、`-smp 1`。

## 目录

```text
doc/lab4/
├── README.md
├── docs/
│   └── lab4-experiment-report.md
└── images/
    ├── lab4-state-machine.mmd / .png
    ├── lab4-context-switch.mmd / .png
    └── lab4-sleep-wakeup.mmd / .png
```

唯一中文主报告见 [`docs/lab4-experiment-report.md`](docs/lab4-experiment-report.md)。PNG 是由 Graphviz 从同目录机制图生成的结构示意，MMD 是可编辑源文件；本轮以可复现的 UART 文本记录实测，不生成伪造终端截图。设计笔记见 [`course-config/lab4-design-notes.md`](../../course-config/lab4-design-notes.md)，要求对照见 [`course-config/lab4-v3-requirements.md`](../../course-config/lab4-v3-requirements.md)。

图片索引：`lab4-state-machine.png`、`lab4-state-machine.mmd`、`lab4-context-switch.png`、`lab4-context-switch.mmd`、`lab4-sleep-wakeup.png`、`lab4-sleep-wakeup.mmd`。

## 查验命令

```bash
make -C code clean && make -C code -j2
python3 -m unittest discover -s code/tests -p "test_*.py" -v
python3 code/tests/verify_lab4_qemu.py --tree code --tests lab4check alt bench stride_test
```

官方测试应输出 `PASS: bench`、`PASS: stride` 及对应 `RESULT`；命令、输出和退出状态写入主报告。提交前检查 `git diff --check`；最终 `main`、`lab4`、`lab4-submit` 指向同一提交，生成 `提交-lab4-2024302111427.zip` 和 `code/archives/lab4-code-delta-2024302111427.zip`，ZIP 不提交。
