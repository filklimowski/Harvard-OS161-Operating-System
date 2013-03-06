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

	// Atomically Wake up things waiting on this pid and then go to zombie mode.
	// If parent is already exited then do not put in zombie mode, die
	// Close all your threads, We are assuming only one thread per process
	// Destroy Process
void sys_exit(int exitcode){
	
	int spl = splhigh();
			
	pidlist_exit(curthread->pid, exitcode);

	thread_wakeup(curthread->pid);
	splx(spl);
	thread_exit();

}

	
