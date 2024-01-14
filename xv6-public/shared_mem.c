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

int
find_shared_page(int id){
    for (int i = 0; i < NUM_OF_SHARED_PAGES; i++){
        if (shared_memory.table[i].id == id){
            return i;
        }
    }
    for (int i = 0; i < NUM_OF_SHARED_PAGES; i++){
        if (shared_memory.table[i].num_of_refs == 0){
            shared_memory.table[i].id = id;
            return i;
        }
    }
    return -1;
}

char*
open_shared_mem(int id){
    struct proc* proc = myproc();
    pde_t *pgdir = proc->pgdir;
    if (proc->shm != 0){
        return 0;
    }
    acquire(&shared_memory.lock);
    int index = find_shared_page(id);
    if (index == -1){
        release(&shared_memory.lock);
        return 0;
    }
    if (shared_memory.table[index].num_of_refs == 0){
        shared_memory.table[index].frame = kalloc();
        memset(shared_memory.table[index].frame, 0, PGSIZE);
    }
    char* start_mem = (char*)PGROUNDUP(proc->sz);
    //cprintf("start_mem: %d\n", start_mem);

    mappages(pgdir, start_mem, PGSIZE, V2P(shared_memory.table[index].frame), PTE_W|PTE_U);
    shared_memory.table[index].num_of_refs++;
    shared_memory.table[index].id = id;
    proc->shm = start_mem;
    proc->shm_id = id;

    release(&shared_memory.lock);
    return start_mem;
}

void
close_shared_mem(int id){
    struct proc* proc = myproc();
    pde_t *pgdir = proc->pgdir;
    if (proc->shm_id != id || proc->shm == 0){
        return;
    }
    acquire(&shared_memory.lock);
    int index = find_shared_page(id);
    shared_memory.table[index].num_of_refs--;

    //delete from proc's page table
    uint a = PGROUNDUP((uint)proc->shm);
    pte_t *pte = walkpgdir(pgdir, (char*)a, 0);
    if(!pte)
        a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
    else if((*pte & PTE_P) != 0){
        *pte = 0;
    }

    proc->shm = 0;
    proc->shm_id = 0;
    
    if (shared_memory.table[index].num_of_refs == 0){
        kfree(shared_memory.table[index].frame);
        
    }

    release(&shared_memory.lock);
}