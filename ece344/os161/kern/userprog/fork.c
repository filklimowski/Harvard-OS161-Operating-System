#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <machine/spl.h>
#include <test.h>
#include <addrspace.h>
#include <trapframe.h>
#include <synch.h>
#include <scheduler.h>
#include <dev.h>
#include <vfs.h>
#include <vm.h>
#include <syscall.h>
#include <version.h>
#include <hello.h>
#include <lib.h>
#include <curthread.h>
#include <thread.h>
#include <fork.h>


void init_pid() {
	//Changed to one because I was getting confused with the parent and return value of 0.
	global_pid=1;
	curthread->pid = 1;

	pidlist = kmalloc(sizeof(struct pid_list));
	pidlist->pid = 1;
	pidlist->has_exited = 0;
	pidlist->next = NULL;

}

pid_t get_globalpid(){
	return global_pid;
}

pid_t get_newpid() {
	//incrementing design. take global pid which ahs the most recent pid, increment that, and then return as new
	global_pid++;
	return global_pid;
}



pid_t
sys_fork(struct trapframe *parent_tf){

/*
* So we need to pass parent’s trapframe pointer to sys_fork, and store a full copy of it in the kernel heap (i.e. allocated by kmalloc). 
* Then pass the pointer to child’s fork *entry function (I name it as child_forkentry).
*/
		
	struct trapframe *parent_tf_copy = kmalloc(sizeof(struct trapframe));
	if (parent_tf_copy == NULL){
		return -1;
	}
	struct addrspace *parent_as_copy = NULL;
	
	memcpy(parent_tf_copy, parent_tf, sizeof(struct trapframe)); 
	
	as_copy(curthread->t_vmspace, &parent_as_copy);

	if (parent_as_copy == NULL){
		return -1;	
	}
	
	struct thread* temp;
	int err = thread_fork(curthread->t_name,(void *)parent_tf_copy, (unsigned long)parent_as_copy, (void *)md_forkentry, &temp);
	if (err) {
		return -1;
	}
/*
* 	To keep track of processes we created a global variable containing a list of processes and its assocaited status.
*/
	lock_acquire(pid_lock);

	temp->pid = get_newpid();

	struct pid_list *new_pid;
	new_pid = pidlist_create(get_globalpid());
	if (new_pid == NULL){
		return -1;
	}

	struct pid_list *current = pidlist;

	while (current->next != NULL)
		current = current->next;

	current->next = new_pid;

	lock_release(pid_lock);

return temp->pid; //for now

}
static 
void
md_forkentry(void *parent_tf, unsigned long paddr )
{

	struct trapframe *new_tf = (struct trapframe *)parent_tf;

	new_tf->tf_v0 = 0; //child retval
	new_tf->tf_a3 = 0;  //signal no error
	new_tf->tf_epc += 4;	//advance program counter to avoid calling again
	
	//struct trapframe *child_tf = kmalloc(sizeof(struct trapframe));
	
	memcpy(&curthread->t_stack[sizeof(struct trapframe)], new_tf, sizeof(struct trapframe));
	curthread->t_vmspace = (struct addrspace *)paddr;
	as_activate(curthread->t_vmspace);


	mips_usermode(&curthread->t_stack[sizeof(struct trapframe)]);
	
}

struct pid_list
*pidlist_create(pid_t pid)
{
	struct pid_list *new_pid;
	new_pid = kmalloc(sizeof(struct pid_list));
	if (new_pid == NULL) {
		return NULL;
	}

	new_pid->pid = pid;
	new_pid->has_exited = 0;
	new_pid->next = NULL;
	return new_pid;
	
	
}

int pidlist_search(pid_t pid, int *exitcode){
	if (pid_lock == NULL)
		pid_lock = lock_create("pid");
	//lock_acquire(pid_lock);
	
	struct pid_list *current;
	current = pidlist;
	while ((current != NULL) && (pid != current->pid))
		current = current->next;
	if (current == NULL){
		lock_release(pid_lock);
		return 0; // ERROR shouldnt get here
	} else {
		*exitcode = current->exitcode;
		//lock_release(pid_lock);
		return current->has_exited;
	}
	
}

int pidlist_exit(pid_t pid, int exitcode){
	struct pid_list *current;
	current = pidlist;
	while ((current != NULL) && (pid != current->pid))
		current = current->next;
	if (current == NULL){
		return 0;// Error
	} else {
		current->exitcode = exitcode;
		current->has_exited = 1;
	}
	return 0;
}
