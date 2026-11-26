// file.c —— 系统打开文件表管理与文件读写分发（lab6 阶段四支撑脚手架）
//
// 说明：本文件封装全局打开文件表 ftable[NFILE] 与 CONSOLE 设备读写转接。
// 其中 filewrite() 已内置与个性化参数 LAB6_BUF_SLOTS（8..16）及 LAB6_LOG_BLOCKS（24..39）
// 协同的分块写入逻辑，确保单笔子事务固定的脏块数不超过 LOG_PIN_CAP。
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "stat.h"
#include "proc.h"
#include "course_sid.h"

#define LOG_PIN_CAP   ((LAB6_BUF_SLOTS - 3 < LAB6_LOG_BLOCKS) ? (LAB6_BUF_SLOTS - 3) : LAB6_LOG_BLOCKS)
#define OP_MAX_BLOCKS ((LOG_PIN_CAP < MAXOPBLOCKS) ? LOG_PIN_CAP : MAXOPBLOCKS)

struct devsw devsw[NDEV];

struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable;

static int
console_dev_read(int user_dst, uint64 dst, int n)
{
  (void)user_dst;
  return consoleread(dst, n);
}

static int
console_dev_write(int user_src, uint64 src, int n)
{
  (void)user_src;
  return consolewrite(src, n);
}

void
fileinit(void)
{
  initlock(&ftable.lock, "ftable");
  devsw[CONSOLE].read = console_dev_read;
  devsw[CONSOLE].write = console_dev_write;
}

// 在全局打开文件表中分配一个空闲槽位（ref = 1）。
struct file *
filealloc(void)
{
  struct file *f;

  acquire(&ftable.lock);
  for (f = ftable.file; f < ftable.file + NFILE; f++) {
    if (f->ref == 0) {
      memset(f,0,sizeof(*f));
      f->ref = 1;
      release(&ftable.lock);
      return f;
    }
  }
  release(&ftable.lock);
  return 0;
}

// 递增文件结构体 f 的引用计数。
struct file *
filedup(struct file *f)
{
  acquire(&ftable.lock);
  if (f->ref < 1)
    panic("filedup");
  f->ref++;
  release(&ftable.lock);
  return f;
}

// 关闭文件 f（递减引用计数，归零时在事务内调用 iput 释放底层 inode）。
void
fileclose(struct file *f)
{
  struct file ff;

  acquire(&ftable.lock);
  if (f->ref < 1)
    panic("fileclose");
  if (--f->ref > 0) {
    release(&ftable.lock);
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  release(&ftable.lock);

  if (ff.type == FD_INODE || (ff.type == FD_DEVICE && ff.ip != 0)) {
    begin_op();
    iput(ff.ip);
    end_op();
  }
}

// 获取文件 f 的元数据并拷出至用户态虚拟地址 addr。
int
filestat(struct file *f, uint64 addr)
{
  struct proc *p = myproc();
  struct stat st;

  if (f->type == FD_INODE || (f->type == FD_DEVICE && f->ip != 0)) {
    ilock(f->ip);
    stati(f->ip, &st);
    iunlock(f->ip);
    if (copyout(p->pagetable, addr, (char *)&st, sizeof(st)) < 0)
      return -1;
    return 0;
  }
  return -1;
}

// 从文件 f 读取至多 n 字节到用户虚拟地址 addr。
int
fileread(struct file *f, uint64 addr, int n)
{
  int r = 0;

  if (f->readable == 0)
    return -1;

  if (f->type == FD_DEVICE) {
    if (f->major < 0 || f->major >= NDEV || !devsw[f->major].read)
      return -1;
    r = devsw[f->major].read(1, addr, n);
  } else if (f->type == FD_INODE) {
    ilock(f->ip);
    if ((r = readi(f->ip, 1, addr, f->off, n)) > 0)
      f->off += r;
    iunlock(f->ip);
  } else {
    panic("fileread");
  }

  return r;
}

// 向文件 f 写入来自用户虚拟地址 addr 的 n 字节数据。
// 当 LAB6_BUF_SLOTS == 8（OP_MAX_BLOCKS == 5）时，单笔子事务按块边界切分，至多修改 4 个磁盘块。
int
filewrite(struct file *f, uint64 addr, int n)
{
  int r, ret = 0;

  if (f->writable == 0)
    return -1;

  if (f->type == FD_DEVICE) {
    if (f->major < 0 || f->major >= NDEV || !devsw[f->major].write)
      return -1;
    ret = devsw[f->major].write(1, addr, n);
  } else if (f->type == FD_INODE) {
    int max = ((OP_MAX_BLOCKS - 1 - 1 - 2) / 2) * BSIZE;
    int i = 0;
    while (i < n) {
      int n1 = n - i;
      if (max > 0) {
        if (n1 > max)
          n1 = max;
      } else {
        int rem = BSIZE - (f->off % BSIZE);
        if (n1 > rem)
          n1 = rem;
      }

      begin_op();
      ilock(f->ip);
      if ((r = writei(f->ip, 1, addr + i, f->off, n1)) > 0)
        f->off += r;
      iunlock(f->ip);
      end_op();

      if (r != n1)
        break;
      i += r;
    }
    ret = (i == n ? n : -1);
  } else {
    panic("filewrite");
  }

  return ret;
}
