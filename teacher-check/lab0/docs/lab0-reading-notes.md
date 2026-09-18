# Lab0 阅读与剖析笔记

## 选定场景与版本说明

追踪场景是 Shell 执行 `echo hi`。参考树是课程提供的新版 xv6-riscv：用户返回准备函数名为 `prepare_return`，睡眠接口拆为 `sleep_prepare` 与 `sleep`。课程基线只用于后续实验，本笔记没有把公开 xv6 实现复制进基线。

三张图采用同一个假设：单核运行，Shell 已由 init 启动，文件描述符 0、1、2 都指向 console。图中“无锁”表示当前没有持有自旋锁；UART 的 `tx_lock` 是睡眠锁，会单独标出。

## 材料一的源码链路

1. Shell 在 `getcmd -> gets -> read` 中等待输入。`consoleread` 持有 `cons.lock` 检查缓冲区；没有完整输入时先在 `&cons.r` 注册等待，再释放控制台锁并睡眠，避免丢失唤醒。
2. 回车引发 UART 外部中断。`kernelvec -> kerneltrap -> devintr -> plic_claim -> uartintr -> consoleintr` 把字符放入环形缓冲区；遇到换行后更新 `cons.w` 并 `wakeup(&cons.r)`。
3. 调度器把 Shell 从 RUNNABLE 切回其内核栈。`consoleread` 把 `echo hi\n` 复制到用户缓冲区，系统调用返回用户态。
4. Shell 解析命令后执行 `fork`。内核 `kfork` 创建子进程，`uvmcopy` 复制低地址用户页，复制 trapframe；对子进程设置 `a0=0`，所以同一条 `fork` 返回指令之后，父进程得到子 PID，子进程得到 0。
5. 父 Shell 调用 `wait` 并睡眠。子进程执行 `exec("echo", argv)`；`kexec` 先建立新页表并加载 ELF，只有全部成功后才替换 `p->pagetable`，随后释放旧页表。失败分支只销毁尚未提交的新页表，所以原进程镜像仍可继续运行。
6. echo 的 `main` 调用 `write(1,"hi",2)` 和 `write(1,"\n",1)`。链路是 `uservec -> usertrap -> syscall -> sys_write -> filewrite -> devsw[CONSOLE].write -> consolewrite -> uartwrite -> THR`。UART 忙时写进程在 `tx_chan` 睡眠，发送完成中断将其唤醒。
7. echo 调用 `exit(0)`。`kexit` 逐个关闭 `ofile`，把进程设为 ZOMBIE，唤醒 Shell；Shell 的 `wait` 调用 `freeproc` 回收进程槽、trapframe、页表和用户页。

## 材料二的精确截面

截面定义为：echo 子进程的 `kexec` 已把新页表提交给 `struct proc`，但 `prepare_return -> userret -> sret` 尚未让 echo 执行第一条用户指令。此时 CPU 仍在 S 态、echo 的进程内核栈上，echo 状态为 RUNNING。

课程参考包中 `_echo` 的 ELF 有两个 LOAD 段：

- `0x0000-0x0969`：代码和只读数据，用户可读、可执行，PTE 为 `V|R|X|U`。
- `0x1000-0x1020`：数据和 BSS，用户可读写，PTE 为 `V|R|W|U`。

`kexec` 向上按页对齐后再分配一页 guard 和一页用户栈，因此 `p->sz=0x4000`：

- `0x2000-0x2fff`：guard page，PTE 有效且内核可读写，但清除 `PTE_U`。
- `0x3000-0x3fff`：用户栈，`V|R|W|U`，栈向低地址增长。
- 当前没有额外堆页；以后 `sbrk` 从低地址映像末端扩展。
- `TRAPFRAME=0x3fffffe000`：`V|R|W`，无 `U`。
- `TRAMPOLINE=0x3ffffff000`：`V|R|X`，无 `U`，所有进程映射同一物理代码页。

Sv39 的低地址叶子通过 `L2[0] -> L1[0] -> L0[0..3]`，最高两页通过 `L2[255] -> L1[511] -> L0[510..511]`。中间页表项本身只有 `V`，叶子才携带 R/W/X/U 权限。

