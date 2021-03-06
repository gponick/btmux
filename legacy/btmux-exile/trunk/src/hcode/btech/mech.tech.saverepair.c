#include "mech.h"
#include "mech.events.h"
#include "mech.tech.h"

static FILE *cheat_file;
static int ev_type;

#define CHESA(var) fwrite(&var, sizeof(var), 1, cheat_file)
#define CHELO(var)  if (!fread(&var, sizeof(var), 1, f)) return

static void save_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int data = (int) e->data2;
    int t;

    t = e->tick - event_tick;
    t = MAX(1, t);
    if (e->function == very_fake_func)
	t = 0 - t;
    CHESA(mech->mynum);
    CHESA(ev_type);
    CHESA(t);
    CHESA(data);
}

void saverepairs(FILE * f)
{
    int i;
    dbref d = -1;

    cheat_file = f;
    for (i = FIRST_TECH_EVENT; i <= LAST_TECH_EVENT; i++) {
	ev_type = i;
	event_gothru_type(i, save_event);
    }
    CHESA(d);
}

void loadrepairs(FILE * f)
{
    dbref d, player;
    int type;
    int data;
    int time;
    MECH *mech;
    int loaded = 0;
    int fake;

    if (feof(f))
	return;
    fread(&d, sizeof(d), 1, f);
    while (d > 0 && !feof(f)) {
	loaded++;
	CHELO(type);
	CHELO(time);
	CHELO(data);
	fake = (time < 0);
	time = abs(time);
	if (!(mech = FindObjectsData(d)))
	    continue;
	player = data / PLAYERPOS;
	data = data % PLAYERPOS;
	if (fake)
	    FIXEVENT(time, mech, data, very_fake_func, type);
	else
	    switch (type) {
	    case EVENT_REPAIR_MOB:
		FIXEVENT(time, mech, data, event_mech_mountbomb, type);
		break;
	    case EVENT_REPAIR_UMOB:
		FIXEVENT(time, mech, data, event_mech_umountbomb, type);
		break;
	    case EVENT_REPAIR_REPL:
		FIXEVENT(time, mech, data, event_mech_repairpart, type);
		break;
	    case EVENT_REPAIR_REPLG:
		FIXEVENT(time, mech, data, event_mech_replacegun, type);
		break;
	    case EVENT_REPAIR_REPAP:
		FIXEVENT(time, mech, data, event_mech_repairpart, type);
		break;
	    case EVENT_REPAIR_REPAG:
		FIXEVENT(time, mech, data, event_mech_repairgun, type);
		break;
	    case EVENT_REPAIR_REAT:
		FIXEVENT(time, mech, data, event_mech_reattach, type);
		break;
	    case EVENT_REPAIR_RELO:
		FIXEVENT(time, mech, data, event_mech_reload, type);
		break;
	    case EVENT_REPAIR_FIX:
		FIXEVENT(time, mech, data, event_mech_repairarmor, type);
		break;
	    case EVENT_REPAIR_FIXI:
		FIXEVENT(time, mech, data, event_mech_repairinternal,
		    type);
		break;
	    case EVENT_REPAIR_SCRL:
		FIXEVENT(time, mech, data, event_mech_removesection, type);
		break;
	    case EVENT_REPAIR_SCRG:
		FIXEVENT(time, mech, data, event_mech_removegun, type);
		break;
	    case EVENT_REPAIR_SCRP:
		FIXEVENT(time, mech, data, event_mech_removepart, type);
		break;
	    }
	CHELO(d);
    }
    if (loaded)
	fprintf(stderr, "LOADED: %d tech events.\n", loaded);
}
