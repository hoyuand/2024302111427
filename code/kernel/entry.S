/* Lab2 汇编入口：启动、用户陷阱和内核等待态陷阱。 */

#include "course_sid.h"

        .section .text
        .globl  _entry
        .type   _entry, @function
_entry:
        /* 启动阶段没有时钟/设备中断，先关闭 machine-mode 中断。 */
        csrw    mie, zero
        csrr    tp, mhartid
        bnez    tp, secondary_hart

        /* 栈顶位于 BSS 高地址端，容量由个性化宏决定。 */
        la      sp, bootstack
        li      t0, (LAB1_STACK_KB * 1024)
        add     sp, sp, t0

        /* start 在 M 态完成 PMP、委托与 mret 配置。 */
        call    start

primary_hart_stopped:
        wfi
        j       primary_hart_stopped

secondary_hart:
        /* 课程验收以单核为准；从核不碰共享初始化状态。 */
        wfi
        j       secondary_hart

        .balign 4
        .globl  m_trap_vector
m_trap_vector:
        /* Preserve the interrupted U/S temporaries in per-hart M scratch. */
        csrrw   t0, mscratch, t0
        sd      t1, 0(t0)
        sd      t2, 8(t0)
        sd      t3, 16(t0)

        /* Reset CLINT mtimecmp and forward one software interrupt to S mode. */
        li      t1, 0x0200bff8
        ld      t2, 0(t1)
        li      t3, (LAB2_TICK * 1000000)
        add     t2, t2, t3
        li      t1, 0x02004000
        sd      t2, 0(t1)
        li      t1, 0x02000000
        li      t2, 1
        sw      t2, 0(t1)

        ld      t1, 0(t0)
        ld      t2, 8(t0)
        ld      t3, 16(t0)
        csrrw   t0, mscratch, t0
        mret

        .balign 4
        .globl  s_trap_vector
s_trap_vector:
        /* sscratch contains the current process trapframe while in U mode. */
        csrrw   a0, sscratch, a0

        /* Keep this layout in lockstep with struct trapframe. */
        sd      ra, 40(a0)
        sd      sp, 48(a0)
        sd      gp, 56(a0)
        sd      tp, 64(a0)
        sd      t0, 72(a0)
        sd      t1, 80(a0)
        sd      t2, 88(a0)
        sd      s0, 96(a0)
        sd      s1, 104(a0)
        sd      a1, 120(a0)
        sd      a2, 128(a0)
        sd      a3, 136(a0)
        sd      a4, 144(a0)
        sd      a5, 152(a0)
        sd      a6, 160(a0)
        sd      a7, 168(a0)
        sd      s2, 176(a0)
        sd      s3, 184(a0)
        sd      s4, 192(a0)
        sd      s5, 200(a0)
        sd      s6, 208(a0)
        sd      s7, 216(a0)
        sd      s8, 224(a0)
        sd      s9, 232(a0)
        sd      s10, 240(a0)
        sd      s11, 248(a0)
        sd      t3, 256(a0)
        sd      t4, 264(a0)
        sd      t5, 272(a0)
        sd      t6, 280(a0)

        csrr    t0, sscratch
        sd      t0, 112(a0)
        csrw    sscratch, a0

        ld      sp, 8(a0)
        ld      t0, 16(a0)
        jalr    t0
        j       userret_direct

        /* Used only while a blocked read is waiting in S mode. */
        .balign 4
        .globl  kernel_trap_vector
kernel_trap_vector:
        addi    sp, sp, -240
        sd      ra, 0(sp)
        sd      gp, 8(sp)
        sd      tp, 16(sp)
        sd      t0, 24(sp)
        sd      t1, 32(sp)
        sd      t2, 40(sp)
        sd      s0, 48(sp)
        sd      s1, 56(sp)
        sd      a0, 64(sp)
        sd      a1, 72(sp)
        sd      a2, 80(sp)
        sd      a3, 88(sp)
        sd      a4, 96(sp)
        sd      a5, 104(sp)
        sd      a6, 112(sp)
        sd      a7, 120(sp)
        sd      s2, 128(sp)
        sd      s3, 136(sp)
        sd      s4, 144(sp)
        sd      s5, 152(sp)
        sd      s6, 160(sp)
        sd      s7, 168(sp)
        sd      s8, 176(sp)
        sd      s9, 184(sp)
        sd      s10, 192(sp)
        sd      s11, 200(sp)
        sd      t3, 208(sp)
        sd      t4, 216(sp)
        sd      t5, 224(sp)
        sd      t6, 232(sp)
        mv      a0, sp
        call    kerneltrap
        ld      ra, 0(sp)
        ld      gp, 8(sp)
        ld      tp, 16(sp)
        ld      t0, 24(sp)
        ld      t1, 32(sp)
        ld      t2, 40(sp)
        ld      s0, 48(sp)
        ld      s1, 56(sp)
        ld      a0, 64(sp)
        ld      a1, 72(sp)
        ld      a2, 80(sp)
        ld      a3, 88(sp)
        ld      a4, 96(sp)
        ld      a5, 104(sp)
        ld      a6, 112(sp)
        ld      a7, 120(sp)
        ld      s2, 128(sp)
        ld      s3, 136(sp)
        ld      s4, 144(sp)
        ld      s5, 152(sp)
        ld      s6, 160(sp)
        ld      s7, 168(sp)
        ld      s8, 176(sp)
        ld      s9, 184(sp)
        ld      s10, 192(sp)
        ld      s11, 200(sp)
        ld      t3, 208(sp)
        ld      t4, 216(sp)
        ld      t5, 224(sp)
        ld      t6, 232(sp)
        addi    sp, sp, 240
        sret

        .balign 4
        .globl  userret_direct
userret_direct:
        csrr    a0, sscratch
        ld      ra, 40(a0)
        ld      sp, 48(a0)
        ld      gp, 56(a0)
        ld      tp, 64(a0)
        ld      t0, 72(a0)
        ld      t1, 80(a0)
        ld      t2, 88(a0)
        ld      s0, 96(a0)
        ld      s1, 104(a0)
        ld      a1, 120(a0)
        ld      a2, 128(a0)
        ld      a3, 136(a0)
        ld      a4, 144(a0)
        ld      a5, 152(a0)
        ld      a6, 160(a0)
        ld      a7, 168(a0)
        ld      s2, 176(a0)
        ld      s3, 184(a0)
        ld      s4, 192(a0)
        ld      s5, 200(a0)
        ld      s6, 208(a0)
        ld      s7, 216(a0)
        ld      s8, 224(a0)
        ld      s9, 232(a0)
        ld      s10, 240(a0)
        ld      s11, 248(a0)
        ld      t3, 256(a0)
        ld      t4, 264(a0)
        ld      t5, 272(a0)
        ld      t6, 280(a0)
        ld      a0, 112(a0)
        sret
