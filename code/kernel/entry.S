/* Lab1 汇编入口：建立 C 栈，主核进入 start，从核保持等待。 */

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
        wfi
        j       m_trap_vector

        .balign 4
        .globl  s_trap_vector
s_trap_vector:
        wfi
        j       s_trap_vector
