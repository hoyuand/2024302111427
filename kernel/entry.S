/*
 * Lab1 启动入口：QEMU -bios none 从 0x80000000 开始执行。
 * 本文件完成 M 态配置、PMP 授权、降权到 S 态和从核 park。
 */

#include "course_sid.h"

        .section .text
        .globl  _entry
        .type   _entry, @function
_entry:
        /* 启动阶段没有时钟/设备中断，先关闭 machine-mode 中断。 */
        csrw    mie, zero
        csrr    tp, mhartid
        bnez    tp, secondary_hart

        /* 每个 hart 只保留一个启动栈；栈顶位于 BSS 的高地址端。 */
        la      sp, bootstack
        li      t0, (LAB1_STACK_KB * 1024)
        add     sp, sp, t0

        /* M/S 态意外陷阱均落入对齐的兜底处理器。 */
        la      t0, m_trap_vector
        csrw    mtvec, t0

        /* 现代 QEMU 要求降权后的取指/访存落在 PMP 授权区域内。 */
        li      t0, 0x3fffffffffffff       /* pmpaddr0: 全物理地址 NAPOT */
        csrw    pmpaddr0, t0
        li      t0, 0xf                    /* pmpcfg0: A=NAPOT,R=W=X=1,L=0 */
        csrw    pmpcfg0, t0

        /* 把异常和中断委托给 S 态；本实验暂不打开设备中断。 */
        li      t0, -1
        csrw    medeleg, t0
        csrw    mideleg, t0
        csrw    mie, zero

        /* 早期没有页表，明确使用 Bare 模式并刷新 TLB。 */
        csrw    satp, zero
        sfence.vma zero, zero

        /* mret 的目标为 start，且 MPP=S；MIE 再次显式清零。 */
        la      t0, start
        csrw    mepc, t0
        csrr    t1, mstatus
        li      t2, (3 << 11)
        not     t3, t2
        and     t1, t1, t3
        li      t2, (1 << 11)              /* MSTATUS.MPP = supervisor */
        or      t1, t1, t2
        li      t2, (1 << 3)               /* MSTATUS.MIE = 0 */
        not     t3, t2
        and     t1, t1, t3
        csrw    mstatus, t1
        mret

secondary_hart:
        /* 课程验收以单核为准；从核不碰共享初始化状态。 */
        wfi
        j       secondary_hart

        .balign 4
        .globl  m_trap_vector
m_trap_vector:
        wfi
        j       m_trap_vector

        .balign 4
        .globl  s_trap_vector
s_trap_vector:
        wfi
        j       s_trap_vector

        .section .bss
        .balign 4096
        .globl  bootstack
bootstack:
        .space  (LAB1_STACK_KB * 1024)
