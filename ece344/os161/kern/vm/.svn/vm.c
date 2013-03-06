#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <vm.h>
#include <bitmap.h>

u_int32_t coremap_start;
u_int32_t coremap_size;
static int al_index;

void
vm_bootstrap(void)
{
	if_setupcomplete=0;
	u_int32_t firstaddr, lastaddr, freeaddr; // First is the bottom of mem, Last is top, freeaddr is usable memory for pages.
	//int max_page_num; // Max number of pages that can be used without memory overflow

	//ram_getsize(&firstaddr, &lastaddr);
	// Because it is messing up our values for firstaddr and lastaddr in physical memory
	// We will alternate and use ram_getsize(0) for start and mips_ramsize for end
	firstaddr = ram_stealmem(0);
	lastaddr = mips_ramsize();

	max_page_num = ( lastaddr - (firstaddr + PAGE_SIZE) ) / PAGE_SIZE; //Excluding preused memory
	gbl_bmp = bitmap_create(max_page_num);
	assert(gbl_bmp != NULL);//ERROR ENOMEM

	freeaddr = firstaddr + PAGE_SIZE;
	//coremap = kmalloc(max_page_num * sizeof(struct page_table_entry));


	coremap = (struct page_table_entry*)kmalloc(max_page_num * sizeof(struct page_table_entry));
	coremap_start = ram_stealmem(0);
	coremap_size = max_page_num;
	alloc = (struct alloc_status*)kmalloc(64*sizeof(struct alloc_status));
	

	assert(coremap != NULL);//ERROR ENOMEM, DEALLOCATE GBL_BMP	


	int i;
	for (i = 1; i < max_page_num; i++) {
		coremap[i].pa = (coremap_start + (i * PAGE_SIZE));
	}
	bitmap_mark(gbl_bmp, 0); // Setting the first bit to valid to keep our page tables in tact.	

	//mem_start = freeaddr; // or firstaddr
	if_setupcomplete=1; //Initilaztion of memory is complete
	kprintf("COREMAP INIT: %d %d\n",coremap_start,coremap_size);
}

u_int32_t get_singlepage (u_int32_t *index) {
	int spl = splhigh(); //probably should use a lock
	int result;
	result = bitmap_alloc(gbl_bmp, index); // Set 1 bit and label it set. And tell us where it is
	//if result returns success then we have one empty page
	//else we need to swap. For now assume we filled the hole.
	splx(spl);
	if (!result)
		return coremap[*index].pa;
	else
		return 0;
}	

u_int32_t pt_insert (u_int32_t vaddr, u_int32_t paddr) {
	int spl=splhigh();
	paddr = paddr & PAGE_FRAME;
	int index = (paddr - mem_start) / PAGE_SIZE; // Gives the index of the page table

	coremap[index].va = vaddr; //Sets the virtual address of the page table to the virtual address required
	coremap[index].pa = SET_VALID(paddr);
 
	if(!bitmap_isset(gbl_bmp, index))
		bitmap_mark(gbl_bmp, index);
	splx(spl);
	return 0;
}

/*
*	
*/
static
paddr_t
getppages(unsigned long npages)
{
	assert (npages > 0);
	int spl = splhigh(); //probably should use a lock

	int i, count;
	u_int32_t index, paddr;

	//While setting up, program is allowed to ram_steal pages without worrying	` about coremap
	if (if_setupcomplete == 0){
		paddr = ram_stealmem(npages);
		splx(spl);
		return paddr & PAGE_FRAME;
	}
	// coremap has now been initialized.
	else {
		// If you only want one page then call the predefined bitmap_alloc function
		if (npages == 1){
			paddr = get_singlepage(&index); // Set 1 bit and label it set. And tell us where it is

			if (paddr == 0){
				splx(spl);
				return 0;
			}
			//assert(coremap[index].valid == 0);
			//Do work to allocate this page.
			//coremap[index].valid = 1;
			paddr = SET_VALID(paddr);
			pt_insert( PADDR_TO_KVADDR(paddr), paddr);

			splx(spl);
			return (paddr & PAGE_FRAME);
		}
		//We need more than one page, search the bitmap list for free spaces
		else {
			for (i = 0; i < max_page_num; i++){
				if (bitmap_isset(gbl_bmp, i)){ // if the page is occupied returns 1 so count is still 0 
					count = 0;
				} else {
					count ++; // we found a page YAYAYAY! 
					if ((unsigned long)count == npages) // have we found enough pages??? 
						break;
				}
			}
			
			//Make sure you found a hole. If not you need to swap
			if (count != npages){
				splx(spl);
				return 0;
			}


			//Move back count from i to get the start index.
			//Set these bits to 1
			else {
				index = i  + 1 - count; // start of the contiguos pages is index!
				for(i = 0; i < count; i++){
					bitmap_mark(gbl_bmp, index+i);
					paddr = coremap[index+i].pa;
					paddr = SET_VALID(paddr);
					pt_insert( PADDR_TO_KVADDR(paddr), paddr);
				}
				paddr = coremap[index].pa;
				coremap[index].npages = npages;
			}
			splx(spl);
			return paddr & PAGE_FRAME;	
		}
	}
						
}

