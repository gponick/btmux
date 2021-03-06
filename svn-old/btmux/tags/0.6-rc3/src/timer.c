
/*
 * timer.c -- Subroutines for (system-) timed events 
 */

/*
 * $Id: timer.c,v 1.5 2005/08/08 09:43:07 murrayma Exp $ 
 */

#include "copyright.h"
#include "config.h"

#include <signal.h>

#include "mudconf.h"
#include "config.h"
#include "db.h"
#include "interface.h"
#include "match.h"
#include "externs.h"
#include "command.h"

extern void pool_reset(void);
extern void do_second(void);
extern void fork_and_dump(int key);
extern unsigned int alarm(unsigned int seconds);
extern void pcache_trim(void);
extern void check_events(void);

void timer_callback(int fd, short event, void *arg);

static struct timeval tv = { 0, 100000 };
static struct event timer_event;

void init_timer() {
    mudstate.now = time(NULL);
    mudstate.dump_counter =
        ((mudconf.dump_offset ==
          0) ? mudconf.dump_interval : mudconf.dump_offset) +
        mudstate.now;
    mudstate.check_counter =
        ((mudconf.check_offset ==
          0) ? mudconf.check_interval : mudconf.check_offset) +
        mudstate.now;
    mudstate.idle_counter = mudconf.idle_interval + mudstate.now;
    mudstate.mstats_counter = 15 + mudstate.now;
    mudstate.events_counter = 900 + mudstate.now;
    // alarm(1);
    evtimer_set(&timer_event, timer_callback, NULL);
    evtimer_add(&timer_event, &tv);
}

#undef DISPATCH_DEBUG

#ifdef DISPATCH_DEBUG
#define DPSET(n) mudstate.debug_cmd = (char *) n
#else
#define DPSET(n)
#endif

void dispatch() {
    char *cmdsave;

#ifdef USE_PYTHON
    mudstate.debug_cmd = "< Python >";
    updatePython();
#endif

    cmdsave = mudstate.debug_cmd;
    DPSET("< dispatch >");
    /*
     * this routine can be used to poll from interface.c 
     */

    if (!mudstate.alarm_triggered)
        return;
    mudstate.alarm_triggered = 0;
    mudstate.now = time(NULL);

    do_second();

    /*
     * Free list reconstruction 
     */

    if ((mudconf.control_flags & CF_DBCHECK) &&
            (mudstate.check_counter <= mudstate.now)) {
        mudstate.check_counter = mudconf.check_interval + mudstate.now;
        DPSET("< dbck >");
        do_dbck(NOTHING, NOTHING, 0);
        pcache_trim();
    }
    /*
     * Database dump routines 
     */

    if ((mudconf.control_flags & CF_CHECKPOINT) &&
            (mudstate.dump_counter <= mudstate.now)) {
        mudstate.dump_counter = mudconf.dump_interval + mudstate.now;
        DPSET("< dump >");
        fork_and_dump(0);
    }
    /*
       Mech stuff ; hopefully it means once ~per sec, although you
       can never be sure - therefore, the code does 'timejumps' as
       needed (see UpdateSpecialObjects for details)
       */

    if (mudconf.have_specials)
        UpdateSpecialObjects();
    
    /*
     * Idle user check 
     */

    if ((mudconf.control_flags & CF_IDLECHECK) &&
            (mudstate.idle_counter <= mudstate.now)) {
        mudstate.idle_counter = mudconf.idle_interval + mudstate.now;
        DPSET("< idlecheck >");
        check_idle();

    }
    /*
     * Check for execution of attribute events 
     */

    if ((mudconf.control_flags & CF_EVENTCHECK) &&
            (mudstate.events_counter <= mudstate.now)) {
        mudstate.events_counter = 900 + mudstate.now;
        DPSET("< eventcheck >");
        check_events();
    }
    
#ifdef HAVE_GETRUSAGE
    /*
     * Memory use stats 
     */

    if (mudstate.mstats_counter <= mudstate.now) {

        int curr;

        mudstate.mstats_counter = 15 + mudstate.now;
        curr = mudstate.mstat_curr;
        if (mudstate.now > mudstate.mstat_secs[curr]) {

            struct rusage usage;

            curr = 1 - curr;
            getrusage(RUSAGE_SELF, &usage);
            mudstate.mstat_ixrss[curr] = usage.ru_ixrss;
            mudstate.mstat_idrss[curr] = usage.ru_idrss;
            mudstate.mstat_isrss[curr] = usage.ru_isrss;
            mudstate.mstat_secs[curr] = mudstate.now;
            mudstate.mstat_curr = curr;
        }
    }
#endif

    mudstate.debug_cmd = cmdsave;
}

void timer_callback(int fd, short event, void *arg) {
    mudstate.alarm_triggered = 1;
    evtimer_add(&timer_event, &tv);
    dispatch();
}


/*
 * ---------------------------------------------------------------------------
 * * do_timewarp: Adjust various internal timers.
 */

void do_timewarp(dbref player, dbref cause, int key, char *arg) {
    int secs;

    secs = atoi(arg);

    if ((key == 0) || (key & TWARP_QUEUE))	/*
                                             * Sem/Wait queues 
                                             */
        do_queue(player, cause, QUEUE_WARP, arg);
    if (key & TWARP_DUMP)
        mudstate.dump_counter -= secs;
    if (key & TWARP_CLEAN)
        mudstate.check_counter -= secs;
    if (key & TWARP_IDLE)
        mudstate.idle_counter -= secs;
    if (key & TWARP_EVENTS)
        mudstate.events_counter -= secs;
}
