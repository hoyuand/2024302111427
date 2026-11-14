# Lab5 V3 要求对照

依据用户指定的 2026-11-09 V3 说明书和 v2 增量包。原件 SHA-256 见 [输入校验](lab5-sources.sha256)，预置文件校验见 [合并清单](lab5-course-files.sha256)。附件中的技术要求用于本实验，用户最新请求决定目录、回归、Git 推送和 ZIP 交付。

| 要求 | 实现或证据 |
|---|---|
| 重构前基线 | lab5-start 指向 Lab4 最终提交 be314e9；保留所有历史标签 |
| 先设计后实现 | [设计笔记](lab5-design-notes.md)：起点、三态快照、四类引用操作、方案比较、锁序、回滚 |
| 个人参数 | [course_sid.h](../code/kernel/course_sid.h)：SCHEME=2、BUDGET=195、KILL=1；Lab1/2 参数保持 |
| 统一全局 refcnt | [kalloc.c](../code/kernel/kalloc.c)：64KiB 数组、kmem.lock、仅归零毒化、O(1)共享计数 |
| 分配器自测 | kref_selftest 在 main 启动调用，验证 1→3→2→1→0 与 free/shared 守恒 |
| COW fork | [vm.c](../code/kernel/vm.c)：父子同步清 W、置 COW，跳过 guard，只读代码不置 COW，成功映射后增引用 |
| 缺页/独占优化 | cow_alloc 与 usertrap cause15；ref=1 不复制；sfence.vma；非法地址和 OOM 终止用户进程 |
| copyout/范围预检 | copyout 与 uvm_check_range 共用 cow_alloc，sys_read 在消费输入前预检 |
| pmc=26 | syscall.c / lab5.h；累计实际复制数、当前共享物理页数，非法 which=-1 |
| exec 参数事务 | sys_exec 用临时物理页提取参数；proc_exec 新页表准备完成后提交，16B对齐、NULL哨兵、返回argc |
| 预置文件不修改 | 本次 syscall.h/user.h/usys.pl/sh.c 和3项测试原始字节；旧 vm.h/swtch.S/ulib.c 等继续保留 |
| Shell 换行兼容 | 自编 sh_input.c，只对 sh 使用 gets=lab5_gets；不改变预置源码或UART语义 |
| 课程三测试 | zerocopy delta=1；cowpair delta=2；echo hi argc=2，均真实QEMU通过 |
| 综合与错误路径 | cowtest：跨页/多路共享/copyout/只读guard/参数回滚/预算重置；cowoom：真实耗尽下部分fork回滚和写缺页失败 |
| 全面回归 | Lab0资产、Lab1banner、Lab2四串口脚本、Lab3 badaccess/memcap、Lab4五项QEMU通过 |
| 文档与图片 | [中文主报告](../doc/lab5/docs/lab5-experiment-report.md)，三张PNG机制图和同名Mermaid源 |
| 归档 | main/lab5/lab5-submit 统一最终提交；完整ZIP和相对lab4-submit代码增量ZIP均不入Git |

说明书文字将分配器称为 kalloc.c、参数加载称为 kexec；本基线原先分配器位于 vm.c，加载入口名为 proc_exec。本次迁移分配器并沿用 proc_exec 名称，行为按 V3 完成。原 vm.h 无 uvm_check_range 声明，新增接口集中放在自编 lab5.h，保持课程 vm.h 原始字节。

V3 的字符串示意图与“按 i=0..argc-1 依次压入”算法顺序不一致。本实现遵循算法：echo 字符串在0x4ff0、hi在0x4fe0，argv数组从0x4fc0开始；指针值与对齐由 echo 程序实测校验。
