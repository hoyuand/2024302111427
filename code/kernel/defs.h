#ifndef LAB6_DEFS_H
#define LAB6_DEFS_H
#include "types.h"
#include "riscv.h"
#include "lab5.h"
struct proc; struct spinlock; struct sleeplock; struct buf; struct superblock; struct inode; struct file; struct stat;
void printf(const char*,...); void panic(const char*) __attribute__((noreturn));
void *memset(void*,int,uint); void *memmove(void*,const void*,uint);
void initlock(struct spinlock*,char*);void acquire(struct spinlock*);void release(struct spinlock*);int holding(struct spinlock*);
struct proc *myproc(void);void sleep(void*,struct spinlock*);void wakeup(void*);
int argint(int);uint64 argaddr(int);int argstr(int,char*,int);
void initsleeplock(struct sleeplock*,char*);void acquiresleep(struct sleeplock*);void releasesleep(struct sleeplock*);int holdingsleep(struct sleeplock*);
void binit(void);struct buf *bread(uint,uint);void bwrite(struct buf*);void brelse(struct buf*);void bpin(struct buf*);void bunpin(struct buf*);
void virtio_disk_init(void);void virtio_disk_rw(struct buf*,int);void virtio_disk_intr(void);
void initlog(int,struct superblock*);void begin_op(void);void end_op(void);void log_write(struct buf*);
void crash_at(int);int crash_should_fire(int);
void iinit(void);void fsinit(int);struct inode *ialloc(uint,short);void iupdate(struct inode*);struct inode *idup(struct inode*);void ilock(struct inode*);void iunlock(struct inode*);void iput(struct inode*);void iunlockput(struct inode*);void itrunc(struct inode*);void stati(struct inode*,struct stat*);
int readi(struct inode*,int,uint64,uint,uint);int writei(struct inode*,int,uint64,uint,uint);
int namecmp(const char*,const char*);struct inode *dirlookup(struct inode*,char*,uint*);int dirlink(struct inode*,char*,uint);struct inode *namei(char*);struct inode *nameiparent(char*,char*);
void fileinit(void);struct file *filealloc(void);struct file *filedup(struct file*);void fileclose(struct file*);int filestat(struct file*,uint64);int fileread(struct file*,uint64,int);int filewrite(struct file*,uint64,int);
int consoleread(uint64,int);int consolewrite(uint64,int);
void bio_selftest(void);void log_selftest(void);
#endif
