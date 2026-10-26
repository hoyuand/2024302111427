# Lab4 实现前设计笔记（2026-10-26）

依据：Lab4 学生说明书 V3（2026-10-09）与 lab4-v2-20261009 增量包。用户最新要求优先；文档中的实验步骤作为课程规范，不作为额外外部操作授权。

## 起点与结构

当前 Lab3 用 current_proc 指针在 fork 时直接切到子进程、exit 时直接返回父进程，read 通过 WFI 等待；没有真正的内核上下文切换、每进程锁或调度器。保留已有 Sv39、copyin/copyout、页分配、fork 深拷贝与 exec 事务构建。

引入 spinlock（所属 CPU）、cpu（context/proc/noff/intena）、proc.lock/context/chan/killed/cpu_ticks/last_slice_ticks/tickets/stride/pass。进程表与静态内核栈继续使用已有存储，只有 wait 回收 ZOMBIE 的页表后才能重用槽位。首进程与新 fork 进程 context.ra=forkret，context.sp=各自内核栈顶。

swtch(a0=old,a1=new) 保存 ra/sp/s0-s11 共 14 个寄存器，字段偏移 0..104；ret 跳到恢复的 ra。调用者保存寄存器由 C 调用约定负责，用户 31 寄存器由 trapframe/trampoline 保存，二者不能混淆。调度器运行在启动栈，A 内核栈→调度器启动栈→B 内核栈，避免退出进程栈仍被调度器使用。

## 不变式与锁序

进入 sched 必须同时持有本进程锁、noff==1、状态非 RUNNING、中断关闭；swtch 前后保存并恢复 intena。p->lock 跨 swtch 交接，首次 forkret 释放锁。scheduler 只选 RUNNABLE，切回后清空 CPU 当前进程再释放进程锁。

状态迁移：UNUSED→USED(分配槽)→RUNNABLE(装载/fork成功)→RUNNING(scheduler)；RUNNING→RUNNABLE(yield)、SLEEPING(sleep)、ZOMBIE(exit)；SLEEPING→RUNNABLE(wakeup/kill)；ZOMBIE→UNUSED(wait)。迁移持 p->lock，父子关系额外持 wait_lock。

统一顺序 wait_lock→p->lock。sleep(chan,lk) 先 acquire(p->lock)，再 release(lk)，设 chan/SLEEPING 后 sched，醒来释放 p->lock 并重新获取 lk。wakeup 跳过当前进程并持目标锁修改状态。条件锁→进程锁保证检查条件与入睡之间的唤醒不会丢失；错误的先释放条件锁方案可在中间被生产者唤醒，从而永久睡眠。

UART 输入锁保护128字节环形缓冲，read 在输入通道睡眠；中断加入字符后 wakeup。tickslock 保护全局 tick，pause 持锁检查已过 tick 并 sleep(&ticks,&tickslock)。只在用户态时钟陷入计入 cpu_ticks 并抢占，内核处理保持非抢占。

## 策略与个人参数

学号 2024302111427：LAB4_POLICY_ID=7，步长调度。默认票数 hi=1、spin=2、sh=4，其余=18（7+SID%13）。setickets(n) 覆盖默认，n<1返回-1，整数 stride=1000/tickets（极大票数下最小1，避免零步长）。fork 采用名称默认票数，exec 重新按新名称取默认；显式 setickets 用于课程测试改票。

每次从就绪进程选最小 pass，同值轮转；按消耗的用户时钟片更新 pass。新进程和刚被唤醒者对齐当前最小活动 pass，防止旧 pass 为0而长期独占。LAB2_TICK=3 是时钟周期倍数，一次中断计1 tick，时钟周期3,000,000 time计数。last_slice_ticks 每次切入清零，只累计用户态时钟中断。

## 错误与回滚

fork 分配失败释放子页表并回到 UNUSED，父进程不变；exec 仅在新页表全部成功后提交并释放旧页表。wait 无子进程/非法状态指针返回-1，非法状态指针不回收子进程，可重试。schedstat 在目标锁内读快照、锁外 copyout；非法 pid/地址返回-1。pause 负值返回-1；kill 仅置标志，目标自行退出；不在 kill 中释放仍运行的栈/页表。

## 预先定义的验证

1. 官方 bench：以增量包真实源码为准，100轮 fork/exit/wait + 8子进程并发；与说明书旧示例16轮不同。
2. 官方 stride_test：7:21票，6个50tick窗口，b/a在2.4–3.6之间且每窗口均有CPU。
3. 自写策略测试：改票前后1:3与3:1份额反转；自写边界测试：setickets(0)、非法pid/指针、pause、退出码与反复回收。
4. 自写alt检查计算进程A/B交错；真实重现丢失唤醒的隔离调试构建，恢复正确顺序后bench通过。
5. Lab0–Lab3静态检查、真实构建、Lab1启动、Lab2四组UART、Lab3 badaccess/memcap及页表dump回归。

仅交付并验收 -smp 1；不声称双核支持。本文件先于Lab4实现写入。
