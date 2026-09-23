# Lab2 设计笔记

## 目标与个人参数

学号 `2024302111427` 对应 `LAB2_TICK=3`、`LAB2_BUF_SEMANTICS=1`（字符流）和 `LAB2_BUF_SIZE=128`。本轮保留 Lab1 banner，并把 UART 输出、用户态陷阱、系统调用和最小 Shell 串成一个可实测闭环。

## Trapframe 与返回不变式

- `struct trapframe` 与赠送 `trampoline.S` 的固定偏移一致：`kernel_satp`、`kernel_sp`、`kernel_trap`、`epc` 在前 40 字节，通用寄存器从偏移 40 开始；`a0` 位于 112 字节。
- `uservec` 只在 U 态进入时使用 `sscratch` 交换用户 `a0` 和固定高地址的 trapframe，完整保存寄存器后切到进程内核栈；`usertrap` 处理完后由 `userret` 切回用户页表并 `sret`。阻塞读暂时在 S 态运行 `kernel_trap_vector`，外部中断只保存内核现场。
- 任何 syscall 都先把 `sepc` 前移 4；未知调用号返回 `-1`，不允许 panic。
- 当前已建立最小 SV39 用户页表：低地址映射 `[0, PROC_MEM_SIZE)` 的用户镜像，内核代码按 `KERNBASE` 直映，UART、CLINT、PLIC 及其 supervisor claim 窗口保留给内核，最高页双重映射 `TRAMPOLINE` 与 `TRAPFRAME`。赠送 `kernel.ld`、`trampoline.S` 与 `user.ld` 均保持原样；新增 `kernel/lab2.ld` 作为链接 overlay，并用可执行锚点让 flagless 的 `trampsec` 进入独立的 RX 加载段，trampoline 本体仍位于页首。赠送 `user.ld` 仍把镜像链接到基址 0；构建增加 `-mno-relax`，保持平铺镜像的 PC-relative 引用。用户指针只接受 `[0, PROC_MEM_SIZE)` 范围，后续 Lab 再扩展真正的页分配和隔离。

## 进程数据结构与错误回滚

每个 `struct proc` 保存 pid、父进程、状态、活动镜像地址、`memsz/brk`、内核栈和一份 page-aligned trapframe。当前实现建立一套紧凑的 SV39 页表：用户虚拟地址 `[0, 256 KiB)` 映射到 `.userimage`，内核在 `KERNBASE` 的 1 GiB 身份映射、CLINT/UART/PLIC 两个 MMIO 窗口，以及 `TRAPFRAME/TRAMPOLINE` 高地址映射同时保留。PLIC 的 supervisor enable/claim 区域位于 `0x0c200000` 窗口，必须单独映射，否则用户页表下的外部中断会在 claim 读处再次陷入。由于本轮允许假调度，所有进程暂时共享一个物理活动槽；`fork` 把父镜像复制到 `fork_backup`，子进程复制 trapframe 并将 `a0=0`，父进程获得子 pid。子进程 `exec` 失败只返回 `-1`，不会破坏旧镜像；子进程 `exit` 前把备份恢复给父进程并标记 `ZOMBIE`，父进程随后 `wait` 回收槽位。

## 系统调用参数与分发

`a7` 是调用号，`a0–a2` 是参数，返回值写回 trapframe 的 `a0`。`argint`/`argaddr` 只读取三个 ABI 参数，`argstr` 限制字符串扫描在 256 字节内并逐字节检查活动槽范围。当前实现了 `fork/exit/wait/read/write/exec/getpid/sbrk`；其余已声明调用号统一返回 `-1`。`read` 在本轮尚无 sleep/wakeup 队列，因此先开启 supervisor 中断并以 `wfi` 等待 UART；中断返回后重新检查环形缓冲，不把忙等误称为调度器阻塞。

## UART 输入缓冲不变式

`input_buf[128]` 由 `input_r/input_w/input_count` 管理，`consoleintr` 是生产者：读取 UART RHR、满时丢弃最旧字节、递增换行计数并回显。字符流模式的可读条件是 `input_count != 0`，因此每个到达字符都能唤醒读者；单次 `console_read` 在换行处返回，令官方 `bufstorm` 的四次读有确定边界。行缓冲模式只需把可读条件切换为 `input_lines != 0`。

M 态时钟向量先通过 `mscratch` 保存 `t0–t3`，重置 `mtimecmp` 后置位 CLINT `MSIP`，再完整恢复被中断现场。S 态 trap 清除 `MSIP` 并累计 tick，防止高频输入期间定时器破坏 C 代码临时寄存器。UART 外部中断的实测 `-d int` 日志包含 `desc=s_external`，说明 IER → PLIC → `sie` → `sstatus` 的链路完整闭合。

## 自测与回归

1. `hi` 与重复 `hi`：覆盖 `fork → exec → write → exit → wait` 以及父子返回值。
2. `badecall`：90–99 号未知 ecall 全部返回 `-1`，Shell 继续存活。
3. `spin` 期间注入 `ping`：覆盖忙循环中的 UART 输入和输出交错。
4. `bufstorm`：四行短输入与超过 128 字节的高频输入，检查 `lines=4`、不死锁、不 panic；本次超长输入实测为 `bytes=397`。

## 已知边界

完整调度器、磁盘文件系统和 argv 深拷贝按说明书分别留给 Lab4、Lab6 和 Lab5；本轮的 SV39 页表只覆盖最小用户镜像、内核、MMIO、trapframe 和 trampoline 所需范围，没有把它宣称为后续实验的完整虚拟内存子系统。
