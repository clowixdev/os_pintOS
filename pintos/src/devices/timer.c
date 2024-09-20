#include "devices/timer.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include "devices/pit.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "threads/thread.h"

/* See [8254] for hardware details of the 8254 timer chip. */

#if TIMER_FREQ < 19
#error 8254 timer requires TIMER_FREQ >= 19
#endif
#if TIMER_FREQ > 1000
#error TIMER_FREQ <= 1000 recommended
#endif

/* Number of timer ticks since OS booted. */
static int64_t ticks;

/* Number of loops per timer tick.
   Initialized by timer_calibrate(). */
static unsigned loops_per_tick;

static intr_handler_func timer_interrupt;
static bool too_many_loops(unsigned loops);
static void busy_wait(int64_t loops);
static void real_time_sleep(int64_t num, int32_t denom);
static void real_time_delay(int64_t num, int32_t denom);

/* Sets up the timer to interrupt TIMER_FREQ times per second,
   and registers the corresponding interrupt. */
void timer_init(void)
{
	pit_configure_channel(0, 2, TIMER_FREQ);
	intr_register_ext(0x20, timer_interrupt, "8254 Timer");
}

/* Calibrates loops_per_tick, used to implement brief delays. */
void timer_calibrate(void)
{
	unsigned high_bit, test_bit;

	ASSERT(intr_get_level() == INTR_ON);
	printf("Calibrating timer...  ");

	/* Approximate loops_per_tick as the largest power-of-two
	   still less than one timer tick. */
	loops_per_tick = 1u << 10;
	while (!too_many_loops(loops_per_tick << 1))
	{
		loops_per_tick <<= 1;
		ASSERT(loops_per_tick != 0);
	}

	/* Refine the next 8 bits of loops_per_tick. */
	high_bit = loops_per_tick;
	for (test_bit = high_bit >> 1; test_bit != high_bit >> 10; test_bit >>= 1)
		if (!too_many_loops(high_bit | test_bit))
			loops_per_tick |= test_bit;

	printf("%'" PRIu64 " loops/s.\n", (uint64_t)loops_per_tick * TIMER_FREQ);
}

/* Returns the number of timer ticks since the OS booted. */
int64_t
timer_ticks(void)
{
	enum intr_level old_level = intr_disable();
	int64_t t = ticks;
	intr_set_level(old_level);
	return t;
}

/* Returns the number of timer ticks elapsed since THEN, which
   should be a value once returned by timer_ticks(). */
int64_t
timer_elapsed(int64_t then)
{
	return timer_ticks() - then;
}

//! LAB 1 S

struct st_list* stl = NULL;

/* Function that shows current state of an dynamic-queue */
void show_queue(void) {
	struct st_list *stl_elem = stl;
	printf("queue: ");
	while (stl_elem != NULL) {
		printf("[%d %d %d] ", 
			stl_elem->ticks_awake_at, 
			stl_elem->t->tid, 
			stl_elem->next_st
		);
		stl_elem = stl_elem->next_st;
	}
	printf("\n");
}

/* Function that is allocating memory and creating 
	an array element with all data initialized */
struct st_list* create_elem(struct thread *t, int64_t ticks) {

	struct st_list* stl_elem = (struct st_list*)calloc(1, sizeof(struct st_list));

	stl_elem->ticks_awake_at = ticks;
	stl_elem->t = t;
	stl_elem->next_st = NULL;

	return stl_elem;
}

/* Function that inserts sleeping thread into a sleeping_thread_list(stl)
	taking into account at what ticks this thread should be awaken */
void add_thread_to_stl(struct thread *t, int64_t ticks) {
	struct st_list* stl_elem = create_elem(t, ticks);
	if (stl == NULL) {
		stl = stl_elem;
		/*printf("thread %d added as root\n", t->tid);
		show_queue();*/
		return;
	} else {
		struct st_list *current = stl;
		struct st_list *before = NULL;

		while (ticks >= current->ticks_awake_at) {
			before = current;
			current = current->next_st;

			if (current == NULL) {
				before->next_st = stl_elem;
				/*printf("thread %d added after %d\n", t->tid, before->t->tid);
				show_queue();*/
				return;
			}
		}

		if (current == stl) {
			stl = stl_elem;
			stl->next_st = current;

			/*printf("thread %d added and changed root\n", t->tid);*/
		} else {
			before->next_st = stl_elem;
			stl_elem->next_st = current;

			/*printf("thread %d added after %d and before %d\n", 
				t->tid,
				before->t->tid,
				current->t->tid
			);*/
		}

		// show_queue();
	}
}

/* Creating an active waiting for the closest process to be 
	awaken -> for the first element of an dynamic-queue */
void unblock_thread_from_stl() {
	while (stl != NULL && stl->ticks_awake_at <= ticks) {
		thread_unblock(stl->t);
		/*printf("thread %d unblocked\n", stl->t->tid);*/
		stl = stl->next_st;
	}
}

