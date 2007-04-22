#define MAX_STRING_LENGTH 8192
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mech.h"
#include "create.h"
#include "mech.events.h"
#include "coolmenu.h"
#include "aero.bomb.h"
#include "mech.partnames.h"
#include "p.mech.utils.h"
#include "p.mech.partnames.h"
#include "p.mech.consistency.h"
#include "p.map.conditions.h"
#include "p.aero.bomb.h"
#include "p.mech.mechref_ident.h"
#include "p.crit.h"

#define MODE_UNKNOWN 0
#define MODE_NORMAL 1

char *load_cmds[] = {
    "Reference", "Type", "Move_Type", "Tons", "Tac_Range",
    "LRS_Range", "Radio_Range", "Scan_Range", "Heat_Sinks",
    "Max_Speed", "Specials", "Armor", "Internals", "Rear",
    "Config", "Computer", "Name", "Jump_Speed", "Radio",
    "SI", "Fuel", "Comment", "RadioType", "Mech_BV", "Cargo_Space",
    "Specials2", "Pod_Space", "Max_Ton",
    NULL
};

char *internals[] = {
    "ShoulderOrHip",
    "UpperActuator",
    "LowerActuator",
    "HandOrFootActuator",
    "LifeSupport",
    "Sensors",
    "Cockpit",
    "Engine",
    "Gyro",
    "HeatSink",
    "JumpJet",
    "Case",
    "FerroFibrous",
    "EndoSteel",
    "TripleStrengthMyomer",
    "TargetingComputer",
    "Masc",
    "C3Master",
    "C3Slave",
    "BeagleProbe",
    "ArtemisIV",
    "Ecm",
    "Axe",
    "Sword",
    "Mace",
    "Claw",
    "DSAeroDoor",
    "DSMechDoor",
    "Fuel_Tank",
    "TAG",
    "DSVehicleDoor",
    "DSCargoDoor",
    "LAM_Equipment",
    "NullSig_Device",
    "Capacitor",
    "SuperCharger",
    "AngelEcm",
    "BloodhoundProbe",
    "Hardpoint",
    "C3i",
    "StealthArmor",
    "Bomb_Rack",
    NULL
};

char *cargo[] = {
    "Ammo_LBX2",
    "Ammo_LBX5_LBX",
    "Ammo_LBX10_LBX",
    "Ammo_LBX20_LBX",
    "Ammo_LRM",
    "Ammo_SRM",
    "Ammo_SSRM",
    "Ammo_LRM_NARC",
    "Ammo_SRM_NARC",
    "Ammo_SSRM_NARC",
    "Ammo_LRM_Artemis",
    "Ammo_SRM_Artemis",
    "Ammo_SSRM_Artemis",
    "Petroleum",
    "Phosphorus",
    "Hydrogen",
    "Gold",
    "Natural_Extracts",
    "Sillicon",
    "Sulfur",
    "Sodium",
    "Plutonium",
    "Ore",
    "Steel",
    "Plastics",
    "Medical_Supplies",
    "Computers",
    "Explosives",
    "ES_Internal",
    "FF_Armor",
    "XL_Engine",
    "Double_HeatSink",
    "ICE_Engine",
    "Electric",
    "Internal",
    "Armor",
    "Actuator",
    "Aero_Fuel",
    "DS_Fuel",
    "VTOL_Fuel",
    "Ammo_LRM_Swarm",
    "Ammo_LRM_Swarm1",
    "Ammo_SRM_Inferno",
    "XXL_Engine",
    "Compact_Engine",
    "HD_Armor",
    "RE_Internals",
    "CO_Internals",
    "Ammo_MRM",
    "Ammo_Thunderbolt",
    "Ammo_Mortar",
    "Ammo_Mortar_Smoke",
    "Ammo_Mortar_Cluster",
    "Ammo_LR_DFM",
    "Ammo_SR_DFM",
    "Ammo_AC20_Pierce",
    "Ammo_Mortar_Incendiary",
    "Ammo_AC10_Pierce",
    "Ammo_AC5_Pierce",
    "Ammo_AC2_Pierce",
    "Ammo_AC20_Caseless",
    "Ammo_AC10_Caseless",
    "Ammo_AC5_Caseless",
    "Ammo_AC2_Caseless",
    "Ammo_AC20_Tracer",
    "Ammo_AC10_Tracer",
    "Ammo_AC5_Tracer",
    "Ammo_AC2_Tracer",
    "Ammo_LRM_SGuided",
    "Ammo_ATM-ER",
    "Ammo_ATM-HE",
    "Light_Engine",
    "Compact_Gyro",
    "XL_Gyro",
    "HeavyDuty_Gyro",
    "Ammo_ATM",
    "Ammo_AC2_Precision",
    "Ammo_AC5_Precision",
    "Ammo_AC10_Precision",
    "Ammo_AC20_Precision",
    "Ammo_Rocket",
    "Ammo_LRM_Stinger",
    "Compact_HeatSink",
    "Timbiqui_Dark",
    "Uronium",
    "Klaxanite",
    "Ferdonite",
    "Lexan",
    "Uranium",
    "Jeranitium",
    "Maronite",
    "Copper",
    "Iron",
    "Titanium",
    "Platinum",
    "Silver",
    "Lumber",
    "Diamond",
    "Ruby",
    "Sapphire",
    "Gem",
    "Wood",
    "Water",
    "Marble",
    "Machinery",
    "Men",
    "Women",
    "Children",
    "Food",
    "Furniture",
    "Crude_Oil",
    "Dirt",
    "Rock",
    "Fabric",
    "Clothing",
    "Ferrocrete",
    "Kelvinium",
    "Coal",
    "Sensors_10",
    "Sensors_20",
    "Sensors_30",
    "Sensors_40",
    "Sensors_50",
    "Sensors_60",
    "Sensors_70",
    "Sensors_80",
    "Sensors_90",
    "Sensors_100",
    "Myomer_10",
    "Myomer_20",
    "Myomer_30",
    "Myomer_40",
    "Myomer_50",
    "Myomer_60",
    "Myomer_70",
    "Myomer_80",
    "Myomer_90",
    "Myomer_100",
    "TripleMyomer_10",
    "TripleMyomer_20",
    "TripleMyomer_30",
    "TripleMyomer_40",
    "TripleMyomer_50",
    "TripleMyomer_60",
    "TripleMyomer_70",
    "TripleMyomer_80",
    "TripleMyomer_90",
    "TripleMyomer_100",
    "Internal_10",
    "Internal_20",
    "Internal_30",
    "Internal_40",
    "Internal_50",
    "Internal_60",
    "Internal_70",
    "Internal_80",
    "Internal_90",
    "Internal_100",
    "ES_Internal_10",
    "ES_Internal_20",
    "ES_Internal_30",
    "ES_Internal_40",
    "ES_Internal_50",
    "ES_Internal_60",
    "ES_Internal_70",
    "ES_Internal_80",
    "ES_Internal_90",
    "ES_Internal_100",
    "JumpJet_10",
    "JumpJet_20",
    "JumpJet_30",
    "JumpJet_40",
    "JumpJet_50",
    "JumpJet_60",
    "JumpJet_70",
    "JumpJet_80",
    "JumpJet_90",
    "JumpJet_100",
    "UpperArmActuator_10",
    "UpperArmActuator_20",
    "UpperArmActuator_30",
    "UpperArmActuator_40",
    "UpperArmActuator_50",
    "UpperArmActuator_60",
    "UpperArmActuator_70",
    "UpperArmActuator_80",
    "UpperArmActuator_90",
    "UpperArmActuator_100",
    "LowerArmActuator_10",
    "LowerArmActuator_20",
    "LowerArmActuator_30",
    "LowerArmActuator_40",
    "LowerArmActuator_50",
    "LowerArmActuator_60",
    "LowerArmActuator_70",
    "LowerArmActuator_80",
    "LowerArmActuator_90",
    "LowerArmActuator_100", 
    "HandActuator_10",
    "HandActuator_20",
    "HandActuator_30",
    "HandActuator_40",
    "HandActuator_50",
    "HandActuator_60",
    "HandActuator_70",
    "HandActuator_80",
    "HandActuator_90",
    "HandActuator_100",
    "UpperLegActuator_10",
    "UpperLegActuator_20",
    "UpperLegActuator_30",
    "UpperLegActuator_40",
    "UpperLegActuator_50",
    "UpperLegActuator_60",
    "UpperLegActuator_70",
    "UpperLegActuator_80",
    "UpperLegActuator_90",
    "UpperLegActuator_100",
    "LowerLegActuator_10",
    "LowerLegActuator_20",
    "LowerLegActuator_30",
    "LowerLegActuator_40",
    "LowerLegActuator_50",
    "LowerLegActuator_60",
    "LowerLegActuator_70",
    "LowerLegActuator_80",
    "LowerLegActuator_90",
    "LowerLegActuator_100",
    "FootActuator_10",
    "FootActuator_20", 
    "FootActuator_30",
    "FootActuator_40",
    "FootActuator_50",
    "FootActuator_60",
    "FootActuator_70",
    "FootActuator_80",
    "FootActuator_90",
    "FootActuator_100",
    "Engine_20",
    "Engine_40",
    "Engine_60",
    "Engine_80",
    "Engine_100",
    "Engine_120",
    "Engine_140",
    "Engine_160",
    "Engine_180",
    "Engine_200",
    "Engine_220",
    "Engine_240",
    "Engine_260",
    "Engine_280",
    "Engine_300",
    "Engine_320",
    "Engine_340",
    "Engine_360",
    "Engine_380",
    "Engine_400",
    "XL_Engine_20",
    "XL_Engine_40",
    "XL_Engine_60",
    "XL_Engine_80",
    "XL_Engine_100",
    "XL_Engine_120",
    "XL_Engine_140",
    "XL_Engine_160",
    "XL_Engine_180",
    "XL_Engine_200",
    "XL_Engine_220",
    "XL_Engine_240",
    "XL_Engine_260",
    "XL_Engine_280",
    "XL_Engine_300",
    "XL_Engine_320",
    "XL_Engine_340",
    "XL_Engine_360",
    "XL_Engine_380",
    "XL_Engine_400",
    "ICE_Engine_20",
    "ICE_Engine_40",
    "ICE_Engine_60",
    "ICE_Engine_80",
    "ICE_Engine_100",
    "ICE_Engine_120",
    "ICE_Engine_140",
    "ICE_Engine_160",
    "ICE_Engine_180",
    "ICE_Engine_200",
    "ICE_Engine_220",
    "ICE_Engine_240",
    "ICE_Engine_260",
    "ICE_Engine_280",
    "ICE_Engine_300",
    "ICE_Engine_320",
    "ICE_Engine_340",
    "ICE_Engine_360",
    "ICE_Engine_380",
    "ICE_Engine_400",
    "Light_Engine_20",
    "Light_Engine_40",
    "Light_Engine_60",
    "Light_Engine_80",
    "Light_Engine_100",
    "Light_Engine_120",
    "Light_Engine_140",
    "Light_Engine_160",
    "Light_Engine_180",
    "Light_Engine_200",
    "Light_Engine_220",
    "Light_Engine_240",
    "Light_Engine_260",
    "Light_Engine_280",
    "Light_Engine_300",
    "Light_Engine_320",
    "Light_Engine_340",
    "Light_Engine_360",
    "Light_Engine_380",
    "Light_Engine_400",
    "CO_Internal_10",
    "CO_Internal_20",
    "CO_Internal_30",
    "CO_Internal_40",
    "CO_Internal_50",
    "CO_Internal_60",
    "CO_Internal_70",
    "CO_Internal_80",
    "CO_Internal_90",
    "CO_Internal_100",
    "RE_Internal_10",
    "RE_Internal_20",
    "RE_Internal_30",
    "RE_Internal_40",
    "RE_Internal_50",
    "RE_Internal_60",
    "RE_Internal_70",
    "RE_Internal_80",
    "RE_Internal_90",
    "RE_Internal_100",
    "Gyro_1",
    "Gyro_2",
    "Gyro_3",
    "Gyro_4",
    "XL_Gyro_1",
    "XL_Gyro_2",
    "XL_Gyro_3",
    "XL_Gyro_4",
    "HD_Gyro_1",
    "HD_Gyro_2",
    "HD_Gyro_3",
    "HD_Gyro_4",
    "Compact_Gyro_1",
    "Compact_Gyro_2",
    "Compact_Gyro_3",
    "Compact_Gyro_4",
    "XXL_Engine_20",
    "XXL_Engine_40",
    "XXL_Engine_60",
    "XXL_Engine_80",
    "XXL_Engine_100",
    "XXL_Engine_120",
    "XXL_Engine_140",
    "XXL_Engine_160",
    "XXL_Engine_180",
    "XXL_Engine_200",
    "XXL_Engine_220",
    "XXL_Engine_240",
    "XXL_Engine_260",
    "XXL_Engine_280",
    "XXL_Engine_300",
    "XXL_Engine_320",
    "XXL_Engine_340",
    "XXL_Engine_360",
    "XXL_Engine_380",
    "XXL_Engine_400",
    "Compact_Engine_20",
    "Compact_Engine_40",
    "Compact_Engine_60",
    "Compact_Engine_80",
    "Compact_Engine_100",
    "Compact_Engine_120",
    "Compact_Engine_140",
    "Compact_Engine_160",
    "Compact_Engine_180",
    "Compact_Engine_200",
    "Compact_Engine_220",
    "Compact_Engine_240",
    "Compact_Engine_260",
    "Compact_Engine_280",
    "Compact_Engine_300",
    "Compact_Engine_320",
    "Compact_Engine_340",
    "Compact_Engine_360",
    "Compact_Engine_380",
    "Compact_Engine_400",
    NULL
};

