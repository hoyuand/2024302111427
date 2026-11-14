/* Locally authored Lab5 runtime checks; course tests remain byte exact. */
#include "kernel/types.h"
#include "kernel/course_sid.h"
#include "user/user.h"
#define PAGE 4096
static void check(int ok, const char *msg) {
  if (!ok) { printf("COWTEST FAIL: %s\n", msg); exit(1); }
}
static void budget(void) {
  int n = LAB5_COW_BUDGET + 3;
  volatile char *p = sbrk(n * PAGE);
  check(p != (void *)-1, "budget alloc");
  for (int i=0; i<n; i++) p[i*PAGE] = 0x31;
  int pid = fork(); check(pid >= 0, "budget fork");
  if (!pid) { pause(10000); exit(0); }
  for (int i=0; i<n; i++) p[i*PAGE] = 0x52;
  check(kill(pid) == 0, "budget kill");
  check(wait(0) == pid, "budget reap");
  for (int i=0; i<n; i++) check(p[i*PAGE] == 0x52, "budget isolation");
  check(sbrk(-n*PAGE) != (void *)-1, "budget shrink");
  check(pmc(1) == 0, "budget shared cleanup");
  printf("COWTEST budget copies >= %d, continued\n", n);
}
int __attribute__((section(".text.aaa"))) main(int argc, char **argv) {
  if (argc == 2 && !strcmp(argv[1], "reset")) {
    budget();
    printf("COWTEST PASS: exec reset and budget recovery\n"); exit(0);
  }
  check(pmc(-1) == (uint64)-1 && pmc(2) == (uint64)-1, "pmc invalid");
  char *p = sbrk(2*PAGE); check(p != (void *)-1, "two pages");
  memset(p, 0x11, 2*PAGE);
  int ids[3];
  for (int j=0; j<3; j++) {
    ids[j] = fork(); check(ids[j] >= 0, "three way fork");
    if (!ids[j]) {
      for (int k=PAGE-2; k<PAGE+2; k++) p[k] = 0x20+j;
      pause(1);
      for (int k=PAGE-2; k<PAGE+2; k++) check(p[k] == 0x20+j, "child cross page");
      exit(0);
    }
  }
  check(pmc(1) > 0, "multiway shared observation");
  for (int j=0; j<3; j++) { int st=-9; check(wait(&st)>0 && !st, "multiway reap"); }
  for (int k=PAGE-2; k<PAGE+2; k++) check(p[k] == 0x11, "parent cross page");
  check(pmc(1) == 0, "multiway shared cleanup");
  int pid = fork(); check(pid >= 0, "copyout fork");
  if (!pid) { pause(2); check(p[PAGE-2]==0x11 && p[PAGE]==0x11, "copyout isolation"); exit(37); }
  int cpu, slice;
  check(schedstat(pid,&cpu,&slice)==0, "schedstat COW copyout");
  check(wait((int *)(p+PAGE-2))==pid, "cross page wait copyout");
  int st; memmove(&st,p+PAGE-2,sizeof(st)); check(st==37, "cross page status");
  check(pmc(1)==0, "copyout shared cleanup");
  pid=fork(); check(pid>=0,"readonly fork");
  if (!pid) { *(volatile char *)0=9; exit(88); }
  check(wait(&st)==pid && st==-1,"readonly killed");
  pid=fork(); check(pid>=0,"guard fork");
  if (!pid) { *(volatile char *)(p-PAGE-1)=9; exit(88); }
  check(wait(&st)==pid && st==-1,"guard killed");
  char *ok[]={"echo","recovered",0};
  char *bad[]={ (char *)0x3ffffff000UL,0};
  check(exec("missing",ok)==-1,"missing program");
  check(exec("echo",(char **)0x3ffffff000UL)==-1,"bad argv vector");
  check(exec("echo",bad)==-1,"bad argv string");
  check(exec((char *)0x3ffffff000UL,ok)==-1,"bad path");
  char *many[33]; for(int i=0;i<33;i++) many[i]="a";
  check(exec("echo",many)==-1,"unterminated maxarg");
  memset(p,'x',2*PAGE);
  char *longarg[]={p,0};
  check(exec("echo",longarg)==-1,"unterminated argument");
  p[PAGE-1]=0;
  check(exec("echo",longarg)==-1,"stack overflow rollback");
  check(p[0]=='x',"old address space preserved");
  pid=fork(); check(pid>=0,"exec recovery fork");
  if (!pid) { exec("echo",ok); exit(88); }
  check(wait(&st)==pid && st==0,"exec recovery");
  check(sbrk(-2*PAGE)!=(void *)-1,"two page shrink");
  check(pmc(1)==0,"error shared cleanup");
  printf("COWTEST PASS: cross-page multiway copyout readonly guard argv rollback\n");
  budget();
  char *again[]={"cowtest","reset",0};
  exec("cowtest",again);
  check(0,"budget exec");
  exit(1);
}
