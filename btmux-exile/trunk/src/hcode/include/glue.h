/*
  Header for special command rooms...
  Based on the original by MUSE folks
*/

/* Parameter to the save/load function */
#ifndef _GLUE_H
#define _GLUE_H

#define VERIFY 0
#define SAVE 1
#define LOAD 2

#define XCODE_VERSION 2

#define SPECIAL_FREE 0
#define SPECIAL_ALLOC 1

#define GFLAG_ALL 0
#define GFLAG_MECH 1
#define GFLAG_GROUNDVEH 2
#define GFLAG_AERO 4
#define GFLAG_DS 8
#define GFLAG_VTOL 16
#define GFLAG_NAVAL 32
#define GFLAG_BSUIT 64
#define GFLAG_MW 128

#include "glue_types.h"

#define Have_MechPower(a,b) (((Powers2((Owner(a))) & (b)) || Wizard(Owner(a))) && Inherits((a)))

typedef struct CommandsStruct {
    int flag;
    char *name;
    char *helpmsg;
    void (*func) ();
} CommandsStruct;

typedef struct SpecialObjectStruct {
    char *type;			/* Type of the object */
    CommandsStruct *commands;	/* Commands array */
    long datasize;		/* Size of private buffer */
    void (*allocfreefunc) ();
    int updateTime;		/* Amount of time between updates */
    /* (secs) */
    void (*updatefunc) ();	/* called for every */
    /* object at every */
    /* update */
    int power_needed;		/* WHat power is needed to do */
    /* restricted commands */
} SpecialObjectStruct;

#ifdef _GLUE_C

#ifdef FT_ENABLED
#include "f_map.h"
#include "f_mob.h"
#endif
#include "p.mech.move.h"
#include "p.debug.h"
#include "turret.h"
#include "p.aero.move.h"
#include "p.mech.maps.h"
#include "p.ds.bay.h"
#include "p.mech.notify.h"
#include "p.mech.utils.h"
#include "p.mech.combat.h"
#include "p.mech.update.h"
#include "p.mechrep.h"
#include "p.mech.restrict.h"
#include "p.mech.advanced.h"
#include "p.mech.tic.h"
#include "p.ds.turret.h"
#include "p.mech.contacts.h"
#include "p.mech.status.h"
#include "p.mech.scan.h"
#include "p.mech.sensor.h"
#include "p.map.h"
#include "p.mech.pickup.h"
#include "p.eject.h"
#include "p.mech.c3.h"
#include "p.bsuit.h"
#include "p.mech.startup.h"
#include "p.mech.consistency.h"
#include "p.mech.physical.h"
#include "mech.tech.h"
#include "p.mech.tech.repairs.h"
#include "p.glue.scode.h"
#include "mechrep.h"
#include "p.mine.h"
#include "mech.custom.h"
#include "p.mech.custom.h"
#include "scen.h"
#include "p.btechstats.h"
#include "autopilot.h"
#include "p.events.h"
#include "p.mech.tech.commands.h"

void newautopilot(dbref, void **, int);
void newturret(dbref, void **, int);
void newfreemech(dbref, void **, int);

ECMD(f_mapblock_set);
ECMD(f_mapblock_setxy);
ECMD(ListForms);
ECMD(initiate_ood);
ECMD(mech_Raddstuff);
ECMD(mech_Rfixstuff);
ECMD(mech_Rremovestuff);
ECMD(mech_Rresetstuff);
ECMD(mech_bomb);
ECMD(mech_loadcargo);
ECMD(mech_losemit);
ECMD(mech_manifest);
ECMD(mech_stores);
ECMD(mech_domystuff);
ECMD(mech_unloadcargo);
ECMD(tech_magic);
ECMD(mech_inferno);
ECMD(mech_swarm);
ECMD(mech_swarm1);
ECMD(mech_dig);
ECMD(mech_standcheck);
ECMD(mech_vector);

ECMD(f_map_loadmap);

ECMD(f_draw);
ECMD(f_sheath);
ECMD(f_hold);
ECMD(f_put);

ECMD(f_shout);
ECMD(f_emote);
ECMD(f_say);
ECMD(f_whisper);

/* Flag: 0 = all, 1=mech, 2=groundveh, 4=aero, 8=ds, 16=vtol */

/* Categories:
   - Movement
   - Radio
   - Weapons
   - Physical
   - Status
   - Navigation
   - Repairing
   - Special
   - Information
   - TICs
   */


#define SHEADER(a,b) \
{ a, b, b, NULL }
#define HEADER(a) SHEADER(0,a)

#ifdef BT_ENABLED

