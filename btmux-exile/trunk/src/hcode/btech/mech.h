#ifndef MECH_H
#define MECH_H

#include "config.h"
#include "externs.h"
#include "db.h"
#include "attrs.h"
#include "powers.h"
#include "mech.stat.h"

#include "btconfig.h"
#include "mymath.h"

#define USE_OLD_STRUCTS		0
#define MECHNAME_SIZE	32
#define MECHREF_SIZE	32

#define NUM_ITEMS   5120
#define NUM_ITEMS_M 4608
#define NUM_BAYS    4
#define NUM_TURRETS 3

#define LEFTSIDE    1
#define RIGHTSIDE   2
#define FRONT       3
#define BACK        4

#define STAND       1
#define FALL        0

#define TURN        30		/* 30 sec turn */
#define KPH_PER_MP 11
#define MP_PER_KPH 0.090909091	/* 1/KPH_PER_MP  */
#define MP_PER_UPDATE_PER_KPH 0.003030303	/* MP_PER_KPH/30 */
#define SCALEMAP 322.5		/* 1/update      */
#define HEXLEVEL 5		/* levels/hex    */
#define ZSCALE 64.5		/* scalemap/hexlevel */
#define XSCALE 0.1547		/* hex constant  */
#define YSCALE2 9.61482e-6	/* update**2     */
#define MP1 11		/* 2*MS_PER_MP   */
#define MP2 22		/* 2*MS_PER_MP   */
#define MP3 33		/* 3*MS_PER_MP   */
#define MP4 44		/* 4*MS_PER_MP   */
#define MP5 55		/* 5*MS_PER_MP   */
#define MP6 66		/* 6*MS_PER_MP   */
#define MP7 77
#define MP8 88
#define MP9 99		/* 9*MS_PER_MP   */
#define MP10 110
#define MP11 121
#define MP12 132
#define MP13 143
#define MP14 154
#define MP15 165
#define MP16 176
#define MP17 187
#define MP18 198
#define MP19 209
#define MP20 220
#define MP21 231
#define MP22 242
#define MP23 253
#define MP24 264
#define MP25 275


#define DELTAFACING 1440.0

#define DEFAULT_FREQS 5
#define FREQS 16

#define FREQ_DIGITAL 1
#define FREQ_MUTE    2		/* For digital transmissions */
#define FREQ_RELAY   4		/* For digital transmissions */
#define FREQ_INFO    8		/* For digital transmissions */
#define FREQ_SCAN   16
#define FREQ_REST   32

#define RADIO_RELAY  1		/* ability to relay things */
#define RADIO_INFO   2		/* ability to see where (digital) message comes from */
#define RADIO_SCAN   4		/* ability to scan for frequencies */

#define CHTITLELEN 15

#define NOT_FOUND -1
#define NUM_CRITICALS 12

#define ARMOR 1
#define INTERNAL 2
#define REAR 3

#define NOARC      	0
#define FORWARDARC 	1
#define REARARC    	2
#define LSIDEARC   	4
#define RSIDEARC   	8
#define TURRETARC  	16
#define LHALFARC 	32
#define RHALFARC 	64

/*
   Critical Types
   0       Empty
   1-192   Weapons
   193-384 Ammo
   395-404 Bombs (Aero/VTOL droppable)
   405-511 Special startings...
 */

/* Critical Types... */
#define NUM_WEAPONS      2240
#define NUM_BOMBS        9

#define EMPTY                  0
#define WEAPON_BASE_INDEX      1
#define AMMO_BASE_INDEX        (WEAPON_BASE_INDEX + NUM_WEAPONS)
#define BOMB_BASE_INDEX        (AMMO_BASE_INDEX + NUM_WEAPONS)
#define SPECIAL_BASE_INDEX     (BOMB_BASE_INDEX + NUM_BOMBS)
#define OSPECIAL_BASE_INDEX    4316
#define CARGO_BASE_INDEX       4608

#define SPECIALCOST_SIZE	(CARGO_BASE_INDEX - SPECIAL_BASE_INDEX)
#define AMMOCOST_SIZE		NUM_WEAPONS
#define WEAPCOST_SIZE		NUM_WEAPONS
#define CARGOCOST_SIZE		(NUM_ITEMS - NUM_ITEMS_M)
#define BOMBCOST_SIZE		NUM_BOMBS

#define IsAmmo(a)    ((a) >= AMMO_BASE_INDEX && (a) < BOMB_BASE_INDEX)
#define IsBomb(a)    ((a) >= BOMB_BASE_INDEX && (a) < SPECIAL_BASE_INDEX)
#define IsSpecial(a) ((a) >= SPECIAL_BASE_INDEX && (a) < CARGO_BASE_INDEX)
#define IsCargo(a)   ((a) >= CARGO_BASE_INDEX)
#define IsActuator(a) (IsSpecial(a) && a <= I2Special(HAND_OR_FOOT_ACTUATOR))
#define IsWeapon(a)  ((a) >= WEAPON_BASE_INDEX && (a) < AMMO_BASE_INDEX)
#define IsArtillery(a) (MechWeapons[a].type==TARTILLERY)
#define IsMissile(a)   (MechWeapons[a].type==TMISSILE)
#define IsAutocannon(a) (MechWeapons[a].type==TAMMO)
#define IsEnergy(a) (MechWeapons[a].type==TBEAM)
/* Spooge weapons! Acid, bio matter, etc! */
#define IsSpooge(a) (MechWeapons[a].class==WCLASS_FLAM)
#define IsFlamer(a) (strstr(MechWeapons[a].name, "Flamer"))
#define IsCoolant(a) (strstr(MechWeapons[a].name, "Coolant"))
#define IsAcid(a)	(strstr(MechWeapons[a].name, "Acid"))
#define IsMG(a)		(strstr(MechWeapons[a].name, "MachineGun"))


/* Modified to support realistic & playable arty. Mor like MiniArty.. MechMortar. - DJ */
/* #define GunRange(a)     (IsArtillery(a)?(ARTILLERY_MAPSHEET_SIZE * MechWeapons[a].longrange):(MechWeapons[a].longrange)) */
#define GunRange(a, mode)     (GunStat(a, mode, GUN_LONGRANGE))

#define EGunRange(a, mode)    ((mudconf.btech_erange && (GunStat(a, mode, GUN_MEDRANGE) * 2) > GunRange(a, mode)) ? \
				(GunStat(a, mode, GUN_MEDRANGE) * 2) : GunRange(a, mode))
#define Ammo2WeaponI(a) ((a) - AMMO_BASE_INDEX)
#define Ammo2Weapon(a)  Ammo2WeaponI(a)
#define Ammo2I(a)       Ammo2Weapon(a)
#define Bomb2I(a)       ((a) - BOMB_BASE_INDEX)
#define Special2I(a)    ((a) - SPECIAL_BASE_INDEX)
#define Cargo2I(a)      ((a) - CARGO_BASE_INDEX)
#define Weapon2I(a)     ((a) - WEAPON_BASE_INDEX)
#define I2Bomb(a)       ((a) + BOMB_BASE_INDEX)
#define I2Weapon(a)     ((a) + WEAPON_BASE_INDEX)
#define I2Ammo(a)       ((a) + AMMO_BASE_INDEX)
#define I2Special(a)    ((a) + SPECIAL_BASE_INDEX)
#define I2Cargo(a)      ((a) + CARGO_BASE_INDEX)
#define Special         I2Special
#define Cargo           I2Cargo

/* To define one of these-> x=SPECIAL_BASE_INDEX+SHOULDER_OR_HIP */
#define SHOULDER_OR_HIP        0
#define UPPER_ACTUATOR         1
#define LOWER_ACTUATOR         2
#define HAND_OR_FOOT_ACTUATOR  3
#define LIFE_SUPPORT           4
#define SENSORS                5
#define COCKPIT                6
#define ENGINE                 7
#define GYRO                   8
#define HEAT_SINK              9
#define JUMP_JET               10
#define CASE                   11
#define FERRO_FIBROUS          12
#define ENDO_STEEL             13
#define TRIPLE_STRENGTH_MYOMER 14
#define TARGETING_COMPUTER     15
#define MASC                   16
#define C3_MASTER              17
#define C3_SLAVE               18
#define BEAGLE_PROBE           19
#define ARTEMIS_IV             20
#define ECM                    21
#define AXE                    22
#define SWORD                  23
#define MACE                   24
#define CLAW                   25
#define DS_AERODOOR            26
#define DS_MECHDOOR            27
#define FUELTANK               28	/* For aeros ; weighs as much as 50p has 2000 fuel */
#define TAG                    29
#define DS_TANKDOOR            30
#define DS_CARGODOOR           31
#define LAMEQUIP               32
#define NULLSIG		       33
#define CAPACITOR	       34
#define SUPERCHARGER	       35
#define ANGEL_ECM	       36
#define BLOODHOUND_PROBE       37
#define HARDPOINT              38
#define C3I		       39
#define STEALTHARM	       40
#define BOMB_RACK	       41

