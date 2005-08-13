#include "mech.h"
#include "coolmenu.h"
#include "mycool.h"
#include "p.mech.custom.h"
#include "p.mech.utils.h"
#include "mech.partnames.h"
#include <math.h>

static char mech_loc_table[][2] = {
    {CTORSO, 1},
    {LTORSO, 2},
    {RTORSO, 2},
    {LARM, 3},
    {RARM, 3},
    {LLEG, 4},
    {RLEG, 4},
    {-1, 0}
};

static char quad_loc_table[][2] = {
    {CTORSO, 1},
    {LTORSO, 2},
    {RTORSO, 2},
    {LARM, 4},
    {RARM, 4},
    {LLEG, 4},
    {RLEG, 4},
    {-1, 0}
};

static char int_data[][5] = {
    {10, 4, 3, 1, 2},
    {15, 5, 4, 2, 3},
    {20, 6, 5, 3, 4},
    {25, 8, 6, 4, 6},
    {30, 10, 7, 5, 7},
    {35, 11, 8, 6, 8},
    {40, 12, 10, 6, 10},
    {45, 14, 11, 7, 11},
    {50, 16, 12, 8, 12},
    {55, 18, 13, 9, 13},
    {60, 20, 14, 10, 14},
    {65, 21, 15, 10, 15},
    {70, 22, 15, 11, 15},
    {75, 23, 16, 12, 16},
    {80, 25, 17, 13, 17},
    {85, 27, 18, 14, 18},
    {90, 29, 19, 15, 19},
    {95, 30, 20, 16, 20},
    {100, 31, 21, 17, 21},
    {-1, 0, 0, 0, 0}
};

static short engine_data[][2] = {
    {0, 0},
    {10, 1},
    {15, 1},
    {20, 1},
    {25, 1},
    {30, 2},
    {35, 2},
    {40, 2},
    {45, 2},
    {50, 3},
    {55, 3},
    {60, 3},
    {65, 4},
    {70, 4},
    {75, 4},
    {80, 5},
    {85, 5},
    {90, 6},
    {95, 6},
    {100, 6},
    {105, 7},
    {110, 7},
    {115, 8},
    {120, 8},
    {125, 8},
    {130, 9},
    {135, 9},
    {140, 10},
    {145, 10},
    {150, 11},
    {155, 11},
    {160, 12},
    {165, 12},
    {170, 12},
    {175, 14},
    {180, 14},
    {185, 15},
    {190, 15},
    {195, 16},
    {200, 17},
    {205, 17},
    {210, 18},
    {215, 19},
    {220, 20},
    {225, 20},
    {230, 21},
    {235, 22},
    {240, 23},
    {245, 24},
    {250, 25},
    {255, 26},
    {260, 27},
    {265, 28},
    {270, 29},
    {275, 31},
    {280, 32},
    {285, 33},
    {290, 35},
    {295, 36},
    {300, 38},
    {305, 39},
    {310, 41},
    {315, 43},
    {320, 45},
    {325, 47},
    {330, 49},
    {335, 51},
    {340, 54},
    {345, 57},
    {350, 59},
    {355, 63},
    {360, 66},
    {365, 69},
    {370, 73},
    {375, 77},
    {380, 82},
    {385, 87},
    {390, 92},
    {395, 98},
    {400, 105},
    {405, 113},
    {410, 122},
    {415, 133},
    {420, 145},
    {425, 159},
    {430, 72 * 2 + 1},
    {435, 97 * 2},
    {440, 107 * 2 + 1},
    {445, 119 * 2 + 1},
    {450, 133 * 2 + 1},
    {455, 150 * 2},
    {460, 168 * 2 + 1},
    {465, 190 * 2},
    {470, 214 * 2 + 1},
    {475, 243 * 2 + 1},
    {480, 275 * 2 + 1},
    {485, 313 * 2},
    {490, 356 * 2},
    {495, 405 * 2 + 1},
    {500, 462 * 2 + 1},
    {-1, 0}
};

int susp_factor(MECH * mech)
{
    int t = MechTons(mech);

    if (MechMove(mech) == MOVE_TRACK)
	return 0;
    if (MechMove(mech) == MOVE_WHEEL)
	return 20;
#define MAP(a,b) if (t <= a) return b
    if (MechMove(mech) == MOVE_FOIL) {
	MAP(10, 60);
	MAP(20, 105);
	MAP(30, 150);
	MAP(40, 195);
	MAP(50, 255);
	MAP(60, 300);
	MAP(70, 345);
	MAP(80, 390);
	MAP(90, 435);
	return 480;
    }
    if (MechMove(mech) == MOVE_HOVER) {
	MAP(10, 40);
	MAP(20, 85);
	MAP(30, 130);
	MAP(40, 175);
	return 235;
    }
    if (MechMove(mech) == MOVE_HULL || MechMove(mech) == MOVE_SUB)
	return 30;
    if (MechMove(mech) == MOVE_VTOL) {
	MAP(10, 50);
	MAP(20, 95);
	return 140;
    }
    return 0;
}


