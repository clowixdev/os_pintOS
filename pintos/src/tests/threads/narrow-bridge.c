
/* File for 'narrow_bridge' task implementation.  
   SPbSTU, IBKS, 2017 */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "narrow-bridge.h"

/* Creating all the needed semaphores. These semaphores will 
	block exact groups of cars */
struct semaphore l_norm_sema;
struct semaphore r_norm_sema;
struct semaphore l_emer_sema;
struct semaphore r_emer_sema;

/* Creating all the needed constants. They will contain 
	specific car amounts and current bridge direction  */
int cars_on_bridge;
int l_norm_amt;
int r_norm_amt;
int l_emer_amt;
int r_emer_amt;
enum car_direction bridge_dir;

/* Function will try to notify emergency cars on the specified side, 
	that can go through the bridge. Also, amount of cars will be 
	decremented and amount of cars on the bridge will be incremented. 
	Parameters: 
		[l - left, r - right][e - emergency, n - normal] */
void notify(char arg[])
{
	char side = arg[0];
	char car = arg[1];

	if (side == 'r') {
		if (car == 'e') {
			while (r_emer_amt != 0 && cars_on_bridge < 2) {
				r_emer_amt--;
				cars_on_bridge++;
				sema_up(&r_emer_sema);
			}
		} else if (car == 'n') {
			while (r_norm_amt != 0 && cars_on_bridge < 2) {
				r_norm_amt--;
				cars_on_bridge++;

				sema_up(&r_norm_sema);
			}
		}
	} else if (side == 'l'){
		if (car == 'e') {
			while (l_emer_amt != 0 && cars_on_bridge < 2) {
				l_emer_amt--;
				cars_on_bridge++;
				sema_up(&l_emer_sema);
			}
		} else if (car == 'n') {
			while (l_norm_amt != 0 && cars_on_bridge < 2) {
				l_norm_amt--;
				cars_on_bridge++;
				sema_up(&l_norm_sema);
			}
		}
	}
}

// Called before test. Can initialize some synchronization objects.
void narrow_bridge_init(void)
{
	sema_init(&l_norm_sema, 0);
	sema_init(&r_norm_sema, 0);
	sema_init(&l_emer_sema, 0);
	sema_init(&r_emer_sema, 0);

	cars_on_bridge = 0;
	l_norm_amt = 0;
	r_norm_amt = 0;
	l_emer_amt = 0;
	r_emer_amt = 0;
}

void arrive_bridge(enum car_priority prio, enum car_direction dir) 
{
	if (cars_on_bridge == 0) {
		cars_on_bridge++;
		bridge_dir = dir;
	} else if (cars_on_bridge == 1 && bridge_dir == dir) {
		cars_on_bridge++;
	} else {
		if (prio == 0 && dir == 0) {
			l_norm_amt++;
			sema_down(&l_norm_sema);
		} else if (prio == 0 && dir == 1) {
			r_norm_amt++;
			sema_down(&r_norm_sema);
		} else if (prio == 1 && dir == 0) {
			l_emer_amt++;
			sema_down(&l_emer_sema);
		} else if (prio == 1 && dir == 1) {
			r_emer_amt++;
			sema_down(&r_emer_sema);
		}

		bridge_dir = dir;
	}
}

void exit_bridge(enum car_priority prio, enum car_direction dir) 
{
	cars_on_bridge--;

	if (cars_on_bridge == 0) {
		if (bridge_dir == 0)	{
			if (r_emer_amt != 0) {
				notify("re");
				notify("rn");
			} else if (l_emer_amt != 0) {
				notify("le");
				notify("ln");
			} else if (r_norm_amt != 0) {
				notify("rn");
			} else if (l_norm_amt != 0) {
				notify("ln");
			}
		} else {
			if (l_emer_amt != 0) {
				notify("le");
				notify("ln");
			} else if (r_emer_amt != 0) {
				notify("re");
				notify("rn");
			} else if (l_norm_amt != 0) {
				notify("ln");
			} else if (r_norm_amt != 0) {
				notify("rn");
			}
		}
	}
}