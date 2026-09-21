# Lab2 要求对照

本轮依据 `lab2-实验说明书-学生版.docx` 和 `lab2.zip` 实施；冲突时按最新用户要求和当轮接口约定执行。学号 `2024302111427` 对应：

| 参数 | 值 | 实现位置 |
|---|---:|---|
| `LAB2_TICK` | 3 | `code/kernel/course_sid.h`、CLINT 定时器 |
| `LAB2_BUF_SEMANTICS` | 1 | 字符流可读条件 |
| `LAB2_BUF_SIZE` | 128 | `input_buf[LAB2_BUF_SIZE]` |

## 核心要求

- 赠送的 `trampoline.S`、用户库、用户程序、系统调用号、公共头文件和 `user.ld` 保持原样；实现代码放在新增内核文件中。
- 系统调用 ABI 使用 `a7/a0–a2/a0`，`sepc` 在 ecall 后前移 4，未知调用号返回 `-1`。
- UART 由 PLIC 外部中断接收并进入 128 字节环形缓冲；字符流模式每个字符到达即满足可读条件，单次读取在换行处结束。
- 程序通过 `_uprog_table` 内嵌；本轮没有磁盘和文件系统。
- `satp=0` 阶段使用单一活动物理槽和 fork 回滚备份；用户代码以基址 0 链接并通过 `-mno-relax` 保持 PC-relative。
- 单核 QEMU 进入 `sh>`，通过 `hi`、`spin`、`badecall`、`bufstorm`，并回归 Lab1 banner。

## 证据

完整实现与查验路径见 [`doc/lab2/`](../doc/lab2/)。本轮创建本地 `lab2-start`、`lab2`、`lab2-submit` 和 ZIP，但遵照最新要求不推送 GitHub。
