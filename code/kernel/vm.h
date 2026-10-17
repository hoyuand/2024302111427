#ifndef LAB3_VM_H
#define LAB3_VM_H

#include "types.h"
#include "riscv.h"
#include "course_sid.h"

#define SBRK_EAGER 1
#define SBRK_LAZY 2

struct proc;

void kinit(void);
void *kalloc(void);
void kfree(void *pa);
uint kfreecount(void);

void kvminit(void);
void kvminithart(void);
uint64 kvmsatp(void);
pagetable_t uvmcreate(struct proc *p);
int uvmmap(pagetable_t pagetable, uint64 va, uint64 pa, uint64 size, int perm);
void uvmfree(pagetable_t pagetable, uint64 sz);
int uvmcopy(pagetable_t old, pagetable_t new, uint64 sz);
int uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int perm);
uint64 uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz);
uint64 walkaddr(pagetable_t pagetable, uint64 va, int write);
pte_t *vm_walk(pagetable_t pagetable, uint64 va, int alloc);
int copyin(pagetable_t pagetable, void *dst, uint64 srcva, uint64 len);
int copyout(pagetable_t pagetable, uint64 dstva, const void *src, uint64 len);
void dump_pagetable(pagetable_t pagetable);

#endif
