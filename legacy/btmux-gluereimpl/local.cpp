#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "db.h"
#include "stringutil.h"
#include "externs.h"
#include "flags.h"

#include "btech.h"

/* local.cpp
 *
 * Inspired by Penn's local extensions; implemented for MUX2.4 by
 * M. Hassman (June 2005)
 */

BTech btech;

// Called after all normal MUX initialization is complete
//
void local_startup(void)
{
	dbref i;
	char *name = "BTech Master Object";		// name of master object, this should be in a conf file somewhere
											// or just store the dbref of this object

	for(i=0;i<=mudstate.db_top-1;i++)		// cycle through database looking for object
		if(!*name || string_match(PureName(i), name))
			break;

	if(i == mudstate.db_top-1)				// if it wasnt found, create it
		i = create_obj(GOD, TYPE_THING, name, 0);

	btech.Init(i);
	btech.LoadSpecialObjects();
	btech.LoadEconDatabase();
	btech.InitTimer();
}

// Called prior to the game database being dumped.   Called by the
// periodic dump timer, @restart, @shutdown, etc.  The argument
// dump_type is one of the 5 DUMP_I_x defines declared in externs.h
//
void local_dump_database(int dump_type)
{
	btech.SaveSpecialObjects();
}

// Called when the game is shutting down, after the game database has
// been saved but prior to the logfiles being closed.
//
void local_shutdown(void)
{
	btech.Shutdown();
}

// Called after the database consistency check is completed.   Add
// checks for local data consistency here.
//
void local_dbck(void)
{
	btech.CheckSpecialObjects();
	btech.CheckEconDatabase();
}

// Called when a player connects or creates at the connection screen.
// isnew of 1 indicates it was a creation, 0 is for a connection.
// num indicates the number of current connections for player.
//
void local_connect(dbref player, int isnew, int num)
{
}

// Called when player disconnects from the game.  The parameter 'num' is
// the number of connections the player had upon being disconnected.
// Any value greater than 1 indicates multiple connections.
//
void local_disconnect(dbref player, int num)
{
}

// Called after any object type is created.
//
void local_data_create(dbref object)
{
}

// Called when an object is cloned.  clone is the new object created
// from source.
//
void local_data_clone(dbref clone, dbref source)
{
}

// Called when the object is truly destroyed, not just set GOING
//
void local_data_free(dbref object)
{
	if(btech.Is_SpecialObject(object))
		btech.DisposeSpecialObject(object);
}

// hooks the fh_ function, is called when a flag is set or unset
int local_handled_flag(dbref target, dbref player, FLAG flag, int fflags, bool reset)
{
	if (flag & MARK_9) {
		if (reset) 
			btech.DisposeSpecialObject(target);
		else
		    btech.NewSpecialObject(target);
	    return 1;
	} else 
		return 0;
}

// Called before mux function lookup to enable builtin functions to be overridden
FUN *local_handled_function(char *fname, int n)
{
	funcmap::iterator iter;
	iter = btech.m_mFunc.find(fname);
	if(iter != btech.m_mFunc.end())
		return iter->second;
	else
		return NULL;
}

// Called before the main command parsing loop to enable builtin commands to be overridden
int local_handled_command(dbref executor, char *pCommand)
{
	if(btech.HandledCommand(executor, pCommand))
		return 1;
	else
		return 0;
}
