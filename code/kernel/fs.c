// fs.c —— 文件系统核心实现脚手架（lab6 阶段四）
//
// 本文件已预置超级块读取（readsb/fsinit）、空闲块位图扫描（balloc/bfree）、
// 内存 inode 表基础维护（iinit/ialloc/iupdate/iget/idup/ilock/iunlock/readi/writei）及路径分词（skipelem）。
// 请重点完成以下 5 处核心机制（检索 TODO(lab6-stage4)）：
//   1. bmap()      ：直接块（0..11）与一级间接块（12..267）映射及按需分配；
//   2. itrunc()    ：释放文件的全部直接块、一级间接块内数据块及间接块自身；
//   3. iput()      ：当 ref == 1 && valid && nlink == 0 时截断并回收磁盘 dinode；
//   4. dirlookup() 与 dirlink()：目录项遍历查找与新目录项写入；
//   5. namex()     ：逐级路径解析（兼容 namei 与 nameiparent）。
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"
#include "file.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

struct superblock sb;

static void
readsb(int dev, struct superblock *sbp)
{
  struct buf *bp = bread(dev, 1);
  memmove(sbp, bp->data, sizeof(*sbp));
  brelse(bp);
}

// 初始化文件系统（必须在首个进程的 forkret 上下文中调用，不可在 main 关中断阶段直接调用）。
void
fsinit(int dev)
{
  readsb(dev, &sb);
  if (sb.magic != FSMAGIC)
    panic("invalid file system");
  initlog(dev, &sb);
}

static void
bzero(int dev, int bno)
{
  struct buf *bp = bread(dev, bno);
  memset(bp->data, 0, BSIZE);
  log_write(bp);
  brelse(bp);
}

// ---------------- 数据块位图分配与回收 ----------------

static uint
balloc(uint dev)
{
  for (int b = 0; b < (int)sb.size; b += BPB) {
    struct buf *bp = bread(dev, BBLOCK(b, sb));
    for (int bi = 0; bi < BPB && b + bi < (int)sb.size; bi++) {
      int m = 1 << (bi % 8);
      if ((bp->data[bi / 8] & m) == 0) {
        bp->data[bi / 8] |= m;
        log_write(bp);
        brelse(bp);
        bzero(dev, b + bi);
        return b + bi;
      }
    }
    brelse(bp);
  }
  printf("balloc: out of blocks\n");
  return 0;
}

static void
bfree(int dev, uint b)
{
  struct buf *bp = bread(dev, BBLOCK(b, sb));
  int bi = b % BPB;
  int m = 1 << (bi % 8);
  if ((bp->data[bi / 8] & m) == 0)
    panic("freeing free block");
  bp->data[bi / 8] &= ~m;
  log_write(bp);
  brelse(bp);
}

// ---------------- Inode 缓存、生命周期与块地址映射 ----------------

struct {
  struct spinlock lock;
  struct inode inode[NINODE];
} itable;

void
iinit(void)
{
  initlock(&itable.lock, "itable");
  for (int i = 0; i < NINODE; i++) {
    initsleeplock(&itable.inode[i].lock, "inode");
  }
}

static struct inode *iget(uint dev, uint inum);

struct inode *
ialloc(uint dev, short type)
{
  for (uint inum = 1; inum < sb.ninodes; inum++) {
    struct buf *bp = bread(dev, IBLOCK(inum, sb));
    struct dinode *dip = (struct dinode *)bp->data + inum % IPB;
    if (dip->type == 0) {
      memset(dip, 0, sizeof(*dip));
      dip->type = type;
      log_write(bp);
      brelse(bp);
      return iget(dev, inum);
    }
    brelse(bp);
  }
  printf("ialloc: no inodes\n");
  return 0;
}

void
iupdate(struct inode *ip)
{
  struct buf *bp = bread(ip->dev, IBLOCK(ip->inum, sb));
  struct dinode *dip = (struct dinode *)bp->data + ip->inum % IPB;
  dip->type = ip->type;
  dip->major = ip->major;
  dip->minor = ip->minor;
  dip->nlink = ip->nlink;
  dip->size = ip->size;
  memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
  log_write(bp);
  brelse(bp);
}

