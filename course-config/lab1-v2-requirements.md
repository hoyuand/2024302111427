# Lab1 V2 要求对照

本文件把《lab1-实验说明书-学生版-V2》中的硬件、实现、测试和归档要求映射到仓库路径。

## 硬件与启动链路

| 说明书要点 | 实现位置 | 复核方式 |
|---|---|---|
| 链接/加载基址 `0x80000000` | [`code/kernel/kernel.ld`](../code/kernel/kernel.ld) | `make -C code` 后查看链接命令 |
| M 态关闭中断、挂起从核、初始化栈 | [`code/kernel/entry.S`](../code/kernel/entry.S) | `test_startup_invariants_are_present` |
| `LAB1_STACK_KB` 不硬编码 | [`code/kernel/course_sid.h`](../code/kernel/course_sid.h)、`entry.S` | 栈大小由学号参数宏参与汇编 |
| `start()` 配置 PMP、委托寄存器、`satp=0`、`mepc/mstatus` 并执行 `mret` | [`code/kernel/start.c`](../code/kernel/start.c) | 启动图和静态测试 |
| UART `0x10000000`、LSR `0x10000005`、THRE `0x20` | [`code/kernel/console.c`](../code/kernel/console.c) | QEMU 真实终端截图 |

## 分阶段实现

| 阶段 | 说明书验收现象 | 当前实现/证据 |
|---|---|---|
| 一 | 初始栈与裸机单字符输出 | `entry.S` 栈和主核路径保留在最终实现中 |
| 二 | M 态切换到 S 态并输出 | `start()` 中完成 `PMP + mret`，见 [`doc/lab1/images/lab1-startup-sequence.png`](../doc/lab1/images/lab1-startup-sequence.png) |
| 三 | UART 轮询输出完整字符串 | `uartputc_sync()` 轮询 LSR，见 [`doc/lab1/docs/lab1-design-notes.md`](../doc/lab1/docs/lab1-design-notes.md) |
| 四 | `printf` 与个性化 Banner | `%d/%s/%x/%%`、协议 2 和边界行，见 [`code/kernel/printf.c`](../code/kernel/printf.c)、[`code/kernel/main.c`](../code/kernel/main.c) |

## 输出与边界测试

学号 `2024302111427` 对应协议 2，`sid % 97 = 0x17`。`%x` 会输出小写 `0x` 前缀，边界行覆盖 0、负数、最大/最小 32 位整数、空字符串、十六进制和 `%%`。期望输出和实测输出保存在 [`doc/lab1/docs/lab1-qemu-output.txt`](../doc/lab1/docs/lab1-qemu-output.txt)，真实运行截图为 [`doc/lab1/images/lab1-terminal-run.png`](../doc/lab1/images/lab1-terminal-run.png)。

复核命令：

```bash
make -C code clean && make -C code
python3 code/check_expect.py 2024302111427 code/expect_banner.txt
python3 -m unittest discover -s code/tests -p "test_*.py" -v
python3 code/tests/verify_lab1_qemu.py
```

## 版本归档

按照说明书使用 `lab1-submit` 标签和 `git archive` 归档，具体命令与远端核对记录见 [`doc/lab1/docs/lab1-git-proof.txt`](../doc/lab1/docs/lab1-git-proof.txt)。