int count_special_items()
{
    int i = 0;

    while (internals[i])
	i++;
    return i;
}

char *section_configs[] = {
    "Case", "Destroyed",
    NULL
};

char *move_types[] = {
    "Biped", "Track", "Wheel", "Hover", "VTOL", "Hull", "Foil", "Fly",
    "Quad",
    "Sub", "None",
    NULL
};

char *mech_types[] = {
    "Mech", "Vehicle", "VTOL", "Naval", "Spheroid_DropShip", "AeroFighter",
    "Mechwarrior", "Aerodyne_DropShip", "Battlesuit", NULL
};

char *crit_modes[] = {
    "RearMount",
    "UltraMode",
    "OnTC",
    "LBX/Cluster",
    "Artemis/Mine",
    "Narc/Smoke",
    "Hotload",
    "Inferno",
    "Swarm",
    "Swarm1",
    "OneShot",
    "Disabled",
    "Pierce",
    "Heat",
    "OneShot_Used",
    "Destroyed",
    "Caseless",
    "Tracer",
    "SemiGuided",
    "Jammed",
    "DeadFire",
    "ATM-ER",
    "ATM-HE",
    "FourShot",
    "SixShot",
    "Precision",
    "LaserHP",
    "AmmoHP",
    "MissileHP",
    "SpecialHP",
    "Stinger",
    "GaussHP",
  NULL
};

char *specials[] = {
    "TripleMyomerTech", "CL_AMS", "IS_AMS",
    "DoubleHS", "Masc", "Clan", "FlipArms", "C3MasterTech",
    "C3SlaveTech", "ArtemisIV", "ECM", "BeagleProbe", "SalvageTech",
    "CargoTech", "SearchLight",
    "CSStealth_Tech", "AntiAircraft", "NoSensors",
    "SS_Ability", "FerroFibrous_Tech", "EndoSteel_Tech",
    "XLEngine_Tech",
    "ICEEngine_Tech",
    "DCStealth_Tech",
    "FWLStealth_Tech",
    "XXL_Tech",
    "CompactEngine_Tech",
    "ReinforcedInternal_Tech",
    "CompositeInternal_Tech",
    "HardenedArmor_Tech",
    "CritProof_Tech",
    "NullSig_Tech",
    NULL
};

char *specialsabrev[] = {
    "TSM", "CLAMS", "ISAMS", "DHS", "MASC", "CLTECH", "FA", "C3M", "C3S",
    "AIV", "ECM", "BAP", "SAL", "CAR", "SL", "TCS", "AA", "NOSEN", "SS",
    "FF", "ES", "XL", "ICE", "TAS", "TBS", "XXL", "CENG", "RINT", "CINT",
    "HARM", "CP", "NULL",
    NULL
};

