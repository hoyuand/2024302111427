# Lab2 要求矩阵

| 要求 | 实现/证据 | 状态 |
|---|---|---|
| trampoline 保存与恢复现场 | `code/kernel/trampoline.S` 的 `uservec/userret`、`code/kernel/entry.S` 的 `kernel_trap_vector`；`struct trapframe` | 通过 |
| ecall ABI 与 `sepc+4` | `code/kernel/trap.c`、`code/kernel/syscall.c` | 通过 |
| 未知 syscall 返回 `-1` | `code/user/badecall.c`；`lab2-build-and-smoke.txt` | 通过 |
| 参数提取与用户范围检查 | `argint/argaddr/argstr`、`user_range` | 通过 |
| UART 中断接收与环形缓冲 | `code/kernel/console.c`、`code/kernel/trap.c` 的 PLIC 初始化；`-d int` 含 `s_external` | 通过 |
| 用户页表与 trampoline 双映射 | `code/kernel/proc.c` 的 SV39 根表、MMIO 窗口、`TRAPFRAME/TRAMPOLINE` 映射；`code/kernel/lab2.ld` 提供独立 RX 段，预置 `kernel.ld` 未修改 | 通过 |
| 个人缓冲参数 | `course_sid.h`: tick 3、字符流、128 字节 | 通过 |
| 内嵌程序表 exec | `code/kernel/userimg.S`、`proc_exec`、原版 `user.ld` 与 `-mno-relax` | 通过 |
| Shell/hi/fork/wait | `code/user/sh.c`、`proc.c`；hi 实测 pid 输出 | 通过 |
| spin 期间输入 | `lab2-spin-output.txt` 含 `ping` 与连续 spin 输出 | 通过 |
| 缓冲边界 | `lab2-bufstorm-driver.txt` EXPECT 全通过；`lab2-overflow-output.txt` 超长行后内核存活 | 通过 |
| Lab0/Lab1 回归 | `check_expect.py`、`verify_lab1_qemu.py`、全量 12 个单元/资产测试 | 通过 |
| 真实运行证据 | `images/lab2-terminal-run.png`、`docs/*.txt`、`lab2-qemu-int.txt` 中的 `s_external` | 通过 |
| Git 归档与远端核验 | `lab2-start`、`lab2`、`lab2-submit`、两个 ZIP、`lab2-git-proof.txt` | 已提交、已推送，脱引用对象核验通过 |