static struct inode *
iget(uint dev, uint inum)
{
  struct inode *ip, *empty = 0;

  acquire(&itable.lock);
  for (ip = &itable.inode[0]; ip < &itable.inode[NINODE]; ip++) {
    if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
      ip->ref++;
      release(&itable.lock);
      return ip;
    }
    if (empty == 0 && ip->ref == 0)
      empty = ip;
  }

  if (empty == 0)
    panic("iget: no inodes");

  ip = empty;
  ip->dev = dev;
  ip->inum = inum;
  ip->ref = 1;
  ip->valid = 0;
  release(&itable.lock);

  return ip;
}

struct inode *
idup(struct inode *ip)
{
  acquire(&itable.lock);
  ip->ref++;
  release(&itable.lock);
  return ip;
}

void
ilock(struct inode *ip)
{
  if (ip == 0 || ip->ref < 1)
    panic("ilock");

  acquiresleep(&ip->lock);
  if (ip->valid == 0) {
    struct buf *bp = bread(ip->dev, IBLOCK(ip->inum, sb));
    struct dinode *dip = (struct dinode *)bp->data + ip->inum % IPB;
    ip->type = dip->type;
    ip->major = dip->major;
    ip->minor = dip->minor;
    ip->nlink = dip->nlink;
    ip->size = dip->size;
    memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
    brelse(bp);
    ip->valid = 1;
    if (ip->type == 0)
      panic("ilock: no type");
  }
}