/* Sleeps for approximately TICKS timer ticks.  Interrupts must
   be turned on. */
void timer_sleep(int64_t ticks) {
	ASSERT(intr_get_level() == INTR_ON);
	intr_set_level(INTR_OFF);

	if (ticks > 0) {
		add_thread_to_stl(thread_current(), timer_ticks() + ticks);
		thread_block();
	}

	ASSERT(intr_get_level() == INTR_OFF);
	intr_set_level(INTR_ON);
}

//! LAB1 E

/* Sleeps for approximately MS milliseconds.  Interrupts must be
   turned on. */
void timer_msleep(int64_t ms)
{
	real_time_sleep(ms, 1000);
}

/* Sleeps for approximately US microseconds.  Interrupts must be
   turned on. */
void timer_usleep(int64_t us)
{
	real_time_sleep(us, 1000 * 1000);
}

/* Sleeps for approximately NS nanoseconds.  Interrupts must be
   turned on. */
void timer_nsleep(int64_t ns)
{
	real_time_sleep(ns, 1000 * 1000 * 1000);
}

/* Busy-waits for approximately MS milliseconds.  Interrupts need
   not be turned on.

   Busy waiting wastes CPU cycles, and busy waiting with
   interrupts off for the interval between timer ticks or longer
   will cause timer ticks to be lost.  Thus, use timer_msleep()
   instead if interrupts are enabled. */
void timer_mdelay(int64_t ms)
{
	real_time_delay(ms, 1000);
}

/* Sleeps for approximately US microseconds.  Interrupts need not
   be turned on.

   Busy waiting wastes CPU cycles, and busy waiting with
   interrupts off for the interval between timer ticks or longer
   will cause timer ticks to be lost.  Thus, use timer_usleep()
   instead if interrupts are enabled. */
void timer_udelay(int64_t us)
{
	real_time_delay(us, 1000 * 1000);
}

/* Sleeps execution for approximately NS nanoseconds.  Interrupts
   need not be turned on.

   Busy waiting wastes CPU cycles, and busy waiting with
   interrupts off for the interval between timer ticks or longer
   will cause timer ticks to be lost.  Thus, use timer_nsleep()
   instead if interrupts are enabled.*/
void timer_ndelay(int64_t ns)
{
	real_time_delay(ns, 1000 * 1000 * 1000);
}

/* Prints timer statistics. */
void timer_print_stats(void)
{
	printf("Timer: %" PRId64 " ticks\n", timer_ticks());
}

/* Timer interrupt handler. */
static void
timer_interrupt(struct intr_frame *args UNUSED)
{
	ticks++;
	thread_tick();
	//! LAB1 S
	unblock_thread_from_stl();
	//! LAB2 E
}

/* Returns true if LOOPS iterations waits for more than one timer
   tick, otherwise false. */
static bool
too_many_loops(unsigned loops)
{
	/* Wait for a timer tick. */
	int64_t start = ticks;
	while (ticks == start)
		barrier();

	/* Run LOOPS loops. */
	start = ticks;
	busy_wait(loops);

	/* If the tick count changed, we iterated too long. */
	barrier();
	return start != ticks;
}

/* Iterates through a simple loop LOOPS times, for implementing
   brief delays.

   Marked NO_INLINE because code alignment can significantly
   affect timings, so that if this function was inlined
   differently in different places the results would be difficult
   to predict. */
static void NO_INLINE
busy_wait(int64_t loops)
{
	while (loops-- > 0)
		barrier();
}

/* Sleep for approximately NUM/DENOM seconds. */
static void
real_time_sleep(int64_t num, int32_t denom)
{
	/* Convert NUM/DENOM seconds into timer ticks, rounding down.

		  (NUM / DENOM) s
	   ---------------------- = NUM * TIMER_FREQ / DENOM ticks.
	   1 s / TIMER_FREQ ticks
	*/
	int64_t ticks = num * TIMER_FREQ / denom;

	ASSERT(intr_get_level() == INTR_ON);
	if (ticks > 0)
	{
		/* We're waiting for at least one full timer tick.  Use
		   timer_sleep() because it will yield the CPU to other
		   processes. */
		timer_sleep(ticks);
	}
	else
	{
		/* Otherwise, use a busy-wait loop for more accurate
		   sub-tick timing. */
		real_time_delay(num, denom);
	}
}

/* Busy-wait for approximately NUM/DENOM seconds. */
static void
real_time_delay(int64_t num, int32_t denom)
{
	/* Scale the numerator and denominator down by 1000 to avoid
	   the possibility of overflow. */
	ASSERT(denom % 1000 == 0);
	busy_wait(loops_per_tick * num / 1000 * TIMER_FREQ / (denom / 1000));
}
