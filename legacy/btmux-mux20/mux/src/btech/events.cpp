
/*
 * $Id: events.c,v 1.2 2005/06/22 22:07:17 murrayma Exp $
 *
 * Author: Markus Stenberg <fingon@iki.fi>
 *
 *  Copyright (c) 1998 Markus Stenberg
 *  Copyright (c) 1998-2002 Thomas Wouters 
 *  Copyright (c) 2000-2002 Cord Awtry 
 *       All rights reserved
 *
 * Created: Wed Apr 29 20:17:02 1998 fingon
 * Last modified: Tue Jul 28 10:20:35 1998 fingon
 *
 */


#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "db.h"
#include "stringutil.h"
#include "alloc.h"
#include "muxevent.h"
#include "mech.h"
#include "mech.events.h"
#include "p.glue.h"
#include "p.mech.notify.h"

#define MAX_EVENTS 100

extern char *muxevent_names[];

int muxevent_exec_count[MAX_EVENTS];

void muxevent_count_initialize()
{
    int i;

    for (i = 0; i < MAX_EVENTS; i++)
        muxevent_exec_count[i] = 0;
}

static int muxevent_mech_event[] = {
    0, 1, 0, 1, 1, 1, 1, 1, 1, 0,    /* 0-9 */
    1, 0, 1, 1, 0, 1, 1, 0, 0, 1,    /*10-19 */
    1, 1, 1, 0, 0, 0, 0, 0, 0, 1,    /*20-29 */
    1, 0, 1, 1, 1, 0, 1, 1, 0, 1,    /*30-39 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /*40-49 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /*50-59 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /*60-69 */
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0,    /*70-79 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void debug_EventTypes(dbref player, void *data, char *buffer)
{
    int i, j, k, tot = 0;

    if (buffer && *buffer) {
        int t[MAX_EVENTS];
        int tot_ev = 0;

        for (i = 0; i < MAX_EVENTS; i++) {
            t[i] = i;
            tot_ev += muxevent_exec_count[i];
        }
    
        for (i = 0; i < (MAX_EVENTS - 1); i++)
            for (j = i + 1; j < MAX_EVENTS; j++)
                if (muxevent_exec_count[t[i]] > muxevent_exec_count[t[j]]) {
                    int s = t[i];

                    t[i] = t[j];
                    t[j] = s;
                }
        
        /* Then, display */
        notify(player, "Event history (by use)");
        for (i = 0; i < MAX_EVENTS; i++)
            if (muxevent_exec_count[t[i]])
                notify(player, tprintf("%-3d%-20s%10d %.3f%%", t[i], muxevent_names[t[i]], muxevent_exec_count[t[i]], ((float) 100.0 * muxevent_exec_count[t[i]] / (tot_ev ? tot_ev : 1))));

            return;
    }
    
    notify(player, "Events by type: ");
    notify(player, "-------------------------------");
    k = muxevent_last_type();
    
    for (i = 0; i <= k; i++) {
        j = muxevent_count_type(i);
        if (!j)
            continue;
        tot += j;
        notify(player, tprintf("%-20s%d", muxevent_names[i], j));
    }
    
    if (tot)
        notify(player, "-------------------------------");
    
    notify(player, tprintf("%d total", tot));
    notify(player, tprintf("%d scheduled - %d executed / %d zombies.", events_scheduled, events_executed, events_zombies));

    if ((i = abs(events_scheduled - (j = (tot + events_executed + events_zombies)))))
        notify(player, tprintf("ERROR: %d events %s!", i, events_scheduled > j ? "missing" : "too many"));
    else
        notify(player, "Events seem to have been executed perfectly.");
}


void prerun_event(EVENT * e)
{
    static char buf[LBUF_SIZE];
    MECH *mech = (MECH *) e->data;

    /* Magic 2-hour uptime means that we are 'supposedly' stable
       [ read: crashy as hell, but.. :> you never know ] */
    if (muxevent_tick <= 7200) {
        if (muxevent_mech_event[(int) e->type])
            sprintf(buf, "< %s event for #%d[%s] driven by #%d[%s] >", muxevent_names[(int) e->type], mech->mynum, GetMechID(mech), MechPilot(mech), Good_obj(MechPilot(mech)) ? Name(MechPilot(mech)) : "Nobody");
        else
            sprintf(buf, "< %s event >", muxevent_names[(int) e->type]);
        mudstate.debug_cmd = buf;
    }
}

void postrun_event(EVENT * e)
{
    muxevent_exec_count[(int) e->type]++;
}
