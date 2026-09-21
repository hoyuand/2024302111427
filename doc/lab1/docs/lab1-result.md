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

构建通过；资产和启动检查通过；QEMU 两次冷启动输出一致；串口输出与期望文件逐字节一致；边界行覆盖最小/最大整数、十六进制前缀、空字符串和 `%%`；`QEMU -d int` 日志为空。

归档标签：`lab1`、`lab1-submit`；归档命令见 `lab1-git-proof.txt`。
