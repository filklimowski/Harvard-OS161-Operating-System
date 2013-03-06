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
#include <wait.h>
#include <get_pid.h>
#include <fork.h>


// We need to get the exit code and store it into status.

pid_t sys_waitpid(pid_t pid, int *status, int options) {
	if (options!=0) return EINVAL;
	if(status==NULL) return EFAULT; //your fault
	int has_exited = 0;
	
	//Case 1a pid does not exist, What about same pid???
	if (pid > get_globalpid())
		return pid;

	//Case 1b Waiting on a parent process is not allowed. Also prevents Deadlocks
	else if (pid <= sys_get_pid())
		return pid;

	else {	
	//Case 2 pid exists
		while (has_exited == 0){
			int spl = splhigh();
			
			has_exited = pidlist_search(pid, status);

			if (has_exited == 1){
			//Case 2a Process with pid has already exited and status contains the status code.
				splx(spl);
				return pid;
			} else {
			//Case 2b pid exists and wait for it to exit
				//kprintf("sleep time I am %d waiting on %d\n", sys_get_pid(), pid);
				thread_sleep(pid); // <---- Going to sleep on pid
				splx(spl);
			}
		}	
	}
	return pid;

}