char *specials2[] = {
    "SuperCharger_Tech",
    "LightEngine_Tech",
    "TorsoCockpit_Tech",
    "CompactGyro_Tech",
    "XLGyro_Tech",
    "HDGyro_Tech",
    "SmallCockpit_Tech",
    "Angel_ECM",
    "Bloodhound_Probe",
    "Omni_Tech",
    "StealthArmor_Tech",
    "HeavyTank_Tech",
    "Hidden_Tech",
    "Carrier_Tech",
    "Adv_AntiAircraft",
    "CompactHS",
    "HeavyFerroFibrous_Tech",
    "LightFerroFibrous_Tech",
    "NoReentry_Tech",
    "Fragile_Tech",
    "Waterproof_Tech",
    "Personal_ECM",
    "Grapple_Claw",
    NULL
};

char *specialsabrev2[]= {
    "SCHARGE", "LENG", "TCPIT", "CGYRO", "XLGYRO", "HDGYRO",
    "SCPIT", "AECM", "BLP", "OMNI", "STHA", "HVYTNK", "HIDT", "CART", "ADAA",
    "CHS", "HFF", "LFF", "NORE", "FRAG", "WPRF", "PECM", "GCLAW",
    NULL
};

int compare_array(char *list[], char *command)
{
    int x;

    if (!list)
	return -1;
    for (x = 0; list[x]; x++)
	if (!strcasecmp(list[x], command))
            return x;
    return -1;
}

char *one_arg(char *argument, char *first_arg)
{
    if (isspace(*argument))
	for (argument++; isspace(*argument); argument++);

    while (*argument && !isspace(*argument))
	*(first_arg++) = *(argument++);
    *first_arg = '\0';
    return argument;
}

char *BuildBitString(char *bitdescs[], int data)
{
    static char crit[MAX_STRING_LENGTH];
    long bv;
    int x;

    crit[0] = 0;
    for (x = 0; (x < 33 && bitdescs[x]); x++) {
	bv = 1 << x;
	if (data & bv) {
	    strcat(crit, bitdescs[x]);
	    strcat(crit, " ");
	}
    }
    if ((x = strlen(crit)) > 0)
	if (crit[x - 1] == ' ')
	    crit[x - 1] = 0;
    return crit;
}

#define QDM(a) case I2Special(a): return 1
static int InvalidVehicleItem(MECH * mech, int x, int y)
{
    int t;

    t = GetPartType(mech, x, y);
    switch (t) {
	QDM(SHOULDER_OR_HIP);
	QDM(UPPER_ACTUATOR);
	QDM(LOWER_ACTUATOR);
	QDM(HAND_OR_FOOT_ACTUATOR);
	QDM(ENGINE);
	QDM(GYRO);
/*	QDM(JUMP_JET); */
/* Added to support tank JJ's */
    }
    return 0;
}

extern int num_def_weapons;
int internal_count = sizeof(internals) / sizeof(char *) - 1;
int cargo_count = sizeof(cargo) / sizeof(char *) - 1;

#ifndef CLAN_SUPPORT
#define CLCH(a) do { if (MechWeapons[a].special & CLAT) return NULL; } while (0)
#else
#define CLCH(a) \
do { if (MechWeapons[a].special & CLAT) isclan=1; } while (0)
#endif

static char *part_figure_out_name_sub(int i, int j)
{
    static char buf[MBUF_SIZE];
    int isclan = 0;

    if (!i)
	return NULL;
    if (IsWeapon(i) && i < I2Weapon(num_def_weapons)) {
	CLCH(Weapon2I(i));
	return &MechWeapons[Weapon2I(i)].name[(j && !isclan) ? 3 : 0];
    } else if (IsAmmo(i) && i < I2Ammo(num_def_weapons)) {
	CLCH(Ammo2WeaponI(i));
/*	if (MechWeapons[Ammo2WeaponI(i)].type != TBEAM &&
	    MechWeapons[Ammo2WeaponI(i)].type != THAND &&
	    !(MechWeapons[Ammo2WeaponI(i)].special & PCOMBAT)) { */
	    sprintf(buf, "Ammo_%s", &MechWeapons[Ammo2WeaponI(i)].name[(j
			&& !isclan) ? 3 : 0]);
	    return buf;
/*	} */
    } if (IsBomb(i)) {
	sprintf(buf, "Bomb_%s", bomb_name(Bomb2I(i)));
	return buf;
    } else if (IsSpecial(i) && i < I2Special(internal_count))
	return internals[Special2I(i)];
    else if (IsCargo(i) && i < I2Cargo(cargo_count))
	return cargo[Cargo2I(i)];
    return NULL;
}

char *my_shortform(char *buf)
{
    static char buf2[MBUF_SIZE];
    char *c, *d;

    if (!buf)
	return NULL;
    if (strlen(buf) <= 4 && !strchr(buf, '/'))
	strcpy(buf2, buf);
    else {
	for (c = buf, d = buf2; *c; c++)
	    if (isdigit(*c) || isupper(*c) || *c == '_')
		*d++ = *c;
	*d = 0;
	if (strlen(buf2) == 1)
	    strcat(d, tprintf("%c", buf[1]));
    }
    return buf2;
}

#undef CLCH
#ifdef CLAN_SUPPORT
#define CLCH(a) ((!strncasecmp(MechWeapons[a].name, "CL.", 2)) ? 0 : 3)
#else
#define CLCH(a) 3
#endif

char *part_figure_out_shname(int i)
{
    char buf[MBUF_SIZE];

    if (!i)
	return NULL;
    buf[0] = 0;
    if (IsWeapon(i) && i < I2Weapon(num_def_weapons)) {
	strcpy(buf, &MechWeapons[Weapon2I(i)].name[CLCH(Weapon2I(i))]);
    } else if (IsAmmo(i) && i < I2Ammo(num_def_weapons)) {
	sprintf(buf, "Ammo_%s",
	    &MechWeapons[Ammo2WeaponI(i)].name[CLCH(Ammo2WeaponI(i))]);
    } else if (IsBomb(i))
	sprintf(buf, "Bomb_%s", bomb_name(Bomb2I(i)));
    else if (IsSpecial(i) && i < I2Special(internal_count))
	strcpy(buf, internals[Special2I(i)]);
    if (IsCargo(i) && i < I2Cargo(cargo_count))
	strcpy(buf, cargo[Cargo2I(i)]);
    if (!buf[0])
	return NULL;
    return my_shortform(buf);
}

char *part_figure_out_name(int i)
{
    return part_figure_out_name_sub(i, 0);
}

char *part_figure_out_sname(int i)
{
    return part_figure_out_name_sub(i, 1);
}

#define TCAble(t) \
((MechWeapons[Weapon2I(t)].type == TBEAM || MechWeapons[Weapon2I(t)].type == TAMMO) && MechWeapons[Weapon2I(t)].class != WCLASS_FLAM && !(MechWeapons[Weapon2I(t)].special & PCOMBAT))


static int dump_item(FILE * fp, MECH * mech, int x, int y)
{
    char crit[10];
    int y1;
    int flaggo = 0;
    int m, z;

    if (!GetPartType(mech, x, y))
	return 1;
    if (MechType(mech) != CLASS_MECH && InvalidVehicleItem(mech, x, y))
	return 1;
    for (y1 = y + 1; y1 < 12; y1++) {
	if (GetPartType(mech, x, y1) != GetPartType(mech, x, y))
	    break;
	if (GetPartData(mech, x, y1) != GetPartData(mech, x, y))
	    break;
	if (GetPartMode(mech, x, y1) != GetPartMode(mech, x, y))
	    break;
    }
    y1--;
    if (IsWeapon(GetPartType(mech, x, y))) {
	/* Nonbeams, or flamers don't have TC */
	if (!TCAble(GetPartType(mech, x, y)))
	    flaggo = ON_TC;
	if (((y1 - y) + 1) > (z =
		GetWeaponCrits(mech, Weapon2I(GetPartType(mech, x, y)))))
	    y1 = y + z - 1;
    }
    if (y != y1)
	sprintf(crit, "CRIT_%d-%d", y + 1, y1 + 1);
    else
	sprintf(crit, "CRIT_%d", y + 1);
    m = GetPartMode(mech, x, y);
    m &= ~flaggo;
    if (IsWeapon(GetPartType(mech, x, y)))
	fprintf(fp, "    %s		  { %s - %s %s}\n", crit,
	    get_parts_vlong_name(GetPartType(mech, x, y)),
	    m ? BuildBitString(crit_modes, m) : "-",
	    "");
    else if (IsAmmo(MechSections(mech)[x].criticals[y].type))
	fprintf(fp, "    %s		  { %s %d %s - }\n", crit,
	    get_parts_vlong_name(GetPartType(mech, x, y)),
	    FullAmmo(mech, x, y),
	    MechSections(mech)[x].
	    criticals[y].mode ? BuildBitString(crit_modes,
	    MechSections(mech)[x].criticals[y].mode) : "-");
    else if (IsBomb(MechSections(mech)[x].criticals[y].type))
	fprintf(fp, "    %s		  { %s - - - }\n", crit,
	    get_parts_vlong_name(GetPartType(mech, x, y)));
    else {
      if (IsDataSpecial(Special2I(GetPartType(mech, x, y)))) {
 	fprintf(fp, "    %s		  { %s %d - %s}\n", crit,
	    get_parts_vlong_name(GetPartType(mech, x, y)),
	    GetPartData(mech, x, y),
	    "");
      } else {
        fprintf(fp, "    %s               { %s - %s %s}\n", crit,
            get_parts_vlong_name(GetPartType(mech, x, y)),
	    MechSections(mech)[x].criticals[y].mode ?  BuildBitString(crit_modes, MechSections(mech)[x].criticals[y].mode) : "-",
            "");
      }
    }
    return (y1 - y + 1);
}

