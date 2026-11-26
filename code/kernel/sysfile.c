// sysfile.c —— 文件描述符与目录系统调用脚手架（lab6 阶段四）
//
// 本文件已预置 fd 分配与基础包装（argfd/fdalloc/sys_dup/sys_read/sys_write/sys_close/
// sys_fstat/isdirempty/create/sys_mkdir/sys_mknod/sys_chdir）。
// 请重点完成以下 3 个核心系统调用（检索 TODO(lab6-stage4)），并在 proc.c 中实现 ELF kexec：
//   1. sys_open()   ：事务边界控制、O_CREATE 分支与普通打开分支、分配 struct file 与 fd、处理 O_TRUNC；
//   2. sys_link()   ：在单笔事务内递增目标 inode 的 nlink 并在新父目录写入目录项（失败时回滚 nlink）；
//   3. sys_unlink() ：禁止删除 "." 与 ".." 及非空目录，清空父目录项并递减目标 nlink。
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"

static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd = argint(n);
  struct file *f;

  if (fd < 0 || fd >= NOFILE || (f = myproc()->ofile[fd]) == 0)
    return -1;
  if (pfd)
    *pfd = fd;
  if (pf)
    *pf = f;
  return 0;
}

static int
fdalloc(struct file *f)
{
  struct proc *p = myproc();

  for (int fd = 0; fd < NOFILE; fd++) {
    if (p->ofile[fd] == 0) {
      p->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

uint64
sys_dup(void)
{
  struct file *f;
  int fd;

  if (argfd(0, 0, &f) < 0)
    return -1;
  if ((fd = fdalloc(f)) < 0)
    return -1;
  filedup(f);
  return fd;
}

uint64
sys_read(void)
{
  struct file *f;
  int n = argint(2);
  uint64 p = argaddr(1);

  if (n < 0 || argfd(0, 0, &f) < 0)
    return -1;
  if (uvm_check_range(myproc()->pagetable, p, (uint64)n, PTE_R | PTE_W) < 0)
    return -1;
  return fileread(f, p, n);
}

uint64
sys_write(void)
{
  struct file *f;
  int n = argint(2);
  uint64 p = argaddr(1);

  if (n < 0 || argfd(0, 0, &f) < 0)
    return -1;
  if (uvm_check_range(myproc()->pagetable, p, (uint64)n, PTE_R) < 0)
    return -1;
  return filewrite(f, p, n);
}

uint64
sys_close(void)
{
  int fd;
  struct file *f;

  if (argfd(0, &fd, &f) < 0)
    return -1;
  myproc()->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

uint64
sys_fstat(void)
{
  struct file *f;
  uint64 st = argaddr(1);

  if (argfd(0, 0, &f) < 0)
    return -1;
  return filestat(f, st);
}

static int
isdirempty(struct inode *dp)
{
  struct dirent de;

  for (uint off = 2 * sizeof(de); off < dp->size; off += sizeof(de)) {
    if (readi(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if (de.inum != 0)
      return 0;
  }
  return 1;
}

static struct inode *
create(char *path, short type, short major, short minor)
{
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if ((dp = nameiparent(path, name)) == 0)
    return 0;

  ilock(dp);
  if ((ip = dirlookup(dp, name, 0)) != 0) {
    iunlockput(dp);
    ilock(ip);
    if (type == T_FILE && (ip->type == T_FILE || ip->type == T_DEVICE))
      return ip;
    iunlockput(ip);
    return 0;
  }

  if ((ip = ialloc(dp->dev, type)) == 0) {
    iunlockput(dp);
    return 0;
  }

  ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  iupdate(ip);

  if (type == T_DIR) {
    if (dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      goto fail;

  }

  if (dirlink(dp, name, ip->inum) < 0)
    goto fail;

  if(type==T_DIR){dp->nlink++;iupdate(dp);}
  iunlockput(dp);
  return ip;

fail:
  ip->nlink = 0;
  iupdate(ip);
  iunlockput(ip);
  iunlockput(dp);
  return 0;
}

uint64
sys_link(void)
{
  char old[MAXPATH],new[MAXPATH],name[DIRSIZ];struct inode *ip,*dp;
  if(argstr(0,old,MAXPATH)<0 || argstr(1,new,MAXPATH)<0)return -1;
  begin_op();ip=namei(old);if(!ip){end_op();return -1;}
  ilock(ip);if(ip->type==T_DIR){iunlockput(ip);end_op();return -1;}
  ip->nlink++;iupdate(ip);iunlock(ip);
  dp=nameiparent(new,name);
  if(dp){ilock(dp);if(dp->dev==ip->dev && dirlink(dp,name,ip->inum)==0){iunlockput(dp);iput(ip);end_op();return 0;}iunlockput(dp);}
  ilock(ip);ip->nlink--;iupdate(ip);iunlockput(ip);end_op();return -1;
}

uint64
sys_unlink(void)
{
  char path[MAXPATH],name[DIRSIZ];struct inode *dp,*ip;uint off;struct dirent de;
  if(argstr(0,path,MAXPATH)<0)return -1;
  begin_op();dp=nameiparent(path,name);if(!dp){end_op();return -1;}
  ilock(dp);
  if(namecmp(name,".")==0 || namecmp(name,"..")==0)goto bad;
  ip=dirlookup(dp,name,&off);if(!ip)goto bad;
  ilock(ip);if(ip->type==T_DIR && !isdirempty(ip)){iunlockput(ip);goto bad;}
  memset(&de,0,sizeof(de));
  if(writei(dp,0,(uint64)&de,off,sizeof(de))!=sizeof(de))panic("unlink write");
  if(ip->type==T_DIR){dp->nlink--;iupdate(dp);}
  iunlockput(dp);ip->nlink--;iupdate(ip);iunlockput(ip);end_op();return 0;
 bad: iunlockput(dp);end_op();return -1;
}

uint64
sys_open(void)
{
  char path[MAXPATH];int mode=argint(1),fd;struct inode *ip;struct file *f;
  if(argstr(0,path,MAXPATH)<0)return -1;
  begin_op();
  if(mode&O_CREATE)ip=create(path,T_FILE,0,0);
  else {ip=namei(path);if(ip)ilock(ip);}
  if(!ip){end_op();return -1;}
  if((ip->type==T_DIR && mode!=O_RDONLY) || (ip->type==T_DEVICE && (ip->major<0 || ip->major>=NDEV))){iunlockput(ip);end_op();return -1;}
  f=filealloc();
  if(!f){iunlockput(ip);end_op();return -1;}
  fd=fdalloc(f);if(fd<0){fileclose(f);iunlockput(ip);end_op();return -1;}
  f->type=ip->type==T_DEVICE?FD_DEVICE:FD_INODE;f->major=ip->major;f->ip=ip;f->off=0;
  f->readable=!(mode&O_WRONLY);f->writable=(mode&O_WRONLY)||(mode&O_RDWR);
  if((mode&O_TRUNC) && ip->type==T_FILE)itrunc(ip);
  iunlock(ip);end_op();return fd;
}

uint64
sys_mkdir(void)
{
  char path[MAXPATH];
  struct inode *ip;

  begin_op();
  if (argstr(0, path, MAXPATH) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0) {
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

uint64
sys_mknod(void)
{
  struct inode *ip;
  char path[MAXPATH];
  int major = argint(1);
  int minor = argint(2);

  begin_op();
  if (argstr(0, path, MAXPATH) < 0 ||
      (ip = create(path, T_DEVICE, major, minor)) == 0) {
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

uint64
sys_chdir(void)
{
  char path[MAXPATH];
  struct inode *ip;
  struct proc *p = myproc();

  begin_op();
  if (argstr(0, path, MAXPATH) < 0 || (ip = namei(path)) == 0) {
    end_op();
    return -1;
  }
  ilock(ip);
  if (ip->type != T_DIR) {
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  if (p->cwd)
    iput(p->cwd);
  end_op();
  p->cwd = ip;
  return 0;
}
