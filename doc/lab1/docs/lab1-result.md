# Lab1 结果

## 目标

完成从 `_entry` 到 `main` 的最小启动链路，配置 PMP 和 UART，并用无 libc 的 `printf` 输出个性化 banner 与边界测试。

## 查验

在仓库根目录执行：

```bash
make -C code clean && make -C code
python3 code/check_expect.py 2024302111427 code/expect_banner.txt
python3 -m unittest discover -s code/tests -p "test_*.py" -v
python3 code/tests/verify_lab1_qemu.py
```

真实运行截图为 `../images/lab1-terminal-run.png`；原始串口输出见 `lab1-qemu-output.txt`。

## 结果

构建通过；`consoleinit()`、`consputc()` 和 `uartputc_sync()` 的接口检查通过；资产和启动检查通过；QEMU 两次冷启动的 Lab1 前缀一致；Lab1 前缀与期望文件逐字节一致。当前工作树同时包含后续 Lab2 用户程序，因此前缀后允许出现 `sh> ` 提示符；边界行覆盖最小/最大整数、十六进制前缀、空字符串和 `%%`。在合并后续实验代码的工作树上，`QEMU -d int` 仅记录已知的 `user_ecall` 和 `m_timer`，脚本会单独校验并标注这两类后续实验陷阱；Lab1-only 镜像则要求日志为空。

归档标签：`lab1`、`lab1-submit`；归档命令见 `lab1-git-proof.txt`。
