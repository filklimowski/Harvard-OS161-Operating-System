/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>
#include <syscall.h>
#include <machine/spl.h>
#include <trapframe.h>
#include <synch.h>
#include <scheduler.h>
#include <dev.h>
#include <syscall.h>
#include <version.h>
#include <thread.h>

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
runprogram(char *progname, int nargs, char **args)
{
	
	//find out how many arguments we have
	int i, numargs=nargs, numchars;
	int actual, offset;
	int *argoffsets;
	int total_data;

	numchars = 0;

	argoffsets = kmalloc(numargs*sizeof(int));

	for(i=0;i<numargs;i++) {
		numchars += strlen(args[i]) + (16 - strlen(args[i])%16);
	}

	args[numargs] = NULL;
	
	offset = 4*(numargs+1); //4 bytes for each argument, plus the 4 for the NULL argument
	argoffsets[0] = offset; //offset

	for(i=1;i<numargs;i++) 
		argoffsets[i]=argoffsets[i-1]+strlen(args[i-1]) + (16 - strlen(args[i-1])%16); //arguments[1] = 16+(16...

	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, &v);
	if (result) {
		return result;
	}

	/* We should be a new thread. */
	assert(curthread->t_vmspace == NULL);

	///* Create a new address space. */
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Activate it. */
	as_activate(curthread->t_vmspace);

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		return result;
	}
	
	total_data = (numargs+1)*sizeof(char*) + numchars + sizeof(int);
	stackptr -= total_data;
	curthread->t_vmspace->as_stackvbase = stackptr;
	argoffsets[numargs] = total_data;
	vaddr_t temp_stptr = stackptr;
	vaddr_t src;

	copyout(&numargs, temp_stptr, sizeof(int)); //copyout the numargs arguments
	temp_stptr += sizeof(int); //going to first put numargs (argc) into the stack

	for (i = 0; i < numargs; i ++){
		src = stackptr+4+argoffsets[i];
		copyout(&src, temp_stptr, sizeof(args[i])); //copyout is not copying out...
		temp_stptr += sizeof(char*);
	}

	temp_stptr += sizeof(char*); //for the NULL allocation

	for (i = 0; i < numargs; i ++){
		copyoutstr(args[i], temp_stptr, strlen(args[i]), &actual);
		temp_stptr += strlen(args[i]) + (16 - strlen(args[i])%16);
	}

	/* Warp to user mode. */
	md_usermode(numargs /*argc*/, (stackptr+4) /*userspace addr of argv*/,
		    stackptr, entrypoint);
	
	/* md_usermode does not return */
	panic("md_usermode returned\n");
	return EINVAL;
}

