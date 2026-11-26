/* 预置支撑代码（请勿修改） —— 打开文件结构、内存 inode 结构与设备开关表
 */
#ifndef FILE_H
#define FILE_H

#include "types.h"
#include "sleeplock.h"
#include "fs.h"

struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE, FD_DEVICE } type;
  int ref;                     // 引用计数
  char readable;
  char writable;
  struct inode *ip;            // FD_INODE 和 FD_DEVICE
  uint off;                    // FD_INODE 当前读写字节偏移
  short major;                 // FD_DEVICE 主设备号
};

#define major(dev)  ((dev) >> 16 & 0xFFFF)
#define minor(dev)  ((dev) & 0xFFFF)
#define mkdev(m, n) ((uint)((m) << 16 | (n)))

// 内存中的活动 inode 副本
struct inode {
  uint dev;                    // 设备号
  uint inum;                   // Inode 编号
  int ref;                     // 内存引用计数（指针引用数）
  struct sleeplock lock;       // 保护下方有效性与磁盘副本字段的睡眠锁
  int valid;                   // 是否已从磁盘读入 dinode 内容

  short type;                  // 磁盘 dinode 副本字段
  short major;
  short minor;
  short nlink;
  uint size;
  uint addrs[NDIRECT + 1];
};

// 主设备号到设备读写函数的映射表
struct devsw {
  int (*read)(int, uint64, int);
  int (*write)(int, uint64, int);
};

extern struct devsw devsw[];

#define CONSOLE 1

#endif