int engine_weight(MECH * mech)
{
    int s = MechEngineSize(mech);
    int i, e = 0;

    if (MechType(mech) != CLASS_MECH && !is_aero(mech))
	s -= susp_factor(mech);
    else if (MechType(mech) == CLASS_AERO)
	s = (int) ((WalkingSpeed(MechMaxSpeed(mech)) / KPH_PER_MP) - 2) * MechTons(mech);
    if (s > 400 && MechType(mech) == CLASS_MECH) {
	for (i = 0; i < NUM_CRITICALS; i++)
	    if (IsSpecial(GetPartType(mech, CTORSO, i)) && Special2I(GetPartType(mech, CTORSO, i)) == ENGINE)
		e++;
	if (e <= 6)
	    SendError(tprintf("Mech #%d - Large Engine (rating +400) but %d engine crits!", mech->mynum, e));
	}
    if (MechType(mech) == CLASS_DS || MechType(mech) == CLASS_SPHEROID_DS) {
	return (int) ((0.065 * (WalkingSpeed(MechMaxSpeed(mech)) / KPH_PER_MP)) * (MechTons(mech) * 1024));
	}

    for (i = 0; engine_data[i][0] >= 0; i++)
	if (s == engine_data[i][0]) {

/* Engine ICE Fixes for non mecha go in here */
 	    if (MechType(mech) != CLASS_MECH && !is_aero(mech)) {
	   	return (int) engine_data[i][1] * 512 * (MechSpecials(mech) & ICE_TECH ? 2 : 1.5 );
		}
	    return engine_data[i][1] * 512 * (MechSpecials(mech) & ICE_TECH ? 2 : 1);
	}
    SendError(tprintf("Mech #%d - No engine found!", mech->mynum));
    return 0;
}

#define ROUND(from,size) \
                         (((int) (from) + size - 1) / size)
#define ROUNDM(from,size) (ROUND(from,size) * size)

static void calc_ints(MECH * mech, int *n, int *tot)
{
    int i;

    *n = 0;
    *tot = 0;
    if (is_aero(mech)) {
	*n = AeroSIOrig(mech);
	*tot = AeroSIOrig(mech);
    } else {
	for (i = 0; i < NUM_SECTIONS; i++) {
	    *n += GetSectInt(mech, i);
	    *tot += GetSectOInt(mech, i);
	}
	*tot = MAX(1, *tot);
    } 
}

static int ammo_weight(MECH * mech)
{
    int i, j, t, w = 0;

    for (i = 0; i < NUM_SECTIONS; i++)
	if (!SectIsDestroyed(mech, i))
	    for (j = 0; j < CritsInLoc(mech, i); j++)
		if (IsAmmo((t = GetPartType(mech, i, j))))
		    w +=
/*			GetPartData(mech, i, j) * 1024 / MechWeapons[Ammo2I(GetPartType(mech, i, j))].ammoperton; */
                        (GetPartData(mech, i, j) / FullAmmo(mech, i, j)) * 1024;
    return w;
}