CommandsStruct mechcommands[] = {
    /* Movement */
    HEADER("Movement"), 
    {0, "HEADING [num] [R|L]", "Shows/Changes your heading to <NUM> (<NUM> in degrees), optionally relativly turn right or left",
	mech_heading},
    {0, "SPEED [num|walk|run|stop|back|flank|cruise]",
	    "Without arguments shows your present speed, otherwise changes your speed to <NUM> or \
		the specified speed (run/cruise = 1x maxspeed, walk/flank = 2/3x maxspeed, stop = 0, back = -2/3x maxspeed)",
	mech_speed},
    {195, "SPRINT", "Toggles sprinting mode.", mech_sprint}, 
    {195, "EVADE", "Toggles evade mode.", mech_evade}, 
    {1, "DODGE", "Toggles dodge mode.", mech_dodge},
    {48, "VERTICAL [num]", "Shows/Changes your vertical speed to <NUM>.", mech_vertical}, 
    {12, "CLIMB [angle]", "Shows/Changes the climbing angle to <NUM>.", aero_climb},
    {12, "AEROSTATUS", "Itemizes a list of crits to your aerospace fighter/Dropship.", aero_status},
    {12, "DIVE [angle]", "Shows/Changes the diving angle to <NUM>.", aero_dive},
    {12, "THRUST [num|hover|atmo|turn|rturn|safe|over|rsafe|rover|<+|-> <#>]", "Sets thrust to <NUM> or the specified \
	speed name. [#] is read for [+|-]. No argument returns status info.", aero_thrust},
    {19, "LATERAL [fl|fr|rl|rr|-]", "Change your lateral movement mode (quad only). fl/fr/rl/rr = Directions, - = Disable lateral movement.",
	mech_lateral},
    {129, "STAND", "Stand up after a fall or dropping prone.", mech_stand},
    {129, "STANDCHECK", "Shows the BTH to stand up.", mech_standcheck},
    {1, "PRONE", "Force your 'mech to drop prone where it is.", mech_drop},
    {67, "JUMP [<TARGET-ID>|<BEARING> <RANGE>]", "Jump on default target / given target / bearing + range.", mech_jump},
    {0, "ENTERBASE [N|W|S|E]", "Enters base/hangar/whatnever from selected dir.", mech_enterbase}, 
    {0, "ENTERBAY [REF]", "Enters bay of a moving(?) hangar (DropShip for example). Ref is target ref, and it is optional.", mech_enterbay}, 
    /* Radio */
    HEADER("Radio"),
    {0, "LISTCHANNELS", "Lists set frequencies + comtitles for them.", mech_list_freqs},
    {0, "SENDCHANNEL <LETTER>=<STRING>", "Sends <string> on channel <letter>'s freq.", mech_sendchannel},
    {0, "RADIO <ID>=<STRING>", "Radioes (LOS) <ID> with <STRING>", mech_radio},
    {0, "SETCHANNELFREQ <LETTER>=<NUMBER>", "Sets channel <letter> to frequency <number>.", mech_set_channelfreq},
    {0, "SETCHANNELMODE <LETTER>=<STRING>", "Sets channel <letter> mode <string> (available letters: DIUES, color codes).", mech_set_channelmode},
    {0, "SETCHANNELTITLE <LETTER>=<STRING>", "Sets channel <letter> comtitle to <string>.", mech_set_channeltitle}, 
    /* Weapons */
    HEADER("Weapons"),
    {0, "LOCK [<TARGET-ID>|<X> <Y> |<X> <Y> <B|H|I|C> | -]",
	    "Sets the target to the (3rd argument :  B = building, C = clear, I = ignite,\
		H = hex (clear/ignite/break ice/destroy bridge)) /, - = Clears present lock.",
	mech_settarget},
    {0, "SIGHT <WEAPNUM> [<TARGET-ID>|<X> <Y>]", "Computes base-to-hit for given weapon and target.", mech_sight},
    {0, "FIRE <WEAPNUM> [<TARGET-ID>|<X> <Y>]", "Fires weapon <weapnum> at def. target or specified target.", mech_fireweapon},
    {0, "TARGET <SECTION|->", "Sets your aimed shot target / Disables targetting.", mech_target},
    {0, "AMS <weapnum>", "Toggles Anti-Missile System on and off.", mech_ams},
    {0, "SGUIDED <weapnum>", "Sets weapon to and from SGUIDED Mode", mech_sguided},
    {0, "STINGER <weapnum>", "Sets weapon to and from STINGER Mode", mech_stinger},
    {0, "DEADFIRE <weapnum>", "Sets a weapon to and from DEADFIRE Mode", mech_deadfire},
    {0, "ATMER <weapnum>", "Sets a weapon to and from ATMER Mode", mech_atmer},    
    {0, "ATMHE <weapnum>", "Sets a weapon to and from ATMHE Mode", mech_atmhe},
    {0, "ARTEMIS <weapnum>", "Sets Weapon to and from ARTEMIS Mode.", mech_artemis},
    {0, "EXPLOSIVE <weapnum>", "Toggles between explosive/normal rounds", mech_explosive},
    {0, "FIRECLUSTER <weapnum>", "Sets/unsets artillery weapon to fire cluster rounds.", mech_cluster},
    {0, "FIREMINE <weapnum>", "Sets/unsets artillery weapon to fire mine rounds.", mech_mine},
    {0, "FIRESMOKE <weapnum>", "Sets/unsets artillery weapon to fire smoke rounds.", mech_smoke},
    {0, "FIREINCEND <weapnum>", "Sets/unsets artillery weapon to fire indendiary rounds.", mech_incendarty},
    {0, "FIRESWARM <weapnum>", "Sets/Unsets the LRM launcher to shoot swarm missiles", mech_swarm},
    {0, "FIRESWARM1 <weapnum>", "Sets/Unsets the LRM launcher to shoot swarm missiles", mech_swarm1},
    {0, "HOTLOAD <weapnum>", "Sets/Unsets the LRM launcher to hotload missiles, removing short-range penalties, but adding to chance of jamming.",
	mech_hotload},
    {0, "INFERNO <weapnum>", "Sets/Unsets the SRM launcher to shoot inferno missiles", mech_inferno},
    {0, "LBX <weapnum>", "Sets weapon to and from LBX Mode.", mech_lbx},
    {0, "PIERCE <weapnum>", "Sets weapon to and from PIERCE Mode.", mech_pierce},
    {0, "PRECISION <weapnum>", "Sets weapon to and from PRECISION Mode.", mech_precision},
    {0, "CASELESS <weapnum>", "Sets weapon to and from CASELESS Mode.", mech_caseless}, 
    {0, "TRACER <weapnum>", "Sets weapon to and from TRACER mode.", mech_tracer},
    {0, "NARC <weapnum>", "Sets weapon to and from NARC Mode.", mech_narc}, 
    {0, "ULTRA <weapnum>", "Sets weapon to and from Ultra Mode.", mech_ultra},
    {0, "ROTTWO <weapnum>", "Sets weapon to and from Rotary Two Mode.", mech_rottwo},
    {0, "ROTFOUR <weapnum>", "Sets weapon to and from Rotary Four Mode.", mech_rotfour},
    {0, "ROTSIX <weapnum>", "Sets weapon to and from Rotary Six Mode.", mech_rotsix}, 
    {0, "HEAT <weapnum>", "Sets weapon to and from Heat Mode.", mech_heat},
    {0, "DISABLE <weapnum>", "Disables the weapon (Gauss only)", mech_disableweap},
    {0, "CAPACITATE <weapnum>", "Begins Capacitor charging (PPC's only)", mech_capacitate},
    {0, "UNJAM <weapnum>", "Attempt unjamming of weapon.", mech_unjamweap},
    {0, "ADDTIC  <NUM> <WEAPNUM|LOWNUM-HIGHNUM>", "Adds weapon <weapnum>, or weapons from <lownum> to <highnum> to TIC <num>.", mech_addtic},
    {0, "CLEARTIC <NUM>", "Clears the TIC <num>.", mech_cleartic}, 
    {0, "DELTIC <NUM> <WEAPNUM>", "Deletes weapon number <weapnum> from TIC <num>.", mech_deltic},
    {0, "FIRETIC <NUM> [<TARGET>|<X Y>]", "Fires the weapons in TIC <num>.", mech_firetic},
    {0, "LISTTIC <NUM>", "Lists weapons in the TIC <num>.", mech_listtic}, 
    /* Information */
    HEADER("Information"), 
    {0, "BRIEF [<LTR> <VAL>]", "Shows brief status / Sets brief for <ltr> to <val>.", mech_brief},
    {0, "CONTACTS [<Prefix>|<TARGET-ID>]", "List all current contacts", mech_contacts},
    {0, "CRITSTATUS <SECTION>", "Shows the Critical hits status", mech_critstatus},
    {0, "WEAPSTATUS <WEAP#>", "Shows the multiple-crits effects on a weapon.", mech_weapstatus},
    {0, "REPORT [<TARGET-ID>|<X Y>]", "Information on default target, num, or x,y", mech_report},
    {0, "SCAN [<TARGET-ID>|<X Y>|<X Y> <B|H>]", "Scans the default target, chosen target, or hex", mech_scan}, 
    {0, "SENSOR [LONG|[<V|L|I|E|S|B|BL|R> <V|L|I|E|S|B|BL|R>]]", "Shows/Changes your sensor mode (1 argument: Long, \
	otherwise short description about sensor mode)",
		mech_sensor}, 
    {0, "STATUS [A(rmor)|I(nfo)|W(eapons)|S(hort)|H(eat)|R(?)|N(?)]", "Prints the mech's status", mech_status},
    {0, "VIEW [<TARGET-ID>]", "View the war painting on the target", mech_view},
    {0, "WEAPONSPECS", "Shows the specifications for your weapons", mech_weaponspecs}, 
    /* Navigation */
    HEADER("Navigation"),
    {0, "BEARING [<X> <Y>|<X> <Y> <X> <Y>|<X> <Y> <Z> <X> <Y> <Z>]", "Same format as range.", mech_bearing}, 
    {0, "ETA [<X> <Y>]", "Estimates time to target (/default target)", mech_eta},
    {0, "FINDCENTER", "Shows distance/bearing to center of hex.", mech_findcenter},
    {0, "NAVIGATE", "Shows the hex and surroundings graphically", mech_navigate},
    {0, "RANGE [<X> <Y>|<X> <Y> <X> <Y>]", "Range to def. target / range to x y / range to x,y from x,y", mech_range},
    {0, "LRS <M(ech)|T(errain)|E(lev)> [<BEARING> <RANGE> | <TARGET-ID>]", "Shows the long range map", mech_lrsmap},
    {0, "TACTICAL [C(liff)|T(ankcliff)|B(locked DS LZ)] [<BEARING> <RANGE> | <TARGET-ID>]",
	"Shows the tactical map at the mech's location / at bearing and range / around chosen target",
		mech_tacmap},
    {0, "VECTOR [<X Y> <X Y>]", "Same format as range.", mech_vector},
    {0, "SHOWPODS", "Show all (i)NARC pods on your unit and their location.", mech_showpods},
    /* Special */
    HEADER("Special"), 
    {12, "CHECKLZ", "Checks if the landing-zone is good for a landing", aero_checklz},
    {0, "@OOD <X> <Y> [Z]", "@Initiates OOD drop at the orbit altitude to <X> <Y> (optional Z altitude to start from)", initiate_ood},
    {0, "@LOSEMIT <MESSAGE>", "@Sends message to everyone seeing the 'mech right now", mech_losemit},
    {0, "@DAMAGE <NUM> <CLUSTERSIZE> <ISREAR> <ISCRITICAL>",
	"@Causes <NUM> pt of damage to be done to 'mech in <CLUSTERSIZE> point clusters \
		(if <ISREAR> is 1, damage is done to rear arc ; if <ISCRITICAL> is 1, damage does crit-thru-armor)",
	mech_damage},
    {0, "@WEIGHT", "@Checks the weight allocated in the mech", mech_weight},
    {4, "BOMB [list|drop <num>|aim]", "Lists bombs / drops bomb <num> / aims where a bomb would fall.", mech_bomb},
    {2, "DIG", "Starts burrowing for cover [non-hovers only].", mech_dig}, 
    {1, "HULLDOWN", "Begins Quad hulldown maneuver.", mech_dig}, 
    {0, "EXPLODE <AMMO|REACTOR|STOP>",
	"<AMMO|REACTOR> specifies which to ignite ; ammo causes all ammo on your mech to go *bang* (in no particular order),\
		reactor disables control systems. Do note that neither are instant. STOP allows you to stop existing countdown.",
	mech_explode},
    {12, "OVERTHRUST [ON|OFF]", "Enable/Disable throttle to be able to enter Overthrust (if already in tough luck).", mech_overthrust},
    {12, "CRUISE [ON|OFF]", "Enable/Disable Velocity Cruise control in atmosphere for aeros.", mech_cruise},
    {12, "SETCRUISE [#|info]", "Sets your Velocity Cruise Control value. 'info' shows status info about atmo layer", aero_setcruise},
    {0, "SAFETY [ON|OFF]", "Enable/Disable Safeties against killing MechWarriors.", mech_safety},
    {0, "SLWARN [ON|OFF]", "Enable/Disable SearchLight warnings.", mech_slwarn},
    {0, "NORADIO [ON|OFF]", "Enable/Disable SearchLight warnings.", mech_noradio}, 
    {0, "AUTOFALL [ON|OFF]", "Enable/Disable Pilotroll fails.", mech_autofall}, 
    {51, "TURNMODE [TIGHT|STRAFE]", "Set turn mode.", mech_turnmode},
#if 0
    {51, "TURNPERCENT [1-100]", "Sets your optional turn rate in the FASA-style turn code. (no others)", mech_turnpercent},
#endif
    {0, "DROPOFF", "Drops the mech you are carrying.", mech_dropoff},
    {0, "PICKUP [ID]", "Picks up [ID].", mech_pickup},
    {0, "DUMP <WEAPNUM|LOCATION|ALL|STOP> [<CRIT>]",
	"Dumps the ammunition for the weapon / in the location [ crit ] / all ammunition in the 'mech / stops all dumping in progress.", mech_dump},
    {1, "FLIPARMS", "Flips the arms to and from the rear arcs, if possible.", mech_fliparms},
    {0, "NULLSIG", "Turns your NullSignature Device On/Off (only applicable if you have one)", mech_nullsig}, 
    {0, "STEALTHARM", "Turns your StealthArmor On/Off (only applicable if you have one)", mech_stealtharm},
    {0, "ECM", "Turns your Guardian ECM On/Off (only applicable if you have one)", mech_ecm},
    {0, "ECCM", "Turns your Guardian ECM On/Off ECCM Mode (only applicable if you have one)", mech_eccm},
    {0, "DISEMBARK", "Gets the hell out of the 'mech / vehicle.", mech_disembark},
    {0, "UDISEMBARK", "Get a Battlesuit out of  vehicle.", mech_udisembark},
    {0, "EMBARK", "Climb into a 'mech / vehicle", mech_embark},
    {1, "MASC", "Toggles MASC on and off", mech_masc},
    {0, "SCHARGE", "Toggles SuperCharger on and off", mech_scharge}, {0, "LAND", "Terminate your jump or land a VTOL/Aero/DS", mech_land},
    {-35, "TAKEOFF", "VTOL/Aero take off command", aero_takeoff},
    {1, "ROTTORSO <L(eft) | R(ight) | C(enter)>", "Rotates the torso 60 degrees right or left.", mech_rotatetorso},
    {0, "SLITE", "Turns your searchlight on/off", mech_slite}, 
    {0, "SPOT [ID|-|OWNID]", "Sets someone as your spotter / makes you stop spotting / sets you as a spotter.", mech_spot},
    {0, "STARTUP [OVERRIDE]", "Commences startup cycle.", mech_startup},
    {0, "SHUTDOWN", "Shuts down the mech.", mech_shutdown},
    {34, "TURRET", "Set the turret facing.", mech_turret},

#ifdef C3_SUPPORT
    /* C3 */ 
    {0, "C3MESSAGE <MSG>", "Sends a message to all others connected to your C3", mech_c3_message},
    {0, "C3TARGETS", "Shows C3 targeting information available", mech_c3_targets},
    {0, "C3MASTER [ID|-]", "Sets/Unsets your C3 Master", mech_c3_set_master},
#endif 
    {0, "HEATCUTOFF", "Sets your heat dissipation so that you wont go under 9 heat for TSM", heat_cutoff}, 
    /* Physical */
    SHEADER(1, "Physical"),
    {1, "AXE [R|L|B] [<TARGET-ID>]", "Axes a target", mech_axe}, 
    {3, "CHARGE [<TARGET-ID> | - ]", "Charges a target. '-' removes charge command.", mech_charge},
    {1, "CHOP [R|L|B] [<TARGET-ID>]", "Chops target with a sword", mech_sword},
    {1, "CLUB [<TARGET-ID>]", "Clubs a target with a tree", mech_club},
    {1, "BASH [R|L|B] [<TARGET-ID>]", "Bashes a target with a mace", mech_bash},
    {1, "KICK [R|L] [<TARGET-ID>]", "Kicks a target", mech_kick},
    {1, "PUNCH [R|L|B] [<TARGET-ID>]", "Punches a target", mech_punch}, 
    {64, "ATTACKLEG [<TARGET-ID>]", "Attacks legs of the target battlemech", bsuit_attackleg},
    {0, "HIDE", "Attempts to hide your team ; doesn't work if any hostiles have their eyeballs on you", bsuit_hide},
    {64, "SWARM [<TARGET-ID> | -]", "Swarms the target / drop off target (-)", bsuit_swarm},
    {1, "THRASH", "Try to shake off battlesuits swarming you", mech_thrash}, 
    /* Repairing */
    HEADER("Repair"),
    {0, "CHECKSTATUS", "Checks mech's techstatus", tech_checkstatus},
    {0, "DAMAGES", "Shows the mech's damages", show_mechs_damage},
    {0, "FIX [<NUM>|<LOW-HI>]", "Fixes entry <NUM> from mech's damages", tech_fix},
    {0, "FIXARMOR <LOC>", "Repairs armor in <loc>", tech_fixarmor},
    {0, "FIXINTERNAL <LOC>", "Repairs internals in <loc>", tech_fixinternal},
    {0, "REATTACH <LOC>", "Reattaches the limb", tech_reattach},
    {0, "RESEAL <LOC>", "Reseals the limb", tech_reseal}, 
    {0, "RELOAD <LOC> <POS> [TYPE]", "Reloads the ammo compartment in <loc>/<pos> (optionally with [type])", tech_reload},
    {0, "REMOVEGUN <NUM>", "Removes the gun", tech_removegun},
    {0, "REMOVEPART <LOC> <POS>", "Removes the part", tech_removepart},
    {0, "REMOVESECTION <LOC>", "Removes the section", tech_removesection},
    {0, "OADDGUN <NAME> <LOC> <CRIT> [RO]", "Adds gun into empty hardpoint", tech_oaddgun},
    {0, "OREMGUN <NAME> <LOC> <CRIT> [RO]", "Removes gun from hardpoints", tech_oremgun}, 
    {4, "MOUNTBOMB <LOC> <CRIT> <TYPE>", "Adds/Removes bomb from empty critslot.", tech_mountbomb},
    {0, "OADDSP <NAME> <LOC> <CRIT> <DATA>", "Adds special crits into hardpoints", tech_oaddsp}, 
    {0, "OMODAMMO <NAME|EMPTY> <LOC> <CRIT> <H>", "Modify ammo bins", tech_omodammo}, 
    {0, "REPLACEGUN [<NUM> | <LOC> <POS>] [ITEM]", "Replaces the gun in the position (optionally with [ITEM], like Martell.MediumLaser)",
	tech_replacegun}, 
    {0, "REPAIRGUN [<NUM> | <LOC> <POS>]", "Repairs the gun in the position", tech_repairgun},
    {0, "REPLACEPART <LOC> <POS>", "Replaces the part in the position", tech_replacepart},
    {0, "REPAIRPART <LOC> <POS>", "Repairs the part in the position", tech_repairpart},
    {0, "REPAIRS", "Shows repairs/scrapping in progress", tech_repairs},
    {0, "UNLOAD <LOC> <POS>", "Unloads the ammo compartment in <loc>/<pos>", tech_unload}, 
    {0, "@MAGIC", "@Fixes the unfixable - skirt crits etc (wiz-only)", tech_magic}, 
    /* Cargo */
    HEADER("Cargo"), 
    {0, "LOADCARGO <NAME> <COUNT>", "Loads up <COUNT> <NAME>s from the bay.", mech_loadcargo},
    {0, "MANIFEST", "Lists stuff carried by mech.", mech_manifest},
    {0, "STORES", "Lists stuff in the bay.", mech_stores},
    {0, "UNLOADCARGO <NAME> <COUNT>", "Unloads <COUNT> <NAME>s to the bay.", mech_unloadcargo}, 
    /* Restricted commands */
    HEADER("@Restricted"),
    {0, "@CREATEBAYS [.. list of DBrefs, seperated by space]", "@Creates / Disables bays on a DS", mech_createbays},
    {0, "@SETMECH <NAME> <VALUE|DATA>", "@Sets xcode value on object", set_xcodestuff},
    {0, "@SETXCODE <NAME> <VALUE|DATA>", "@Sets xcode value on object", set_xcodestuff},
    {0, "@VIEWXCODE", "@Views xcode values on object", list_xcodestuff}, 
    {0, "SNIPE <ID> <WEAPON>", "@Lets you 'snipe' (=shoot artillery weapons with movement prediction)", mech_snipe},
    {0, "ADDSTUFF <NAME> <COUNT>", "@Adds <COUNT> <NAME> to mech's inventory", mech_Raddstuff}, 
    {0, "FIXSTUFF", "@Fixes consistency errors in econ data", mech_Rfixstuff},
    {0, "CLEARSTUFF", "@Removes all stuff from 'mech", mech_Rresetstuff}, 
    {0, "REMOVESTUFF <NAME> <COUNT>", "@Removes <COUNT> <NAME> from mech's inventory", mech_Rremovestuff},
    {0, "SETMAPINDX <NUM>", "@Sets the mech's map index to num.", mech_Rsetmapindex},
    {0, "SETTEAM <NUM>", "@Sets the teams.", mech_Rsetteam},
    {0, "SETXY <X> <Y>", "@Sets the x & y value of the mech.", mech_Rsetxy},
    {0, NULL, NULL, NULL}
};