u_int32_t coremap_remove (u_int32_t paddr, int index) {
	paddr = paddr & PAGE_FRAME;
	assert( (coremap[index].pa & PAGE_FRAME) == (paddr & PAGE_FRAME) );
	coremap[ index ].va = 0;
	bitmap_unmark(gbl_bmp, index);	
	return 0;
}

void 
free_kpages(vaddr_t addr)
{
	int spl=splhigh();
	//find its index in coremap
	int i, j;
 	for (i = 1; i < max_page_num; i ++){
		if (coremap[i].va == addr+1)
			break;
	} // i now equals the index of the position in coremap
	if (coremap[i].va != addr+1){
		assert(1 == 0);
	// Test failed	
	}
	for (j = 0; j < coremap[i].npages; j++){
		coremap_remove(coremap[i+j].pa, i+j);
	}

	splx(spl);
}

/*
* 	To avoid Errors
*/
#define DUMBVM_STACKPAGES    12

void enter_status(vaddr_t vaddr,int no_pages)
{
	alloc[al_index].vaddr=vaddr;
	alloc[al_index].npages=no_pages;
	al_index++;
}

vaddr_t 
alloc_kpages(int npages)
{
	paddr_t pa;
	pa = getppages(npages);
	enter_status(PADDR_TO_KVADDR(pa),npages);
	if (pa==0) {
		return 0;
	}
	return PADDR_TO_KVADDR(pa);
}