#define MyGetSectOArmor(m,l) (interactive>=0?GetSectOArmor(m,l):GetSectArmor(m,l))
#define MyGetSectORArmor(m,l) (interactive>=0?GetSectORArmor(m,l):GetSectRArmor(m,l))
#define PLOC(a) if (interactive >= 0 || !SectIsDestroyed(mech,a))
#define MyMechNumOsinks(m) ((interactive >= 0) ? (MechNumOsinks(m)) : (MechRealNumsinks(m)))
int mech_weight_sub_mech(dbref player, MECH * mech, int interactive)
{
    int pile[NUM_ITEMS_M];
    int i, j, w, cl, id;
    int armor = 0, armor_o;
    int total = 0;
    coolmenu *c = NULL;
    int shs_size;
    int hs_eff;
    char buf[MBUF_SIZE];
    int ints_c, ints_tot;
    int temp, temp2, temp3;

    bzero(pile, sizeof(pile));
    if (interactive > 0) {
	addline();
	cent(tprintf("Weight totals for %s", GetMechID(mech)));
	addline();
    }
    calc_ints(mech, &ints_c, &ints_tot);
    for (i = 0; i < NUM_SECTIONS; i++) {
	armor += MyGetSectOArmor(mech, i);
	armor += MyGetSectORArmor(mech, i);
	PLOC(i)
	    for (j = 0; j < NUM_CRITICALS; j++)
	    if (interactive >= 0 || !IsAmmo(GetPartType(mech, i, j)))
		pile[GetPartType(mech,i,j)] += AmmoMod(mech, i, j);
    }
    shs_size =
	MechSpecials(mech) & CLAN_TECH ? 2 : MechSpecials(mech) &
	DOUBLE_HEAT_TECH ? 3 : 1;
    hs_eff = (MechSpecials(mech) & (DOUBLE_HEAT_TECH|CLAN_TECH) ? 2 : MechSpecials2(mech) & COMPACT_HEAT_TECH ? 2 : 1);
    cl = MechSpecials(mech) & CLAN_TECH;
#define ADDENTRY(text,weight) \
  if (weight) { if (interactive>0) { addmenu(text);addmenu(tprintf("      %6.2f(%d)", (float) (weight) / 1024.0, weight));}; total += weight; }
#define ADDENTRY_C(text,count,weight) \
  if (weight) { if (interactive>0) { addmenu(text);addmenu(tprintf("%5d %6.2f(%d)", count, (float) (weight) / 1024.0, weight));}; total += weight; }
    sprintf(buf, "%-12s(%d rating)",
	MechSpecials(mech) & XL_TECH ? "Engine (XL)" : MechSpecials(mech) &
	XXL_TECH ? "Engine (XXL)" : MechSpecials(mech) & CE_TECH ?
	"Engine (Compact)" : MechSpecials2(mech) & LENG_TECH ? "Engine (Light)" : "Engine", MechEngineSize(mech));

 #define engine_mod(mech, wt) \
  (int) (wt * (MechSpecials(mech) & CE_TECH ? 1.5 : \
         MechSpecials2(mech) & LENG_TECH ? .75 : \
         MechSpecials(mech) & XXL_TECH ? .33333 : \
         MechSpecials(mech) & XL_TECH ? .5 : 1))

    PLOC(CTORSO)
    ADDENTRY(buf, ROUNDM(engine_mod(mech, engine_weight(mech)),256));
/*	ADDENTRY(buf, engine_mod(mech, engine_weight(mech)) + ((temp = (engine_mod(mech, engine_weight(mech)) % 512)) ? (-temp + 512) : 0));*/
    PLOC(HEAD)
	ADDENTRY("Cockpit", (MechSpecials2(mech) & TORSOCOCKPIT_TECH ? 4 * 1024 :
		MechSpecials2(mech) & SMALLCOCKPIT_TECH ? 2 * 1024 : 3 * 1024));
/*    PLOC(CTORSO)
        ((int) ((int) MechEngineSize(mech) + 50) / 100) * 1024); */
    if (CargoSpace(mech))
	ADDENTRY(tprintf("CargoSpace (%.2ft)", (float) CargoSpace(mech) / 100),
	    (int) (((float) CargoSpace(mech) / (MechSpecials2(mech) & CARRIER_TECH ? 1000 : MechSpecials(mech) & CARGO_TECH ? 100 : 500)) * 1024));
    if (MechSpecials2(mech) & OMNI_TECH)
	ADDENTRY("PodSpace",
            (int) (((float) PodSpace(mech) / 100) * 1024));
    PLOC(CTORSO)
/*	ADDENTRY(tprintf("%s",(MechSpecials2(mech) & XLGYRO_TECH ? "Gyro (XL)" :
	  MechSpecials2(mech) & CGYRO_TECH ? "Gyro (Compact)" : MechSpecials2(mech) & HDGYRO_TECH ?
	  "Gyro (HD)" : "Gyro")), ((MechEngineSize(mech) <= 400 ?
	((int) ((int) ((int) MechEngineSize(mech) + (MechEngineSize(mech) % 50 ? (-(MechEngineSize(mech) % 50) + 50) : 0)) / 50) * 512) : 5 * 1024) *
	(MechSpecials2(mech) & XLGYRO_TECH ? .5 : MechSpecials2(mech) & CGYRO_TECH ? 1.5 :
	 MechSpecials2(mech) & HDGYRO_TECH ? 2 : 1))); */
	ADDENTRY(tprintf("%s",(MechSpecials2(mech) & XLGYRO_TECH ? "Gyro (XL)" :
	  MechSpecials2(mech) & CGYRO_TECH ? "Gyro (Compact)" : MechSpecials2(mech) & HDGYRO_TECH ?
	  "Gyro (HD)" : "Gyro")),
	  ROUNDM((MechEngineSize(mech) / 5 * 512 * (MechSpecials2(mech) & XLGYRO_TECH ? 0.5 :
	  MechSpecials2(mech) & CGYRO_TECH ? 1.5 : MechSpecials2(mech) & HDGYRO_TECH ? 2 : 1) / 10), 256));
/*	  ((temp2 = (temp = (((MechEngineSize(mech) <= 400 ?
	(((MechEngineSize(mech) + ((MechEngineSize(mech) % (temp3 = (mudconf.btech_faccount ?
	50 : 100))) ? -(MechEngineSize(mech)
        % temp3) + temp3 : 0)) / temp3) * (temp3 == 50 ? 512 : 1024)) : 5 * 1024)) *
	(MechSpecials2(mech) & XLGYRO_TECH ? .5 : MechSpecials2(mech) & CGYRO_TECH ? 1.5 :
	 MechSpecials2(mech) & HDGYRO_TECH ? 2 : 1))) % (mudconf.btech_faccount ? 512 : 1024))
        ? temp + (-temp2 + (mudconf.btech_faccount ? 512 : 1024)) : temp));*/

    ADDENTRY(MechSpecials(mech) & REINFI_TECH ? "Internals (Reinforced)" :
	MechSpecials(mech) & COMPI_TECH ? "Internals (Composite)" :
	MechSpecials(mech) & ES_TECH ? "Internals (ES)" : "Internals",
		ROUNDM(MechTons(mech) * 1024 * (interactive >= 0 ? ints_tot : ints_c) /
		(MechSpecials(mech) & REINFI_TECH ? 5 : MechSpecials(mech) & (ES_TECH|COMPI_TECH) ? 20 : 10) /
		ints_tot, 256));

/*	(temp = (MechTons(mech) * 1024 * (interactive >=
	    0 ? ints_tot : ints_c) / 5 / ints_tot /
	(MechSpecials(mech) & REINFI_TECH ? 1 : (MechSpecials(mech) &
		(ES_TECH | COMPI_TECH)) ? 4 : 2))) +
	(mudconf.btech_faccount ? 0 : (temp % 512 ? (-(temp % 512) + 512) : 0)));*/
    armor_o = armor;
    if (MechSpecials(mech) & FF_TECH)
	armor = armor * 50 / (cl ? 60 : 56);
    if (MechSpecials2(mech) & LFF_TECH)
	armor = armor * 50 / 53;
    if (MechSpecials2(mech) & HFF_TECH)
	armor = armor * 50 / 62;
    ADDENTRY_C(MechSpecials(mech) & HARDA_TECH ? "Armor (Hardened)" :
	       MechSpecials(mech) & FF_TECH ? "Armor (FF)" :
	       MechSpecials2(mech) & LFF_TECH ? "Armor (LFF)" :
	       MechSpecials2(mech) & HFF_TECH ? "Armor (HFF)" :
	       MechSpecials2(mech) & STEALTHARMOR_TECH ? "Armor (Stealth)" :  "Armor", armor_o,
	       ROUNDM(armor * 64 * (MechSpecials(mech) & HARDA_TECH ? 2 : 1), 1));
    if (MyMechNumOsinks(mech)) {
	pile[Special(HEAT_SINK)] =
	    MAX(0,
	    MyMechNumOsinks(mech) * shs_size / hs_eff - (MechSpecials(mech) & ICE_TECH ? 0 : 10) * shs_size);
    } else if (interactive > 0)
	cent(tprintf("WARNING: HS count may be off, due to certain odd things."));
    for (i = 1; i < NUM_ITEMS_M; i++)
	if (pile[i]) {
	    if (IsWeapon(i)) {
		id = Weapon2I(i);
		ADDENTRY_C(MechWeapons[id].name,
		    pile[i] / GetWeaponCrits(mech, id), ROUNDM(crit_weight(mech,
			i) * pile[i], 256));
	    } else {
		if ((w = crit_weight(mech, i)))
		    ADDENTRY_C(get_parts_long_name(i), pile[i],
			ROUNDM(w * pile[i], 256));
	    }
	}
    if (interactive > 0) {
	addline();
	vsi(tprintf("%%cgTotal: %s%.2f(%d) tons (offset: %.2f)%%cn",
		(total / 1024) > MechTons(mech) ? "%ch%cr" : "",
		(float) (total) / 1024.0, total,
		MechTons(mech) - (float) (total) / 1024.0));
	addline();
	ShowCoolMenu(player, c);
    }
    KillCoolMenu(c);
    if (interactive < 0)
	total += ammo_weight(mech);
    return MAX(1, total);
}

