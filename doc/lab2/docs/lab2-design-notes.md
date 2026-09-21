# Lab2 设计笔记

## 目标与个人参数

学号 `2024302111427` 对应 `LAB2_TICK=3`、`LAB2_BUF_SEMANTICS=1`（字符流）和 `LAB2_BUF_SIZE=128`。本轮保留 Lab1 banner，并把 UART 输出、用户态陷阱、系统调用和最小 Shell 串成一个可实测闭环。

## Trapframe 与返回不变式

- `struct trapframe` 与赠送 `trampoline.S` 的固定偏移一致：`kernel_satp`、`kernel_sp`、`kernel_trap`、`epc` 在前 40 字节，通用寄存器从偏移 40 开始；`a0` 位于 112 字节。
- `s_trap_vector` 只在 U 态进入时使用 `sscratch` 交换用户 `a0` 和 trapframe 地址，完整保存寄存器后切到进程内核栈；`usertrap` 处理完后由 `userret_direct` 恢复并 `sret`。
- 任何 syscall 都先把 `sepc` 前移 4；未知调用号返回 `-1`，不允许 panic。
- 当前是 `satp=0` 过渡阶段，没有 SV39 隔离。赠送 `user.ld` 仍把镜像链接到基址 0；构建增加 `-mno-relax`，保持全局对象和调用为 PC-relative，再把平铺镜像装到内核活动物理槽。用户指针只在该槽范围内检查；Lab3 将把这部分替换为页表映射。

## 进程数据结构与错误回滚

每个 `struct proc` 保存 pid、父进程、状态、活动镜像地址、`memsz/brk`、内核栈和一份 trapframe。由于本轮允许假调度，所有进程暂时共享一个物理活动槽；`fork` 把父镜像复制到 `fork_backup`，子进程复制 trapframe 并将 `a0=0`，父进程获得子 pid。子进程 `exec` 失败只返回 `-1`，不会破坏旧镜像；子进程 `exit` 前把备份恢复给父进程并标记 `ZOMBIE`，父进程随后 `wait` 回收槽位。

## 系统调用参数与分发

`a7` 是调用号，`a0–a2` 是参数，返回值写回 trapframe 的 `a0`。`argint`/`argaddr` 只读取三个 ABI 参数，`argstr` 限制字符串扫描在 256 字节内并逐字节检查活动槽范围。当前实现了 `fork/exit/wait/read/write/exec/getpid/sbrk`；其余已声明调用号统一返回 `-1`。

## UART 输入缓冲不变式

`input_buf[128]` 由 `input_r/input_w/input_count` 管理，`consoleintr` 是生产者：读取 UART RHR、满时丢弃最旧字节、递增换行计数并回显。字符流模式的可读条件是 `input_count != 0`，因此每个到达字符都能唤醒读者；单次 `console_read` 在换行处返回，令官方 `bufstorm` 的四次读有确定边界。行缓冲模式只需把可读条件切换为 `input_lines != 0`。

M 态时钟向量先通过 `mscratch` 保存 `t0–t3`，重置 `mtimecmp` 后置位 CLINT `MSIP`，再完整恢复被中断现场。S 态 trap 清除 `MSIP` 并累计 tick，防止高频输入期间定时器破坏 C 代码临时寄存器。

## 自测与回归

1. `hi` 与重复 `hi`：覆盖 `fork → exec → write → exit → wait` 以及父子返回值。
2. `badecall`：90–99 号未知 ecall 全部返回 `-1`，Shell 继续存活。
3. `spin` 期间注入 `ping`：覆盖忙循环中的 UART 输入和输出交错。
4. `bufstorm`：四行短输入与超过 128 字节的高频输入，检查 `lines=4`、不死锁、不 panic。

## 已知边界

完整调度器、SV39 页表、磁盘文件系统和 argv 深拷贝按说明书分别留给 Lab3、Lab4、Lab6 和 Lab5；本轮没有伪造这些功能的证据。
