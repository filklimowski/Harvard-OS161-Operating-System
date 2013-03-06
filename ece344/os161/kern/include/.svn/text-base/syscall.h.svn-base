#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <machine/types.h>
#include <kern/types.h>

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */
void sys_exit(int code);

int sys_reboot(int code);

int sys_printchar(char word);

int sys_readchar();

pid_t sys_fork(struct trapframe *parent_tf);
pid_t fork(void);

pid_t sys_get_pid();

pid_t sys_wait(pid_t pid, int *status, int options);
int waitpid(pid_t pid, int *returncode, int flags);


#endif /* _SYSCALL_H_ */