static int tank_in_pieces(MECH * mech)
{
    int i;

    for (i = 0; i < NUM_SECTIONS; i++)
	if (GetSectInt(mech, i))
	    return 0;
    return 1;
}

int mech_weight_sub_veh(dbref player, MECH * mech, int interactive)
{
    int pile[NUM_ITEMS_M];
    int i, j, w, cl, id, t;
    int armor = 0, armor_o;
    int total = 0;
    coolmenu *c = NULL;
    int shs_size;
    int hs_eff;
    char buf[MBUF_SIZE];
    int es;
    int turr_stuff = 0;
    int ints_c, ints_tot;
    int temp, temp2;
    int pamp = 0;

    bzero(pile, sizeof(pile));
    calc_ints(mech, &ints_c, &ints_tot);
    if (interactive > 0) {
	addline();
	cent(tprintf("Weight totals for %s", GetMechID(mech)));
	addline();
    }
    for (i = 0; i < NUM_SECTIONS; i++) {
	armor += MyGetSectOArmor(mech, i);
	armor += MyGetSectORArmor(mech, i);
	for (j = 0; j < CritsInLoc(mech, i); j++) {
	    if (!(t = GetPartType(mech, i, j)))
		continue;
	    if (interactive >= 0 || !SectIsDestroyed(mech, i)) {
		if (interactive >= 0 || !IsAmmo(t))
		    pile[t] += AmmoMod(mech, i, j);
		if (i == TURRET && (MechType(mech) == CLASS_VEH_GROUND ||
			MechType(mech) == CLASS_VEH_NAVAL))
		    if (IsWeapon(t))
			turr_stuff += crit_weight(mech, t);
	    if (IsWeapon(t) && IsEnergy(t))
	    	pamp += crit_weight(mech, t);
	    }
	}
    }
    shs_size =
	MechSpecials(mech) & CLAN_TECH ? 2 : MechSpecials(mech) &
	DOUBLE_HEAT_TECH ? 3 : 1;
    hs_eff = MechSpecials(mech) & (DOUBLE_HEAT_TECH | CLAN_TECH) ? 2 :
      MechSpecials2(mech) & COMPACT_HEAT_TECH ? 2 : 1;
    cl = MechSpecials(mech) & CLAN_TECH;
    es = susp_factor(mech);
    if (es)
	sprintf(buf, "%-12s(%d->%d eff/wt rat)",
	    MechSpecials(mech) & CE_TECH ? "Engine (Compact)" :
	    MechSpecials(mech) & XXL_TECH ? "Engine (XXL)" :
	    MechSpecials(mech) & XL_TECH ? "Engine (XL)" :
	    MechSpecials(mech) & ICE_TECH ? "Engine (ICE)" :
	    MechSpecials2(mech) & LENG_TECH ? "Engine (Light)" : "Engine",
	    MechEngineSize(mech),
	    MechEngineSize(mech) - susp_factor(mech));
    else
	sprintf(buf, "%-12s(%d rating)",
	    MechSpecials(mech) & CE_TECH ? "Engine (Compact)" :
	    MechSpecials(mech) & XXL_TECH ? "Engine (XXL)" :
	    MechSpecials(mech) & XL_TECH ? "Engine (XL)" :
	    MechSpecials(mech) & ICE_TECH ? "Engine (ICE)" :
	    MechSpecials2(mech) & LENG_TECH ? "Engine (Light)" : "Engine",
	    MechEngineSize(mech));
    if (!tank_in_pieces(mech)) {
/*	ADDENTRY(buf, (es = (engine_mod(mech, engine_weight(mech)) + ((temp = (engine_mod(mech, engine_weight(mech)) % 512)) ? (-temp + 512) : 0))));*/
	ADDENTRY(buf, ROUNDM(engine_mod(mech,engine_weight(mech)), 256));
/*	if (MechMove(mech) == MOVE_HOVER &&                                              */
/*	    es <                                                                         */
/*	    (MechTons(mech) * 1024 /							 */
/* 5)) ADDENTRY("Engine size fix (-> 1/5 hover wt.)", MechTons(mech) * 1024 / 5 - es);   */
/* Removed for variation and lack of screwing dynspeed code - DJ - 5/16/00 		 */
	ADDENTRY("Cockpit", ROUNDM(MechTons(mech) * 1024 / 20, 256));
	if (MechType(mech) == CLASS_VEH_VTOL ||
	    MechMove(mech) == MOVE_HOVER || MechMove(mech) == MOVE_SUB)
	    ADDENTRY("LiftEquipment",
		ROUNDM(MechTons(mech) * 1024 / 10, 256));
    }
    if (CargoSpace(mech))
        ADDENTRY(tprintf("CargoSpace (%.2ft)", (float) CargoSpace(mech) / 100),
            (int) (((float) CargoSpace(mech) / (MechSpecials2(mech) & CARRIER_TECH ? 1000 : MechSpecials(mech) & CARGO_TECH ? 100 : 500)) * 1024));
    if (MechSpecials2(mech) & OMNI_TECH)
        ADDENTRY("PodSpace",
            (int) (((float) PodSpace(mech) / 100) * 1024));
    PLOC(TURRET)
	if (turr_stuff)
	ADDENTRY("Turret", ROUNDM(turr_stuff / 10, 256));
	if (pamp && (MechSpecials(mech) & ICE_TECH))
	ADDENTRY("PowerAmps", ROUNDM(pamp / 10, 256));
    ADDENTRY(MechSpecials(mech) & REINFI_TECH ? "Internals (Reinforced)" :
	MechSpecials(mech) & COMPI_TECH ? "Internals (Composite)" :
	MechSpecials(mech) & ES_TECH ? "Internals (ES)" : "Internals",
		ROUNDM(MechTons(mech) * 1024 * (interactive >= 0 ? ints_tot : ints_c) /
		(MechSpecials(mech) & REINFI_TECH ? 5 : MechSpecials(mech) & (ES_TECH|COMPI_TECH) ? 20 : 10) /
		ints_tot, 256));
/*        (temp = (MechTons(mech) * 1024 * (interactive >=
            0 ? ints_tot : ints_c) / 5 / ints_tot /
        (MechSpecials(mech) & REINFI_TECH ? 1 : (MechSpecials(mech) &
                (ES_TECH | COMPI_TECH)) ? 4 : 2))) +
	(mudconf.btech_faccount ? 0 : (temp % 512 ? (-(temp % 512) + 512) : 0)));*/

    armor_o = armor;
    if (MechSpecials(mech) & FF_TECH)
	armor = armor * 50 / (cl ? 60 : 56);
    if (MechSpecials2(mech) & LFF_TECH) armor = armor * 50 / 53;
    if (MechSpecials2(mech) & HFF_TECH) armor = armor * 50 / 62;
    ADDENTRY_C(MechSpecials(mech) & HARDA_TECH ? "Armor (Hardened)" :
	       MechSpecials(mech) & FF_TECH ? "Armor (FF)" :
	       MechSpecials2(mech) & LFF_TECH ? "Armor (LFF)" :
	       MechSpecials2(mech) & HFF_TECH ? "Armor (HFF)" :
	       MechSpecials2(mech) & STEALTHARMOR_TECH ? "Armor (Stealth)" : "Armor", armor_o,
	       ROUNDM(armor * 64 * (MechSpecials(mech) & HARDA_TECH ? 2 : 1), 1));
    pile[Special(HEAT_SINK)] =
	MAX(0, MechRealNumsinks(mech) * shs_size / hs_eff - (MechSpecials(mech) & ICE_TECH ? 0 : 10) * shs_size);
    for (i = 1; i < NUM_ITEMS_M; i++)
	if (pile[i]) {
	    if (IsWeapon(i)) {
		id = Weapon2I(i);
		ADDENTRY_C(MechWeapons[id].name,
		    pile[i] / GetWeaponCrits(mech, id), crit_weight(mech,
			i) * pile[i]);
	    } else if ((w = crit_weight(mech, i)))
		ADDENTRY_C(get_parts_long_name(i), pile[i],
		    w * pile[i]);
	}
    if (interactive > 0) {
	addline();
	vsi(tprintf("%%cgTotal: %s%.2f(%d) tons (offset: %.2f)%%cn",
		(total / 1024) > MechTons(mech) ? "%ch%cr" : "",
		(float) (total) / 1024.0, total,
		MechTons(mech) - (float) (total) / 1024.0));
	addline();
	ShowCoolMenu(player, c);
    }
    KillCoolMenu(c);
    if (interactive < 0)
	total += ammo_weight(mech);
    return MAX(1, total);
}