void dump_locations(FILE * fp, MECH * mech, const char *locdesc[])
{
    int x, y, l;
    char buf[512];
    char *ch;

    for (x = 0; locdesc[x]; x++) {
	if (!GetSectOInt(mech, x) && !(is_aero(mech)))
	    continue;
	strcpy(buf, locdesc[x]);
	for (ch = buf; *ch; ch++)
	    if (*ch == ' ')
		*ch = '_';
	fprintf(fp, "%s\n", buf);
	if (GetSectOArmor(mech, x))
	    fprintf(fp, "  Armor            { %d }\n", GetSectOArmor(mech,
		    x));
	if (GetSectOInt(mech, x))
	    fprintf(fp, "  Internals        { %d }\n", GetSectOInt(mech,
		    x));
	if (GetSectORArmor(mech, x))
	    fprintf(fp, "  Rear             { %d }\n", GetSectORArmor(mech,
		    x));
#if 0				/* Shouldn't be neccessary to save at all */
	fprintf(fp, "  Recycle          { %d }\n",
	    MechSections(mech)[x].recycle);
#endif
	y = MechSections(mech)[x].config;
	y &= ~CASE_TECH;
	if (y)
	    fprintf(fp, "  Config           { %s }\n",
		BuildBitString(section_configs, y));
	l = CritsInLoc(mech, x);
	for (y = 0; y < l;)
	    y += dump_item(fp, mech, x, y);
    }
}

float generic_computer_multiplier(MECH * mech)
{
    switch (MechComputer(mech)) {
    case 1:
	return 0.8;
    case 2:
	return 1;
    case 3:
	return 1.25;
    case 4:
	return 1.5;
    case 5:
	return 1.75;
    }
    return 0;
}

int generic_radio_type(int i, int isClan)
{
    int f = DEFAULT_FREQS;

    if (isClan || i >= 4)
	f += FREQS * RADIO_RELAY;
    if (i < 3)
	f -= (3 - i) * 2 - 1;	/* 2 or 4 */
    else
	f += (i - 3) * 3;	/* 5 / 8 / 11 */
    return f;
}

float generic_radio_multiplier(MECH * mech)
{
    switch (MechRadio(mech)) {
    case 1:
	return 0.8;
    case 2:
	return 1;
    case 3:
	return 1.25;
    case 4:
	return 1.5;
    case 5:
	return 1.75;
    }
    return 0.0;
}

#define MechComputersScanRange(mech) \
(generic_computer_multiplier(mech) * DEFAULT_SCANRANGE)

#define MechComputersLRSRange(mech) \
(generic_computer_multiplier(mech) * DEFAULT_LRSRANGE)

#define MechComputersTacRange(mech) \
(generic_computer_multiplier(mech) * DEFAULT_TACRANGE)

#define MechComputersRadioRange(mech) \
(DEFAULT_RADIORANGE * generic_radio_multiplier(mech))

void computer_conversion(MECH * mech)
{
    int l = 0;

    switch (MechScanRange(mech)) {
    case 20:
	l = 2;
	break;
    case 25:
	l = 3;
	break;
    case 30:
	l = 4;
	break;
    }
    if (l) {
	MechComputer(mech) = l;
	MechScanRange(mech) = MechComputersScanRange(mech);
	MechTacRange(mech) = MechComputersTacRange(mech);
	MechLRSRange(mech) = MechComputersLRSRange(mech);
	MechRadioRange(mech) = MechComputersRadioRange(mech);
    }
}


void try_to_find_name(char *mechref, MECH * mech)
{
    const char *c;

    if ((c = find_mechname_by_mechref(mechref)))
	strncpy(MechType_Name(mech), c, MECHNAME_SIZE);
}

int DefaultFuelByType(MECH * mech)
{
    int mod = 2;

    switch (MechType(mech)) {
    case CLASS_VTOL:
	return 2000 * mod;
    case CLASS_AERO:
	return 1200 * mod;
    case CLASS_DS:
    case CLASS_SPHEROID_DS:
	return 3600 * mod;
    }
    return 0;
}

int save_template(dbref player, MECH * mech, char *reference, char *filename)
{
    FILE *fp;
    int x, x2;
    const char **locs;
    char *d, *c = ctime(&mudstate.now);

    if (!MechComputer(mech))
	computer_conversion(mech);
    if (!MechType_Name(mech)[0])
	try_to_find_name(reference, mech);
    if (!(fp = fopen(filename, "w")))
	return -1;
    if (MechType_Name(mech)[0])
	fprintf(fp, "Name             { %s }\n", MechType_Name(mech));
    fprintf(fp, "Reference        { %s }\n", reference);
    fprintf(fp, "Type             { %s }\n",
	mech_types[(short) MechType(mech)]);
    fprintf(fp, "Move_Type        { %s }\n",
	move_types[(short) MechMove(mech)]);
    fprintf(fp, "Tons             { %d }\n", MechTons(mech));
    if ((d = strrchr(c, '\n')))
	*d = 0;
    fprintf(fp, "Comment          { Saved by: %s(#%d) at %s }\n",
	Name(player), player, c);
#define SILLY_UTTERANCE(ran,cran,dran,name) \
  if ((!MechComputer(mech) && ran != dran) || \
      (MechComputer(mech) && ran != cran)) \
      fprintf(fp, "%-16s { %d }\n", name, ran)

    SILLY_UTTERANCE(MechTacRange(mech), MechComputersTacRange(mech),
	DEFAULT_TACRANGE, "Tac_Range");
    SILLY_UTTERANCE(MechLRSRange(mech), MechComputersLRSRange(mech),
	DEFAULT_LRSRANGE, "LRS_Range");
    SILLY_UTTERANCE(MechScanRange(mech), MechComputersScanRange(mech),
	DEFAULT_SCANRANGE, "Scan_Range");
    SILLY_UTTERANCE(MechRadioRange(mech), MechComputersRadioRange(mech),
	DEFAULT_RADIORANGE, "Radio_Range");

#define SILLY_OUTPUT(def,now,name) \
  if ((def) != (now)) \
     fprintf(fp, "%-16s { %d }\n", name, now)

    SILLY_OUTPUT(DEFAULT_COMPUTER, MechComputer(mech), "Computer");
    SILLY_OUTPUT(DEFAULT_RADIO, MechRadio(mech), "Radio");
/*    SILLY_OUTPUT(DEFAULT_HEATSINKS, MechRealNumsinks(mech), "Heat_Sinks"); -- No longer so silly! -- Reb */
    fprintf(fp, "Heat_Sinks       { %d }\n", MechRealNumsinks(mech));
    SILLY_OUTPUT(generic_radio_type(MechRadio(mech),
	    MechSpecials(mech) & CLAN_TECH), MechRadioType(mech),
	"RadioType");
    SILLY_OUTPUT(0, MechBV(mech), "Mech_BV");
    SILLY_OUTPUT(0, CargoSpace(mech), "Cargo_Space");
    SILLY_OUTPUT(0, CarMaxTon(mech), "Max_Ton");
    SILLY_OUTPUT(0, PodSpace(mech), "Pod_Space");
    SILLY_OUTPUT(0, AeroSIOrig(mech), "SI");

    SILLY_OUTPUT(DefaultFuelByType(mech), AeroFuelOrig(mech), "Fuel");

    fprintf(fp, "Max_Speed        { %.2f }\n", MechMaxSpeed(mech));
    if (MechJumpSpeed(mech) > 0.0)
	fprintf(fp, "Jump_Speed       { %.2f }\n", MechJumpSpeed(mech));
    x = MechSpecials(mech);
    /* Remove AMS'es, they're re-generated back on loadtime */
    x &= ~(CL_ANTI_MISSILE_TECH | IS_ANTI_MISSILE_TECH | SS_ABILITY);
    x &=			/* Calculated at load-time */
	~(BEAGLE_PROBE_TECH | TRIPLE_MYOMER_TECH | MASC_TECH | ECM_TECH |
	C3_SLAVE_TECH | C3_MASTER_TECH | ARTEMIS_IV_TECH | ES_TECH |
	FF_TECH | NULLSIG_TECH);
    x2 = MechSpecials2(mech);
    x2 &= ~(SCHARGE_TECH|ANGEL_ECM_TECH|BLOODHOUND_PROBE_TECH|OMNI_TECH|STEALTHARMOR_TECH|LFF_TECH|HFF_TECH);

    if (MechType(mech) == CLASS_MECH) {
	x &= ~(XL_TECH | XXL_TECH | CE_TECH);
	x2 &= ~(LENG_TECH|TORSOCOCKPIT_TECH|XLGYRO_TECH|CGYRO_TECH|SMALLCOCKPIT_TECH);
	}
    if (x)
	fprintf(fp, "Specials         { %s }\n", BuildBitString(specials,
		x));
    if (x2)
	fprintf(fp, "Specials2        { %s }\n", BuildBitString(specials2,
		x2));
    if ((locs =
	    ProperSectionStringFromType(MechType(mech), MechMove(mech)))) {
	dump_locations(fp, mech, locs);
	fclose(fp);
	return 0;
    }
    fclose(fp);
    return -1;
}