#define LBX2_AMMO              0
#define LBX5_AMMO              1
#define LBX10_AMMO             2
#define LBX20_AMMO             3
#define LRM_AMMO               4
#define SRM_AMMO               5
#define SSRM_AMMO              6
#define NARC_LRM_AMMO          7
#define NARC_SRM_AMMO          8
#define NARC_SSRM_AMMO         9
#define ARTEMIS_LRM_AMMO       10
#define ARTEMIS_SRM_AMMO       11
#define ARTEMIS_SSRM_AMMO      12

#define PETROLEUM              13
#define PHOSPHORUS             14
#define HYDROGEN               15
#define GOLD                   16
#define NATURAL_EXTRACTS       17
#define SILLICON               18
#define SULFUR                 19
#define SODIUM                 20
#define PLUTONIUM              21
#define ORE                    22
#define METAL                  23
#define PLASTICS               24
#define MEDICAL_SUPPLIES       25
#define COMPUTERS              26
#define EXPLOSIVES             27

#define ES_INTERNAL            28
#define FF_ARMOR               29
#define XL_ENGINE              30
#define DOUBLE_HEAT_SINK       31
#define IC_ENGINE              32

#define S_ELECTRONIC           33	/* Shouldn't occur @RL */
#define S_INTERNAL             34
#define S_ARMOR                35
#define S_ACTUATOR             36
#define S_AERO_FUEL            37
#define S_DS_FUEL              38
#define S_VTOL_FUEL            39

#define SWARM_LRM_AMMO         40
#define SWARM1_LRM_AMMO        41
#define INFERNO_SRM_AMMO       42

#define XXL_ENGINE             43
#define COMP_ENGINE            44

#define HD_ARMOR               45
#define RE_INTERNAL            46
#define CO_INTERNAL            47
#define MRM_AMMO	       48
#define THUNDERBOLT_AMMO       49
#define MORTAR_AMMO	       50
#define MORTAR_SMOKE_AMMO      51
#define MORTAR_CLUSTER_AMMO    52
#define LR_DFM_AMMO	       53
#define SR_DFM_AMMO	       54
#define AC20_AMMO_PIERCE       55
#define MORTAR_INCENDIARY_AMMO 56
#define AC10_AMMO_PIERCE       57
#define AC5_AMMO_PIERCE        58
#define AC2_AMMO_PIERCE        59
#define AC20_AMMO_CASELESS     60
#define AC10_AMMO_CASELESS     61
#define AC5_AMMO_CASELESS      62
#define AC2_AMMO_CASELESS      63
#define AC20_AMMO_TRACER       64
#define AC10_AMMO_TRACER       65
#define AC5_AMMO_TRACER	       66
#define AC2_AMMO_TRACER	       67
#define AMMO_LRM_SGUIDED       68
#define AMMO_ATMER	       69
#define AMMO_ATMHE	       70
#define LIGHT_ENGINE	       71
#define COMP_GYRO	       72
#define XL_GYRO		       73
#define HD_GYRO		       74
#define AMMO_ATM	       75
#define AC2_AMMO_PRECISION     76
#define AC5_AMMO_PRECISION     77
#define AC10_AMMO_PRECISION     78
#define AC20_AMMO_PRECISION     79
#define ROCKET_AMMO		80
#define AMMO_LRM_STINGER	81
#define COMPACT_HEAT_SINK       82
#define TIMBIQUI_DARK		83
#define URONIUM			84 
#define KLAXANITE		85
#define FERDONITE		86
#define LEXAN			87
#define URANIUM			88
#define JERANITIUM		89
#define MARONITE		90
#define COPPER			91
#define IRON			92
#define TITANIUM		93
#define PLATINUM		94
#define SILVER			95
#define LUMBER			96
#define DIAMOND			97
#define RUBY			98
#define SAPPHIRE		99
#define GEM			100
#define WOOD			101
#define CARGO_WATER		102
#define MARBLE			103
#define MACHINERY		104
#define MEN			105
#define WOMEN			106
#define CHILDREN		107
#define FOOD			108
#define FURNITURE		109
#define CRUDE_OIL		110
#define DIRT			111
#define ROCK			112
#define FABRIC			113
#define CLOTHING		114
#define FERROCRETE		115
#define KELVINIUM		116
#define COAL			117

#define TON_SENSORS_FIRST	118
#define TON_SENSORS_LAST	(TON_SENSORS_FIRST + 9)

#define TON_MYOMER_FIRST	128
#define TON_MYOMER_LAST		(TON_MYOMER_FIRST + 9)

#define TON_TRIPLEMYOMER_FIRST	138
#define TON_TRIPLEMYOMER_LAST	(TON_TRIPLEMYOMER_FIRST + 9)

#define TON_INTERNAL_FIRST	148
#define TON_INTERNAL_LAST	(TON_INTERNAL_FIRST + 9)

#define TON_ESINTERNAL_FIRST	158
#define TON_ESINTERNAL_LAST	(TON_ESINTERNAL_FIRST + 9)

#define TON_JUMPJET_FIRST	168
#define TON_JUMPJET_LAST	(TON_JUMPJET_FIRST + 9)

#define TON_ARMUPPER_FIRST	178
#define TON_ARMUPPER_LAST	(TON_ARMUPPER_FIRST + 9)

#define TON_ARMLOWER_FIRST	188
#define TON_ARMLOWER_LAST	(TON_ARMLOWER_FIRST + 9)

#define TON_ARMHAND_FIRST	198
#define TON_ARMHAND_LAST	(TON_ARMHAND_FIRST + 9)

#define TON_LEGUPPER_FIRST	208
#define TON_LEGUPPER_LAST	(TON_LEGUPPER_FIRST + 9)

#define TON_LEGLOWER_FIRST	218
#define TON_LEGLOWER_LAST	(TON_LEGLOWER_FIRST + 9)

#define TON_LEGFOOT_FIRST	228
#define TON_LEGFOOT_LAST	(TON_LEGFOOT_FIRST + 9)

#define TON_ENGINE_FIRST	238
#define TON_ENGINE_LAST		(TON_ENGINE_FIRST + 19)

#define TON_ENGINE_XL_FIRST	258
#define TON_ENGINE_XL_LAST	(TON_ENGINE_XL_FIRST + 19)

#define TON_ENGINE_ICE_FIRST	278
#define TON_ENGINE_ICE_LAST	(TON_ENGINE_ICE_FIRST + 19)


#define TON_ENGINE_LIGHT_FIRST	298
#define TON_ENGINE_LIGHT_LAST	(TON_ENGINE_LIGHT_FIRST + 19)

#define TON_COINTERNAL_FIRST	318
#define TON_COINTERNAL_LAST	(TON_COINTERNAL_FIRST + 9)


#define TON_REINTERNAL_FIRST	328
#define TON_REINTERNAL_LAST	(TON_REINTERNAL_FIRST + 9)

#define TON_GYRO_FIRST		338
#define TON_GYRO_LAST		(TON_GYRO_FIRST + 3)

#define TON_XLGYRO_FIRST	342
#define TON_XLGYRO_LAST		(TON_XLGYRO_FIRST + 3)

#define TON_HDGYRO_FIRST	346
#define TON_HDGYRO_LAST		(TON_HDGYRO_FIRST + 3)

#define TON_CGYRO_FIRST		350
#define TON_CGYRO_LAST		(TON_CGYRO_FIRST + 3)

#define TON_ENGINE_XXL_FIRST	354
#define TON_ENGINE_XXL_LAST	(TON_ENGINE_XXL_FIRST + 19)

#define TON_ENGINE_COMP_FIRST	374
#define TON_ENGINE_COMP_LAST	(TON_ENGINE_COMP_FIRST + 19)

#define PLACEHOLDER_NEXT_CARGO  394
/* HACK HACK HACK HACK HACK */
#define IsAmmoCargo(a) \
    (XXX)

/* Weapons structure and array... */
#define TBEAM      0
#define TMISSILE   1
#define TARTILLERY 2
#define TAMMO      3
#define THAND      4

/* Tic status */

#define TIC_NUM_DESTROYED -2
#define TIC_NUM_RELOADING -3
#define TIC_NUM_RECYCLING -4
#define TIC_NUM_PHYSICAL  -5
#define TIC_NUM_JAMMED    -6 /* Added for nicer jam code - DJ */

/* This is the max weapons per area- assuming 12 critical location and */
 /* the smallest weapon requires 1 */
#define MAX_WEAPS_SECTION 12