int fuel_per_ton(MECH * mech)
{
int type = MechType(mech);
int ton = MechTons(mech);

if (type == CLASS_AERO)
    return 80;
if (ton < 399)
    return 80;
else if (ton < 799)
    return 70;
else if (ton < 1199)
    return 60;
else if (ton < 1899)
    return 50;
else if (ton < 2999)
    return 40;
else if (ton < 19990)
    return 30;
else if (ton < 39999)
    return 20;
else
    return 10;
}

int mech_weight_sub_aero(dbref player, MECH * mech, int interactive)
{
    int pile[NUM_ITEMS_M];
    int i, j, w, cl, id, t;
    int armor = 0, armor_o;
    int total = 0;
    coolmenu *c = NULL;
    int shs_size;
    int hs_eff;
    char buf[MBUF_SIZE];
    int es;
    int turr_stuff = 0;
    int ints_c, ints_tot;
    int temp, temp2;
    int pamp = 0;
    int free_hs = 0;

    bzero(pile, sizeof(pile));
    calc_ints(mech, &ints_c, &ints_tot);
    if (interactive > 0) {
        addline();
        cent(tprintf("Weight totals for %s", GetMechID(mech)));
        addline();
    }
    for (i = 0; i < NUM_SECTIONS; i++) {
        armor += MyGetSectOArmor(mech, i);
        for (j = 0; j < CritsInLoc(mech, i); j++) {
            if (!(t = GetPartType(mech, i, j)))
                continue;
            if (interactive >= 0 || !SectIsDestroyed(mech, i)) {
                if (interactive >= 0 || !IsAmmo(t))
                    pile[t] += AmmoMod(mech, i, j);
		if (IsWeapon(t) && IsEnergy(t))
		    pamp += crit_weight(mech, t);
            }
        }
    }
    shs_size = MechSpecials(mech) & CLAN_TECH ? 2 : MechSpecials(mech) & DOUBLE_HEAT_TECH ? 3 : 1;
    hs_eff = MechSpecials(mech) & (DOUBLE_HEAT_TECH | CLAN_TECH) ? 2 : MechSpecials2(mech) & COMPACT_HEAT_TECH ? 2 : 1;
    cl = MechSpecials(mech) & CLAN_TECH;
    sprintf(buf, "%-12s(%d rating)",
        MechSpecials(mech) & CE_TECH ? "Engine (Compact)" :
        MechSpecials(mech) & XXL_TECH ? "Engine (XXL)" :
        MechSpecials(mech) & XL_TECH ? "Engine (XL)" :
        MechSpecials(mech) & ICE_TECH ? "Engine (ICE)" :
        MechSpecials2(mech) & LENG_TECH ? "Engine (Light)" : "Engine",
	(int) ((WalkingSpeed(MechMaxSpeed(mech)) / KPH_PER_MP) - 2) * MechTons(mech));
    ADDENTRY(buf, ROUNDM(engine_mod(mech,engine_weight(mech)), 256));
    ADDENTRY("Control Components", ROUNDM(MechType(mech) == CLASS_AERO ? 3072 : (MechTons(mech) * 1024) * 0.0075, 256));
    ADDENTRY("Structural Integrity", ROUNDM(MechType(mech) == CLASS_AERO ? 0 : MechType(mech) == CLASS_DS ?
	(AeroSI(mech) * (MechTons(mech) * 1024)) / 200 : (AeroSI(mech) * (MechTons(mech) * 1024)) / 500, 256));
    ADDENTRY("Fuel", ROUNDM((AeroFuel(mech) / fuel_per_ton(mech)) * 1024, 256));
    if (MechType(mech) == CLASS_DS || MechType(mech) == CLASS_SPHEROID_DS) {
	int crew = 0, weaps = 0, cnt, critical[MAX_WEAPS_SECTION];
	unsigned char weaparray[MAX_WEAPS_SECTION];
	unsigned char weapdata[MAX_WEAPS_SECTION];
	for (cnt = 0; cnt < NUM_SECTIONS; cnt++)
	    weaps += FindWeapons(mech, cnt, weaparray, weapdata, critical);
	crew = 4 + (MechTons(mech) / 5000) + (weaps / 6);
	ADDENTRY("Crew Quarters", 8704 * crew);
	ADDENTRY("Spare Parts", ROUNDM((MechTons(mech) * 1024) * 0.01, 256)); 
	}

#define AeroFreeSinks(a) (MechType(a) == CLASS_AERO ? 10 : MechType(a) == CLASS_DS ? (engine_mod(a, engine_weight(a)) / 1024) / 20 : SpheroidDS(a) ? \
	(int) sqrt(abs((engine_mod(a, engine_weight(a)) / 1024) * 6.8)) : 0)