ECMD(map_addice);
ECMD(map_delice);
ECMD(map_addsnow);
ECMD(map_delsnow);
ECMD(map_setconditions);
ECMD(map_growforest);
ECMD(map_fixbridge);
ECMD(map_fixwall);

CommandsStruct mapcommands[] = {
    {0, "@VIEWXCODE", "@Views xcode values on object", list_xcodestuff},
    {0, "@SETXCODE <NAME> <VALUE|DATA>", "@Sets xcode value on object", set_xcodestuff},
    {0, "@SETMAP <NAME> <VALUE|DATA>", "@Sets xcode value on object", set_xcodestuff}, 
    {0, "ADDICE <NUMBER>", "@Adds ice (<NUMBER> percent chance for each watery hex connected to land/ice)", map_addice},
    {0, "DELICE <NUMBER>", "@Deletes first-melting ices at <NUMBER> chance", map_delice},
    {0, "GROWFOREST <NUMBER>", "Fixes with <NUMBER> odds all adjacent rough to forest hexes.", map_growforest},
    {0, "FIXBRIDGE <NUMBER>", "Fixed with <NUMBER> odds any damaged bridge back to normal bridge.", map_fixbridge}, 
    {0, "FIXWALL <NUMBER>", "Fixed with <NUMBER> odds any damaged wall back to normal bridge.", map_fixwall},
    {0, "ADDSNOW <NUMBER>", "@Adds snow <NUMBER> (Percentage per hex)", map_addsnow},
    {0, "DELSNOW <NUMBER>", "@Deletes snow at percentage <NUMBER> each hex", map_delsnow},
    {0, "PATHFIND <X1> <Y1> <X2> <Y2> [OPTFACT]",
	"@Finds shortest path from x1,y1 to x2,y2 using A* approx algorithm \
		(using optfact optimization factor, 0-100, smaller = slower, more accurate)",
	map_pathfind},
    {0, "SETCOND <GRAV> <TEMP> [CLOUDBASE [VACUUM]]",
	"@Sets the map attributes (gravity: in 1/100'ths of Earth gravity, temperature: in Celsius, vacuum: optional, number (0 or 1)",
		map_setconditions},
    {0, "VIEW <X> <Y>", "@Shows the map centered at X,Y", map_view}, 
    {0, "ADDBLOCK <X> <Y> <DIST> [TEAM#_TO_ALLOW]", "@Adds no-landings zone of DIST hexes to X Y", map_add_block},
    {0, "ADDMINE <X> <Y> <TYPE> <STRENGTH> [OPT]", "@Adds mine to X,Y", map_add_mine},
    {0, "ADDHEX <X> <Y> <TERRAIN> <ELEV>", "@Changes the terrain and elevation of the given hex", map_addhex},
    {0, "SETLINKED", "@Sets the map linked", map_setlinked},
    {0, "@MAPEMIT <MESSAGE>", "@Emits stuff to the map", map_mapemit},
    {0, "FIXMAP", "@Fixes inconsistencies in map", debug_fixmap},
    {0, "LOADMAP <NAME>", "@Loads the named map", map_loadmap},
    {0, "SAVEMAP <NAME>", "@Saves the map as name", map_savemap},
    {0, "SETMAPSIZE <X> <Y>", "@Sets x and y size of map", map_setmapsize}, 
    {0, "LIST [MECHS | OBJS]", "@Lists mechs/objects on the map", map_listmechs},
    {0, "CLEARMECHS [DBNUM]", "@Clears mechs from the map", map_clearmechs},
    {0, "ADDFIRE [X] [Y] [DURATION]", "@Adds fire that lasts <duration> secs", map_addfire},
    {0, "ADDSMOKE [X] [Y] [DURATION]", "@Adds smoke that lasts <duration> secs", map_addsmoke},
    {0, "DELOBJ [[TYPE] | [X] [Y] | [TYPE] [X] [Y]]", "@Deletes objects of either type or at x/y", map_delobj},
    {0, "UPDATELINKS", "@Updates CodeLinks from the database objs (recursive)", map_updatelinks}, 
    {0, "STORES", "Lists stuff in the hangar.", mech_manifest},
    {0, "ADDSTUFF <NAME> <COUNT>", "@Adds <COUNT> <NAME> to map", mech_Raddstuff}, 
    {0, "FIXSTUFF", "@Fixes consistency errors in econ data", mech_Rfixstuff},
    {0, "REMOVESTUFF <NAME> <COUNT>", "@Removes <COUNT> <NAME> from map", mech_Rremovestuff},
    {0, "CLEARSTUFF", "@Removes all stuff from map", mech_Rresetstuff},
    {0, NULL, NULL, NULL}
};


