# Lab4 V3 要求对照

来源是 Lab4 学生说明书 V3（2026-10-09）和 `lab4-v2-20261009.zip`；来源文件哈希见 [`lab4-sources.sha256`](lab4-sources.sha256)。用户最新要求优先；文档中的实验指令作为课程规范，不作为额外外部操作授权。

## 个人参数和范围

学号 `2024302111427`，`LAB2_TICK=3`，`LAB4_POLICY_ID=7`，使用步长调度。课程表规定名称默认票数 `hi=1 / spin=2 / sh=4`，其余进程为 `7 + SID % 13 = 18`；`setickets` 覆盖默认值。验收环境固定为 QEMU `virt`、`-smp 1`，双核不是本轮已验证结论。

## 实现对照

| 要求 | 实现位置与约束 |
|---|---|
| 上下文切换 | `kernel/swtch.S` 保存/恢复 ra、sp、s0–s11 共 14 个寄存器；偏移 0..104 与 context 一致 |
| 每 CPU/进程状态 | cpu 持 context/proc/noff/intena；每进程持锁、内核栈、context、chan、统计与策略字段 |
| sched 不变式 | 持当前进程锁，noff==1，状态非 RUNNING，关闭中断；跨 swtch 交接进程锁 |
| sleep/wakeup | 先 acquire(p->lock) 再释放条件锁，设置 SLEEPING 后 sched；唤醒持目标锁 |
| wait/exit | wait_lock→p->lock；wait 写回退出码，只有回收后才能重用进程槽 |
| 时钟抢占 | 用户态时钟陷入计 CPU tick 并 yield；一次中断计一 tick，LAB2_TICK 为周期倍数 |
| pause | SYS_pause=13，通过 tickslock 与 sleep/wakeup 实现 tick 等待 |
| 调度观测 | SYS_schedstat=23，pid=-1 表示自身；锁内快照，锁外 copyout |
| time_rdtime | SYS_time_rdtime=24，返回 time CSR，单位不是 tick |
| 改票 | SYS_setickets=25；n<1 返回-1，覆盖默认票数 |
| 步长策略 | stride=max(1,1000/tickets)，选择最小 pass，平局轮转；新建/唤醒对齐活动最小 pass |

## 测试与交付

官方 `bench` 实际源码为 100 轮 fork/exit/wait，再回收 8 个并发子进程；应输出 `RESULT avg_fork_wait`、`RESULT concurrent_children = 8` 和 `PASS: bench`。说明书某段旧示例 16 轮不覆盖最新测试源码。

官方 `stride_test` 使用两个 1:3 票数子进程，6 个 50-tick 窗口；`RESULT stride_ratio=a:b` 必须满足 2.4<b/a<3.6，每个窗口两进程都得到 CPU，最后 `PASS: stride`。自编测试补充 A/B 交错抢占、非法 pid/指针、setickets(0) 和 pause 边界。

Lab0–Lab3 回归包括静态资产、真实构建、Lab1 启动、Lab2 四组 UART 注入、Lab3 badaccess/memcap 和 dump_pagetable。源码结构检查只能证明文件和接口形状，行为结论必须来自真实 QEMU。最终提交、标签、远端、ZIP 和工作区状态记入唯一中文主报告。
