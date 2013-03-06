/*
 * Synchronization primitives.
 * See synch.h for specifications of the functions.
 */

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <machine/spl.h>
#include <queue.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *namearg, int initial_count)
{
	struct semaphore *sem;

	assert(initial_count >= 0);

	sem = kmalloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}

	sem->name = kstrdup(namearg);
	if (sem->name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->count = initial_count;
	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	spl = splhigh();
	assert(thread_hassleepers(sem)==0);
	splx(spl);

	/*
	 * Note: while someone could theoretically start sleeping on
	 * the semaphore after the above test but before we free it,
	 * if they're going to do that, they can just as easily wait
	 * a bit and start sleeping on the semaphore after it's been
	 * freed. Consequently, there's not a whole lot of point in 
	 * including the kfrees in the splhigh block, so we don't.
	 */

	kfree(sem->name);
	kfree(sem);
}

void 
P(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	assert(in_interrupt==0);

	spl = splhigh();
	while (sem->count==0) {
		thread_sleep(sem);
	}
	assert(sem->count>0);
	sem->count--;
	splx(spl);
}

void
V(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);
	spl = splhigh();
	sem->count++;
	assert(sem->count>0);
	thread_wakeup(sem);
	splx(spl);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;
	
	lock = kmalloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->name = kstrdup(name);
	if (lock->name == NULL) {
		kfree(lock);
		return NULL;
	}

	lock->thread_addr = NULL;
	
	return lock;
}

void
lock_destroy(struct lock *lock)
{
/*
* 	To destroy lock make sure no one is holding it.
*/

	assert(lock != NULL);

	kfree(lock->name);
	kfree(lock);

	//kfree(lock->name);
	//kfree(lock);

}

void
lock_acquire(struct lock *lock)
{
	
/**
*	Check to ensure a lock has been created. Then wait on the lock until it
* 	is available. Then hold the lock.
*/
	int spl;
	assert(lock != NULL);

	assert(in_interrupt==0);
	spl = splhigh();

	while (lock->thread_addr != NULL && (lock_do_i_hold(lock) != 1)){
		thread_sleep(lock); // <---- Going to sleep on lock
	}
	lock->thread_addr = curthread;
	splx(spl);
	

	//(void)lock;  // suppress warning until code gets written
}

void
lock_release(struct lock *lock)
{
/**
*	Check to ensure a lock has been created. Then ensure that it is this thread who
* 	owns the lock and release it.
*/
	int spl;
	assert(lock != NULL);

	assert(in_interrupt==0);
	spl = splhigh(); 

	if (lock_do_i_hold(lock) == 1){
		lock->thread_addr = NULL;
		thread_wakeup(lock);
	}
	splx(spl);  

	//(void)lock;  // suppress warning until code gets written
}

int
lock_do_i_hold(struct lock *lock)
{
	if (lock == NULL)
		return 0;
	else if (lock->thread_addr == curthread)
		return 1;
	else
		return 0;

	//(void)lock;  // suppress warning until code gets written
	//return 1;    // dummy until code gets written
}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->name = kstrdup(name);
	if (cv->name==NULL) {
		kfree(cv);
		return NULL;
	}

	cv->cv_queue = q_create(32);
	if (cv->cv_queue == NULL) {
		panic("scheduler: Could not create run queue\n");
	}
	
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	if (cv->cv_queue != NULL){
		q_destroy(cv->cv_queue);
	}

	if (cv->name!=NULL) {
		kfree(cv->name);
	}
	kfree(cv);
	
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	q_addtail(cv->cv_queue, curthread);

	int spl = splhigh();	
	lock_release(lock);
	thread_sleep(cv);
	
	lock_acquire(lock);
	splx(spl);
	
}


/*
*	Wakes up the first thread in the lineup. If there is no thread then this does nothing.
*	After waking it up the head pointer points to the next thread in the lineup.
*/
void
cv_signal(struct cv *cv, struct lock *lock)
{
	int spl = splhigh();
	assert (lock != NULL);

	if (q_empty(cv->cv_queue)){
		return;
	} else 	{
		thread_wakeup_head(cv, cv->cv_queue);		
	}
	splx(spl);

}


/*
*	Walks through the lineup waking up each thread. Wakes up threads one by one
* 	until it reaches the end.
*/

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	int spl = splhigh();
	
	//While (!q_empty(cv->cv_queue)) {	
	thread_wakeup(cv);
	
	splx(spl);
}