CommandsStruct mechrepcommands[] = {
    {0, "SETTARGET <NUM>", "@Sets the mech to be repaired/built to num", mechrep_Rsettarget}, 
    {0, "LOADNEW <TYPENAME>", "@Loads a new mech template.", mechrep_Rloadnew},
    {0, "RESTORE", "@Completely repairs and reloads mech. ", mechrep_Rrestore}, 
/* {0,"SAVENEW <TYPENAME>","@Saves the mech as a template.", mechrep_Rsavetemp}, */
    {0, "SAVENEW <TYPENAME>", "@Saves the mech as a new-type template.", mechrep_Rsavetemp2},
    {0, "SETARMOR <LOC> <AVAL> <IVAL> <RVAL>", "@Sets the armor, int. armor, and rear armor.", mechrep_Rsetarmor},
    {0, "ADDWEAP <NAME> <LOC> <CRIT SECS> [RTO]", "@Adds weapon to the mech, using given loc and crit slots", mechrep_Raddweap},
    {0, "SETHP <LOC> <CRIT> <TYPE>", "Sets a crit to a Ammo (A) or Laser (L) Hardpooint", mechrep_Rsethp},
    {0, "SETCARGOSPACE <VAL> <MAXTON>", "Sets CargoSpace and max tonnage", mechrep_setcargospace},
    {0, "SETPODSPACE <VAL>", "Sets PodSpace", mechrep_setpodspace},
    {0, "RESETCRITS", "@Resets criticals of the toy to base of type.", mechrep_Rresetcrits},
    {0, "REPAIR <LOC> <TYPE> <[VAL | SUBSECT]>", "@Repairs the mech.", mechrep_Rrepair},
    {0, "RELOAD <NAME> <LOC> <SUBSECT> [L|A|N(|C|M|S)]", "@Reloads weapon in location and critical subsection.", mechrep_Rreload},
    {0, "ADDSP <ITEM> <LOC> <SUBSECT> [<DATA>]", "@Adds a special item in location & critical subsection.", mechrep_Raddspecial},
    {0, "DISPLAY <LOC>", "@Displays all the items in the location.", mechrep_Rdisplaysection},
    {0, "SHOWTECH", "@Shows the advanced technology of the mech.", mechrep_Rshowtech},
    {0, "ADDTECH <TYPE>", "@Adds the advanced technology to the mech.", mechrep_Raddtech},
    {0, "DELTECH", "@Deletes the advanced technology of the mech, slots set to empty.", mechrep_Rdeltech},
    {0, "SETTONS <NUM>", "@Sets the mech tonnage", mechrep_Rsettons},
    {0, "SETTYPE <MECH | GROUND | VTOL | NAVAL | AERO | DS>", "@Sets the mech type", mechrep_Rsettype},
    {0, "SETMOVE <TRACK | WHEEL | HOVER | VTOL | HULL | FOIL | FLY>", "@Sets the mech movement type", mechrep_Rsetmove},
    {0, "SETMAXSPEED <NUM>", "@Sets the max speed of the mech.", mechrep_Rsetspeed},
    {0, "SETHEATSINKS <NUM>", "@Sets the number of heat sinks.", mechrep_Rsetheatsinks},
    {0, "SETJUMPSPEED <NUM>", "@Sets the jump speed of the mech.", mechrep_Rsetjumpspeed},
    {0, "SETLRSRANGE <NUM>", "@Sets the lrs range of the mech.", mechrep_Rsetlrsrange},
    {0, "SETTACRANGE <NUM>", "@Sets the tactical range of the mech.", mechrep_Rsettacrange},
    {0, "SETSCANRANGE <NUM>", "@Sets the scan range of the mech.", mechrep_Rsetscanrange},
    {0, "SETRADIORANGE <NUM>", "@Sets the radio range of the mech.", mechrep_Rsetradiorange},
    {0, NULL, NULL, NULL}
};

