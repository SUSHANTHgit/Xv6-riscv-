#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "shm.h"

#define physical_pages 1000
struct mem_reg gt[physical_pages];

void shm_init(){
    for(int i=0;i<physical_pages;i++){
        gt[i].no_of_process=0;
        gt[i].pa=NULL;
        initlock(&gt[i].lock,"key_lock");
    }
}
int shm_create(int key){
    if(gt[key].pa!=NULL){
        return key;
    }
    gt[key].pa=kalloc();
    if(gt[key].pa==0){
        return -1;
    }
    memset(gt[key].pa,0,PGSIZE);
    return key;
}

void *shm_get(int key){
 uint64 shared_va = 0x40000000-((key+1)*PGSIZE);
 acquire(&gt[key].lock);
if(shm_create(key)==-1){
    release(&gt[key].lock);
    return NULL;
}
if( mappages(myproc()->pagetable,shared_va,PGSIZE,(uint64)gt[key].pa,PTE_W | PTE_R | PTE_U)==-1)printf("mapping failed");
 gt[key].no_of_process++;
 release(&gt[key].lock);
 return (void*)shared_va;
}

int shm_close(int key){
    acquire(&gt[key].lock);
    if(gt[key].no_of_process==0){
        release(&gt[key].lock);
        return 1;
    }
    if(gt[key].no_of_process>1){
        gt[key].no_of_process--;
        uvmunmap(myproc()->pagetable, 0x40000000-((key+1)*PGSIZE), 1, 0);
        release(&gt[key].lock);
        return 1;
    }
    else{
        gt[key].no_of_process--;
        uvmunmap(myproc()->pagetable, 0x40000000-((key+1)*PGSIZE), 1, 1);
        gt[key].pa=NULL;
        release(&gt[key].lock);
        return 1;
    }
}