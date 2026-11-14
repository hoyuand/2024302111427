/* Exercise real allocation failures, including partial COW fork rollback. */
#include "kernel/types.h"
#include "user/user.h"
int main(void) {
  volatile char *shared=sbrk(4096);
  if(shared==(void *)-1) { printf("COWOOM FAIL: initial alloc\n"); exit(1); }
  shared[0]=7;
  int pid=fork();
  if(pid<0) { printf("COWOOM FAIL: initial fork\n"); exit(1); }
  if(!pid) {
    int pages=0;
    while(sbrk(4096)!=(void *)-1) pages++;
    if(pages<100 || sbrk(-6*4096)==(void *)-1) { printf("COWOOM FAIL: exhaustion\n"); exit(1); }
    int failed=fork();
    if(!failed) exit(99);
    if(failed!=-1) { printf("COWOOM FAIL: fork should exhaust page tables\n"); exit(1); }
    printf("COWOOM partial fork rollback returned -1\n");
    while(sbrk(4096)!=(void *)-1) {}
    printf("COWOOM exhausted: next shared store must be killed\n");
    shared[0]=9;
    printf("COWOOM FAIL: OOM store survived\n"); exit(1);
  }
  /* A status copyout itself could need a page at OOM; wait(0) can always
   * reap and reclaim the child's complete address space without one. */
  if(wait(0)!=pid || shared[0]!=7 || pmc(1)!=0 || sbrk(4096)==(void *)-1) {
    printf("COWOOM FAIL: reclaim/isolation\n"); exit(1);
  }
  printf("COWOOM PASS: partial fork rollback, store OOM killed, pool recovered\n");
  exit(0);
}