struct weapon_struct {
/*   char *shortname; */
    char *name;
    char vrt;
    char type;
    char heat;
    char damage;
    char min;
    int shortrange;
    int medrange;
    int longrange;
    char criticals;
    unsigned char ammoperton;
    unsigned char weight;	/* divide by 10 and put into float for actual */
    /* screws CL.Magine.Gun by .05 tons */
    int special;
    int battlevalue;
    int abattlevalue;
    int reptime;
    char class;
    char aero_weaprange;
};

/* special weapon effects */
#define NONE     0x00000000
#define PULSE    0x00000001
#define LBX      0x00000002
#define ULTRA    0x00000004
#define STREAK   0x00000008
#define GAUSS    0x00000010
#define NARC     0x00000020
#define IDF      0x00000040	/* Can be used w/ IDF */
#define DAR      0x00000080	/* Has artillery-level delay on hit (1sec/2hex) */
#define DLR      0x00000100	/* Has LRM-level delay on hit (1sec/5hex) */
#define NEWT     0x00000200	/* Non-3025 */
#define CLAT     0x00000400	/* Clan-tech */
#define PC_HEAT  0x00001000	/* Heat-based PC weapon (laser/inferno/..) */
#define PC_IMPA  0x00002000	/* Impact (weapons) */
#define PC_SHAR  0x00004000	/* Shrapnel / slash (various kinds of weapons) */
#define AMS      0x00008000	/* AntiMissileSystem */
#define NOBOOM   0x00010000	/* No ammo boom */
#define MUN      0x00020000	/* Tac CheeseMunchBook stuff */
#define DFM      0x00040000	/*DFM - 2 worst rolls outta 3 for missiles */
#define ELRM     0x00080000	/*ELRM - 2 worst rolls outta 3 for missiles under */
#define MRM      0x00100000	/*MRM - +1 BTH */
#define CHEAT    0x00200000	/* Causes heat, +dam points */
#define HVYW	 0x00400000	/* Clam HeavyWeapons (call 'm so cuz FA$A will undoubtly bring more variants to the lasers) */
#define AC       0x00800000	/* Generic Ballistic Autocannon - No Gausses. For AC special ammo. - DJ */
#define RFM      0x01000000     /* Rapid Fire Mode - To differentiate Non-Ultra AC's in ULTRA mode. - DJ */
#define RFMMG    0x02000000	/* Rapid Fire Mode for MG's - Variable effects - DJ */
#define ATM      0x04000000     /* Clam Advanced Tactical Gerbils! - DJ */
#define ROTARY   0x08000000     /* Yawn. More newer techno stuff - DJ */
#define OS_WEAP  0x10000000	/* Primarily for rockets. Forced OS type */
#define RANDDAM  0x20000000	/* Does 1-DAMAGE in damage randomly */
#define PCOMBAT  (PC_HEAT|PC_IMPA|PC_SHAR)

struct weapon_class_struct {
    int wclass;
    int max_weaps;
};

/* Weapon Classes. Primarily for Omni code. Groups general categories within */
/* a 'type' grouping - DJ */
#if 0
#define WCLASS_NONE     0
#define WCLASS_SLAS	1
#define WCLASS_MLAS	2
#define WCLASS_LLAS	3
#define WCLASS_PPC	4
#define WCLASS_MG	5
#define WCLASS_AMS	6
#define WCLASS_AC2 	7
#define WCLASS_AC5	8
#define WCLASS_AC10	9
#define WCLASS_AC20	10
#define WCLASS_LRM	11
#define WCLASS_MRM	12
#define WCLASS_SRM	13
#define WCLASS_NARC	14
#define WCLASS_ARTY	15
#define WCLASS_GAUSS	16
#define WCLASS_PC	17
#define WCLASS_ATM	18
#define WCLASS_FLAM     19
#define WCLASS_LGAUSS	20
#else
#define WCLASS_NONE	0
#define WCLASS_SHORT	1
#define WCLASS_MED	2
#define WCLASS_LONG	3
#define WCLASS_VLONG	4
#define WCLASS_AMS	5
#define WCLASS_SHORT_AC 6
#define WCLASS_MED_AC	7
#define WCLASS_LONG_AC	8
#define WCLASS_NARC	9
#define WCLASS_ARTY	10
#define WCLASS_GAUSS	11
#define WCLASS_LGAUSS	12
#define WCLASS_PC	13
#define WCLASS_ATM	14
#define WCLASS_FLAM	15
#define WCLASS_STREAK	16
#endif

/* AeroTech2 Classes of Weapon Ranges */
#define AERORNG_NONE	0
#define AERORNG_SHORT	1
#define AERORNG_SHORT_RNG	6
#define AERORNG_MEDIUM	2
#define AERORNG_MEDIUM_RNG	12
#define AERORNG_LONG	3
#define AERORNG_LONG_RNG	20
#define AERORNG_EXT	4
#define AERORNG_EXT_RNG		25

/* TargComp types */
#define TARGCOMP_NORMAL	0
#define TARGCOMP_SHORT	1
#define TARGCOMP_LONG	2
#define TARGCOMP_MULTI	3
#define TARGCOMP_AA	4

#define MAX_ROLL 11
struct missile_hit_table_struct {
    char *name;
    int key;
    int num_missiles[MAX_ROLL];
};


/**** Section #defs... */

/* The unusual order is related to the locations of weapons of high */

/* magnitude versus weapons of low mag */
#define LARM   0
#define RARM   1
#define LTORSO 2
#define RTORSO 3
#define CTORSO 4
#define LLEG   5
#define RLEG   6
#define HEAD   7
#define NUM_SECTIONS 8

/*  These defs are for Vehicles */
#define LSIDE  0
#define RSIDE  1
#define FSIDE  2
#define BSIDE  3
#define TURRET 4
#define ROTOR  5
#define NUM_VEH_SECTIONS 6

/* Aerofighter */
#define AERO_NOSE    0
#define AERO_LWING   1
#define AERO_RWING   2
#define AERO_AFT     3

/* #define AERO_FUSEL   3 */
/* #define AERO_COCKPIT 4 */
/* #define AERO_ENGINE  5 */
/* #define NUM_AERO_SECTIONS 6 */
#define NUM_AERO_SECTIONS 4

#define NUM_BSUIT_MEMBERS 8

#define DS_NOSE    0
#define DS_LWING   1
#define DS_RWING   2
#define DS_AFT     3

#ifdef HEAVY_TANK
#define HT_FRIGHT    0
#define HT_FLEFT     1
#define HT_RLEFT     2
#define HT_RRIGHT    3
#define HT_AFT       4
#define HT_FSIDE     5
#define HT_TURRET    6
#endif

#define NUM_DS_SECTIONS 4

#define SpheroidDS(a) (MechType(a)==CLASS_SPHEROID_DS)

#define NUM_TICS              4
#define MAX_WEAPONS_PER_MECH 96	/* Thanks to int size */
#define SINGLE_TICLONG_SIZE  32
#define TICLONGS             (MAX_WEAPONS_PER_MECH / SINGLE_TICLONG_SIZE)

  /* structure for each critical hit section */
#if USE_OLD_STRUCTS
struct critical_slot {
    unsigned short type;	/* Type of item that this is a critical for */
    unsigned int mode; 		/* Holds info like rear mount, ultra mode... */
    unsigned int damagemode;
    unsigned char data;		/* Holds information like ammo remaining, etc */
};
#else
struct critical_slot {
    unsigned int mode;          /* Holds info like rear mount, ultra mode... */
    unsigned int damagemode;
    unsigned short type;        /* Type of item that this is a critical for */
    unsigned char data;         /* Holds information like ammo remaining, etc */
    char unused;		/* 1byte spare */
};
#endif

/* criticalslot.damagemode flags for multiple crit weapons */
#define MTX_TOHIT1		0x01
#define MTX_TOHIT2		0x02
#define MTX_TOHIT3		0x04
#define MTX_TOHIT4		0x08
#define MTX_TODAM1		0x10
#define MTX_TODAM2		0x20
#define MTX_TODAM3		0x40
#define MTX_TODAM4		0x80
#define MTX_TOHIT1_RANGE	0x100
#define MTX_TOHIT2_RANGE	0x200
#define MTX_TOHIT3_RANGE	0x400
#define MTX_TOHIT4_RANGE	0x800
#define MTX_HEAT1		0x1000
#define MTX_HEAT2		0x2000
#define MTX_HEAT3		0x4000
#define MTX_HEAT4		0x8000
#define MTX_BOOM1		0x10000
#define MTX_BOOM2		0x20000
#define MTX_BOOM3		0x40000
#define MTX_BOOM4		0x80000
#define MTX_JAM1		0x100000
#define MTX_JAM2		0x200000
#define MTX_JAM3		0x400000
#define MTX_JAM4		0x800000
#define MTX_MODELOCK		0x1000000

