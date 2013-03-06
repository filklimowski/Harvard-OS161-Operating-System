#ifndef _VM_H_
#define _VM_H_

#include <machine/vm.h>

/*
 * VM system-related definitions.
 *
 * You'll probably want to add stuff here.
 */


/* Fault-type arguments to vm_fault() */
#define VM_FAULT_READ        0    /* A read was attempted */
#define VM_FAULT_WRITE       1    /* A write was attempted */
#define VM_FAULT_READONLY    2    /* A write to a readonly page was attempted*/

/* Initialization function */
void vm_bootstrap(void);
void coremap_init(void);
void swap_init(void);

/* Fault handling function called by trap code */
int vm_fault(int faulttype, vaddr_t faultaddress);

/* Allocate/free kernel heap pages (called by kmalloc/kfree) */
vaddr_t alloc_kpages(int npages);
void free_kpages(vaddr_t addr);

struct page_table_entry {
	vaddr_t va;
	paddr_t pa;
	struct addrspace *addr;
	unsigned long npages;
	u_int32_t time;
	// A bit that each has its own meaning. 
		u_int8_t valid;
		u_int8_t dirty;
		//u_int8_t swapped ;
		u_int8_t locked ;
		u_int8_t writeable ;

	//u_int32_t page;
};

struct alloc_status {
	vaddr_t vaddr;
	int npages;
};

struct tlb_entry {
	u_int32_t ehi;
	u_int32_t elo;
};

static struct page_table_entry *coremap;
static struct page_table_entry *swap_holder;
static struct alloc_status* alloc;
static struct bitmap* gbl_bmp;
static struct bitmap* swap_bmp;
static struct tlb_entry *tlblist;

u_int32_t mem_start;
u_int32_t firstaddr, lastaddr, freeaddr;
u_int32_t coremap_remove (u_int32_t paddr, int index);
static int if_setupcomplete, max_page_num;

u_int32_t coreswap_insert (u_int32_t vaddr, u_int32_t offset);
u_int32_t coreswap_remove (u_int32_t offset);
int swap_handler(int index, vaddr_t vaddr);
int evict (vaddr_t vaddr);
int swap_in (vaddr_t vaddr, int index);

int pgdir_walk(vaddr_t vaddr, struct addrspace *as);
u_int32_t get_single_page (u_int32_t *index);
u_int32_t pt_insert (u_int32_t vaddr, u_int32_t paddr);
static paddr_t getppages(unsigned long npages);



#endif /* _VM_H_ */