//    ADDENTRY("Heatsinks", abs(MechRealNumsinks(mech) - AeroFreeSinks(mech)) * 1024);

    if (CargoSpace(mech))
        ADDENTRY(tprintf("CargoSpace (%.2ft)", (float) CargoSpace(mech) / 100),
            (int) (((float) CargoSpace(mech) / (MechSpecials2(mech) & CARRIER_TECH ? 1000 : MechSpecials(mech) & CARGO_TECH ? 100 : 500)) * 1024));

    armor_o = armor;
    if (MechSpecials(mech) & FF_TECH) armor = armor * 50 / (cl ? 60 : 56);
    if (MechSpecials2(mech) & LFF_TECH) armor = armor * 50 / 53;
    if (MechSpecials2(mech) & HFF_TECH) armor = armor * 50 / 62;
    ADDENTRY_C(MechSpecials(mech) & HARDA_TECH ? "Armor (Hardened)" :
               MechSpecials(mech) & FF_TECH ? "Armor (FF)" :
               MechSpecials2(mech) & LFF_TECH ? "Armor (LFF)" :
               MechSpecials2(mech) & HFF_TECH ? "Armor (HFF)" :
               MechSpecials2(mech) & STEALTHARMOR_TECH ? "Armor (Stealth)" : "Armor", armor_o,
               ROUNDM(armor * 64 * (MechSpecials(mech) & HARDA_TECH ? 2 : 1), 1));
