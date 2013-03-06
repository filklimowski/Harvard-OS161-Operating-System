#ifndef _FORK_H_
#define _FORK_H_

struct pid_list {
	pid_t pid;
	int has_exited;
	int exitcode;
	struct pid_list *next;
};



static pid_t global_pid;
static struct lock *pid_lock;
static struct pid_list *pidlist;

void           init_pid();
pid_t	       get_globalpid();
pid_t 	       get_newpid();
struct pid_list *pidlist_create(pid_t pid);

int pidlist_search(pid_t pid, int *exitcode);

#endif /* _FORK_H_ */