#define MTX_TOHIT	(MTX_TOHIT1|MTX_TOHIT2|MTX_TOHIT3|MTX_TOHIT4)
#define MTX_TOHIT_MOD(d)	(d & MTX_TOHIT4 ? 4 : d & MTX_TOHIT3 ? 3 : d & MTX_TOHIT2 ? 2 : d & MTX_TOHIT1 ? 1 : 0)
#define MTX_TODAM	(MTX_TODAM1|MTX_TODAM2|MTX_TODAM3|MTX_TODAM4)
#define MTX_TODAM_MOD(d)	(d & MTX_TODAM4 ? 4 : d & MTX_TODAM3 ? 3 : d & MTX_TODAM2 ? 2 : d & MTX_TODAM1 ? 1 : 0)
#define MTX_TOHIT_RANGE	(MTX_TOHIT1_RANGE|MTX_TOHIT2_RANGE|MTX_TOHIT3_RANGE|MTX_TOHIT4_RANGE)
#define MTX_TOHIT_RANGE_MOD(d)	(d & MTX_TOHIT4_RANGE ? 4 : d & MTX_TOHIT3_RANGE ? 3 : d & MTX_TOHIT2_RANGE ? 2 : d & MTX_TOHIT1_RANGE ? 1 : 0)
#define MTX_HEAT	(MTX_HEAT1|MTX_HEAT2|MTX_HEAT3|MTX_HEAT4)
#define MTX_HEAT_MOD(d)	(d & MTX_HEAT4 ? 4 : d & MTX_HEAT3 ? 3 : d & MTX_HEAT2 ? 2 : d & MTX_HEAT1 ? 1 : 0)
#define MTX_BOOM	(MTX_BOOM1|MTX_BOOM2|MTX_BOOM3|MTX_BOOM4)
#define MTX_BOOM_MOD(d)	(d & MTX_BOOM4 ? 6 : d & MTX_BOOM3 ? 5 : d & MTX_BOOM2 ? 4 : d & MTX_BOOM1 ? 3 : 0)
#define MTX_JAM		(MTX_JAM1|MTX_JAM2|MTX_JAM3|MTX_JAM4)
#define MTX_JAM_MOD(d)	(d & MTX_JAM4 ? 6 : d & MTX_JAM3 ? 5 : d & MTX_JAM2 ? 4 : d & MTX_JAM1 ? 3 : 0)

#define REAR_MOUNT     0x01	/* set if weapon is rear mounted */
#define ULTRA_MODE     0x02	/* set if weapon is in Ultra firing mode */
#define ON_TC          0x04	/* Set if the wepons mounted with TC */

/* For normal missiles/autocannons */
#define LBX_MODE       0x08	/* set if weapon is firing LBX ammo */
#define ARTEMIS_MODE   0x10	/* artemis compatible missiles/laucher */
#define CAPPED_MODE    0x10     /* Nasty! Cooooooodies! Dual Effect modes! NOOOOOOOOOOOOOOOOOOOOOOO!!! - DJ */
#define NARC_MODE      0x20	/* narc compatible missiles/launcher */
			       /* _if_ NARC weapon, and ammo is in
			          NARC_MODE, it's explosive ammo.
			          _if_ NARC weapon and in NARC_MODE,
			          it's shooting explosive rounds */
/* For artillery */
#define CLUSTER_MODE   0x08	/* Set if weapon is firing cluster ammo */
#define MINE_MODE      0x10	/* Set if weapon's firing mines */
#define SMOKE_MODE     0x20	/* Set if weapon's firing smoke rounds */
#define INCEND_MODE    0x80     /* Set if weapon's firing incendiary rounds */

/* For LRMs: */
#define HOTLOAD_MODE   0x40	/* Weapon's being hotloaded */
#define HALFTON_MODE   0x40	/* Another dual effect *grin* */

#define INFERNO_MODE   0x80	/*SRM's loaded with Inferno rounds (cause heat) */
#define SWARM_MODE     0x100	/*LRM's loaded with Streak rounds */
#define SWARM1_MODE    0x200	/*LRM's loaded with Streak1 rounds (FoF) */
#define OS_MODE        0x400	/*In weapon itself : Weapon's one-shot */
#define DISABLED_MODE  0x800	/* the part is disabled */
#define PIERCE_MODE    0x1000   /* Piercing Rounds for AC tech - DJ */
#define HEAT_MODE      0x2000   /* CHEAT Weap heat/dmg switching - DJ */
#define OS_USED        0x4000	/* One-shot ammo _has_ been already used */
#define DESTROYED_MODE 0x8000	/* the part is destroyed */

/* 16 slots added with integer extension. Stop gap measure? Better to add new array and split effects up? Next time perhaps. - DJ 9/08/00 */

#define CASELESS_MODE  0x10000    /* Caseless AC rounds/mode - DJ */
#define TRACER_MODE    0x20000    /* Tracer Rounds - DJ */
#define SGUIDED_MODE   0x40000    /* Semi-Guided LRM - DJ */
#define JAMMED_MODE    0x80000    /* Wanted -1 Data but is UnsigChar. Top end could be lowered for possible large data numbers, opted this. -DJ */
#define DEADFIRE_MODE  0x100000   /* DFM as a weapon mode - DJ */
#define ATMER_MODE     0x200000   /* ATM Extended Range Ammo! - DJ */
#define ATMHE_MODE     0x400000   /* ATM HE Ammo! - DJ */
#define ROTFOUR_MODE   0x800000   /* Rotary FourShot Mode. Two Shot 'shared' with ULTRA_MODE - DJ */
#define ROTSIX_MODE    0x1000000  /* Surprise! - DJ */
#define PRECISION_MODE 0x2000000  /* Munchy Munitions! - DJ */
#define ENERGY_HPOINT  0x4000000  /* Laser Omni Hardpoint */
#define BALL_HPOINT    0x8000000  /* Ammo Omni Hardpoint */
#define MISSILE_HPOINT 0x10000000 /* Missile Omni Hardpoint */
#define SPECIAL_HPOINT 0x20000000 /* SpecialCrit Omni Hardpoint */
#define STINGER_MODE   0x40000000 /* AntiAir Missiles for LRM */
#define GAUSS_HPOINT   0x80000000 /* Gauss HPoints */

/* All 32bits are now used.  Have to extend to 'long long int' from here for 64bits */

#define HPOINTS (ENERGY_HPOINT|BALL_HPOINT|MISSILE_HPOINT|SPECIAL_HPOINT|GAUSS_HPOINT)

#define ARTILLERY_MODES (CLUSTER_MODE|MINE_MODE|SMOKE_MODE|INCEND_MODE)
#define AMMO_MODES      \
  (LBX_MODE|ARTEMIS_MODE|NARC_MODE|HOTLOAD_MODE|INFERNO_MODE|SWARM_MODE|SWARM1_MODE|OS_MODE|PIERCE_MODE|HEAT_MODE|CASELESS_MODE|TRACER_MODE|SGUIDED_MODE| \
	DEADFIRE_MODE|ATMER_MODE|ATMHE_MODE|ROTFOUR_MODE|ROTSIX_MODE|ULTRA_MODE|PRECISION_MODE|STINGER_MODE)
#define WEAPMODE_EXCLUDE \
(LBX_MODE|ARTEMIS_MODE|NARC_MODE|INFERNO_MODE|SWARM_MODE|SWARM1_MODE|PIERCE_MODE|CASELESS_MODE|TRACER_MODE|SGUIDED_MODE|DEADFIRE_MODE|ATMER_MODE|ATMHE_MODE| \
	ROTFOUR_MODE|ROTSIX_MODE|ULTRA_MODE|PRECISION_MODE|STINGER_MODE)
#define ULTRA_MODES (ULTRA_MODE|ROTFOUR_MODE|ROTSIX_MODE)
#define PAMMO_MODES     (LBX_MODE|ARTEMIS_MODE|NARC_MODE|INFERNO_MODE|SWARM_MODE|SWARM1_MODE|PIERCE_MODE|CASELESS_MODE|TRACER_MODE|SGUIDED_MODE|DEADFIRE_MODE| \
	ATMER_MODE|ATMHE_MODE|PRECISION_MODE|STINGER_MODE)
#define SPECIFIC_AMMO PAMMO_MODES

/* Used for weaponspecs to strip duplicate-mode types unspammily. Also used in GunStats to prevent redundent calculations and speed up procs - DJ */
#define VARIABLE_AMMO   (DEADFIRE_MODE|ATMER_MODE|ATMHE_MODE|ULTRA_MODE|ROTFOUR_MODE|ROTSIX_MODE|TRACER_MODE|STINGER_MODE)


/* Spec list for ammo /forced/ (Sqeeeuuel like a piggy!) to HALFTON_MODE - DJ */
#define LARGE_AMMO    (PIERCE_MODE|PRECISION_MODE|DEADFIRE_MODE)
#define SMALL_AMMO    (CASELESS_MODE)

/* #define FAIL_DESTROYED 5 -- Backward compatible with !MaxTechVehCrit. Prolly never happen but meh -- Reb
   i.e. DON'T USE 5 FOR ANYTHING */

