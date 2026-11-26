/* 预置支撑代码（请勿修改） —— 自旋锁结构体独立头文件
 * 与 lab2-lab5 基线 kernel/proc.h 中的 SPINLOCK_STRUCT_DEFINED 守卫保持兼容，
 * 供 virtio_disk.c、sleeplock.h 等独立包含，避免重复定义冲突。
 */
#ifndef SPINLOCK_H
#define SPINLOCK_H

#include "types.h"

struct cpu;

#ifndef SPINLOCK_STRUCT_DEFINED
#define SPINLOCK_STRUCT_DEFINED
struct spinlock {
  volatile uint locked;
  char *name;
  struct cpu *cpu;                // 当前持有该锁的 CPU
};
#endif

#endif
