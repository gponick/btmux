/* local.cpp
 *
 * Inspired by Penn's local extensions; implemented for MUX2.4 by
 * M. Hassman (June 2005)
 */

#include <map>

#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "externs.h"
#include "functions.h"
#include "command.h"
// btech includes start here
#include "mech.h"
#include "glue.h"
#include "p.glue.h"
#include "p.glue.scode.h"

#include "python.h"

Python *python;

// ----------------------------------------------------------------------------
// local_funlist: List of existing functions in alphabetical order.
//
//   Name          Handler      # of args   min #    max #   flags  permissions
//                               to parse  of args  of args
//
FUN local_funlist[] =
{
    {"BTADDPARTS",          fun_btaddparts,         MAX_ARG, 3, 3, 0,   CA_WIZARD},
    {"BTARMORSTATUS",       fun_btarmorstatus,      MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTCRITSTATUS",        fun_btcritstatus,       MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTDAMAGEMECH",        fun_btdamagemech,       MAX_ARG, 7, 7, 0,   CA_WIZARD},
    {"BTDAMAGES",           fun_btdamages,          MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTDESIGNEX",          fun_btdesignex,         MAX_ARG, 1, 1, 0,   CA_PUBLIC},
    {"BTGETCHARVALUE",      fun_btgetcharvalue,     MAX_ARG, 3, 3, 0,   CA_WIZARD},
    {"BTGETXCODEVALUE",     fun_btgetxcodevalue,    MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTLOADMAP",           fun_btloadmap,          MAX_ARG, 2, 3, 0,   CA_WIZARD},
    {"BTLOADMECH",          fun_btloadmech,         MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTMAKEMECHS",         fun_btmakemechs,        MAX_ARG, 2, 5, 0,   CA_WIZARD},
    {"BTMAKEPILOTROLL",     fun_btmakepilotroll,    MAX_ARG, 3, 3, 0,   CA_WIZARD},
    {"BTMAPELEV",           fun_btmapelev,          MAX_ARG, 3, 3, 0,   CA_WIZARD},
    {"BTMAPTERR",           fun_btmapterr,          MAX_ARG, 3, 3, 0,   CA_WIZARD},
    {"BTMECHFREQS",         fun_btmechfreqs,        MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTPARTMATCH",         fun_btpartmatch,        MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTPARTNAME",          fun_btpartname,         MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTSETARMORSTATUS",    fun_btsetarmorstatus,   MAX_ARG, 4, 4, 0,   CA_WIZARD},
    {"BTSETCHARVALUE",      fun_btsetcharvalue,     MAX_ARG, 4, 4, 0,   CA_WIZARD},
    {"BTSETXCODEVALUE",     fun_btsetxcodevalue,    MAX_ARG, 3, 3, 0,   CA_WIZARD},
    {"BTSTORES",            fun_btstores,           MAX_ARG, 1, 2, 0,   CA_WIZARD},
    {"BTTECHSTATUS",        fun_bttechstatus,       MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTTHRESHOLD",         fun_btthreshold,        MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTUNDERREPAIR",       fun_btunderrepair,      MAX_ARG, 1, 1, 0,   CA_BUILDER},
    {"BTWEAPONSTATUS",      fun_btweaponstatus,     MAX_ARG, 1, 2, 0,   CA_WIZARD},
    {"BTADDSTORES",         fun_btaddstores,        MAX_ARG, 3, 3, 0,   CA_WIZARD},
    {"BTARMORSTATUS_REF",   fun_btarmorstatus_ref,  MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTCHARLIST",          fun_btcharlist,         MAX_ARG, 1, 2, 0,   CA_WIZARD},
    {"BTCRITSLOT",          fun_btcritslot,         MAX_ARG, 3, 4, 0,   CA_WIZARD},
    {"BTCRITSLOT_REF",      fun_btcritslot_ref,     MAX_ARG, 3, 4, 0,   CA_WIZARD},
    {"BTCRITSTATUS_REF",    fun_btcritstatus_ref,   MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTENGRATE",           fun_btengrate,          MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTENGRATE_REF",       fun_btengrate_ref,      MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTFASABASECOST_REF",  fun_btfasabasecost_ref, MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTGETBV",             fun_btgetbv,            MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTGETBV_REF",         fun_btgetbv_ref,        MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTGETPARTCOST",       fun_btgetpartcost,      MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTGETRANGE",          fun_btgetrange,         MAX_ARG, 3, 5, 0,   CA_WIZARD},                                             
    {"BTGETREALMAXSPEED",   fun_btgetrealmaxspeed,  MAX_ARG, 1, 1, 0,   CA_WIZARD},                                    
    {"BTGETREFTECH_REF",    fun_btgetreftech_ref,   MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTGETWEIGHT",         fun_btgetweight,        MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTGETXCODEVALUE_REF", fun_btgetxcodevalue_ref,MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTHEXEMIT",           fun_bthexemit,          MAX_ARG, 4, 4, 0,   CA_WIZARD},
    {"BTHEXINBLZ",          fun_bthexinblz,         MAX_ARG, 3, 3, 0,   CA_WIZARD},
    {"BTHEXLOS",            fun_bthexlos,           MAX_ARG, 3, 3, 0,   CA_WIZARD},
    {"BTID2DB",             fun_btid2db,            MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTLISTBLZ",           fun_btlistblz,          MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTLOSM2M",            fun_btlosm2m,           MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTMAPEMIT",           fun_btmapemit,          MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTNUMREPJOBS",        fun_btnumrepjobs,       MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTPARTTYPE",          fun_btparttype,         MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTPARTWEIGHT",        fun_btgetweight,        MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTPAYLOAD_REF",       fun_btpayload_ref,      MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTREMOVESTORES",      fun_btremovestores,     MAX_ARG, 3, 3, 0,   CA_WIZARD},
    {"BTSETMAXSPEED",       fun_btsetmaxspeed,      MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTSETPARTCOST",       fun_btsetpartcost,      MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTSETXY",             fun_btsetxy,            MAX_ARG, 5, 5, 0,   CA_WIZARD},
    {"BTSHOWCRITSTATUS_REF",fun_btshowcritstatus_ref,MAX_ARG,3, 3, 0,   CA_WIZARD},
    {"BTSHOWSTATUS_REF",    fun_btshowstatus_ref,   MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTSHOWWSPECS_REF",    fun_btshowwspecs_ref,   MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"BTTECH_REF",          fun_bttech_ref,         MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTTECHLIST",          fun_bttechlist,         MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTTECHLIST_REF",      fun_bttechlist_ref,     MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTTECHTIME",          fun_bttechtime,         MAX_ARG, 0, 0, 0,   CA_WIZARD},
    {"BTUNITFIXABLE",       fun_btunitfixable,      MAX_ARG, 1, 1, 0,   CA_WIZARD},
    {"BTWEAPONSTATUS_REF",  fun_btweaponstatus_ref, MAX_ARG, 1, 2, 0,   CA_WIZARD},
    {"BTWEAPSTAT",          fun_btweapstat,         MAX_ARG, 2, 2, 0,   CA_WIZARD},
    {"PYTHON",              fun_python,             MAX_ARG, 1, MAX_ARG, 0, CA_WIZARD},
};

// ---------------------------------------------------------------------------
// Local command tables: Definitions for local hardcode commands.
//
//   Name       Switches    Permissions    Key Calling Seq   hook mask  Handler
//
CMDENT_NO_ARG local_command_table_no_arg[] =
{
    {NULL,          NULL,       0,           0,          0,          0, NULL}
};

CMDENT_ONE_ARG local_command_table_one_arg[] =
{
    {NULL,          NULL,       0,           0,          0,          0, NULL}
};

CMDENT_ONE_ARG_CMDARG local_command_table_one_arg_cmdarg[] =
{
    {NULL,          NULL,       0,           0,          0,          0, NULL}
};

CMDENT_TWO_ARG local_command_table_two_arg[] =
{
    {NULL,          NULL,       0,           0,          0,          0, NULL}
};

CMDENT_TWO_ARG_CMDARG local_command_table_two_arg_cmdarg[] =
{
    {NULL,          NULL,       0,           0,          0,          0, NULL}
};

CMDENT_TWO_ARG_ARGV local_command_table_two_arg_argv[] =
{
    {NULL,          NULL,       0,           0,          0,          0, NULL}
};

CMDENT_TWO_ARG_ARGV_CMDARG local_command_table_two_argv_cmdarg[] =
{
    {NULL,          NULL,       0,           0,          0,          0, NULL}
};

    BTECHDATA btechconf;
CONF2 btechconftable[] = {      
    {"btech_engine",                cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_engine,                 NULL,  0},
    {"btech_failures",              cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_parts,                  NULL,  0},
    {"btech_ic",                    cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_ic,                     NULL,  0},
    {"btech_afterlife_dbref",       cf_int,     CA_GOD,     CA_GOD,         &btechconf.afterlife_dbref,              NULL,  0},
    {"btech_afterscen_dbref",       cf_int,     CA_GOD,     CA_GOD,         &btechconf.afterscen_dbref,              NULL,  0},
    {"btech_vcrit",                 cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_vcrit,                  NULL,  0},
    {"btech_dynspeed",              cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_dynspeed,               NULL,  0},
    {"btech_slowdown",              cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_slowdown,               NULL,  0},
    {"btech_fasaturn",              cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_fasaturn,               NULL,  0},
    {"btech_fasacrit",              cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_fasacrit,               NULL,  0},
    {"btech_fasaadvvtolcrit",       cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_fasaadvvtolcrit,        NULL,  0},
    {"btech_fasaadvvhlcrit",        cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_fasaadvvhlcrit,         NULL,  0},
    {"btech_fasaadvvhlfire",        cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_fasaadvvhlfire,         NULL,  0},
    {"btech_divrotordamage",        cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_divrotordamage,         NULL,  0},
    {"btech_moddamagewithrange",    cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_moddamagewithrange,     NULL,  0},
    {"btech_moddamagewithwoods",    cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_moddamagewithwoods,     NULL,  0},
    {"btech_hotloadaddshalfbthmod", cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_hotloadaddshalfbthmod,  NULL,  0},
    {"btech_nofusionvtolfuel",      cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_nofusionvtolfuel,       NULL,  0},
    {"btech_tankfriendly",          cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_tankfriendly,           NULL,  0},
    {"btech_newcharge",             cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_newcharge,              NULL,  0},
    {"btech_newterrain",            cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_newterrain,             NULL,  0},
    {"btech_xploss",                cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_xploss,                 NULL,  0},
    {"btech_critlevel",             cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_critlevel,              NULL,  0},
    {"btech_tankshield",            cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_tankshield,             NULL,  0},
    {"btech_newstagger",            cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_newstagger,             NULL,  0},
    {"btech_skidcliff",             cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_skidcliff,              NULL,  0},
    {"btech_xp_bthmod",             cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_xp_bthmod,              NULL,  0},
    {"btech_xp_missilemod",         cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_xp_missilemod,          NULL,  0},
    {"btech_xp_ammomod",            cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_xp_ammomod,             NULL,  0},
    {"btech_defaultweapdam",        cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_defaultweapdam,         NULL,  0},
    {"btech_xp_modifier",           cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_xp_modifier,            NULL,  0},
    {"btech_defaultweapbv",         cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_defaultweapbv,          NULL,  0},
    {"btech_xp_usePilotBVMod",      cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_xp_usePilotBVMod,       NULL,  0},
    {"btech_oldxpsystem",           cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_oldxpsystem,            NULL,  0},
    {"btech_xp_vrtmod",             cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_xp_vrtmod,              NULL,  0},
    {"btech_extendedmovemod",       cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_extendedmovemod,        NULL,  0},
    {"btech_stacking",              cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_stacking,               NULL,  0},
    {"btech_stackdamage",           cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_stackdamage,            NULL,  0},
    {"btech_mw_losmap",             cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_mw_losmap,              NULL,  0},
    {"btech_use_tech_bsuit",        cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_use_tech_bsuit,         NULL,  0},
    {"btech_usedmechstore",         cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_usedmechstore,          NULL,  0},
    {"btech_ooc_comsys",            cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_ooc_comsys,             NULL,  0},
    {"btech_complexrepair",         cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_complexrepair,          NULL,  0},
    {"hudinfo_show_mapinfo",        cf_int,     CA_GOD,     CA_GOD,         &btechconf.hudinfo_show_mapinfo,         NULL,  0},
    {"hudinfo_enabled",             cf_int,     CA_GOD,     CA_GOD,         &btechconf.hudinfo_enabled,              NULL,  0},
    {"btech_limitedrepairs",        cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_limitedrepairs,         NULL,  0},
    {"btech_stackpole",             cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_stackpole,              NULL,  0},
    {"phys_use_pskill",             cf_int,     CA_GOD,     CA_GOD,         &btechconf.phys_use_pskill,              NULL,  0},             
    {"btech_erange",                cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_erange,                 NULL,  0},             
    {"btech_dig_only_fs",           cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_dig_only_fs,            NULL,  0},
    {"btech_digbonus",              cf_int,     CA_GOD,     CA_GOD,         &btechconf.btech_digbonus,               NULL,  0},
    {"have_specials",             cf_bool,        CA_STATIC, CA_PUBLIC,   (int *)&btechconf.have_specials,     NULL,        0},
    {"hcode_database",            cf_string_dyn,  CA_GOD,    CA_GOD,      (int *)&btechconf.hcode_db,        NULL, SIZEOF_PATHNAME},
    {"econ_database",             cf_string_dyn,  CA_GOD,    CA_GOD,      (int *)&btechconf.econ_db,         NULL, SIZEOF_PATHNAME},
    {"mech_database",             cf_string_dyn,  CA_GOD,    CA_GOD,      (int *)&btechconf.mech_db,         NULL, SIZEOF_PATHNAME},
    {"map_database",              cf_string_dyn,  CA_GOD,    CA_GOD,      (int *)&btechconf.map_db,          NULL, SIZEOF_PATHNAME},
    { NULL,                       NULL,           0,         0,           NULL,                            NULL,    0},
};

// add function to mux scheduler, and recall itself every 1s
void dispatch_btech(void *pUnused, int iUnused) {
    UpdateSpecialObjects();

    CLinearTimeAbsolute ltaNextTime;
    ltaNextTime.GetUTC();
    ltaNextTime += time_1s;
    scheduler.DeferTask(ltaNextTime, PRIORITY_SYSTEM+1, dispatch_btech, 0, 0);
}

// Called after all normal MUX initialization is complete
//
void local_startup(void)
{
    // Add additional hardcode functions to the above table.
    //
    functions_add(local_funlist);

    // Add additional CMDENT_NO_ARG commands to the above table.
    //
    commands_no_arg_add(local_command_table_no_arg);
    commands_one_arg_add(local_command_table_one_arg);
    commands_one_arg_cmdarg_add(local_command_table_one_arg_cmdarg);
    commands_two_arg_add(local_command_table_two_arg);
    commands_two_arg_cmdarg_add(local_command_table_two_arg_cmdarg);
    commands_two_arg_argv_add(local_command_table_two_arg_argv);
    commands_two_arg_argv_cmdarg_add(local_command_table_two_argv_cmdarg);

    python = new Python;
    python->Load();

    LoadSpecialObjects();

    // start running UpdateSpecialObjects() in 15 seconds to allow everything a chance to get going
    CLinearTimeAbsolute ltaNextTime;
    ltaNextTime.GetUTC();
    ltaNextTime += time_15s;
    scheduler.DeferTask(ltaNextTime, PRIORITY_SYSTEM+1, dispatch_btech, 0, 0);


    btechconf.config_file = StringClone("btech.conf");
    btechconf.hcode_db = StringClone("data/hcode.db");
    btechconf.econ_db = StringClone("data/econ.db");
    btechconf.mech_db = StringClone("mechs");
    btechconf.map_db = StringClone("maps");
    btechconf.have_specials = 1;
    
    FILE *fp = fopen(btechconf.config_file, "rb");
    if (fp == NULL) 
        return; // add log entry

    char *buf = alloc_lbuf("local_startup");
    fgets(buf, LBUF_SIZE, fp);
    while(!feof(fp)) {
        char *zp = buf;
        // Remove comments.  Anything after the '#' is a comment except if it
        // matches:  whitespace + '#' + digit.
        //
        while (*zp != '\0')
            if (*zp == '#' && (zp <= buf || !mux_isspace(zp[-1]) || !mux_isdigit(zp[1]))) 
                *zp = '\0';     // found a comment
            else
                zp++;
        
        // Trim trailing spaces.
        //
        while (buf < zp && mux_isspace(zp[-1])) 
            *(--zp) = '\0';
        
        // Process line.
        //
        char *cp = buf;

        // Trim leading spaces.
        //
        while (mux_isspace(*cp)) 
            cp++;
        
        // Skip over command.
        //
        char *ap;
        for (ap = cp; *ap && !mux_isspace(*ap); ap++) {   
            ; // Nothing.
        }
        
        // Terminate command.
        //
        if (*ap)   
            *ap++ = '\0';
        
        // Skip spaces between command and argument.
        //
        while (mux_isspace(*ap)) 
            ap++;

        if (*cp) {
            CONF2 *tp;
            int i;
            char *buff = 0;

            // Search the config parameter table for the command. If we find
            // it, call the handler to parse the argument.
            //
            for (tp = btechconftable; tp->pname; tp++) {
                if (!strcmp(tp->pname, cp)) {
                    buff = alloc_lbuf("local_startup");
                    strcpy(buff, ap);
                    i = tp->interpreter(tp->loc, ap, tp->pExtra, tp->nExtra, GOD, cp);
                }
            }
        }

        fgets(buf, LBUF_SIZE, fp);
    }
    free_lbuf(buf);
    fclose(fp);
}

// This is called prior to the game syncronizing its own state to its own
// database.  If you depend on the the core database to store your data, you
// need to checkpoint your changes here. The write-protection
// mechanism in MUX is not turned on at this point.  You are guaranteed
// to not be a fork()-ed dumping process.
//
void local_presync_database(void)
{
}

// Like the above routine except that it called from the SIGSEGV handler.
// At this point, your choices are limited. You can attempt to use the core
// database. The core won't stop you, but it is risky.
//
void local_presync_database_sigsegv(void)
{
}

// This is called prior to the game database writing out it's own database.
// This is typically only called from the fork()-ed process so write-
// protection is in force and you will be unable to modify the game's
// database for you own needs.  You can however, use this point to maintain
// your own dump file.
//
// The caveat is that it is possible the game will crash while you are doing
// this, or it is already in the process of crashing.  You may be called
// reentrantly.  Therefore, it is recommended that you follow the pattern in
// dump_database_internal() and write your database to a temporary file, and
// then if completed successfully, move your temporary over the top of your
// old database.
//
// The argument dump_type is one of the 5 DUMP_I_x defines declared in
// externs.h
//
void local_dump_database(int dump_type)
{
    python->Save();

    SaveSpecialObjects(dump_type);
}

// The function is called when the dumping process has completed. Typically,
// this will be called from within a signal handler. Your ability to do
// anything interesting from within a signal handler is severly limited.
// This is also called at the end of the dumping process if either no dumping
// child was created or if the child finished quickly. In fact, this
// may be called twice at the end of the same dump.
//
void local_dump_complete_signal(void)
{
}

// Called when the game is shutting down, after the game database has
// been saved but prior to the logfiles being closed.
//
void local_shutdown(void)
{
    delete python;

    ResetSpecialObjects();
}

// Called after the database consistency check is completed.   Add
// checks for local data consistency here.
//
void local_dbck(void)
{
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
    if(Marker9(object)) 
        DisposeSpecialObject(GOD, object);
}

// btmux specific stuff below here:
//
int local_handled_flag(dbref target, dbref player, FLAG flag, int fflags, bool reset)
{
    if (flag & MARK_9) {
        if(reset) {
            c_Hardcode(target);
            DisposeSpecialObject(player, target);
        } else {
            s_Hardcode(target);
            CreateNewSpecialObject(player, target);
        }
        return 1;
    } else
        return 0;
}

int local_handled_command(dbref executor, char *pCommand)
{
    if(HandledCommand(executor, Location(executor), pCommand)) 
        return 1;
    else {
        char *buffer = alloc_lbuf("local_handled_command");
        char *p = pCommand;
        char *q = buffer;
        char *s;
        int x;

        if(p[0] == ',') {
            *p++;
            if(*p) {
                python->Run(executor, p);
                return 1;
            } else {
                python->Run(executor, "\n");
                return 1;
            }
        }
    
        x = 0;
        strncpy(q, p, LBUF_SIZE);
        while(!mux_isspace(*p) && *p) {    // forward to a space
            *p++; x++;
        }
    
        q[x] = '\0';        // q is everything before the space

        if(string_compare(q,"@python") == 0)
            if(*p) {
                *p++;
                free_lbuf(q);
                python->Run(executor, p);
                return 1;
            } else {
                python->Run(executor, "\n");
                return 1;
            }
        free_lbuf(q);
    }

    return 0;
}
