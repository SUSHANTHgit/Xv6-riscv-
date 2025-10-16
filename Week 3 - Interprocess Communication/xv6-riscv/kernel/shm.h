#include "types.h"
#include "spinlock.h"

#define NULL ((void*)0)

struct mem_reg{
  void *pa;
  int no_of_process;
  struct spinlock lock;
};
#define physical_pages 1000
extern struct mem_reg gt[physical_pages];

// shm.c
void shm_init(void);
int shm_create(int);
void* shm_get(int);
int shm_close(int);