/*
 * catlock.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use LOCKS/CV'S to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>


/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2

#define TIMES_EATING 4
#define CAT 'c'
#define MOUSE 'm'
#define FREE 'f'

/*
 * 
 * Function Definitions
 * 
 */
struct bowl 
{
	int number;
	char who;
};

struct bowl *
bowl_create (int num){
	struct bowl *new_bowl;
	new_bowl = kmalloc(sizeof(struct bowl));

	if (new_bowl == NULL) {
		return NULL;
	}

	new_bowl->number = num;
	new_bowl->who = FREE;
	return new_bowl;
}

struct bowl *bowl1; 
struct bowl *bowl2;

struct animal 
{
	int number;
	char who;
	int times_eaten;
};

struct animal *
animal_create (int num, char wh){
	struct animal *new_animal;
	new_animal = kmalloc(sizeof(struct animal));

	if (new_animal == NULL) {
		return NULL;
	}

	new_animal->number = num;
	new_animal->who = wh;
	new_animal->times_eaten = 0;
	return new_animal;
}

static struct lock *bowl_1;
static struct lock *bowl_2;
static struct lock *openbowl_lock;

static struct cv *free_bowl;
/* who should be "cat" or "mouse" */
static void
lock_eat(const char *who, int num, int bowl, int iteration)
{
        kprintf("%s: %d starts eating: bowl %d, iteration %d\n", who, num, 
                bowl, iteration);
        clocksleep(1);
        kprintf("%s: %d ends eating: bowl %d, iteration %d\n", who, num, 
                bowl, iteration);
}

/*
 * catlock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
catlock(void * unusedpointer, 
        unsigned long catnumber)
{
	int i;
	for (i = 0; i < TIMES_EATING; i++)
	{
	lock_acquire(openbowl_lock);
		while (bowl1->who != FREE && bowl2->who != FREE) {
			cv_wait(free_bowl, openbowl_lock);
		}	
		while (bowl1->who == MOUSE || bowl2->who == MOUSE) {
			cv_wait(free_bowl, openbowl_lock);
		}

		if (bowl1->who == FREE) { 
		//[lock bowl 1, eat from bowl 1], release bowl1_lock, release openbowl_lock, cv_signal(free_bowl, openbowl_lock), increment cats[catnumber];
			
			lock_acquire(bowl_1);
			bowl1->who = CAT;
			lock_release(openbowl_lock);
			lock_eat("cat", catnumber, 1, i);
			bowl1->who = FREE;
			cv_signal(free_bowl, openbowl_lock);
			lock_release(bowl_1);
			
			

		} else if (bowl2 ->who == FREE) {
		//[lock bowl 2, eat from bowl 2], change who==null, release bowl2_lock, release openbowl_lock, cv_signal(free_bowl, openbowl_lock), increment cats[catnumber];

			lock_acquire(bowl_2);	
			bowl2->who = CAT;
			lock_release(openbowl_lock);
			lock_eat("cat", catnumber, 2, i);
			bowl2->who = FREE;
			cv_signal(free_bowl, openbowl_lock);
			lock_release(bowl_2);


		}
		else {
			i--;
			//error shouldn't have made it here!
		}
		// check to see if everyone's eaten the same number of times. If I am more, cv_wait(fair_share), else cv_broadcast(fair_share)
	}
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
        (void) catnumber;
}
	

/*
 * mouselock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
mouselock(void * unusedpointer,
          unsigned long mousenumber)
{
	int i;
	for (i = 0; i < TIMES_EATING; i++)
	{
	lock_acquire(openbowl_lock);
		while (bowl1->who != FREE && bowl2->who != FREE) {
			cv_wait(free_bowl, openbowl_lock);
		}	
		while (bowl1->who == CAT || bowl2->who == CAT) {
			cv_wait(free_bowl, openbowl_lock);
		}


		if (bowl1->who == FREE) {
		//lock bowl 1, release openbowl_lock, eat from bowl 1, cv_signal(free_bowl, openbowl_lock), increment cats[catnumber], release bowl1_lock;

			lock_acquire(bowl_1);
			bowl1->who = MOUSE;
			lock_release(openbowl_lock);
			lock_eat("mouse", mousenumber, 1, i);
			bowl1->who = FREE;
			cv_signal(free_bowl, openbowl_lock);
			lock_release(bowl_1);

		} else if (bowl2 ->who == FREE) {
			//lock bowl 2, release openbowl_lock, eat from bowl 2, cv_signal(free_bowl, openbowl_lock), increment cats[catnumber], release bowl2_lock;
			lock_acquire(bowl_2);	
			bowl2->who = MOUSE;
			lock_release(openbowl_lock);
			lock_eat("mouse", mousenumber, 2, i);
			bowl2->who = FREE;
			cv_signal(free_bowl, openbowl_lock);
			lock_release(bowl_2);


		} else {
			i--;
			//error shouldn't have made it here!
		}
		

// check to see if everyone's eaten the same number of times. If I am more, cv_wait(fair_share), else cv_broadcast(fair_share)
	}
        /*
         * Avoid unused variable warnings.
         */
        
        (void) unusedpointer;
        (void) mousenumber;
}


/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */

int
catmouselock(int nargs,
             char ** args)
{
        int index, error;
   
        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
   
	bowl1 = bowl_create(1);
	bowl2 = bowl_create(2);
	
	
	
	

	bowl_1 = lock_create("bowl_1");
	bowl_2 = lock_create("bowl_2");
	openbowl_lock = lock_create("openbowl_lock");
	free_bowl = cv_create("free_bowl");

        /*
         * Start NCATS catlock() threads.
         */

        for (index = 0; index < NCATS; index++) {
           	animal_create (index, CAT);
                error = thread_fork("catlock thread", 
                                    NULL, 
                                    index, 
                                    catlock, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catlock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        /*
         * Start NMICE mouselock() threads.
         */

        for (index = 0; index < NMICE; index++) {
   		animal_create (index, MOUSE);
                error = thread_fork("mouselock thread", 
                                    NULL, 
                                    index, 
                                    mouselock, 
                                    NULL
                                    );
      
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mouselock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        return 0;
}

/*
 * End of catlock.c
 */