#ifdef MENU_CUSTOMIZE
#include "coolmenu_interface.h"
ECOMMANDSET(cu);
#endif

CommandsStruct customcommands[] = {
#ifdef MENU_CUSTOMIZE
    GCOMMANDSET(cu)
    {0, "@SETXCODE <NAME> <VALUE|DATA>", "@Sets xcode value on object", set_xcodestuff},
    {0, "@VIEWXCODE", "@Views xcode values on object", list_xcodestuff}, 
    {0, "@WEIGHT", "@Checks the weight allocated in the new mech", custom_weight1},
    {0, "@WEIGHTO", "@Checks the weight allocated in the old mech", custom_weight2},
    {0, "EDIT <ref>", "Alters <ref>", custom_edit},
    {0, "FINISH", "Quit editing mode", custom_finish},
    {0, "Z", "Back", custom_back},
    {0, "L", "Shows menu", custom_look},
    {0, "LO", "Shows menu", custom_look},
    {0, "LOO", "Shows menu", custom_look},
    {0, "LOOK", "Shows menu", custom_look},
    {0, "HELP", "Shows help for customization", custom_help}, 
    {0, "CRITSTATUS <SECTION>", "Shows the Critical hits status", custom_critstatus},
    {0, "STATUS [A(rmor)|I(nfo)]|W(eapons)]", "Prints the mech's status", custom_status},
    {0, "WEAPONSPECS", "Shows the specifications for your weapons", custom_weaponspecs},
    {0, NULL, NULL, NULL}
#endif
};

