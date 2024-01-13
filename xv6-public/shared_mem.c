#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

#define NUM_OF_SHARED_PAGES 8


struct shared_page{
    int id;
    int num_of_refs;
    char* frame;
};

struct{
    struct shared_page table[NUM_OF_SHARED_PAGES];
    struct spinlock lock;
} shared_memory;

void
init_shared_mem(){
    acquire(&shared_memory.lock);
    for (int i = 0; i < NUM_OF_SHARED_PAGES; i++){
        shared_memory.table[i].num_of_refs = 0;
    }
    release(&shared_memory.lock);
}

char*
open_shared_mem(int id){
    struct proc* proc = myproc();
    pde_t *pgdir = proc->pgdir;
    //uint shm = proc->shm;
    //cprintf("shm: ")
    acquire(&shared_memory.lock);
    int index = id;
    if (shared_memory.table[index].num_of_refs == 0){
        shared_memory.table[index].frame = kalloc();
        memset(shared_memory.table[index].frame, 0, PGSIZE);
    }
    char* start_mem = (char*)PGROUNDUP(proc->sz);
    mappages(pgdir, start_mem, PGSIZE, V2P(shared_memory.table[index].frame), PTE_W|PTE_U);
    shared_memory.table[index].num_of_refs++;
    proc->shm = start_mem;

    release(&shared_memory.lock);
    return start_mem;

}

void
close_shared_mem(int id){
    struct proc* proc = myproc();
    pde_t *pgdir = proc->pgdir;
    acquire(&shared_memory.lock);
    int index = id;
    shared_memory.table[index].num_of_refs--;

    uint a = PGROUNDUP((uint)proc->shm);
    pte_t *pte = walkpgdir(pgdir, (char*)a, 0);
    if(!pte)
    a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
    else if((*pte & PTE_P) != 0){
        uint pa = PTE_ADDR(*pte);
        if(pa == 0)
            panic("kfree");
        *pte = 0;
    }
    
    if (shared_memory.table[index].num_of_refs == 0){
        kfree(shared_memory.table[index].frame);
        
    }


        
    
    release(&shared_memory.lock);
}