//    free_hs = (MechType(mech) == CLASS_AERO ? 10 : MechType(mech) == CLASS_DS ? engine_weight(mech) / 20 : (int) ((double) sqrt((double) engine_weight(mech)) * (double) 6.8));
    free_hs = AeroFreeSinks(mech);
    pile[Special(HEAT_SINK)] = MAX(0, MechRealNumsinks(mech) * shs_size / hs_eff - (MechSpecials(mech) & ICE_TECH ? 0 : free_hs) * shs_size);
    for (i = 1; i < NUM_ITEMS_M; i++)
        if (pile[i]) {
            if (IsWeapon(i)) {
                id = Weapon2I(i);
                ADDENTRY_C(MechWeapons[id].name, pile[i] / GetWeaponCrits(mech, id), crit_weight(mech, i) * pile[i]);
	    } else if (IsBomb(i)) {
		continue;
            } else if ((w = crit_weight(mech, i)))
                ADDENTRY_C(get_parts_long_name(i), pile[i], w * pile[i]);
        }
    if (interactive > 0) {
        addline();
        vsi(tprintf("%%cgTotal: %s%.2f(%d) tons (offset: %.2f)%%cn",
                (total / 1024) > MechTons(mech) ? "%ch%cr" : "",
                (float) (total) / 1024.0, total,
                MechTons(mech) - (float) (total) / 1024.0));
        addline();
        ShowCoolMenu(player, c);
    }
    KillCoolMenu(c);
    if (interactive < 0)
        total += ammo_weight(mech);
    return MAX(1, total);
}