/* Structure for each of the 8 sections */
#if USE_OLD_STRUCTS
struct section_struct {
    unsigned char armor;	/* External armor value */
    unsigned char internal;	/* Internal armor value */
    unsigned char rear;		/* Rear armor value */
    unsigned char armor_orig;
    unsigned char internal_orig;
    unsigned char rear_orig;
    char basetohit;		/* Holds to hit modifiers for weapons in section */
    char config;		/* flags for CASE, etc. */
    char recycle;		/* after physical attack, set counter */
    struct critical_slot criticals[NUM_CRITICALS];	/* Criticals */
};
#else
struct section_struct {
    unsigned char armor;        /* External armor value */
    unsigned char internal;     /* Internal armor value */
    unsigned char rear;         /* Rear armor value */
    unsigned char armor_orig;
    unsigned char internal_orig;
    unsigned char rear_orig;
    char basetohit;             /* Holds to hit modifiers for weapons in section */
    char config;                /* flags for CASE, etc. */
    struct critical_slot criticals[NUM_CRITICALS];      /* Criticals */
    char recycle;               /* after physical attack, set counter */
    char unused[3];             /* Unused. 3 byte's to go */
};
#endif

/* Section configurations */
#define CASE_TECH              0x01	/* section has CASE technology */
#define SECTION_DESTROYED      0x02	/* section has been destroyed */
#define SECTION_BREACHED       0x04	/* section has been exposed to vacuum */
#define SECTION_FLOODED        0x08	/* section has been flooded with water - Kipsta. 8/3/99 */
/* #define AXED		       0x10*/	/* arm was used to axe/sword someone */
#define STABILIZER_CRIT	       0x20     /* Tanks only (in theory. if FASA) for 2x AtackMove - DJ */
#define SECTION_NARC	       0x40     /* Section is NARC'ed */
#define SECTION_INARC          0x80     /* Section is INARC'ed */
#define SECTION_ACID	       0x10	/* Suck acid!! -- Reb Moved AXED to a status. HACK HACK*/
/* ground combat types */
#define CLASS_MECH            0
#define CLASS_VEH_GROUND      1
#define CLASS_VEH_NAVAL       3

/* Air types */
#define CLASS_VTOL            2
#define CLASS_VEH_VTOL        2
#define CLASS_SPHEROID_DS     4	/* Spheroid DropShip */
#define CLASS_AERO            5
#define CLASS_AEROFIGHTER     5
#define CLASS_MW              6	/* Ejected MechWarrior */
#define CLASS_DS              7	/* AeroDyne DropShip */
#define CLASS_BSUIT           8
#define CLASS_LAST            8

#define DropShip(a) ((a)==CLASS_DS || (a)==CLASS_SPHEROID_DS)
#define IsDS(m)             (DropShip(MechType(m)))

/* ground movement types */
#define MOVE_BIPED            0
#define MOVE_QUAD             8
#define MOVE_TRACK            1
#define MOVE_WHEEL            2
#define MOVE_HOVER            3
#define MOVE_HULL             5
#define MOVE_FOIL             6
#define MOVE_SUB              9

/* Air movenement types */
#define MOVE_VTOL             4
#define MOVE_FLY              7

#define MOVE_NONE             10	/* Stationary, for one reason or another */
#define MOVE_TURRET	      11	/* Stationary, but allow heading changes. */

#define MOVENEMENT_LAST       11

/* Mech Preferences list */
#define MECHPREF_PKILL		0x01	/* Are you sure you want to kill MWs? */
#define MECHPREF_SLWARN		0x02    /* Slite warnings on - DJ */
#define MECHPREF_AUTOFALL	0x04	/* To purposely fail pilot rolls - DJ */
#define MECHPREF_TURNMODE	0x08
#define MECHPREF_NORADIO	0x10
#define MECHPREF_CRUISE		0x20
#define MECHPREF_OT		0x40

#if USE_OLD_STRUCTS
typedef struct {
    char mech_name[31];		/* Holds the 30 char ID for the mech */
    char mech_type[15];		/* Holds the mechref for the mech */
    char type;			/* The type of this unit */
    char move;			/* The movement type of this unit */
    int tons;			/* How much I weigh */
    short radio_range;		/* Can read/write comfortably at that distance */
    char tac_range;		/* Tactical range for sensors */
    char lrs_range;		/* Long range for sensors */
    char scan_range;		/* Range for scanners */
    char numsinks;		/* number of heatsinks (also engine */
    /* crits ( - from heatsinks) ) */
    struct section_struct sections[NUM_SECTIONS];	/* armor */
    char si;			/* Structural integrity of a craft */
    char si_orig;		/* maximum struct. int */

    int fuel;			/* Fuel left */
    int fuel_orig;		/* Fuel tank capacity */

    float maxspeed;		/* Maxspeed (running) in KPH */

    char computer;		/* Partially replaces tac/lrs/scan/radiorange */
    char radio;
    char radioinfo;
    int mechbv;			/* Fasa BattleValue of this unit */
    int cargospace;		/* Assigned cargo space * 100 for half and quarter tons */
    int podspace;               /* Assigned pod space * 100 for half and quarter tons */
    char targcomp;		/* New targeting systems code */
    char carmaxton;		/* Max Tonnage variable for carrier sizing */
    int unused[5];		/* Space for future expansion */
} mech_ud;
#else
typedef struct {
    char mech_name[MECHNAME_SIZE];         /* Holds the 30 char ID for the mech */
    char mech_type[MECHREF_SIZE];         /* Holds the mechref for the mech */
    int tons;                   /* How much I weigh */
    short radio_range;          /* Can read/write comfortably at that distance */
    char tac_range;             /* Tactical range for sensors */
    char lrs_range;             /* Long range for sensors */
    /* crits ( - from heatsinks) ) */
    struct section_struct sections[NUM_SECTIONS];       /* armor */
    char si;                    /* Structural integrity of a craft */
    char si_orig;               /* maximum struct. int */
    char scan_range;            /* Range for scanners */
    char numsinks;              /* number of heatsinks (also engine */
    int fuel;                   /* Fuel left */
    int fuel_orig;              /* Fuel tank capacity */
    float maxspeed;             /* Maxspeed (running) in KPH */
    char computer;              /* Partially replaces tac/lrs/scan/radiorange */
    char radio;
    char radioinfo;
    char targcomp;              /* New targeting systems code */
    int mechbv;                 /* Fasa BattleValue of this unit */
    int cargospace;             /* Assigned cargo space * 100 for half and quarter tons */
    int podspace;               /* Assigned pod space * 100 for half and quarter tons */
    char carmaxton;             /* Max Tonnage variable for carrier sizing */
    char type;                  /* The type of this unit */
    char move;                  /* The movement type of this unit */
    char unused;                /* Spare 1 byte only */
} mech_ud;
#endif