init 打开一次 console 后两次 `dup(0)`，三个描述符共享同一 `struct file`。随后 init fork 出 shell，shell 又 fork 出 echo，每次 fork 都对三个描述符分别 `filedup`，因此在这个单一进程族的假设下该 file 的引用计数是 9。三者的 `ofile[0..2]` 都指向它；其类型是 `FD_DEVICE`，`major=CONSOLE(1)`，`ip` 指向 console 设备 inode，`devsw[1].write` 指向 `consolewrite`。

## 材料三的关键不变式

- 用户态中断首先进入 trampoline 的 `uservec`。硬件切到 S 态并写 `sepc/scause/sstatus`，但不会自动切换页表或栈；汇编先把所有用户寄存器保存到固定虚拟地址 TRAPFRAME，再装入进程内核栈和内核页表。
- `usertrap` 立即把 `stvec` 改为 `kernelvec`，防止在内核执行时发生嵌套中断却错误进入 `uservec`。
- `devintr` 识别 `scause=0x8000000000000005`，`clockintr` 更新 ticks、唤醒等待者并写 `stimecmp` 安排下一次中断。
- `yield` 必须先持有 `p->lock`，把状态改为 RUNNABLE，再由 `swtch` 从进程内核栈切到 CPU 的调度器上下文。调度器接过该锁，完成状态交接后释放。
- 被中断进程再次运行时，从原来的 `swtch` 返回。`prepare_return` 关闭中断，写入下次陷入需要的内核页表、内核栈、入口和 hartid，设置 `SPP=0`、`SPIE=1` 与 `sepc`。
- `userret` 切回用户页表并恢复寄存器，最终 `sret` 根据 `sepc` 和 `sstatus` 返回原用户指令流。

## 自查题答案

**fork 后从哪里继续，为什么返回值不同？** 父子进程都从用户态 `fork` 封装函数中 `ecall` 后的下一条指令继续。父进程的系统调用返回值是新 PID；复制 trapframe 后，内核把子进程的 `a0` 改为 0。

**exec 如何改变地址空间？** 它先在独立新页表中装载 ELF、guard、stack 和 argv。全部成功后一次性替换 `p->pagetable/p->sz/epc/sp` 并释放旧页表；PID、parent、打开文件与当前目录保持不变。

**为什么 trampoline 只读且共享？** 它是固定的陷入/返回代码，不应被用户修改，也不包含进程私有数据。所有页表必须把同一物理代码页映射到同一最高虚拟地址，才能在切换 `satp` 的瞬间继续执行。

**Shell 在 read 返回前是什么状态，谁唤醒？** 没有完整输入时是 SLEEPING，等待通道为 `&cons.r`。UART 接收中断中的 `consoleintr` 在读到换行、EOF 或缓冲区满时调用 `wakeup(&cons.r)`。

**write 如何到 UART？** 系统调用分发到 `sys_write`，通过 fd 1 找到 `FD_DEVICE` 文件，再经 `filewrite -> devsw[1].write -> consolewrite -> uartwrite`；驱动检查 LSR 的发送空闲位并写 THR 寄存器。

**不创建第一个用户进程会怎样？** `main` 仍进入 `scheduler`，但进程表没有 RUNNABLE 项，CPU 会执行 `wfi` 等待中断。终端只显示内核启动信息，不会出现 init、Shell 或 `$ ` 提示符。

**子进程已 exit、父进程未 wait 时是什么状态？** 子进程保持 ZOMBIE。用户资源和文件已在 exit 中释放，但进程槽、PID 和退出状态保留，直到父进程 wait 回收。

**exit 如何避免 stdout 泄漏？** fork 时每个继承描述符都增加 file 引用计数；exit 对每个 `ofile` 调用 `fileclose` 并清空槽位。引用数降到 0 时，`fileclose` 再释放对应 inode 或 pipe，因此共享期间不会过早关闭，最后也不会泄漏。

## 现场问答前必须本人确认

1. 能不看图口述一次输入唤醒、fork/exec、write/exit 的顺序，并解释每次 U/S 态切换。
2. 能指出三个栈：用户栈、每进程内核栈、每 CPU 调度器上下文保存的栈，并解释 `swtch` 为什么只保存 callee-saved 寄存器。
3. 能画出 echo 的低地址四页与最高两页，写出每页权限，解释 guard、trapframe、trampoline 为什么没有相同的 U 位。
4. 能解释 `cons.lock`、`p->lock`、`wait_lock`、`tickslock` 和 UART `tx_lock` 各自保护什么，以及何时必须释放。

