#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
#include "console.h"
void initlock(struct spinlock *l,char *name){l->locked=0;l->name=name;l->cpu=0;}
struct proc *myproc(void){return current_proc;}
void sleep(void *c,struct spinlock *l){proc_sleep(c,l);}void wakeup(void *c){proc_wakeup(c);}
int argint(int n){return (int)argaddr(n);}uint64 argaddr(int n){struct trapframe *t=&current_proc->tf;return n==0?t->a0:n==1?t->a1:n==2?t->a2:0;}
int argstr(int n,char *s,int max){return copyinstr(current_proc->pagetable,s,argaddr(n),max);}
int consoleread(uint64 dst,int n){char buf[128];consoleintr();int r=console_read(buf,n>128?128:n);return r>0 && copyout(current_proc->pagetable,dst,buf,r)<0?-1:r;}
int consolewrite(uint64 src,int n){for(int i=0;i<n;i++){char c;consoleintr();if(copyin(current_proc->pagetable,&c,src+i,1)<0)return -1;console_raw_putc(c);}return n;}

void *memset(void *dst,int c,uint n){uchar *p=dst;for(uint i=0;i<n;i++)p[i]=c;return dst;}
void *memmove(void *dst,const void *src,uint n){uchar *d=dst;const uchar *s=src;if(s<d && d<s+n){for(uint i=n;i;i--)d[i-1]=s[i-1];}else for(uint i=0;i<n;i++)d[i]=s[i];return dst;}
void panic(const char *s){intr_off();printf("panic: %s\n",s);for(;;)asm volatile("wfi");}
