# Lab2 要求对照

本文件对照 Lab2 学生版 V2 说明、课程增量包接口、学号 `2024302111427` 的参数及仓库内最终代码。V2 说明中的代码接口优先于旧版记录；本表描述的是当前实现，不把早期设计草稿当作实现事实。

## 个人参数

| 参数 | 值 | 用途 | 实现位置 |
|---|---:|---|---|
| `LAB2_TICK` | 3 | M 态 timer 转发 / tick 周期 | `code/kernel/course_sid.h`、`code/kernel/entry.S`、`code/kernel/trap.c` |
| `LAB2_BUF_SEMANTICS` | 1 | 字符流模式：有任一字符即可读，单次读仍在换行处结束 | `code/kernel/course_sid.h`、`code/kernel/console.c` |
| `LAB2_BUF_SIZE` | 128 | UART 环形缓冲区容量 | `code/kernel/course_sid.h`、`code/kernel/console.c` |

## 要求与实现

| 要求 | 当前实现与查验依据 |
|---|---|
| 保留课程预置接口与个人基线 | 以内嵌程序表装载用户程序；课程给定 `Makefile.upgrade`、`trampoline.S`、用户库/程序、系统调用头文件、公共头文件及 `user.ld` 保持原样。字节哈希检查见 `code/tests/test_lab2_artifacts.py`。 |
| `ecall` / `sret` 与 syscall ABI | 硬件陷入本身只转入 S 态并更新 `scause/sepc`；trampoline 负责保存现场、访问 trapframe 并切换到内核栈。系统调用号在 `a7`，参数为 `a0–a2`，结果写入 `a0`；ecall 路径令 `sepc += 4`，未知号返回 `-1`。 |
| `write` 完整往返 | `sys_write` 验证 fd、长度和用户地址；逐字节经 `console_raw_putc` 输出。UART 驱动在写 THR 前轮询 LSR.THRE（bit 5）；返回长度经 trapframe 的 `a0` 恢复到用户态。流程图与时序图见 [`doc/lab2/README.md`](../doc/lab2/README.md)。 |
| UART 输入 / 外部中断 | PLIC 报告 UART 外部中断后，内核读取 claim、调用 `consoleintr` 从 RHR 取字节、回显并写入 128 字节环形缓冲，最后完成 claim。QEMU `-d int` 留有 `desc=s_external` 真实日志摘要。 |
| 空缓冲读的等待语义 | 当前 `sys_read` 在 S 态开启中断并用 `wfi` 等待 UART；中断处理后重查缓冲并返回数据。此阶段没有进程 sleep/wakeup 队列；报告和图示不把 WFI 伪称为调度器阻塞/唤醒。 |
| 缓冲区边界及两种模式 | 字符流模式可在任一输入字节到达后继续读，但一次 `console_read` 遇换行即结束；满时丢弃最旧字节并维护换行计数。行缓冲模式由 `LAB2_BUF_SEMANTICS=0` 对照分支支持。 |
| 用户镜像及地址空间 | 用户镜像 0 基址链接，并经 `_uprog_table` 内嵌；在启动检查中保证物理 `.userimage` 位于内核结束地址之后。当前实现建立最小 Sv39 页表，映射用户镜像、内核、PLIC/UART/CLINT MMIO 及高地址 `TRAPFRAME`/`TRAMPOLINE`；用户栈顶按 16 字节对齐。 |
| `fork/exec/wait` 与 Shell 测试程序 | 支持 `sh`、`hi`、`spin`、`badecall`、`bufstorm`。由于尚无独立地址空间/调度器，每次 fork/exec 使用单一活动镜像槽和备份回滚机制；不宣称已完成完整进程调度。 |
| 构建、自测和旧实验回归 | QEMU `virt` 单核；构建、全量 unittest、banner 校验、Lab1 双次冷启动、Shell 驱动 `smoke/bufstorm/overflow/spin` 均有留痕。命令、结果和限制见 [`doc/lab2/docs/lab2-experiment-report.md`](../doc/lab2/docs/lab2-experiment-report.md)。 |

## 设计口径更正

旧版工作笔记曾把 `satp=0` 当作 Lab2 方案描述；它不是当前最终实现。最终代码在 `code/kernel/proc.c` 中由 `build_pagetable()` 构造最小 Sv39 用户页表，并通过 `proc_satp()` 返回 `MAKE_SATP(user_root)`。`satp=0` 仍是此前 Lab1 启动阶段的状态，不能用来描述 Lab2 的用户态执行环境。

## 文档入口

教师查验从 [`doc/lab2/README.md`](../doc/lab2/README.md) 开始；该页列出本轮唯一实验报告、PNG 展示图、可编辑 Mermaid 源文件和真实 QEMU 终端截图。