#ifdef MENU_CHARGEN
#include "coolmenu_interface.h"
ECOMMANDSET(cm);
#endif

CommandsStruct chargencommands[] = {
#ifdef MENU_CHARGEN
    GCOMMANDSET(cm)
    {0, "DONE", "Finishes your chargen (permanent)", chargen_done},
    {0, "BEGIN", "Starts chargen", chargen_begin},
    {0, "NEXT", "Goes to next stage of chargen", chargen_next},
    {0, "PREV", "Goes to previous stage of chargen", chargen_prev}, 
    {0, "APPLY", "Applies the values to your character (fixes them, only reset/done can be done after)", chargen_apply},
    {0, "RESET", "Resets your stats and lets you begin again", chargen_reset},
    {0, "L", "Shows menu", chargen_look},
    {0, "LO", "Shows menu", chargen_look},
    {0, "LOO", "Shows menu", chargen_look},
    {0, "LOOK", "Shows menu", chargen_look},
    {0, "STATUS", "Shows menu", chargen_look},
    {0, "HELP", "Shows help for chargen", chargen_help},
#endif
    {0, NULL, NULL, NULL}
};

CommandsStruct autopilotcommands[] = {
    {0, "ENGAGE", "Engages the autopilot", auto_engage},
    {0, "DISENGAGE", "Disengages the autopilot", auto_disengage}, 
    {0, "ADDCOMMAND <NAME> [ARGS]", "Adds a command to queue", auto_addcommand},
    {0, "DELCOMMAND <NUM>", "Removes command <NUM> from queue (-1 = all)", auto_delcommand},
    {0, "LISTCOMMANDS", "Lists whole command queue of the autopilot", auto_listcommands},
    {0, "JUMP <NUM>", "Sets current instruction to <NUM>", auto_jump},
    {0, NULL, NULL, NULL}
};

