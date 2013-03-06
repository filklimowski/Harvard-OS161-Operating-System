#ifndef _SBRK_H_
#define _SBRK_H_

// 16MB limit
#define USER_HEAP_LIMIT 32768

void *sys_sbrk(int change, int *retval);

#endif /* _FORK_H_ */

