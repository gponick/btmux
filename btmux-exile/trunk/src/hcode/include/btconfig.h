#ifndef BTCONFIG_H
#define BTCONFIG_H

#define RS_MECH_IDLE  86400
#define SIM_MECH_IDLE 3600

/* Wether all super-heavy tank code is completely tested.... */
#undef HEAVY_TANK

/* New MW3-like combat skill spread */
#define NEW_STATS	1

#define DS_TO_DS_LZ_RANGE	25

#define ATMO_Z   100 

/* Where the dogfighting becomes 'fun' */
#define ORBIT_Z  200 

/* Orbit elevation */
#define ACCEL_MOD 5

/* At max 5x */
#define AERO_SECS_THRUST  30

/* How many secs it takes to apply one maxthrust
   (mod'ed by location _and_ type of craft) */

#define PIL_XP_EVERY_N_STEPS 10

#define AFTERLIFE_DBREF mudconf.afterlife_dbref
#define AFTERSCEN_DBREF mudconf.afterscen_dbref

/* Where dead pilots go */

#define USED_MW_STORE   mudconf.btech_usedmechstore

/* Where used MW templates go to wait for reincarnation (<g>) */

#define MINE_NEXT_MODIFIER 2/3
#define MINE_MIN           5
#define MINE_TABLE         2	/* 0 = General, 2 = KICK */

#define ODDJUMP

/* Have weird jump code? (undef = basic MUSE one) */

#define BT_PARTIAL

/* Whether we want 'BT' partial or not */

#define ECON_ALLOW_MULTIPLE_LOAD_UNLOAD

/* unload / load, addstuff / removestuff multiple kinds of items at
   once */

#define CLAN_SUPPORT

/* Whether we acknowledge Munchkins exist or not */

#define C3_SUPPORT

/* Whether we support C3 or not */

#define MENU_CHARGEN

/* Whether we want it enabled or not */

#define MENU_CUSTOMIZE

/* Whether we want customization code or not */

#undef BTH_DEBUG

/* Show BTHs on Debug */

#undef AUTOP_DEBUG

/* Show Debug level info on MechAI on Debug */

#undef  TEMPLATE_DEBUG

/* Shows ton of unneccessary debug messages */

#undef JUMPDEBUG

/* Show jump coords on Debug */

#undef SENSOR_BTH_DEBUG

/* Show sensor BTHs on Debug */ /* Turned on 9/09/00 - DJ */

#undef SENSOR_DEBUG

/* Don't see see/dontsee msgs */

#define TEMPLATE_VERBOSE_ERRORS

/* Shows errors whenever need be */

#undef VERBOSE_MAP_STUFF

/* Show loading / saving of map stuff specifically */

#define BUILDINGS_REPAIR_THEMSELVES
#define BUILDINGS_REBUILD_FROM_DESTRUCTION

#define BUILDING_REPAIR_AMOUNT     1
#define BUILDING_REPAIR_DELAY    120	/* 1 pt / 1 min */
#define BUILDING_DREBUILD_DELAY 7200	/* 2 hours */

#define LATERAL_TICK  6
#define HEAT_TICK     2
#define JUMP_TICK     1
#define MOVE_TICK     1		/* How oft da mecha move ;-) */
#define MOVE_MOD      MOVE_TICK / 2
#define WEAPON_TICK   2
#define UNJAM_TICK    30
#define CREWSTUN_TICK 30
#define MECHSTUN_TICK 10

#define ARTY_SPEED    5		/* Artillery round flies 5 hexes / second */
#define ARTILLERY_MAPSHEET_SIZE 20	/* Size of single arty mapsheet */
#define ARTILLERY_MINIMUM_FLIGHT 10	/* How long's the minimum flight time */

#define DROP_TO_STAND_RECYCLE (MOVE_TICK * 12)
#define JUMP_TO_HIT_RECYCLE   (JUMP_TICK * 12 / (MechType(mech) == CLASS_BSUIT ? 6 : 1))

#define INITIAL_PLOS_TICK 1	/* How many secs after startup */
#define LOS_TICK          2	/* Update LOS tables */
#define HIDE_TICK        30 
#define PLOS_TICK         1	/* How many seconds interval between checks */
#define SCHANGE_TICK     10	/* Sensor change */
#define SPOT_TICK        10	/* How oft is the range for spotting checked? */