int
vm_fault(int faulttype, vaddr_t faultaddress) //TLB holds a limited number of virtual to physical translations. If it cant be translated by the TLB. results in a fault. Also write to read only causes a fault. These faults are caught here
{
	vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
	paddr_t paddr;
	int i;
	u_int32_t ehi, elo;
	struct addrspace *as;
	int spl;

	spl = splhigh();

	faultaddress &= PAGE_FRAME;

	DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);

	switch (faulttype) {
	    case VM_FAULT_READONLY:
		/* We always create pages read-write, so we can't get this */
		panic("dumbvm: got VM_FAULT_READONLY\n");
	    case VM_FAULT_READ:
	    case VM_FAULT_WRITE:
		break;
	    default:
		splx(spl);
		return EINVAL;
	}

	as = curthread->t_vmspace;
	if (as == NULL) {
		/*
		 * No address space set up. This is probably a kernel
		 * fault early in boot. Return EFAULT so as to panic
		 * instead of getting into an infinite faulting loop.
		 */
		return EFAULT;
	}

	/* Assert that the address space has been set up properly. */
	assert(as->as_vbase1 != 0);
	assert(as->as_pbase1 != 0);
	assert(as->as_npages1 != 0);
	assert(as->as_vbase2 != 0);
	assert(as->as_pbase2 != 0);
	assert(as->as_npages2 != 0);
	assert(as->as_stackpbase != 0);
	assert((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
	assert((as->as_pbase1 & PAGE_FRAME) == as->as_pbase1);
	assert((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
	assert((as->as_pbase2 & PAGE_FRAME) == as->as_pbase2);
	assert((as->as_stackpbase & PAGE_FRAME) == as->as_stackpbase);

	vbase1 = as->as_vbase1;
	vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
	vbase2 = as->as_vbase2;
	vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
	stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
	stacktop = USERSTACK;

	if (faultaddress >= vbase1 && faultaddress < vtop1) {
		paddr = (faultaddress - vbase1) + as->as_pbase1;
	}
	else if (faultaddress >= vbase2 && faultaddress < vtop2) {
		paddr = (faultaddress - vbase2) + as->as_pbase2;
	}
	else if (faultaddress >= stackbase && faultaddress < stacktop) {
		paddr = (faultaddress - stackbase) + as->as_stackpbase;
	}
	else {
		splx(spl);
		return EFAULT;
	}

	/* make sure it's page-aligned */
	assert((paddr & PAGE_FRAME)==paddr);

	for (i=0; i<NUM_TLB; i++) {
		TLB_Read(&ehi, &elo, i);
		if (elo & TLBLO_VALID) {
			continue;
		}
		ehi = faultaddress;
		elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
		DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
		TLB_Write(ehi, elo, i);
		splx(spl);
		return 0;
	}

	kprintf("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
	splx(spl);
	return EFAULT;
}

struct addrspace *
as_create(void)
{
	struct addrspace *as = kmalloc(sizeof(struct addrspace));
	if (as==NULL) {
		return NULL;
	}

	as->as_vbase1 = 0;
	as->as_pbase1 = 0;
	as->as_npages1 = 0;
	as->as_vbase2 = 0;
	as->as_pbase2 = 0;
	as->as_npages2 = 0;
	as->as_stackpbase = 0;

	return as;
}

void
as_destroy(struct addrspace *as)
{
	kfree(as);
}

void
as_activate(struct addrspace *as)
{ 
	int i, spl;

	(void)as;

	spl = splhigh();

	for (i=0; i<NUM_TLB; i++) {
		TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
	}

	splx(spl);
}

int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
		 int readable, int writeable, int executable)
{
	size_t npages; 

	/* Align the region. First, the base... */
	sz += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;

	/* ...and now the length. */
	sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;

	npages = sz / PAGE_SIZE;

	/* We don't use these - all pages are read-write */
	(void)readable;
	(void)writeable;
	(void)executable;

	if (as->as_vbase1 == 0) {
		as->as_vbase1 = vaddr;
		as->as_npages1 = npages;
		return 0;
	}

	if (as->as_vbase2 == 0) {
		as->as_vbase2 = vaddr;
		as->as_npages2 = npages;
		return 0;
	}

	/*
	 * Support for more than two regions is not available.
	 */
	kprintf("dumbvm: Warning: too many regions\n");
	return EUNIMP;
}

int
as_prepare_load(struct addrspace *as)
{
	assert(as->as_pbase1 == 0);
	assert(as->as_pbase2 == 0);
	assert(as->as_stackpbase == 0);

	as->as_pbase1 = getppages(as->as_npages1);
	if (as->as_pbase1 == 0) {
		return ENOMEM;
	}

	as->as_pbase2 = getppages(as->as_npages2);
	if (as->as_pbase2 == 0) {
		return ENOMEM;
	}

	as->as_stackpbase = getppages(DUMBVM_STACKPAGES);
	if (as->as_stackpbase == 0) {
		return ENOMEM;
	}

	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	assert(as->as_stackpbase != 0);

	*stackptr = USERSTACK;
	return 0;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *new;

	new = as_create();
	if (new==NULL) {
		return ENOMEM;
	}

	new->as_vbase1 = old->as_vbase1;
	new->as_npages1 = old->as_npages1;
	new->as_vbase2 = old->as_vbase2;
	new->as_npages2 = old->as_npages2;

	if (as_prepare_load(new)) {
		as_destroy(new);
		return ENOMEM;
	}

	assert(new->as_pbase1 != 0);
	assert(new->as_pbase2 != 0);
	assert(new->as_stackpbase != 0);

	memmove((void *)PADDR_TO_KVADDR(new->as_pbase1),
		(const void *)PADDR_TO_KVADDR(old->as_pbase1),
		old->as_npages1*PAGE_SIZE);

	memmove((void *)PADDR_TO_KVADDR(new->as_pbase2),
		(const void *)PADDR_TO_KVADDR(old->as_pbase2),
		old->as_npages2*PAGE_SIZE);

	memmove((void *)PADDR_TO_KVADDR(new->as_stackpbase),
		(const void *)PADDR_TO_KVADDR(old->as_stackpbase),
		DUMBVM_STACKPAGES*PAGE_SIZE);
	
	*ret = new;
	return 0;
}
//void 
//free_kpages(vaddr_t addr)
//{
	
//	bitmap_unmark(struct bitmap *, u_int32_t index);

//	(void)addr;
//}
