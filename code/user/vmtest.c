/* Memory lifecycle regression, independent of course counter tests. */
#include "kernel/types.h"
#include "user/user.h"
int main(void) {
  for (int round=0; round<12; round++) {
    char *p=sbrk(16*4096);
    if (p==(void *)-1) { printf("VMTEST FAIL: alloc\n"); exit(1); }
    for(int i=0;i<16;i++) p[i*4096]=round;
    int pid=fork();
    if(pid<0) { printf("VMTEST FAIL: fork\n"); exit(1); }
    if(!pid) {
      for(int i=0;i<16;i++) if(p[i*4096]!=round) exit(1);
      for(int i=0;i<16;i++) p[i*4096]=99;
      exit(0);
    }
    int st=-1;
    if(wait(&st)!=pid || st) { printf("VMTEST FAIL: wait\n"); exit(1); }
    for(int i=0;i<16;i++) if(p[i*4096]!=round) { printf("VMTEST FAIL: isolation\n"); exit(1); }
    if(sbrk(-16*4096)==(void *)-1 || pmc(1)) { printf("VMTEST FAIL: cleanup\n"); exit(1); }
  }
  printf("VMTEST PASS: 12 allocate-fork-write-wait-shrink cycles shared=0\n");
  exit(0);
}
