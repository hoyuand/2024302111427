/* 预置支撑代码（请勿修改） —— lab6 受控断电注入驱动（v2）
 *
 * 原理：QEMU virt 机器带有 SiFive test 设备（MMIO 物理基址 0x100000，须在 kvminit 恒等映射），
 * 向其写入 0x7777 触发整机热复位（system_reset）：保留当前 fs.img 磁盘内容并从 _entry 重启，
 * 重启后由内核 recover_from_log() 执行写前日志崩溃恢复判定。
 *
 * 阶段编号（与《实验说明书 lab6 V3》及 support/tests/crashmap.md 严格对齐为 Stage 0..5）：
 *   0 = CRASH_BEFORE_OP     ：进入写事务（begin_op）之前立即断电
 *   1 = CRASH_LOG_WRITING   ：write_log() 向磁盘日志数据区写入途中（写完前一半日志块）断电
 *   2 = CRASH_LOG_WRITTEN   ：write_log() 日志数据块全部落盘、write_head() 提交头块未写前断电
 *   3 = CRASH_HEADER_DONE   ：write_head() 提交头块（header.n > 0）刚落盘、install_trans 前断电
 *   4 = CRASH_INSTALL_HALF  ：install_trans(0) 向磁盘原位块回写途中（写完前一半块）断电
 *   5 = CRASH_INSTALL_DONE  ：install_trans(0) 全部写回原位、write_head() 清零日志头块前断电
 *
 * 延迟武装机制：
 *   - 用户态调用 crash_at(0) 时立即触发断电复位；
 *   - 用户态调用 crash_at(1..5) 时将 armed_crash_stage 设为对应阶段并返回 0，
 *     随后用户程序紧接着发起 open/write 文件修改事务，内核 log.c 在对应阶段通过
 *     if (crash_should_fire(stage)) crash_at(stage); 精确触发断电复位。
 */

#include "types.h"
#include "riscv.h"
#include "defs.h"

#define TEST_DEVICE_PA       0x100000UL
#define RESET_MAGIC          0x7777U

#define CRASH_BEFORE_OP      0
#define CRASH_LOG_WRITING    1
#define CRASH_LOG_WRITTEN    2
#define CRASH_HEADER_DONE    3
#define CRASH_INSTALL_HALF   4
#define CRASH_INSTALL_DONE   5

static volatile int armed_crash_stage = 0;

int
crash_should_fire(int stage)
{
  return armed_crash_stage == stage;
}

void
crash_at(int stage)
{
  armed_crash_stage = 0;
  printf("[crash] injecting power-off at stage %d\n", stage);
  __sync_synchronize();
  *(volatile uint32 *)TEST_DEVICE_PA = RESET_MAGIC;
  panic("crash_at: reset failed (check TEST_DEVICE_PA mapping in kvminit)");
}

// 系统调用入口（SYS_crash_at = 27）：int crash_at(int stage);
uint64
sys_crash_at(void)
{
  int stage = argint(0);
  if (stage < 0 || stage > 5)
    return -1;
  if (stage == CRASH_BEFORE_OP)
    crash_at(0);
  armed_crash_stage = stage;
  printf("[crash] armed stage %d for next transaction\n", stage);
  return 0;
}
