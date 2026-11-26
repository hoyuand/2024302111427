/* 预置支撑代码（请勿修改） —— 睡眠锁结构体定义
 * 供块缓存（struct buf）与内存索引节点（struct inode）在等待磁盘 I/O 期间持有。
 */
#ifndef SLEEPLOCK_H
#define SLEEPLOCK_H

#include "types.h"
#include "spinlock.h"

struct sleeplock {
  uint locked;           // 锁状态（0=未锁，1=已锁）
  struct spinlock lk;    // 保护本睡眠锁状态字段的自旋锁
  char *name;            // 锁名称（调试用）
  int pid;               // 当前持有该睡眠锁的进程 pid
};

#endif