CommandsStruct turretcommands[] = {
    {0, "@SETTURRET <NAME> <VALUE|DATA>", "@Sets xcode value on object", set_xcodestuff},
    {0, "@SETXCODE <NAME> <VALUE|DATA>", "@Sets xcode value on object", set_xcodestuff},
    {0, "@VIEWXCODE", "@Views xcode values on object", list_xcodestuff},
    {0, "DEINITIALIZE", "De-initializes you as gunner", turret_deinitialize},
    {0, "INITIALIZE", "Sets you as the gunner", turret_initialize}, 
    {0, "BEARING [<X Y>] [<X Y>]", "Same format as range.", turret_bearing},
    {0, "CONTACTS [<Prefix> | <TARGET-ID>]", "List all current contacts", turret_contacts}, 
    {0, "CRITSTATUS <SECTION>", "Shows the Critical hits status", turret_critstatus},
    {0, "ETA [<X> <Y>]", "Estimates time to target (/default target)", turret_eta},
    {0, "FINDCENTER", "Shows distance/bearing to center of hex.", turret_findcenter},
    {0, "FIRE <WEAPNUM> [<TARGET-ID>|<X> <Y>]", "Fires Weap at loc at def. target or specified target.", turret_fireweapon},
    {0, "LOCK [<TARGET-ID>|<X> <Y>|<X> <Y> <B|H> | -]",
	"Sets the target to the arg (in 3rd, B = building, H = hex (clear/ignite)) / Clears lock (-)",
		turret_settarget},
    {0, "LRS <M(ech)|T(errain)|E(lev)> [<BEARING> <RANGE>|<TARGET-ID>]", "Shows the long range map", turret_lrsmap},
    {0, "NAVIGATE", "Shows the hex and surroundings graphically", turret_navigate},
    {0, "RANGE [<X Y>] [<X Y>]", "Range to def. target / range to x y / range to x,y from x,y", turret_range},
    {0, "REPORT [<TARGET-ID>|<X Y>]", "Information on default target, num, or x,y", turret_report},
    {0, "SCAN [<TARGET-ID>|<X Y>|<X Y> <B|H>]", "Scans the default target, chosen target, or hex", turret_scan}, 
    {0, "SIGHT <WEAPNUM> [<TARGET-ID>|<X> <Y>]", "Computes base-to-hit for given weapon and target.", turret_sight},
    {0, "STATUS [A(rmor)|I(nfo)]|W(eapons)|S(hort)]", "Prints the mech's status", turret_status}, 
    {0, "TACTICAL [<BEARING> <RANGE>|<TARGET-ID>]",
	"Shows the tactical map at the mech's location / at bearing and range / around chosen target",
		turret_tacmap},
    {0, "WEAPONSPECS", "Shows the specifications for your weapons", turret_weaponspecs},
    {0, "LISTCHANNELS", "Lists set frequencies + comtitles for them.", turret_list_freqs},
    {0, "SENDCHANNEL <LETTER>=<STRING>", "Sends <string> on channel <letter>'s freq.", turret_sendchannel},
    {0, "SETCHANNELFREQ <LETTER>=<NUMBER>", "Sets channel <letter> to frequency <number>.", turret_set_channelfreq},
    {0, "SETCHANNELMODE <LETTER>=<STRING>", "Sets channel <letter> mode <string> (available letters: DIUES, color codes).", turret_set_channelmode},
    {0, "SETCHANNELTITLE <LETTER>=<STRING>", "Sets channel <letter> comtitle to <string>.", turret_set_channeltitle}, 
    {0, "RADIO <ID>=<STRING>", "Radioes (LOS) <ID> with <STRING>", turret_radio},
    {0, NULL, NULL, NULL}
};

CommandsStruct scencommands[] = {
    {0, "@SETXCODE <NAME> <VALUE|DATA>", "@Sets xcode value on object", set_xcodestuff},
    {0, "@VIEWXCODE", "@Views xcode values on object", list_xcodestuff},
    {0, "ENGAGE", "Starts the scenario", scen_start},
    {0, "END", "Ends the scenario", scen_end}, 
    {0, "STATUS [SIDE]", "Reports status of the scenario [/for one side]", scen_status},
    {0, NULL, NULL, NULL}
};

#else

#define mechcommands sscommands
#define mapcommands sscommands
#define mechrepcommands sscommands
#define customcommands sscommands
#define chargencommands sscommands
#define autopilotcommands sscommands
#define turretcommands sscommands
#define scencommands sscommands
#define newfreemech NULL
#define newfreemechrep NULL
#define newfreemap NULL
#define newautopilot NULL
#define newturret NULL
#define mech_update NULL
#define map_update NULL

#endif


ECMD(debug_makemechs);
ECMD(debug_memory);
ECMD(debug_setvrt);
ECMD(debug_setwrt);
ECMD(debug_setwbv);
ECMD(debug_setabv);
ECMD(debug_setxplevel);

CommandsStruct debugcommands[] = {
    {0, "EVENTSTATS", "@Shows event statistics", debug_EventTypes}, 
    {0, "MEMSTATS [LONG]", "@Shows memory statistics (optionally in long form)", debug_memory},
    {0, "SAVEDB", "@Saves the SpecialObject DB", debug_savedb}, 
#ifdef BT_ENABLED 
    {0, "MAKEMECHS <FACTION> <TONS> [<TYPES> [<OPT_TONNAGE> [<MAX_VARIATION>]]]",
	"@Makes list of 'mechs of <faction> with max tonnage of <tons>, and optimum tonnage for each mech <opt_tonnage> (optional)",
		debug_makemechs},
    {0, "LISTFORMS", "@Shows forms", ListForms}, 
    {0, "SETVRT <WEAPON> <NUM>", "@Sets the VariableRecycleTime for weapon <WEAPON> to <NUM>", debug_setvrt}, 
    {0, "SETWRT <WEAPON> <NUM>", "@Sets the VariableRepairTime for weapon <WEAPON> to <NUM>", debug_setwrt},
    {0, "SETWBV <WEAPON> <NUM>", "@Sets the BV on for weapon <WEAPON> to <NUM>", debug_setwbv},
    {0, "SETABV <WEAPON> <NUM>", "@Sets the AmmoBV on for weapon <WEAPON> to <NUM>", debug_setabv},
    {0, "SETXPLEVEL <SKILL> <NUM>", "@Sets the XP threshold for skill <skill> to <num>", debug_setxplevel},
    {0, "SHUTDOWN <MAP#>", "@Shutdown all mechs on the map and clear it.", debug_shutdown}, 
    {0, "XPTOP <SKILL>", "@Shows list of people champ in the <SKILL>", debug_xptop},
#endif
    {0, NULL, NULL, NULL}
};