#if USE_OLD_STRUCTS
typedef struct {
    float startfx, startfy;	/* in real coords (for jump and goto) */
    float startfz, endfz;	/* startstuff's also aeros' speed */
    short jumplength;		/* in real coords (for jump and goto) */
    char jumptop;		/* How many MPs we've left for vertical stuff? */
    short goingx, goingy;	/* in map coords (for jump and goto) */
    float verticalspeed;	/* VTOL vertical speed in KPH */
    short desiredfacing;	/* You are turning if this != facing */
    short angle;		/* For DS / Aeros */
    float speed;		/* Speed in KPH */
    float desired_speed;	/* Desired speed in KPH */
    float jumpspeed;		/* Jumping distance or current height in km */
    short jumpheading;		/* Jumping head */
    short targx, targy, targz;	/* in map coords, target squares */
    short turretfacing;		/* Jumping head */

    char aim;			/* section of target aimed at */
    char pilotskillbase;	/* holds constant skills mods */
    short turndamage;		/* holds damge taken in 5 sec interval */
    char basetohit;		/* total to hit modifiers from critical hits */

    dbref chgtarget;		/* My CHARGE target */
    dbref dfatarget;		/* My DFA target */
    dbref target;		/* My default target */
    dbref swarming;		/* Swarm target */
    dbref carrying;		/* Who are we lugging about? */
    dbref spotter;		/* Who's spotting for us? */

    char engineheat;		/* +5 per critical hit there */
    float heat;			/* Heat index */
    float weapheat;		/* Weapon heat factor-> see manifesto */
    float plus_heat;		/* how much heat I am producing */
    float minus_heat;		/* how much heat I can dissipate */

    int critstatus; 		/* see key below */
    int status;			/* see key below */
    int specials;		/* see key below */

    char masc_value;		/* MASC roll .. updated up/down as needed */
    time_t last_weapon_recycle;	/* This updated only on 'as needed' basis ;
				   basically, all weapon recycling events
				   compare the current time to the
				   last_weapon_recycle, and send recycled-messages
				   for all recycled weapons. */
    char sensor[2];		/* Primary mode, secondary mode */
    byte fire_adjustment;	/* For artillery mostly */
    int cargo_weight;		/* How much stuff do we have? */

    short lateral;		/* Quad lateral move mode */
    short num_seen;		/* Number of enemies seen */

    /* BTHRandomization stuff (rok) ;) */
    int lastrndu;
    int rnd;

    int last_ds_msg;		/* Used for DS-spam */
    int boom_start;		/* Used for Stackpole-effect */

    int maxfuel;		/* How much fuel fits to this thing anyway? */
    int lastused;		/* Idle timeout thing */
    int cocoon;			/* OOD cocoon */
    int commconv;		/* Evil magic related to commconv, p1 */
    int commconv_last;		/* Evil magic related to commconv, p2 */
    int onumsinks;		/* Original HS (?) */
    int disabled_hs;		/* Disabled (on purpose, not destroyed) HS */
    int autopilot_num;
    char aim_type;		/* Type we aim at */
    int heatboom_last;
    char vis_mod;		/* Should be in range of 0 to 100 ; basically, this
				   is used as _base_ of random element in each sensor type, altered
				   once every heat update (and when mech's sensor mode changes) */
    int last_spinroll;
    int can_see;
    short lx, ly;
    int row;			/* _Own_ weight */
    int rcw;			/* _Carried_ weight */
    float rspd;
    int erat;			/* Engine Rating */
    int per;
    int wxf;
    char chargetimer;		/* # of movement ticks since 'charge' command */
    float chargedist;		/* # of hexes moved since 'charge' command */
    short mech_prefs;		/* Mech preferences */
    int status2;		/* see key below */
    char scharge_value;		/* SuperCharger Value, clone of masc_value - DJ */
    int specials2;		/* Dear Lord. Another expansion - DJ */
    int critstatus2;		/* Mostly for AeroCrits. - DJ */
    float velocity;		/* Aero velocity tracking - DJ */
    short realangle;
    int startup_last;		/* For heat code - DJ */
    int bv_last;		/* For making life easier on BV calcs */
    float thrust;
    int unused[2];		/* Space for future expansion */
} mech_rd;
#else
typedef struct {
    float startfx, startfy;     /* in real coords (for jump and goto) */
    float startfz, endfz;       /* startstuff's also aeros' speed */
    short goingx;		/* in map coords (for jump and goto) */
    short goingy;		/* in map coords (for jump and goto) */
    float verticalspeed;        /* VTOL vertical speed in KPH */
    short desiredfacing;        /* You are turning if this != facing */
    short angle;                /* For DS / Aeros */
    float speed;                /* Speed in KPH */
    float desired_speed;        /* Desired speed in KPH */
    float jumpspeed;            /* Jumping distance or current height in km */
    short jumpheading;          /* Jumping head */
    short targx;		/* in map coords, target squares */
    short targy;		/* in map coords, target squares */
    short targz;		/* in map coords, target squares */
    short turndamage;           /* holds damge taken in 5 sec interval */
    short realangle;
    short jumplength;           /* in real coords (for jump and goto) */
    short turretfacing;         /* Jumping head */
    dbref chgtarget;            /* My CHARGE target */
    dbref dfatarget;            /* My DFA target */
    dbref target;               /* My default target */
    dbref swarming;             /* Swarm target */
    dbref carrying;             /* Who are we lugging about? */
    dbref spotter;              /* Who's spotting for us? */
    float heat;                 /* Heat index */
    float weapheat;             /* Weapon heat factor-> see manifesto */
    float plus_heat;            /* how much heat I am producing */
    float minus_heat;           /* how much heat I can dissipate */
    int critstatus;             /* see key below */
    int status;                 /* see key below */
    int specials;               /* see key below */
    time_t last_weapon_recycle; /* This updated only on 'as needed' basis ;
                                   basically, all weapon recycling events
                                   compare the current time to the
                                   last_weapon_recycle, and send recycled-messages
                                  for all recycled weapons. */
    char sensor[2];             /* Primary mode, secondary mode */
    char masc_value;            /* MASC roll .. updated up/down as needed */
    char engineheat;            /* +5 per critical hit there */
    int cargo_weight;           /* How much stuff do we have? */
    short lateral;              /* Quad lateral move mode */
    short num_seen;             /* Number of enemies seen */
    /* BTHRandomization stuff (rok) ;) */
    int lastrndu;
    int rnd;
    int last_ds_msg;            /* Used for DS-spam */
    int boom_start;             /* Used for Stackpole-effect */
    int maxfuel;                /* How much fuel fits to this thing anyway? */
    int lastused;               /* Idle timeout thing */
    int cocoon;                 /* OOD cocoon */
    int commconv;               /* Evil magic related to commconv, p1 */
    int commconv_last;          /* Evil magic related to commconv, p2 */
    int onumsinks;              /* Original HS (?) */
    int disabled_hs;            /* Disabled (on purpose, not destroyed) HS */
    int autopilot_num;
    int heatboom_last;
    int last_spinroll;
    int can_see;
    short lx;
    short ly;
    int row;                    /* _Own_ weight */
    int rcw;                    /* _Carried_ weight */
    float rspd;
    int erat;                   /* Engine Rating */
    int per;
    int wxf;
    int status2;                /* see key below */
    int specials2;              /* Dear Lord. Another expansion - DJ */
    int critstatus2;            /* Mostly for AeroCrits. - DJ */
    float velocity;             /* Aero velocity tracking - DJ */
    int startup_last;           /* For heat code - DJ */
    int bv_last;                /* For making life easier on BV calcs */
    float thrust;
    short mech_prefs;           /* Mech preferences */
    char vis_mod;               /* Should be in range of 0 to 100 ; basically, this
                                   is used as _base_ of random element in each sensor type, altered
                                   once every heat update (and when mech's sensor mode changes) */
    char aim_type;              /* Type we aim at */
    char chargetimer;           /* # of movement ticks since 'charge' command */
    float chargedist;            /* # of hexes moved since 'charge' command */
    char scharge_value;         /* SuperCharger Value, clone of masc_value - DJ */
    char jumptop;               /* How many MPs we've left for vertical stuff? */
    char aim;                   /* section of target aimed at */
    char basetohit;             /* total to hit modifiers from critical hits */
    char pilotskillbase;        /* holds constant skills mods */
    byte fire_adjustment;       /* For artillery mostly */
#if 0
    unsigned char turnperc;	/* New FASA-like turn code user defined turn percentage */
    unsigned short turntrack;	/* New FASA-like turn code amount turned tracker */
    char unused;		/* Spare 1 byte */
#endif
} mech_rd;
#endif

#if USE_OLD_STRUCTS
typedef struct {
    dbref pilot;		/* My pilot */
    char pilotstatus;		/* damage pilot has taken */
    short hexes_walked;		/* Hexes walked counter */
    char terrain;		/* Terrain I am in */
    char elev;			/* Elevation I am at */
    short facing;		/* 0-359.. */
    dbref master_c3_node;	/* C3 master, if any */
    char team;			/* Only for internal use */
    short x, y, z;		/* hex quantized x,y,z on the map in MP (hexes) */
    short last_x, last_y;	/* last hex entered */
    float fx, fy, fz;		/* exact x, y and z on the map */
    int unusable_arcs;		/* Horrid kludge for disallowing use of some arcs' guns */
    int stall;			/* is this mech in a repair stall? */
    dbref bay[NUM_BAYS];
    dbref turret[NUM_TURRETS];
} mech_pd;
#else
typedef struct {
    dbref pilot;                /* My pilot */
    dbref bay[NUM_BAYS];
    dbref turret[NUM_TURRETS];
    dbref master_c3_node;       /* C3 master, if any */
    float fx, fy, fz;           /* exact x, y and z on the map */
    int unusable_arcs;          /* Horrid kludge for disallowing use of some arcs' guns */
    int stall;                  /* is this mech in a repair stall? */
    short hexes_walked;         /* Hexes walked counter */
    short x;			/* hex quantized x,y,z on the map in MP (hexes) */
    short y;			/* hex quantized x,y,z on the map in MP (hexes) */
    short z;			/* hex quantized x,y,z on the map in MP (hexes) */
    short last_x;		/* last hex entered */
    short last_y;		/* last hex entered */
    short facing;               /* 0-359.. */
    char terrain;               /* Terrain I am in */
    char elev;                  /* Elevation I am at */
    unsigned short team;	/* Only for internal use */
    char pilotstatus;           /* damage pilot has taken */
    char unused;		/* Spare 2 Bytes */
} mech_pd;
#endif

