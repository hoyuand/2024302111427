# Lab2 结果

## 结果摘要

- `make clean && make -j2` 成功，用户程序通过内嵌表编译进内核。
- Lab1 banner 与 `expect_banner.txt` 前缀保持一致；两次冷启动、全量 12 个资产测试和 Lab1 回归通过。
- QEMU `virt` 单核真实运行进入 `sh>`；`hi` 输出用户 pid，重复执行证明 `fork/exec/wait` 回收链路工作。
- `badecall` 打出 `TEST-1 PASS`，非法调用号不导致 panic。
- `spin` 忙循环期间可以注入并回显 `ping`，输出交错后内核仍存活。
- `bufstorm` 串口驱动对短行测试的 `BUFSTORM lines=4 bytes=8` 和后续 `sh>` 全部断言通过；四条 140 字节输入的压力测试输出 `BUFSTORM lines=4 bytes=397`，剩余片段被 Shell 作为命令拒绝，但内核持续响应且未发生死锁或 panic。
- `-d int` 实测日志出现 `desc=s_external`，同时保留 `user_ecall` 与 `m_timer`，没有出现非法指令、取指页故障或递归异常；切片见 [`lab2-qemu-int.txt`](lab2-qemu-int.txt)。

## 查验命令

```bash
make -C code clean && make -C code -j2
python3 -m unittest discover -s code/tests -p "test_*.py" -v
python3 code/check_expect.py 2024302111427 code/expect_banner.txt
python3 code/tests/verify_lab1_qemu.py
python3 code/support/inject_uart.py --tree code --script code/support/lab2-smoke.script
python3 code/support/inject_uart.py --tree code --script code/support/lab2-bufstorm.script
python3 code/support/inject_uart.py --tree code --script code/support/lab2-overflow.script
python3 code/support/inject_uart.py --tree code --script code/support/lab2-spin.script
```

完整 QEMU 原始输出保存在 [`lab2-build-and-smoke.txt`](lab2-build-and-smoke.txt)、[`lab2-spin-output.txt`](lab2-spin-output.txt)、[`lab2-bufstorm-output.txt`](lab2-bufstorm-output.txt) 和 [`lab2-overflow-output.txt`](lab2-overflow-output.txt)；中断切片见 [`lab2-qemu-int.txt`](lab2-qemu-int.txt)。自动串口断言结果见 `lab2-*-driver.txt`。

真实终端截图见 [`../images/lab2-terminal-run.png`](../images/lab2-terminal-run.png)。截图只记录短行 `bufstorm` 的稳定通过回路；超长行证据以原始日志为准。

## 归档

提交包含 Lab2 代码、测试、文档和图片；标签为 `lab2-start`、`lab2`、`lab2-submit`，归档包为 `提交-lab2-2024302111427.zip`。本轮同步推送 `main`、`lab2` 和 `lab2-submit`，远端核验命令见 [`lab2-git-proof.txt`](lab2-git-proof.txt)。