/* Returns: 1024 * MechWeight(in tons) */
int mech_weight_sub(dbref player, MECH * mech, int interactive)
{
    if (MechType(mech) == CLASS_MECH)
	return mech_weight_sub_mech(player, mech, interactive);
    if (MechType(mech) == CLASS_VEH_GROUND ||
	MechType(mech) == CLASS_VEH_VTOL ||
	MechType(mech) ==
	CLASS_VEH_NAVAL)
	return mech_weight_sub_veh(player, mech, interactive);
    if (MechType(mech) == CLASS_AERO || MechType(mech) == CLASS_DS || MechType(mech) == CLASS_SPHEROID_DS)
	return mech_weight_sub_aero(player, mech, interactive);
    if (interactive > 0)
	notify(player, "Invalid vehicle type!");
	if (MechType(mech) == CLASS_BSUIT || MechType(mech) == CLASS_MW)
		return MechTons(mech) * 1024;
    return 1;
}

void mech_weight(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    mech_weight_sub(player, mech, 1);
}

#define Table(i,j) \
(MechMove(mech) == MOVE_QUAD ? quad_loc_table[i][j] : mech_loc_table[i][j])

static int real_int(MECH * mech, int loc, int ti)
{
    int i;

    if (loc == HEAD)
	return 3;
    for (i = 0; Table(i, 0) >= 0; i++)
	if (loc == Table(i, 0))
	    break;
    if (Table(i, 0) < 0)
	return 0;
    return int_data[ti][Table(i, 1)];
}

#define tank_int(mech) \
((MechTons(mech) + 5) / 10)

void vehicle_int_check(MECH * mech, int noisy)
{
    int i, j;

    j = tank_int(mech);
    for (i = 0; i < NUM_SECTIONS; i++)
	if (GetSectOInt(mech, i) && GetSectOInt(mech, i) != j) {
	    if (noisy)
		SendError(tprintf("Mech #%d - Invalid internals in loc %d (should be %d, are %d)", mech->mynum, i, j, GetSectOInt(mech, i)));
	    SetSectOInt(mech, i, j);
	    SetSectInt(mech, i, j);
	}
}

void mech_int_check(MECH * mech, int noisy)
{
    int i, j, k;

    if (MechType(mech) != CLASS_MECH) {
	if (MechType(mech) == CLASS_VEH_GROUND ||
	    MechType(mech) == CLASS_VEH_VTOL ||
	    MechType(mech) == CLASS_VEH_NAVAL) vehicle_int_check(mech,
		noisy);
	return;
    }
    for (i = 0; int_data[i][0] >= 0; i++)
	if (MechTons(mech) == int_data[i][0])
	    break;
    if (int_data[i][0] < 0) {
	if (noisy)
	    SendError(tprintf("Mech #%d - VERY odd tonnage at %d.", mech->mynum,
		    MechTons(mech)));
	return;
    }
    k = i;
    for (i = 0; i < NUM_SECTIONS; i++) {
	if (GetSectOInt(mech, i) != (j = real_int(mech, i, k))) {
	    if (noisy)
		SendError(tprintf("Mech #%d - Invalid internals in loc %d (should be %d, are %d)",
			mech->mynum, i, j,
			GetSectOInt(mech, i)));
	    SetSectOInt(mech, i, j);
	    SetSectInt(mech, i, j);
	}
    }
}