char *read_desc(FILE * fp, char *data)
{
    char keep[MAX_STRING_LENGTH + 500];
    char *t, *tmp;
    char *point;
    int length = 0;
    static char buf[MAX_STRING_LENGTH];

    keep[0] = '\0';
    if ((tmp = strchr(data, '{'))) {
	fscanf(fp, "\n");
	while (isspace(*(++tmp)));
	if ((t = strchr(tmp, '}'))) {
	    while (isspace(*(t--)));
	    *(t++) = '\0';
	    length = strlen(tmp);
	    strcpy(buf, tmp);
	    return buf;
	} else {
	    strcpy(keep, tmp);
	    strcat(keep, "\r\n");
	    t = tmp + strlen(tmp) - 1;
	    length = strlen(t);
	    while (fgets(data, 512, fp)) {
		fscanf(fp, "\n");
		if ((tmp = strchr(data, '}')) != NULL) {
		    *tmp = 0;
		    length += strlen(data);
		    strcat(keep, data);
		    break;
		} else {
		    point = data + strlen(data) - 1;
		    *(point++) = '\r';
		    *(point++) = '\n';
		    *point = '\0';
		    strcat(keep, data);
		    length += strlen(data);
		}
	    }
	}
    }
    strcpy(buf, keep);
    return buf;
}

int find_section(char *cmd, int type, int mtype)
{
    char section[20];
    char *ch;
    const char **locs;

    strcpy(section, cmd);
    for (ch = section; *ch; ch++)
	if (*ch == '_')
	    *ch = ' ';
    locs = ProperSectionStringFromType(type, mtype);
    return compare_array((char **) locs, section);
    return -1;
}

long BuildBitVector(char **list, char *line)
{
    long bv = 0;
    int temp;
    char buf[30];

    if (!strcasecmp(line, "-"))
	return 0;

    while (*line) {
	line = one_arg(line, buf);
	if ((temp = compare_array(list, buf)) == -1)
	    return -1;
        bv += 1 << temp;
    }
    return bv;
}

int WeaponIFromString(char *data)
{
    int x = 0;;

    while (MechWeapons[x].name) {
	if (!strcasecmp(MechWeapons[x].name, data))
	    return x + 1;	/* weapons start at 1 not 0 */
	x++;
    }
    return -1;
}

int AmmoIFromString(char *data)
{
    int x = 0;
    char *ptr;

    ptr = data;
    while (*ptr != '_')
	ptr++;
    ptr++;
    while (MechWeapons[x].name) {
	if (!strcasecmp(MechWeapons[x].name, ptr))
	    return x + 101;
	x++;
    }
    return -1;
}

