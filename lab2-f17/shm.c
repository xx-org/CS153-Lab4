#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {
	int i;
	uint va, pa;
	pte_t* pgdir;
	int pem;//permission (read/write)	
	int flag = 0;
	acquire(&(shm_table.lock));
	for (i = 0; i< 64; i++) {
		if(shm_table.shm_pages[i].id == id)
		{
			pa = V2P(shm_table.shm_pages[i].frame);
			pem = PTE_W|PTE_U;
			va = PGROUNDUP(myproc()->sz);
			pgdir = myproc()->pgdir;
			myproc()->sz +=PGSIZE;
			if(mappages(pgdir, (void*)va, PGSIZE, pa, pem) != -1)
			{
				shm_table.shm_pages[i].refcnt += 1;
			}
			*pointer=(char *)va;
			flag = 1;
		}
	}
	if(flag == 0)
	for(i = 0; i < 64; i++){
			if(shm_table.shm_pages[i].id == 0)
			{
				shm_table.shm_pages[i].id = id;
				shm_table.shm_pages[i].frame = kalloc();
				shm_table.shm_pages[i].refcnt = 1;
				memset(shm_table.shm_pages[i].frame,  0, PGSIZE);
				pa = V2P(shm_table.shm_pages[i].frame);
				pem = PTE_W|PTE_U;
				pgdir = myproc()->pgdir;
			
				va = PGROUNDUP(myproc()->sz);
				if(mappages(pgdir, (void*)va, PGSIZE, pa, pem) != -1)
				{
					myproc()->sz += PGSIZE;
				}
				*pointer=(char*)va;
				break;
			}		
	}
  release(&(shm_table.lock));
return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!
  int i;
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    if(shm_table.shm_pages[i].id == id)
    {
		shm_table.shm_pages[i].id = 0;
		shm_table.shm_pages[i].frame =0;
		shm_table.shm_pages[i].refcnt =0;
	}
  }
  release(&(shm_table.lock));

return 0; //added to remove compiler warning -- you should decide what to return
}