#define PHYSICAL_RECYCLE_TIME 30
#define STARTUP_TIME 30
#define UNCONSCIOUS_TIME 30	/* ORIGINAL authors thought it was UNCONCIOUS */
#define NOAIR_TICK	60	/* No Life Support == No Air!! -- Reb */
#define WEAPON_RECYCLE_TIME 30	/* weapon_tick's */
#define FALL_TICK     3		/* How oft do we call fall event? */
#define FALL_ACCEL    1		/* How much do we accelerate each event? */
#define OOD_SPEED     2		/* 2 Z / tic ; 150 sec for landing */
#define OOD_TICK      1
#define DUMP_TICK     30	/* How long does it take to eject 1 ton of ammo? */
#define DUMP_GRAD_TICK 1	/* This oft we _maybe_ dump stuff */
#define DUMP_SPEED   (DUMP_TICK/DUMP_GRAD_TICK)
#define MASC_TICK      60	/* Time for each MASC regen / fail */
#define ACID_TICK	10	/* Acid burn baby -- Reb */
#define RANDOM_TICK     6	/* How many seconds do we want to use same rnd# for
				   BTHs etc */
#define DS_SPAM_TIME   10	/* At max, 1 mapemit every 10 secs concerning a
				   single DS */

#define MAX_BOOM_TIME  30	/* Max time between first and last CT int hit for
				   fusion explosion */
#define BOOM_BTH        12	/* Roll below this or 'boom' */
#define MAX_C3_SLAVES   3
#define MAX_C3I_UNITS   6
/* Skills used if pilot's not valid and no default mech skills */
#define DEFAULT_GUNNERY   99
#define DEFAULT_PILOTING  99
#define DEFAULT_SPOTTING  8
#define DEFAULT_ARTILLERY 8
#define DEFAULT_COMM      6

/* Default ranges and stuff */
#define DEFAULT_TACRANGE    15
#define DEFAULT_LRSRANGE    25
#define DEFAULT_RADIORANGE  100
#define DEFAULT_SCANRANGE   10
#define DEFAULT_HEATSINKS   10

/* IS guys suck */
#define DEFAULT_COMPUTER     1 
#define DEFAULT_RADIO	     1
#define DEFAULT_PART_LEVEL   1

/* Clans get better stuff - NOT */
#define DEFAULT_CLCOMPUTER   1
#define DEFAULT_CLRADIO	     1
#define DEFAULT_CLPART_LEVEL 1

/* Display Types */
#define LRS_DISPLAY_WIDTH 70
#define LRS_DISPLAY_WIDTH2 35
#define LRS_DISPLAY_HEIGHT 11
#define LRS_DISPLAY_HEIGHT2 5

/* Census config: */

#undef HAVE_LOC_IN_CENSUS

#define NAMELEN 20
#define RANKLEN 20
#define JOBLEN  30
#ifdef HAVE_LOC_IN_CENSUS
#define LOCLEN  20
#endif


/*3030 / MUSE mode */

/*ADVANCED_LOS: If we want the non-100 percent LOS or not */

/*def for 3030, undef for MUSE */

/*SIMPLE_SENSORS: Just basic vislight sensors without option of changing them*/

/*undef for 3030, def for MUSE */

/*LOCK_TICK: 8 for 3030, 0 for MUSE */

#if 1

/* 3030 set */

#define ADVANCED_LOS
#undef  SIMPLE_SENSORS
#define LOCK_TICK     8
#else

/* MUSE set */

#undef ADVANCED_LOS
#define SIMPLE_SENSORS
#define LOCK_TICK     0

#endif

#define INITIAL_RANK 3		/* When starting chargen */
#define FINAL_RANK   2		/* When finished with chargen */

#define ECM_RANGE    6

/* From 160 sec to 3840 sec */

/* #define FIRE_DURATION  ((Number(40,Number(60,960))) * 4) */
#define FIRE_DURATION ((Number(60,180)))

/* From 90 sec to 1200 sec */

/* #define SMOKE_DURATION ((Number(30,Number(60,400))) * 4) */
#define SMOKE_DURATION ((Number(90,150)))

/* What kind of evil magic DFM's affect */
#define DFM_AFFECT_BTH

#define LITE_RANGE 30

typedef unsigned char byte;

#endif				/* BTCONFIG_H */