void update_specials(MECH * mech)
{
    int x, y, t;
    int i, b = 1;
    int masc_count = 0;
    int sc_count = 0;
    int c3_master_count = 0;
    int c3i_count = 0;
    int tsm_count = 0;
    int nullsig_count = 0;
    int ff_count = 0;
    int es_count = 0;
    int tc_count = 0;
    int cl = MechSpecials(mech) & CLAN_TECH;
    int e_count = 0;
    int g_count = 0;
    int st_count = 0;

    MechSpecials(mech) &=
	~(BEAGLE_PROBE_TECH | TRIPLE_MYOMER_TECH | MASC_TECH | ECM_TECH |
	C3_SLAVE_TECH | C3_MASTER_TECH | ARTEMIS_IV_TECH | ES_TECH |
	FF_TECH | IS_ANTI_MISSILE_TECH | CL_ANTI_MISSILE_TECH | NULLSIG_TECH);
    MechSpecials2(mech) &= ~(SCHARGE_TECH|ANGEL_ECM_TECH|BLOODHOUND_PROBE_TECH|OMNI_TECH|STEALTHARMOR_TECH|LFF_TECH|HFF_TECH);
    if (MechType(mech) == CLASS_MECH) {
	MechSpecials(mech) &= ~(XL_TECH | XXL_TECH | CE_TECH);
	MechSpecials2(mech) &= ~(LENG_TECH|TORSOCOCKPIT_TECH|XLGYRO_TECH|CGYRO_TECH|SMALLCOCKPIT_TECH);
	}
    if (PodSpace(mech) && !(MechSpecials2(mech) & OMNI_TECH))
	MechSpecials2(mech) |= OMNI_TECH;
    for (x = 0; x < NUM_SECTIONS; x++) {
	e_count = 0;
	MechSections(mech)[x].config &= ~CASE_TECH;
	for (y = 0; y < CritsInLoc(mech, x); y++) {
	    if ((GetPartMode(mech, x, y) & HPOINTS) &&
		 (!(MechSpecials2(mech) & OMNI_TECH)))
		MechSpecials2(mech) |= OMNI_TECH;
	    if ((t = GetPartType(mech, x, y))) {
		switch (Special2I(t)) {
#define TECHC(item,name) case item: name++; break;
#define TECHCU(item,name) \
case item: if (!PartIsDestroyed(mech, x, y)) name++; break;
#define TECH(item,name) case item: MechSpecials(mech) |= name; break
#define TECH2(item,name) case item: MechSpecials2(mech) |= name; break
#define TECHU(item, name) \
case item: if (!PartIsDestroyed(mech, x, y)) MechSpecials(mech) |= name ; break
		    TECH(ARTEMIS_IV, ARTEMIS_IV_TECH);
		    TECHU(BEAGLE_PROBE, BEAGLE_PROBE_TECH);
		    TECH(ECM, ECM_TECH);
		    TECH2(ANGEL_ECM, ANGEL_ECM_TECH);
		    TECH2(BLOODHOUND_PROBE, BLOODHOUND_PROBE_TECH);
		    TECHU(C3_SLAVE, C3_SLAVE_TECH);
		    TECHCU(MASC, masc_count);
		    TECHCU(SUPERCHARGER, sc_count);
		    TECHCU(C3_MASTER, c3_master_count);
		    TECHCU(C3I, c3i_count);
		    TECHC(TRIPLE_STRENGTH_MYOMER, tsm_count);
		    TECHC(NULLSIG, nullsig_count);
		    TECHC(FERRO_FIBROUS, ff_count);
		    TECHCU(TARGETING_COMPUTER, tc_count);
		    TECHC(ENDO_STEEL, es_count);
		    TECHC(STEALTHARM, st_count);

		case ENGINE:
		    e_count++;
		    break;
		case CASE:
		    MechSections(mech)[(MechType(mech) == CLASS_VEH_GROUND)
			? BSIDE : x].config |= CASE_TECH;
		    break;
		case HARDPOINT:
		    MechSpecials2(mech) |= OMNI_TECH;
		    break;
		case COCKPIT:
		    if (x != CTORSO || MechType(mech) != CLASS_MECH) {
			if (x == HEAD && MechType(mech) == CLASS_MECH)
			    if (!(IsSpecial(GetPartType(mech, HEAD, 5))) || (Special2I(GetPartType(mech, HEAD, 5)) != LIFE_SUPPORT))
				MechSpecials2(mech) |= SMALLCOCKPIT_TECH;
			b = 0;
			break;
			}
		    if (!(IsSpecial(GetPartType(mech, x, 10)) && Special2I(GetPartType(mech, x, 10)) == COCKPIT)) {
			SendError(tprintf("Mech #%d - Torso Mount Cockpit crit error. Center Torso Crit 11 not a Cockpit!",
				mech->mynum));
			b = 0;
			break;
			}
		    if (!(IsSpecial(GetPartType(mech, x, 10)) && Special2I(GetPartType(mech, x, 11)) == SENSORS)) {
                        SendError(tprintf("Mech #%d - Torso Mount Cockpit crit error. Center Torso Crit 12 not a Sensor!",
                                mech->mynum));
			b = 0;
                        break;
                        }
		    for (i = 0; i < NUM_CRITICALS; i++)
			if (IsSpecial(GetPartType(mech, LTORSO, i)) && Special2I(GetPartType(mech, LTORSO, i)) == LIFE_SUPPORT)
			    break;
			else
			    if (i >= 11)
				{
				SendError(tprintf("Mech #%d - Torso Mount Cockpit crit error. Left Torso contains no LifeSupport!",
                                	mech->mynum));
				b = 0;
				break;
				}
		    for (i = 0; i < NUM_CRITICALS; i++)
                        if (IsSpecial(GetPartType(mech, RTORSO, i)) && Special2I(GetPartType(mech, RTORSO, i)) == LIFE_SUPPORT)
                            break;
                        else
                            if (i >= 11)
                                {
                                SendError(tprintf("Mech #%d - Torso Mount Cockpit crit error. Right Torso contains no LifeSupport!",
                                        mech->mynum));
                                b = 0;
				break;
                                }
		    for (i = 0; i < NUM_CRITICALS; i++)
			if (IsSpecial(GetPartType(mech, HEAD, i)) && Special2I(GetPartType(mech, HEAD, i)) == COCKPIT) {
			    SendError(tprintf("Mech #%d - Torso Mount Cockpit crit error. Head contains Cockpit!",
                                        mech->mynum));
			    b = 0;
			    break;
			    }
		    if (b)
		        MechSpecials2(mech) |= TORSOCOCKPIT_TECH;
		    break;
		case GYRO:
		    g_count++;
		    break;
		}
		if (IsWeapon(t) && IsAMS(Weapon2I(t))) {
		    if (MechWeapons[Weapon2I(t)].special & CLAT)
			MechSpecials(mech) |= CL_ANTI_MISSILE_TECH;
		    else
			MechSpecials(mech) |= IS_ANTI_MISSILE_TECH;
		}
	    }
	}
	if (x != CTORSO && e_count) {
#if 0 /* Quick hack for odd templates which rely on partial weapon locations. Need better method to count total engines later */
	    if (e_count < 3 && !cl)
		MechSpecials2(mech) |= LENG_TECH;
	    else if (e_count > 3)
		MechSpecials(mech) |= XXL_TECH;
#else
	    if (e_count > 4) 
		MechSpecials(mech) |= XXL_TECH;
	    else if (e_count < 3 && !cl)
		MechSpecials2(mech) |= LENG_TECH;
#endif
	    else
		MechSpecials(mech) |= XL_TECH;
	} else {
	    if (x == CTORSO && e_count < 4 && MechType(mech) == CLASS_MECH)
		MechSpecials(mech) |= CE_TECH;
	}

    }
    if ((MechSpecials(mech) & (XXL_TECH | XL_TECH)) &&
	(MechSpecials(mech) & CE_TECH))
	SendError(tprintf
	    ("Mech #%d - Apparently is very weird. Compact engine AND XL/XXL?",
		mech->mynum));
    if (tc_count)
	for (x = 0; x < NUM_SECTIONS; x++)
	    for (y = 0; y < CritsInLoc(mech, x); y++)
		if (IsWeapon((t = GetPartType(mech, x, y))))
		    if (TCAble(t))
			GetPartMode(mech, x, y) |= ON_TC;
    if (masc_count >= MAX(1, (MechTons(mech) / (cl ? 25 : 20)))) {
	MechSpecials(mech) |= MASC_TECH;
	MechMASCCounter(mech) = 0;
	}
    if (sc_count > 0) {
	MechSpecials2(mech) |= SCHARGE_TECH;
	MechSChargeCounter(mech) = 0;
	}
    if (g_count) {
	if (g_count > 5) {
	    MechSpecials2(mech) |= XLGYRO_TECH;
	} else if (g_count < 3) {
	    MechSpecials2(mech) |= CGYRO_TECH;
	}
    }
#define ITech(var,cnt,spec) \
  if (((var)) >= ((cnt)) || (MechType(mech) != CLASS_MECH && ((var)>0))) \
      MechSpecials(mech) |= spec
#define ITech2(var,cnt,spec) \
  if (((var)) >= ((cnt)) || (MechType(mech) != CLASS_MECH && ((var)>0))) \
      MechSpecials2(mech) |= spec
    ITech(c3_master_count, 5, C3_MASTER_TECH);
    ITech(c3i_count, 2, C3_MASTER_TECH|C3_SLAVE_TECH);
    if (MechType(mech) != CLASS_MECH) {
      switch (ff_count) {
          case 1:
              ITech2(ff_count, 1, LFF_TECH);
              break;
          case 3:
              ITech2(ff_count, 3, HFF_TECH);
              break;
          default:
              ITech(ff_count, 2, FF_TECH);
              break;
          }
      } else {
      if (!cl && ff_count < 14) ITech2(ff_count, 7, LFF_TECH);
      if (ff_count < 21 ) ITech(ff_count, (cl ? 7 : 14), FF_TECH);
      if (!cl) ITech2(ff_count, 21, HFF_TECH);
      }
    ITech(es_count, (cl ? 7 : 14), ES_TECH);
    ITech(tsm_count, 6, TRIPLE_MYOMER_TECH);
    ITech(nullsig_count, 7, NULLSIG_TECH);
    ITech2(st_count, 12, STEALTHARMOR_TECH);
}

int update_oweight(MECH * mech, int value)
{
    MechCritStatus(mech) |= OWEIGHT_OK;

    /* Check to prevent silliness */
    if (!mudconf.btech_dynspeed || (value == 1 && !Destroyed(mech)))
	value = MechTons(mech) * 1024;
    MechRTonsV(mech) = value;
    return value;
}

int get_weight(MECH * mech)
{
    if (MechCritStatus(mech) & OWEIGHT_OK)
	return MechRTonsV(mech);
    return update_oweight(mech, mech_weight_sub(GOD, mech, -1));
}