CommandsStruct sscommands[] = {
    {0, "@SETXCODE <NAME> <VALUE|DATA>", "@Sets xcode value on object", set_xcodestuff},
    {0, "@VIEWXCODE", "@Views xcode values on object", list_xcodestuff},
    {0, NULL, NULL, NULL}
};

#ifdef FT_ENABLED

CommandsStruct fmapblockcommands[] = {
    {0, "@SETXCODE <NAME> <VALUE|DATA>", "@Sets xcode value on object", set_xcodestuff},
    {0, "@VIEWXCODE", "@Views xcode values on object", list_xcodestuff},
    {0, NULL, NULL, NULL}
};

CommandsStruct fcharcommands[] = {
    HEADER("Communication"),
    {0, "EMOTE <text>", "Makes your character <text>", f_emote},
    {0, "SAY <text>", "Makes your character say the text", f_say},
    {0, "SHOUT <text>", "Makes your character shout the text", f_shout}, 
    {0, "WHISPER <target> <text>", "Makes your character whisper the text to target", f_whisper}, 
    HEADER("Items"), 
    {0, "DRAW [item]", "Draws <item>, or draws all weapons available", f_draw},
    {0, "HOLD <item>", "Holds <item>", f_hold},
    {0, "PUT <item> in <container>", "Puts <item> in <container>", f_put}, 
    {0, "SHEATH [item]", "Sheathes <item>, or alternatively everything in hand", f_sheath}, 
    HEADER("Restricted"), 
    {0, "@SETMAPBLOCK <num>", "@Sets mapblock to the spesified one", f_mapblock_set},
    {0, "@SETXY <x> <y>", "@Sets co-ordinates to spesified ones", f_mapblock_setxy},
    {0, "@SETXCODE <NAME> <VALUE|DATA>", "@Sets xcode value on object", set_xcodestuff},
    {0, "@VIEWXCODE", "@Views xcode values on object", list_xcodestuff},
    {0, NULL, NULL, NULL}
};

CommandsStruct fmapcommands[] = {
    {0, "LOADMAP <NAME>", "@Loads the named map", f_map_loadmap},
    {0, "@SETXCODE <NAME> <VALUE|DATA>", "@Sets xcode value on object", set_xcodestuff},
    {0, "@VIEWXCODE", "@Views xcode values on object", list_xcodestuff},
    {0, NULL, NULL, NULL}
};

#else

#define fcharcommands sscommands
#define fmapcommands sscommands
#define fmapblockcommands sscommands

#endif

#define floccommands sscommands
#define fobjcommands sscommands
#define fmobcommands sscommands


#define LINEB(txt,cmd,str,func,upd,updfunc,power) \
{ txt, cmd, str, func, upd, updfunc, power }
#define LINE(txt,cmd,str,func,upd,updfunc,power) \
LINEB(txt,cmd,sizeof(str),func,upd,updfunc,power)

/* Own init func, no update func */
#define LINE_NU(txt,cmd,str,fu,power) \
LINE(txt,cmd,str,fu,0,NULL,power)

/* No data, no update */
#define LINE_ND(txt,cmd,power) \
LINEB(txt,cmd,0,NULL,0,NULL,power)

/* Just data, no special init, no update func */
#define LINE_NFS(txt,cmd,t,power) \
LINEB(txt,cmd,sizeof(t),NULL,0,NULL,power)

SpecialObjectStruct SpecialObjects[] = {
    LINE("MECH", mechcommands, MECH, newfreemech, HEAT_TICK, mech_update, POW_MECH),
    LINE_ND("DEBUG", debugcommands, POW_SECURITY),
    LINE_NU("MECHREP", mechrepcommands, struct mechrep_data, newfreemechrep, POW_MECHREP),
    LINE("MAP", mapcommands, MAP, newfreemap, LOS_TICK, map_update, POW_MAP),
    LINE_ND("CHARGEN", chargencommands, POW_SECURITY),
    LINE_NU("AUTOPILOT", autopilotcommands, AUTO, newautopilot, POW_SECURITY),
    LINE_NU("TURRET", turretcommands, TURRET_T, newturret, POW_SECURITY),
    LINE_NU("CUSTOM", customcommands, struct custom_struct, newfreecustom, POW_MECHREP),
    LINE_NFS("SCEN", scencommands, SCEN, POW_SECURITY),
    LINE_NFS("SSIDE", sscommands, SSIDE, POW_SECURITY),
    LINE_NFS("SSOBJ", sscommands, SSOBJ, POW_SECURITY),
    LINE_NFS("SSINS", sscommands, SSINS, POW_SECURITY),
    LINE_NFS("SSEXT", sscommands, SSEXT, POW_SECURITY)
#ifdef FT_ENABLED
, LINE_NFS("FMAP", fmapcommands, FMAP, POW_SECURITY),
    LINE_NFS("FMAPBLOCK", fmapblockcommands, FMAPBLOCK, POW_SECURITY),
    LINE_NFS("FLOC", floccommands, FLOC, POW_SECURITY),
    LINE_NFS("FCHAR", fcharcommands, FCHAR, POW_SECURITY),
    LINE_NFS("FOBJ", fobjcommands, FOBJ, POW_SECURITY),
    LINE_NFS("FMOB", fmobcommands, FMOB, POW_SECURITY)
#endif
};

#define NUM_SPECIAL_OBJECTS \
   ((sizeof(SpecialObjects))/(sizeof(struct SpecialObjectStruct)))

extern void send_channel();

#undef HEADER


#endif

/* Something about [new] Linux gcc is braindead.. I just don't know
   what, but this allows the code to link [bleah] */
#ifdef memcpy
#undef memcpy
#endif

void send_channel(char *, char *);

#endif
