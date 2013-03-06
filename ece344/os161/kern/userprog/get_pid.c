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

pid_t sys_get_pid() {
	return curthread->pid;
}