void
iunlock(struct inode *ip)
{
  if (ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
    panic("iunlock");
  releasesleep(&ip->lock);
}

void
iput(struct inode *ip)
{
  acquire(&itable.lock);
  if(ip->ref==1 && ip->valid && ip->nlink==0){
    acquiresleep(&ip->lock); release(&itable.lock);
    itrunc(ip); ip->type=0; iupdate(ip); ip->valid=0;
    releasesleep(&ip->lock); acquire(&itable.lock);
  }
  ip->ref--; release(&itable.lock);
}

void
iunlockput(struct inode *ip)
{
  iunlock(ip);
  iput(ip);
}

static uint
bmap(struct inode *ip, uint bn)
{
  uint addr;
  if(bn<NDIRECT){
    if(!ip->addrs[bn]) ip->addrs[bn]=balloc(ip->dev);
    return ip->addrs[bn];
  }
  bn-=NDIRECT;
  if(bn>=NINDIRECT) return 0;
  if(!ip->addrs[NDIRECT]) ip->addrs[NDIRECT]=balloc(ip->dev);
  if(!ip->addrs[NDIRECT]) return 0;
  struct buf *bp=bread(ip->dev,ip->addrs[NDIRECT]);
  uint *a=(uint*)bp->data;
  if(!a[bn]){a[bn]=balloc(ip->dev);log_write(bp);}
  addr=a[bn];brelse(bp);return addr;
}

void
itrunc(struct inode *ip)
{
  for(int i=0;i<NDIRECT;i++) if(ip->addrs[i]){bfree(ip->dev,ip->addrs[i]);ip->addrs[i]=0;}
  if(ip->addrs[NDIRECT]){
    struct buf *bp=bread(ip->dev,ip->addrs[NDIRECT]);uint *a=(uint*)bp->data;
    for(int i=0;i<NINDIRECT;i++) if(a[i]) bfree(ip->dev,a[i]);
    brelse(bp);bfree(ip->dev,ip->addrs[NDIRECT]);ip->addrs[NDIRECT]=0;
  }
  ip->size=0;iupdate(ip);
}

void
stati(struct inode *ip, struct stat *st)
{
  st->dev = ip->dev;
  st->ino = ip->inum;
  st->type = ip->type;
  st->nlink = ip->nlink;
  st->size = ip->size;
}

int
readi(struct inode *ip, int user_dst, uint64 dst, uint off, uint n)
{
  uint tot, m;
  struct buf *bp;

  if (off > ip->size || off + n < off)
    return 0;
  if (off + n > ip->size)
    n = ip->size - off;

  for (tot = 0; tot < n; tot += m, off += m, dst += m) {
    uint addr = bmap(ip, off / BSIZE);
    if (addr == 0)
      break;
    bp = bread(ip->dev, addr);
    m = min(n - tot, BSIZE - off % BSIZE);
    if (user_dst) {
      if (copyout(myproc()->pagetable, dst, (char *)(bp->data + (off % BSIZE)), m) < 0) {
        brelse(bp);
        tot = -1;
        break;
      }
    } else {
      memmove((char *)dst, bp->data + (off % BSIZE), m);
    }
    brelse(bp);
  }
  return (int)tot;
}

int
writei(struct inode *ip, int user_src, uint64 src, uint off, uint n)
{
  uint tot, m;
  struct buf *bp;

  if (off > ip->size || off + n < off)
    return -1;
  if (off + n > MAXFILE * BSIZE)
    return -1;

  for (tot = 0; tot < n; tot += m, off += m, src += m) {
    uint addr = bmap(ip, off / BSIZE);
    if (addr == 0)
      break;
    bp = bread(ip->dev, addr);
    m = min(n - tot, BSIZE - off % BSIZE);
    if (user_src) {
      if (copyin(myproc()->pagetable, (char *)(bp->data + (off % BSIZE)), src, m) < 0) {
        brelse(bp);
        break;
      }
    } else {
      memmove(bp->data + (off % BSIZE), (char *)src, m);
    }
    log_write(bp);
    brelse(bp);
  }

  if (off > ip->size)
    ip->size = off;

  iupdate(ip);
  return (int)tot;
}

// ---------------- 目录查找、链接与路径解析 ----------------

int
namecmp(const char *s, const char *t)
{
  uint n = DIRSIZ;
  while (n > 0 && *s && *s == *t) {
    n--;
    s++;
    t++;
  }
  if (n == 0)
    return 0;
  return (uchar)*s - (uchar)*t;
}

struct inode *
dirlookup(struct inode *dp, char *name, uint *poff)
{
  struct dirent de;
  if(dp->type!=T_DIR) panic("dirlookup type");
  for(uint off=0;off<dp->size;off+=sizeof(de)){
    if(readi(dp,0,(uint64)&de,off,sizeof(de))!=sizeof(de)) panic("dirlookup read");
    if(de.inum && namecmp(name,de.name)==0){if(poff)*poff=off;return iget(dp->dev,de.inum);}
  }
  return 0;
}

int
dirlink(struct inode *dp, char *name, uint inum)
{
  struct inode *ip=dirlookup(dp,name,0);if(ip){iput(ip);return -1;}
  struct dirent de;uint off;
  for(off=0;off<dp->size;off+=sizeof(de)){
    if(readi(dp,0,(uint64)&de,off,sizeof(de))!=sizeof(de)) return -1;
    if(!de.inum)break;
  }
  memset(&de,0,sizeof(de));for(int i=0;i<DIRSIZ && name[i];i++)de.name[i]=name[i];
  de.inum=inum;return writei(dp,0,(uint64)&de,off,sizeof(de))==sizeof(de)?0:-1;
}

static char *
skipelem(char *path, char *name)
{
  while (*path == '/')
    path++;
  if (*path == 0)
    return 0;
  char *s = path;
  while (*path != '/' && *path != 0)
    path++;
  int len = path - s;
  if (len >= DIRSIZ)
    memmove(name, s, DIRSIZ);
  else {
    memmove(name, s, len);
    name[len] = 0;
  }
  while (*path == '/')
    path++;
  return path;
}

static struct inode *
namex(char *path, int nameiparent, char *name)
{
  struct inode *ip=(*path=='/' || !myproc()->cwd)?iget(ROOTDEV,ROOTINO):idup(myproc()->cwd);
  while((path=skipelem(path,name))!=0){
    ilock(ip);if(ip->type!=T_DIR){iunlockput(ip);return 0;}
    if(nameiparent && !*path){iunlock(ip);return ip;}
    struct inode *next=dirlookup(ip,name,0);iunlockput(ip);
    if(!next)return 0;
    ip=next;
  }
  if(nameiparent){iput(ip);return 0;}return ip;
}

struct inode *
namei(char *path)
{
  char name[DIRSIZ];
  return namex(path, 0, name);
}

struct inode *
nameiparent(char *path, char *name)
{
  return namex(path, 1, name);
}
