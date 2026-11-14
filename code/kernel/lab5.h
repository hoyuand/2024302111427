#ifndef LAB5_H
#define LAB5_H
#include "vm.h"

#define PTE_COW (1L << 8)
#define PTE_RSW1 (1L << 9)

void kref_inc(uint64 pa);
uint kref_get(uint64 pa);
uint64 kshared_count(void);
uint64 kcow_copy_count(void);
void kcow_record_copy(void);
void kref_selftest(void);
int cow_alloc(pagetable_t pt, uint64 va);
int uvm_check_range(pagetable_t pt, uint64 va, uint64 len, int want);
int copyinstr(pagetable_t pt, char *dst, uint64 va, uint64 max);

#endif
