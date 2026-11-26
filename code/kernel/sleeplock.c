// sleeplock.c —— 睡眠锁框架（lab6 阶段一参考脚手架）
//
// 睡眠锁与自旋锁的区别：
//   - 自旋锁（spinlock）关中断忙等，持有期间严禁调用 sleep()/yield() 或等待磁盘中断；
//   - 睡眠锁（sleeplock）内部用一把自旋锁 lk->lk 保护自身的 locked 与 pid 字段：
//     获取睡眠锁时若发现 lk->locked != 0，调用 sleep(lk, &lk->lk) 释放内部自旋锁并休眠让出 CPU；
//     获得睡眠锁后立即释放内部自旋锁 lk->lk，使调用者能够在开中断状态下安全执行磁盘 I/O。
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

void
initsleeplock(struct sleeplock *lk, char *name)
{
  initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
}

void
acquiresleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  while(lk->locked) sleep(lk, &lk->lk);
  lk->locked=1; lk->pid=myproc()->pid;
  release(&lk->lk);
}

void
releasesleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  lk->locked=0; lk->pid=0; wakeup(lk);
  release(&lk->lk);
}

int
holdingsleep(struct sleeplock *lk)
{
  int r;

  acquire(&lk->lk);
  r = lk->locked && (lk->pid == myproc()->pid);
  release(&lk->lk);
  return r;
}
