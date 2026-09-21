# Lab1 验收要求对照

## 阶段与文件

| 说明书阶段 | 关键要求 | 当前代码位置 | 验收证据 |
|---|---|---|---|
| 阶段一 | 关闭中断、识别主核、按 `LAB1_STACK_KB` 建立初始栈、跳转 `start` | `code/kernel/entry.S` | `test_startup_invariants_are_present` |
| 阶段二 | 配置 `mstatus.MPP`、`mepc`、PMP、`satp=0`，执行 `mret` 进入 S 态 | `code/kernel/entry.S`, `code/kernel/start.c` | `lab1-startup-sequence.png` |
| 阶段三 | 轮询 LSR bit5 后写 UART THR，输出完整字符串 | `code/kernel/console.c` | `lab1-terminal-run.png` |
| 阶段四 | 支持 `%d/%s/%x/%%`，`%x` 带小写 `0x` 前缀，输出协议 2 Banner | `code/kernel/printf.c`, `code/kernel/main.c` | `lab1-qemu-output.txt`、QEMU 回归脚本 |

## 边界用例

最终启动输出中的 `printf-boundary` 行覆盖：

* `zero=0`；
* `neg=-42`；
* `max=2147483647`；
* `min=-2147483648`；
* `empty=""`；
* `hex=0x2a`；
* `percent=%`。

连续长字符串和协议 2 校验和位于下一行，冷启动回归要求两次输出逐字节相同。

## 验收命令

```bash
make -C code clean && make -C code
python3 code/check_expect.py 2024302111427 code/expect_banner.txt
python3 -m unittest discover -s code/tests -p "test_*.py" -v
python3 code/tests/verify_lab1_qemu.py
```