#if USE_OLD_STRUCTS
typedef struct {
    dbref mynum;		/* My dbref */
    int mapnumber;		/* My number on the map */
    dbref mapindex;		/* 0..MAX_MAPS (dbref of map object) */
    char ID[2];			/* Only for internal use */
    char brief;			/* toggle brievity */
    unsigned long tic[NUM_TICS][TICLONGS];	/* tics.. */
    char chantitle[FREQS][CHTITLELEN + 1];	/* Channel titles */
    int freq[FREQS];		/* channel frequencies */
    int freqmodes[FREQS];	/* flags for the freq */
    mech_ud ud;			/* UnitData (mostly not bzero'able) */
    mech_pd pd;			/* PositionData(mostly not bzero'able) */
    mech_rd rd;			/* RSdata (mostly bzero'able) */

} MECH;
#else
typedef struct {
    dbref mynum;                /* My dbref */
    mech_ud ud;                 /* UnitData (mostly not bzero'able) */
    mech_pd pd;                 /* PositionData(mostly not bzero'able) */
    mech_rd rd;                 /* RSdata (mostly bzero'able) */
    int mapnumber;              /* My number on the map */
    dbref mapindex;             /* 0..MAX_MAPS (dbref of map object) */
    int freq[FREQS];            /* channel frequencies */
    int freqmodes[FREQS];       /* flags for the freq */
    unsigned long tic[NUM_TICS][TICLONGS];      /* tics.. */
    char chantitle[FREQS][CHTITLELEN + 1];      /* Channel titles */
    char ID[2];                 /* Only for internal use */
    char brief;                 /* toggle brievity */
    char unused;                /* 1 byte spare */
} MECH;
#endif

struct spot_data {
    float tarFX;
    float tarFY;
    float mechFX;
    float mechFY;
    MECH *target;
};

struct repair_data {
    int delta;
    int time;
    int target;
    int code;
};

/* status element... */
#define TORSO_NORMAL  0x01	/* Unused now */
#define LANDED        0x01	/* For VTOL use only */
#define TORSO_RIGHT   0x02	/* Torso heading -= 60 degrees */
#define TORSO_LEFT    0x04	/* Torso heading += 60 degrees */
#define STARTED       0x08	/* Mech is warmed up */
#define PARTIAL_COVER 0x10
#define DESTROYED     0x20
#define JUMPING       0x40	/* Handled in UPDATE */
#define FALLEN        0x80
#define DFA_ATTACK    0x100
#define GETTING_UP    0x200
#define FLIPPED_ARMS  0x400
#define AMS_ENABLED   0x800	/* only settable if mech has ANTI-MISSILE_TECH */
#define NARC_ATTACHED 0x1000	/* set if mech has a NARC beacon attached. */
#define UNCONSCIOUS   0x2000	/* Pilot is unconscious */
#define TOWED         0x4000	/* Someone's towing us */
#define LOCK_TARGET   0x8000	/* We mean business */
#define LOCK_BUILDING 0x10000	/* Hit building */
#define LOCK_HEX      0x20000	/* Hit hex (clear / ignite, d'pend on weapon) */
#define LOCK_HEX_IGN  0x40000
#define LOCK_HEX_CLR  0x80000
#define MASC_ENABLED  0x100000	/* Using MASC */
#define BLINDED       0x200000	/* Pilot has been blinded momentarily by something */
#define COMBAT_SAFE   0x400000	/* Can't be hurt */
#define SLITE_ON      0x800000
#define FIRED         0x1000000	/* Fired at something */
#define ECM_ACTIVE    0x2000000
#define ECM_DISTURBANCE 0x4000000
#define UNDERSPECIAL  0x8000000
#define UNDERGRAVITY  0x10000000
#define UNDERTEMPERATURE 0x20000000
#define UNDERVACUUM   0x40000000
#define JELLIED	      0x80000000
/* Status full with 32 - DJ */

/* status2 element... added 9/09/00 - DJ */
#define ECCM_ACTIVE         0x01
#define ECM_PROTECTED       0x02
#define BOGDOWN	            0x04
#define BEAGLE_ACTIVE       0x08
#define UNJAMMING           0x10
#define NULLSIG_ACTIVE      0x20
#define CAPACITOR_ON        0x40
#define SCHARGE_ENABLED     0x80
#define INARC_ATTACHED      0x100
#define STEALTHARM_ACTIVE   0x200
#define NOC3LIMIT	    0x400
#define EVADING		    0x800
#define SPRINTING	    0x1000
#define DODGING		    0x2000
#define LAXE		    0x4000
#define RAXE		    0x8000
#define CS_ON_LAND	    0x10000
#define ATTACKEMIT_MECH	    0x20000

#define MOVE_MODES	(SPRINTING|EVADING|DODGING)
#define MOVE_MODES_LOCK (SPRINTING|EVADING)

#define CONDITIONS (UNDERSPECIAL | UNDERGRAVITY | UNDERTEMPERATURE | UNDERVACUUM)
#define LOCK_MODES    (LOCK_TARGET|LOCK_BUILDING|LOCK_HEX|LOCK_HEX_IGN|LOCK_HEX_CLR)

#define MechLockFire(mech) \
((MechStatus(mech) & LOCK_TARGET) && \
 !(MechStatus(mech) & (LOCK_BUILDING|LOCK_HEX|LOCK_HEX_IGN|LOCK_HEX_CLR)))

/* Macros for accessing some parts */
#define Uncon(a)      (MechStatus(a) & UNCONSCIOUS)
#define Blinded(a)    (MechStatus(a) & BLINDED)
#define Started(a)    (MechStatus(a) & STARTED)

/* critstatus element */
#define GYRO_DESTROYED         0x01
#define SENSORS_DAMAGED        0x02
#define NO_LEGS                0x04
#define HIDDEN                 0x08	/* Hiding */
#define GYRO_DAMAGED           0x10
#define HIP_DAMAGED            0x20
#define LIFE_SUPPORT_DESTROYED 0x40
#define LEG_DESTROYED          0x80
#define TURRET_LOCKED          0x80	/* Vehicle only */
#define DUG_IN                 0x100
#define DIGGING_IN             0x200
#define SLITE_DEST             0x400
#define SLITE_LIT              0x800
#define LOAD_OK                0x1000	/* Carried load recalculated */
#define OWEIGHT_OK             0x2000	/* Own weight recalculated */
#define SPEED_OK               0x4000	/* Total speed recalculated */
#define HEATCUTOFF	       0x8000
#define CREW_STUNNED	       0x10000  /* Vehicle Only... ? - DJ */
#define TURRET_JAMMED          0x20000  /* Vehicle Only... ? - DJ */
#define TC_DESTROYED           0x40000
#define C3_DESTROYED           0x80000
#define ECM_DESTROYED          0x100000
#define BEAGLE_DESTROYED       0x200000
#define TAILROTOR_DAMAGED      0x400000 /* VTOL Only - DJ */
#define PC_INITIALIZED         0x800000	/* PC-initialization done already */
#define UNUSED_SPINNING	       0x1000000 /* Spinning no longer used. Event + macro */
#define CLAIRVOYANT            0x2000000	/* See everything, regardless of blocked */
#define INVISIBLE              0x4000000	/* Unable to be seen by anyone */
#define CHEAD                  0x8000000	/* Altered heading */
#define OBSERVATORIC           0x10000000
#define NULLSIG_DESTROYED      0x20000000
#define HDGYRO_SPONGE	       0x40000000
#define INFERNO_AMMO           0x80000000
/* Enough Free for now, for cleanliness need to move DUG_IN DIGGING_IN (HIDDEN?) HEATCUTOFF to status2 elements at convenience - DJ 9/09/00 */

/* critstatus2 element */
/* Primarily for aero's but two btmacros functions are setup. A standard MechCritStatus2 to directly show pointer info and
   AeroCritStatus (not '2') that will only return value (else 0) /if/ target is_aero().  MechCritStatus2() can therefore be used
   (and applilcable ones moved from regular critstatus on a needbe basis) for elements that ONLY affect non-aero units.  Over time
   items like NO_LEGS, GYRO_DESTROYED, etc... can be filtered over here and moved to take up these !aero spaces to free up normal
   critstatus for shared elements.  For now, this works, but needs to be changed for expansion in future. - DJ */

#define AVIONICS_ONECRIT	0x01
#define AVIONICS_TWOCRIT	0x02
#define AVIONICS_THREECRIT	0x04
#define ENGINE_ONECRIT		0x08
#define ENGINE_TWOCRIT		0x10
#define ENGINE_DESTROYED	0x20
#define FCS_ONECRIT		0x40
#define FCS_TWOCRIT		0x80
#define FCS_DESTROYED		0x100
#define GEAR_DAMAGED		0x200
#define LIFESUPPORT_DAMAGED	0x400
#define SENSORS_ONECRIT		0x800
#define SENSORS_TWOCRIT		0x1000
#define SENSORS_THREECRIT	0x2000
#define LTHRUST_ONECRIT		0x4000
#define LTHRUST_TWOCRIT		0x8000
#define LTHRUST_THREECRIT	0x10000
#define LTHRUST_DESTROYED	0x20000
#define RTHRUST_ONECRIT         0x40000
#define RTHRUST_TWOCRIT         0x80000
#define RTHRUST_THREECRIT       0x100000
#define RTHRUST_DESTROYED       0x200000
/* Well wasn't a good idea. I was stoned one day 3 years ago. Will have to fix it at some point */
#define STEALTHARM_DESTROYED	0x800000
#define HITCH_DESTROYED		0x1000000