int load_template(dbref player, MECH * mech, char *filename)
{
    char line[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    int x, y, value, i;
    char cmd[MAX_STRING_LENGTH];
    char *ptr, *j, *k, *line2;
    int section = 0, critical, selection, type;
    FILE *fp = fopen(filename, "r");
    char *tmpc;
    int lpos, hpos;
    int ok_count = 0;
    int isClan = 0;
    int mod, t;
    MAP *map;

    if (!fp)
	return -1;

    ptr = strrchr(filename, '/');
    if (ptr == NULL) {
	ptr = filename;
    } else {
	ptr++;
    }
    strncpy(MechType_Ref(mech), ptr, MECHREF_SIZE);
    MechType_Ref(mech)[MECHREF_SIZE - 1] = '\0';

    silly_atr_set(mech->mynum, A_MECHREF, MechType_Ref(mech));
    MechRadioType(mech) = 0;
    while (fgets(line, 512, fp)) {
	line[strlen(line) - 1] = 0;
	j = line;
	while (isspace(*j))
	    j++;
	strcpy(line, j);
	if ((ptr = strchr(line, ' '))) {
	    if ((tmpc = strchr(line, '\t')) < ptr)
		if (tmpc)
		    ptr = tmpc;
	    j = line;
	    k = cmd;
	    while (j != ptr)
		*(k++) = *(j++);
	    *k = 0;
	    for (ptr++; isspace(*ptr); ptr++);
	} else {
	    strcpy(cmd, line);
	    strcpy(line, "");
	    ptr = NULL;
	}
	if (!strncasecmp(cmd, "CRIT_", 5))
	    selection = 9999;
	else if ((selection = compare_array(load_cmds, cmd)) == -1) {
	    /* Initial premise: we will have a mech type before we get to this */
	    section = find_section(cmd, MechType(mech), MechMove(mech));
	    TEMPLATE_ERR(section == -1 &&
		!ok_count,
		"New template loading system: %s is invalid template file.",
		filename);
	    TEMPLATE_GERR(section == -1,
		"Error while loading: Section %s not found.", cmd);
	    MechSections(mech)[section].recycle = 0;
	    ok_count++;
	    continue;
	}
	ok_count++;
	switch (selection) {
	case 0:		/* Reference */
	    tmpc = read_desc(fp, ptr);
	    if (strcmp(tmpc, MechType_Ref(mech))) {
		SendError(tprintf("Mech #%d - Template %s has Reference <-> Filename mismatch : %s <-> %s - It is automatically fixed by saving again.",
			mech->mynum, filename, tmpc, MechType_Ref(mech)));
		tmpc = MechType_Ref(mech);
	    }
	    silly_atr_set(mech->mynum, A_MECHREF, tmpc);
	    strncpy(MechType_Ref(mech), tmpc, MECHREF_SIZE);
	    break;
	case 1:		/* Type */
	    tmpc = read_desc(fp, ptr);
	    MechType(mech) = compare_array(mech_types, tmpc);
	    TEMPLATE_GERR(MechType(mech) == -1,
		"Error while loading: Type %s not found.", tmpc);
	    AeroFuel(mech) = AeroFuelOrig(mech) = DefaultFuelByType(mech);
	    break;
	case 2:		/* Movement Type */
	    tmpc = read_desc(fp, ptr);
	    MechMove(mech) = compare_array(move_types, tmpc);
	    TEMPLATE_GERR(MechMove(mech) == -1,
		"Error while loading: Type %s not found.", tmpc);
	    break;
	case 3:		/* Tons */
	    MechTons(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 4:		/* Tac_Range */
	    MechTacRange(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 5:		/* LRS_Range */
	    MechLRSRange(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 6:		/* Radio Range */
	    MechRadioRange(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 7:		/* Scan Range */
	    MechScanRange(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 8:		/* Heat Sinks */
	    MechRealNumsinks(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 9:		/* Max Speed */
	    SetMaxSpeed(mech, atof(read_desc(fp, ptr)));
	    break;
	case 10:		/* Specials */
	    tmpc = read_desc(fp, ptr);
	    MechSpecials(mech) |= BuildBitVector(specials, tmpc);
	    TEMPLATE_GERR(MechSpecials(mech) == -1,
		"Error while loading: Invalid specials - %s.", tmpc);
	    break;
	case 11:		/* Armor */
	    SetSectOArmor(mech, section, atoi(read_desc(fp, ptr)));
	    SetSectArmor(mech, section, GetSectOArmor(mech, section));
	    break;
	case 12:		/* Internals */
	    SetSectOInt(mech, section, atoi(read_desc(fp, ptr)));
	    SetSectInt(mech, section, GetSectOInt(mech, section));
	    break;
	case 13:		/* Rear */
	    SetSectORArmor(mech, section, atoi(read_desc(fp, ptr)));
	    SetSectRArmor(mech, section, GetSectORArmor(mech, section));
	    break;
	case 14:		/* Config */
	    tmpc = read_desc(fp, ptr);
	    MechSections(mech)[section].config =
		BuildBitVector(section_configs,
		tmpc) & ~(CASE_TECH | SECTION_DESTROYED);
	    TEMPLATE_GERR(MechSections(mech)[section].config == -1,
		"Error while loading: Invalid location config: %s.", tmpc);
	    break;
	case 25:		/* Specials2 */
            tmpc = read_desc(fp, ptr);
            MechSpecials2(mech) |= BuildBitVector(specials2, tmpc);
            TEMPLATE_GERR(MechSpecials(mech) == -1,
                "Error while loading: Invalid specials - %s.", tmpc);
            break;
	case 9999:
	    if ((sscanf(cmd, "CRIT_%d-%d", &x, &y)) == 2) {
		lpos = x - 1;
		hpos = y - 1;
	    } else if ((sscanf(cmd, "CRIT_%d", &x)) == 1) {
		lpos = x - 1;
		hpos = x - 1;
	    } else
		break;
	    critical = lpos;
	    line2 = read_desc(fp, ptr);
	    line2 = one_arg(line2, buf);
	    if (!strncasecmp(buf, "CL.", 3))
		isClan = 1;
	    TEMPLATE_GERR(!(find_matching_vlong_part(buf, NULL, &type)), "Unable to find %s", buf);
	    SetPartType(mech, section, critical, type);
	    if (IsWeapon(type)) {
		/* Thanks to legacy of past, we _do_ have to do this.. sniff */
		if (IsAMS(Weapon2I(type))) {
		    if (MechWeapons[Weapon2I(type)].special & CLAT)
			MechSpecials(mech) |= CL_ANTI_MISSILE_TECH;
		    else
			MechSpecials(mech) |= IS_ANTI_MISSILE_TECH;
		}
		SetPartData(mech, section, critical, 0);
		line2 = one_arg(line2, buf);	/* Don't need the '-' */
		line2 = one_arg(line2, buf);
		mod = BuildBitVector(crit_modes, buf);
		TEMPLATE_GERR(mod == -1,
		    "Error while loading: Invalid crit modes for object: %s.",
		    buf);
		GetPartMode(mech, section, critical) = mod;
		line2 = one_arg(line2, buf);
	    } else if (IsAmmo(type)) {
		line2 = one_arg(line2, buf);
		GetPartData(mech, section, critical) = atoi(buf);
		line2 = one_arg(line2, buf);
		mod = BuildBitVector(crit_modes, buf);
		TEMPLATE_GERR(mod == -1,
		    "Error while loading: Invalid crit modes for object: %s.",
		    buf);
		GetPartMode(mech, section, critical) = mod;
		if (GetPartData(mech, section, critical) < FullAmmo(mech,
			section, critical) && MechType(mech) != CLASS_BSUIT && MechType(mech) != CLASS_MW) {
		       GetPartMode(mech, section, critical) |= HALFTON_MODE;
		}
		if (GetPartData(mech, section, critical) > FullAmmo(mech, section, critical)
			&& MechType(mech) != CLASS_BSUIT && MechType(mech) != CLASS_MW) {
				GetPartMode(mech, section, critical) &= ~HALFTON_MODE;
		}
		if (GetPartData(mech, section, critical) != FullAmmo(mech,
			section, critical) && MechType(mech) != CLASS_MW &&
		    MechType(mech) != CLASS_BSUIT) {
		    SendError(tprintf("Mech #%d - Invalid ammo crit for %s in %s (%d/%d)", mech->mynum,
			    MechWeapons[Ammo2I(type)].name,
			    filename, GetPartData(mech, section, critical),
			    FullAmmo(mech, section, critical))
			);
		    SetPartData(mech, section, critical, FullAmmo(mech,
			    section, critical));
		}
	    } else {
		line2 = one_arg(line2, buf);
		if (IsDataSpecial(Special2I(type)))
		    GetPartData(mech, section, critical) = atoi(buf);
		else
		    GetPartData(mech, section, critical) = 0;
/*		GetPartMode(mech, section, critical) = 0; */
		if ((line2 = one_arg(line2, buf))) {
		    mod = BuildBitVector(crit_modes, buf);
                    TEMPLATE_GERR(mod == -1,
                        "Error while loading: Invalid crit modes for object: %s.",
                        buf);
                GetPartMode(mech, section, critical) = mod;
		}
                if ((line2 = one_arg(line2, buf))) {
			line2 = one_arg(line2, buf);
		    }
	    }
	    for (x = (lpos + 1); x <= hpos; x++) {
		SetPartType(mech, section, x, GetPartType(mech, section,
			lpos));
		SetPartData(mech, section, x, GetPartData(mech, section,
			lpos));
		SetPartMode(mech, section, x, GetPartMode(mech, section,
			lpos));
	    }
	    break;
	case 15:		/* Mech's Computer level */
	    MechComputer(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 16:		/* Name of the mech */
	    strncpy(MechType_Name(mech), read_desc(fp, ptr), MECHNAME_SIZE);
	    break;
	case 17:		/* Jj's */
	    MechJumpSpeed(mech) = atof(read_desc(fp, ptr));
	    break;
	case 18:		/* Radio */
	    MechRadio(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 19:		/* SI */
	    AeroSI(mech) = AeroSIOrig(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 20:		/* Fuel */
	    AeroFuel(mech) = AeroFuelOrig(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 21:		/* Comment */
	    break;
	case 22:		/* Radio_freqs */
	    MechRadioType(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 23:		/* Mech battle value */
	    MechBV(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 24:
	    CargoSpace(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 26:
	    PodSpace(mech) = atoi(read_desc(fp, ptr));
	    break;
	case 27:
	    CarMaxTon(mech) = atoi(read_desc(fp, ptr));
	    break;
	}
    }
    fclose(fp);
    MechEngineSizeV(mech) = MechEngineSizeC(mech);
#define Set(a,b) \
  if (!(a)) a = b
    Set(MechRealNumsinks(mech), (MechSpecials(mech) & ICE_TECH ? 0 : DEFAULT_HEATSINKS));
    if (MechType(mech) == CLASS_MECH)
	do_sub_magic(mech, 1);
    if (MechType(mech) == CLASS_MW)
	Startup(mech);

    if (MechType(mech) == CLASS_MECH)
	value = 8;
    else
	value = 6;

    if (isClan) {
	Set(MechComputer(mech), DEFAULT_CLCOMPUTER);
	Set(MechRadio(mech), DEFAULT_CLRADIO);
    } else {
	Set(MechComputer(mech), DEFAULT_COMPUTER);
	Set(MechRadio(mech), DEFAULT_RADIO);
    }
    if (!MechRadioType(mech))
	MechRadioType(mech) = generic_radio_type(MechRadio(mech), isClan);
    if (!MechComputer(mech)) {
	Set(MechScanRange(mech), DEFAULT_SCANRANGE);
	Set(MechLRSRange(mech), DEFAULT_LRSRANGE);
	Set(MechRadioRange(mech), DEFAULT_RADIORANGE);
	Set(MechTacRange(mech), DEFAULT_TACRANGE);
    } else {
	Set(MechScanRange(mech), MechComputersScanRange(mech));
	Set(MechLRSRange(mech), MechComputersLRSRange(mech));
	Set(MechRadioRange(mech), MechComputersRadioRange(mech));
	Set(MechTacRange(mech), MechComputersTacRange(mech));
    }
#if 1				/* Don't know if we're ready for this yet - aw, what the hell :) */
    if (MechType(mech) == CLASS_MECH)
	if ((GetPartType(mech, LARM, 2) != Special(LOWER_ACTUATOR)) &&
	    (GetPartType(mech, RARM, 2) != Special(LOWER_ACTUATOR)) &&
	    (GetPartType(mech, LARM, 3) != Special(HAND_OR_FOOT_ACTUATOR))
	    && (GetPartType(mech, RARM,
		    3) !=
		Special(HAND_OR_FOOT_ACTUATOR))) MechSpecials(mech) |=
		FLIPABLE_ARMS;
#endif
    update_specials(mech);
    mech_int_check(mech, 1);
    if ((x = mech_weight_sub(GOD, mech, 0)) > (y =
	    (MechTons(mech) * 1024))) 
	SendError(tprintf("Mech #%d - Error in %s template: %.1f tons of 'stuff', yet %d ton frame.", mech->mynum,
		MechType_Ref(mech), x / 1024.0, y / 1024));
	if (x < y)
		SendError(tprintf("Mech #%d - Error in %s template: %d ton frame, but only %.1f tons of equipment.",
		mech->mynum, MechType_Ref(mech), y / 1024, x / 1024.0));
    update_oweight(mech, x);
    MechBVLast(mech) = event_tick - 30;
    MechBV(mech) = CalculateBV(mech, 100, 100);
    if ((map = FindObjectsData(mech->mapindex)))
	UpdateConditions(mech, map);
    /* To prevent certain funny occurences.. */
    for (i = 0; i < NUM_SECTIONS; i++)
	if (!(GetSectOInt(mech, i)))
	    SetSectDestroyed(mech, i);
    return 0;
}

void DumpMechSpecialObjects(dbref player)
{
    coolmenu *c;

    c = AutoCol_StringMenu("MechSpecials available", internals);
    ShowCoolMenu(player, c);
    KillCoolMenu(c);
}

static char *dumpweapon_fun(int i)
{
    static char buf[256];

    buf[0] = 0;
    if (!i)
	sprintf(buf, WDUMP_MASKS);
    else {
	i--;
	sprintf(buf, WDUMP_MASK, MechWeapons[i].name, MechWeapons[i].heat,
	    MechWeapons[i].damage, MechWeapons[i].min,
	    MechWeapons[i].shortrange, MechWeapons[i].medrange,
	    GunRange(i, 0), MechWeapons[i].vrt, MechWeapons[i].criticals,
	    MechWeapons[i].ammoperton);
    }
    return buf;
}

void DumpWeapons(dbref player)
{
    coolmenu *c;

    c =
	SelCol_FunStringMenuK(1, "MechWeapons available", dumpweapon_fun,
	num_def_weapons + 1);
    ShowCoolMenu(player, c);
    KillCoolMenu(c);
}

char *techlist_func(MECH * mech)
{
    static char buffer[MBUF_SIZE];
    char bufa[SBUF_SIZE], bufb[SBUF_SIZE];
    int i, ii, part = 0, axe = 0, mace = 0, sword = 0, hascase = 0, tag = 0;

    snprintf(bufa, SBUF_SIZE, "%s", BuildBitString(specialsabrev, MechSpecials(mech)));
    snprintf(bufb, SBUF_SIZE, "%s", BuildBitString(specialsabrev2, MechSpecials2(mech)));
    snprintf(buffer, MBUF_SIZE, "%s %s", bufa, bufb);

    if (!(strstr(buffer, "XL") || strstr(buffer, "XXL") || strstr(buffer, "LENG") || strstr(buffer, "ICE") || strstr(buffer, "CENG")))
        strcat(buffer, " FUS");
    for (i = 0 ; i < NUM_SECTIONS; i++)
        for (ii = 0; ii < NUM_CRITICALS; ii++) {
	    part = GetPartType(mech, i, ii);
	    if (part == I2Special(TAG) && !tag) {
		tag = 1;
		strcat(buffer, "TAG");
		}
            if (part == I2Special(AXE) && !axe) {
                axe = 1;
                strcat(buffer, " AXE");
                } 
            if (part == I2Special(MACE) && !mace) {
                mace = 1;
                strcat(buffer, " MACE");
                } 
            if (part == I2Special(SWORD) && !sword) {
                sword = 1;
                strcat(buffer, " SWORD");
                }
            if ((MechSections(mech)[i].config & CASE_TECH) && !hascase) {
                hascase = 1;
                strcat(buffer, " CASE");
                }
            }

    if (CargoSpace(mech))
        strcat(buffer, " INFC");

    if (MechType(mech) == CLASS_VTOL)
        strcat(buffer, " VTOL");

    if (MechType(mech) == CLASS_MECH && MechMove(mech) != MOVE_QUAD) {
        if ((OkayCritSectS(RARM, 3, HAND_OR_FOOT_ACTUATOR) && OkayCritSectS(RARM, 0, SHOULDER_OR_HIP)) ||
	    (OkayCritSectS(LARM, 3, HAND_OR_FOOT_ACTUATOR) && OkayCritSectS(LARM, 0, SHOULDER_OR_HIP)) ||
	     MechSpecials(mech) & SALVAGE_TECH)
	    strcat(buffer, " MTOW");
        } else {
            if (MechSpecials(mech) & SALVAGE_TECH)
		strcat(buffer, " MTOW");
        }

    return buffer;
}
