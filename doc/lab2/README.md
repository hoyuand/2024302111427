# Lab2：陷入与中断

本轮在 Lab1 的启动链路上实现最小的用户态运行环境：trampoline 陷阱入口、系统调用分发、UART 中断驱动输入、字符流环形缓冲，以及从内嵌程序表加载 `sh`、`hi`、`spin` 和官方测试程序。基础验收固定使用 QEMU `virt`、`-smp 1`。

## 目录

- 代码：[`code/kernel/`](../../code/kernel/)、[`code/user/`](../../code/user/)、[`code/Makefile`](../../code/Makefile)
- 设计与要求：[`docs/lab2-design-notes.md`](docs/lab2-design-notes.md)、[`docs/lab2-requirement-matrix.md`](docs/lab2-requirement-matrix.md)
- 实测原始输出：[`docs/`](docs/)
- 图示：[`images/lab2-trap-flow.png`](images/lab2-trap-flow.png)、[`images/lab2-console-sequence.png`](images/lab2-console-sequence.png)
- 真实运行截图：[`images/lab2-terminal-run.png`](images/lab2-terminal-run.png)

## 查验

```bash
make -C code clean && make -C code -j2
python3 -m unittest discover -s code/tests -p "test_*.py" -v
python3 code/check_expect.py 2024302111427 code/expect_banner.txt
python3 code/tests/verify_lab1_qemu.py
```

进入 `sh>` 后可运行：

```text
hi
badecall
spin
bufstorm
```

个人参数来自 [`course-config/student-parameters.txt`](../../course-config/student-parameters.txt)：`LAB2_TICK=3`、`LAB2_BUF_SEMANTICS=1`、`LAB2_BUF_SIZE=128`。短行官方测试由串口驱动自动断言通过；超过 128 字节的输入行允许丢弃缓冲区已满时的旧字节，但必须保持内核存活并完成四次读。原始证据见 [`lab2-bufstorm-output.txt`](docs/lab2-bufstorm-output.txt) 和 [`lab2-overflow-output.txt`](docs/lab2-overflow-output.txt)。

## 归档

本轮在本地创建 `lab2-start`、`lab2` 和 `lab2-submit`，并生成 `提交-lab2-2024302111427.zip`。遵照本轮要求，本次不推送 GitHub；远端指针核验留待用户确认后执行。