/* specials element: used to tell quickly what type of tech the mech has */
#define TRIPLE_MYOMER_TECH      0x01
#define CL_ANTI_MISSILE_TECH    0x02
#define IS_ANTI_MISSILE_TECH    0x04
#define DOUBLE_HEAT_TECH        0x08
#define MASC_TECH               0x10
#define CLAN_TECH		0x20
#define FLIPABLE_ARMS           0x40
#define C3_MASTER_TECH          0x80
#define C3_SLAVE_TECH           0x100
#define ARTEMIS_IV_TECH         0x200
#define ECM_TECH                0x400
#define BEAGLE_PROBE_TECH       0x800
#define SALVAGE_TECH            0x1000	/* 2x 'mech carrying capacity */
#define CARGO_TECH              0x2000	/* 2x cargo carrying capacity */
#define SLITE_TECH              0x4000
#define CS_STEALTH_TECH         0x8000
#define AA_TECH                 0x10000
#define NS_TECH                 0x20000
#define SS_ABILITY              0x40000	/* Has sixth sense */
#define FF_TECH                 0x80000	/* Has ferro-fib. armor */
#define ES_TECH                 0x100000	/* Has endo-steel internals */
#define XL_TECH                 0x200000
#define ICE_TECH                0x400000	/* ICE engine */
#define DC_STEALTH_TECH         0x800000
#define FWL_STEALTH_TECH        0x1000000
#define XXL_TECH                0x2000000
#define CE_TECH                 0x4000000
#define REINFI_TECH             0x8000000
#define COMPI_TECH              0x10000000
#define HARDA_TECH              0x20000000
#define CRITPROOF_TECH          0x40000000
#define NULLSIG_TECH		0x80000000

/* Uh oh! We're full! Best get creative with unfurled aray (ack!) or free up slots with dynamic functions! - DJ */
/* Specials2 added. Creative? perhaps not. Easier, yes. Cleaner? a little. Addeding switching into 2000 lines is*/
/* not my idea of a fun evening - DJ */

#define STEALTH_TECH (DC_STEALTH_TECH|FWL_STEALTH_TECH|CS_STEALTH_TECH)

/* specials2 element to tell quickly what type of tech the mech has */
/* Remember, MechSpecials2() when refering! */
#define SCHARGE_TECH		0x01
#define LENG_TECH		0x02
#define TORSOCOCKPIT_TECH	0x04
#define CGYRO_TECH		0x08
#define XLGYRO_TECH		0x10
#define HDGYRO_TECH		0x20
#define SMALLCOCKPIT_TECH	0x40
#define ANGEL_ECM_TECH		0x80
#define BLOODHOUND_PROBE_TECH	0x100
#define OMNI_TECH		0x200 /* 'True' hcoded omni-tech */
#define STEALTHARMOR_TECH	0x400
#define HEAVYTANK_TECH		0x800
#define HIDDEN_TECH		0x1000
#define CARRIER_TECH		0x2000
#define ADV_AA_TECH		0x4000
#define COMPACT_HEAT_TECH       0x8000
#define HFF_TECH                0x10000
#define LFF_TECH                0x20000
#define NOREENTRY_TECH		0x40000
#define FRAGILE_TECH		0x80000
#define WATERPROOF_TECH		0x100000
#define PERSONAL_ECM		0x200000
#define GRAPPLE_CLAW		0x400000

/* Status stuff for common_checks function */
#define MECH_STARTED            0x1
#define MECH_PILOT              0x2
#define MECH_PILOT_CON          0x4
#define MECH_MAP                0x8
#define MECH_CONSISTENT         0x10
#define MECH_PILOTONLY		0x20
#define MECH_USUAL              (MECH_CONSISTENT|MECH_MAP|MECH_PILOT_CON|MECH_PILOT|MECH_STARTED)
#define MECH_USUALS             (MECH_CONSISTENT|MECH_MAP|MECH_PILOT_CON|MECH_PILOT)
#define MECH_USUALSP            (MECH_CONSISTENT|MECH_MAP|MECH_PILOT_CON)
#define MECH_USUALSM            (MECH_CONSISTENT|MECH_PILOT_CON|MECH_PILOT)
#define MECH_USUALM             (MECH_CONSISTENT|MECH_PILOT_CON|MECH_PILOT|MECH_STARTED)
#define MECH_USUALO             (MECH_CONSISTENT|MECH_MAP|MECH_PILOT_CON|MECH_PILOT|MECH_STARTED|MECH_PILOTONLY)
#define MECH_USUALSO            (MECH_CONSISTENT|MECH_MAP|MECH_PILOT_CON|MECH_PILOT|MECH_PILOTONLY)
#define MECH_USUALSPO           (MECH_CONSISTENT|MECH_MAP|MECH_PILOT_CON|MECH_PILOTONLY)
#define MECH_USUALSMO           (MECH_CONSISTENT|MECH_PILOT_CON|MECH_PILOT|MECH_PILOTONLY)
#define MECH_USUALMO            (MECH_CONSISTENT|MECH_PILOT_CON|MECH_PILOT|MECH_STARTED|MECH_PILOTONLY)

extern struct weapon_struct MechWeapons[];
extern struct missile_hit_table_struct MissileHitTable[];
extern struct weapon_class_struct WeaponClassTable[];

#define TELE_ALL   1		/* Tele all, not just mortals */
#define TELE_SLAVE 2		/* Make slaves in progress (not of wizzes, though) */
#define TELE_LOUD  4		/* Loudly teleport */
#define TELE_XP    8		/* Lose 1/3 XP */

#define MINE_STEP 1		/* Someone steps to a hex */
#define MINE_LAND 2		/* Someone lands in a hex */
#define MINE_FALL 3		/* Someone falls in the hex */
#define MINE_DROP 4		/* Someone drops to ground in the hex */

extern void *FindObjectsData(dbref key);

#ifndef ECMD
#define ECMD(a) extern void a (dbref player, void *data, char *buffer)
#endif

#define destroy_object(obj) destroy_thing(obj)
#define create_object(name) create_obj(GOD, TYPE_THING, name, 1)

#define A_MECHREF A_MECHTYPE
#define MECH_PATH mudconf.mech_db
#define MAP_PATH mudconf.map_db

#define WSDUMP_MASK_ER "%-30s %2d     %2d           %2d  %2d    %2d  %3d  %3d %2d"
#define WSDUMP_MASK_NOER "%-30s %2d     %2d           %2d    %2d     %2d  %3d   %2d"
#define WSDUMP_MASKS_ER "%%cgWeapon Name                   Heat  Damage  Range: Min Short Med Long Ext VRT"
#define WSDUMP_MASKS_NOER "%%cgWeapon Name                   Heat  Damage  Range: Min  Short  Med  Long  VRT"
#define WSDUMP_MASK_AERO "%-30s %2d     %2d     %2d    %2d"
#define WSDUMP_MASKS_AERO "%%cgWeapon Name                   Heat  Damage  Range  VRT"

#define WDUMP_MASK  "%-24s %2d     %2d           %2d  %2d    %2d  %3d  %2d  %2d %d"
#define WDUMP_MASKS "%%cgWeapon Name             Heat  Damage  Range: Min Short Med Long VRT C  ApT"

/* GunStat lazyperson stuff - DJ */

#define GUN_MINRANGE    0
#define GUN_SHORTRANGE  1
#define GUN_MEDRANGE    2
#define GUN_LONGRANGE   3
#define GUN_AERORANGE	4
#define GUN_DAMAGE      5
#define GUN_HEAT        6
#define GUN_VRT         7

/* Construct headers for some movemode change handling */
#define MODE_EVADE	0x1
#define MODE_SPRINT	0x2
#define MODE_ON		0x4
#define MODE_OFF	0x8
#define MODE_DODGE	0x10
#define MODE_DG_USED	0x20
#define MODE_MODEFAIL 0x40

/* MechFullRecycle check flags */
#define CHECK_WEAPS	0x1
#define CHECK_PHYS	0x2
#define CHECK_BOTH	(CHECK_WEAPS|CHECK_PHYS)

#include "btmacros.h"
#include "p.glue.hcode.h"
#include "map.h"
#include "map.coding.h"
#include "glue.h"
#include "p.glue.h"
#include "mech.notify.h"

#endif				/* MECH_H */
