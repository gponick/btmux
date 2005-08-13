/* This is the place for
   - loadcargo
   - unloadcargo
   - manifest
   - stores
 */

#include <stdio.h>
#include "mech.h"
#include "coolmenu.h"
#include "aero.bomb.h"
#include "math.h"
#include "mech.partnames.h"
#include "p.aero.bomb.h"
#include "p.econ.h"
#include "p.mech.partnames.h"
#include "p.crit.h"
#include "p.mech.status.h"
#include "p.mech.utils.h"

unsigned long long int specialcost[SPECIALCOST_SIZE] = { 0 };
unsigned long long int ammocost[AMMOCOST_SIZE] = { 0 };
unsigned long long int weapcost[WEAPCOST_SIZE] = { 0 };
unsigned long long int cargocost[CARGOCOST_SIZE] = { 0 };
unsigned long long int bombcost[BOMBCOST_SIZE] = { 0 };

float specialweight[] = {
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 256),
    ((float) 128),
    ((float) 3072),
    ((float) 512),
    ((float) 512),
    ((float) 1024),
    ((float) 1024),
    ((float) 512),
    ((float) 1024 / (float) 18),
    ((float) 1024 / (float) 16),
    ((float) 256),
    ((float) 1024),
    ((float) 512),
    ((float) 1024),
    ((float) 1024),
    ((float) 1536),
    ((float) 1024),
    ((float) 1536),
    ((float) 1024),
    ((float) 768),
    ((float) 1024),
    ((float) 512),
    ((float) 1024),
    ((float) 1024),
    ((float) 128),
    ((float) 1024),
    ((float) 1024),
    ((float) 1024),
    ((float) 1024),
    ((float) 128),
    ((float) 1024),
    ((float) 2048),
    ((float) 1536),
    ((float) 2048),
    ((float) 1024),
    ((float) 1024),
    ((float) 1024 / 16),
    -0.1 
};


float cargoweight[] = {
    ((float) 1024 / (float) 45),
    ((float) 1024 / (float) 20),
    ((float) 1024 / (float) 10),
    ((float) 1024 / (float) 5),
    ((float) 1024 / (float) 120),
    ((float) 1024 / (float) 100),
    ((float) 1024 / (float) 100),
    ((float) 1024 / (float) 120),
    ((float) 1024 / (float) 100),
    ((float) 1024 / (float) 100),
    ((float) 1024 / (float) 120),
    ((float) 1024 / (float) 100),
    ((float) 1024 / (float) 100),
    ((float) 1),
    ((float) 102),
    ((float) 102),
    ((float) 204),
    ((float) 12),
    ((float) 80),
    ((float) 10),
    ((float) 10),
    ((float) 306),
    ((float) 102),
    ((float) 204),
    ((float) 51),
    ((float) 8),
    ((float) 306),
    ((float) 204),
    ((float) 1024 / (float) 16),
    ((float) 1024 / (float) 18),
    ((float) 1024),
    ((float) 1024 / (float) 3),
    ((float) 256),
    ((float) 64),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 16),
    ((float) 1024 / (float) 128),
    ((float) 10),
    ((float) 20),
    ((float) 2),
    ((float) 1024 / (float) 100),
    ((float) 1024 / (float) 80),
    ((float) 1024 / (float) 90),
    ((float) 2048),
    ((float) 1536),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 4),
    ((float) 1024 / (float) 16), 
    ((float) 1024 / (float) 240),
    ((float) 1024 / (float) 12),
    ((float) 1024 / (float) 200),
    ((float) 1024 / (float) 200),
    ((float) 1024 / (float) 200),
    ((float) 1024 / (float) 120),
    ((float) 1024 / (float) 100),
    ((float) 1024 / (float) 5),
    ((float) 1024 / (float) 200),
    ((float) 1024 / (float) 10),
    ((float) 1024 / (float) 20),
    ((float) 1024 / (float) 45),
    ((float) 1024 / (float) 10),
    ((float) 1024 / (float) 20),
    ((float) 1024 / (float) 40),
    ((float) 1024 / (float) 90),
    ((float) 1024 / (float) 5),
    ((float) 1024 / (float) 10),
    ((float) 1024 / (float) 20),
    ((float) 1024 / (float) 45),
    ((float) 1024 / (float) 90),
    ((float) 1024 / (float) 160),
    ((float) 1024 / (float) 160),
    ((float) 768),
    ((float) 512),
    ((float) 512),
    ((float) 1024),
    ((float) 1024 / (float) 160),
    ((float) 1024 / (float) 22),
    ((float) 1024 / (float) 10),
    ((float) 1024 / (float) 5), 
    ((float) 1024 / (float) 2), 
    ((float) 1024 / (float) 180), 
    ((float) 1024 / (float) 60),
    ((float) 3072), /* Compact_HeatSink */
    ((float) 102), /* Uronium */
    ((float) 102), /* Klaxanite */
    ((float) 102), /* Ferdonite */
    ((float) 102), /* Lexan */
    ((float) 102), /* Uranium */
    ((float) 102), /* Jeranitium */
    ((float) 102), /* Maronite */
    ((float) 102), /* Copper */
    ((float) 102), /* Iron */
    ((float) 102), /* Titanium */
    ((float) 102), /* Platinum */
    ((float) 102), /* Silver */
    ((float) 102), /* Lumber */
    ((float) 102), /* Diamond */
    ((float) 102), /* Ruby */
    ((float) 102), /* Sapphire */
    ((float) 102), /* Gem */
    ((float) 102), /* Wood */
    ((float) 2), /* Water */
    ((float) 512), /* Marble */
    ((float) 1024), /* Machinery */
    ((float) 64), /* Men */
    ((float) 56), /* Women */
    ((float) 32), /* Children */
    ((float) 102), /* Food */
    ((float) 102), /* Furniture */
    ((float) 10), /* Crude_Oil */
    ((float) 102), /* Dirt */
    ((float) 204), /* Rock */
    ((float) 64), /* Fabric */
    ((float) 64), /* Clothing */
    ((float) 102), /* FerroCrete */
    ((float) 102), /* Kelvinium */
    ((float) 102), /* Coal */
    ((float) 102),
    ((float) 204),
    ((float) 306),
    ((float) 408),
    ((float) 510),
    ((float) 612),
    ((float) 714),
    ((float) 816),
    ((float) 918),
    ((float) 1020),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102),
    ((float) 102), 
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 102),
    ((float) 204),
    ((float) 306),
    ((float) 408),
    ((float) 510),
    ((float) 612),
    ((float) 714),
    ((float) 816),
    ((float) 918),
    ((float) 1020), 
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 1024 / (float) 128),
    ((float) 51),
    ((float) 102),
    ((float) 153),
    ((float) 204),
    ((float) 255),
    ((float) 306),
    ((float) 357),
    ((float) 408),
    ((float) 459),
    ((float) 510),
    ((float) 561),
    ((float) 612),
    ((float) 763),
    ((float) 814),
    ((float) 865),
    ((float) 916),
    ((float) 967),
    ((float) 1018),
    ((float) 1070),
    ((float) 1020), 
    ((float) 51),
    ((float) 102),
    ((float) 153),
    ((float) 204),
    ((float) 255),
    ((float) 306),
    ((float) 357),
    ((float) 408),
    ((float) 459),
    ((float) 510),
    ((float) 561),
    ((float) 612),
    ((float) 763),
    ((float) 814),
    ((float) 865),
    ((float) 916),
    ((float) 967),
    ((float) 1018),
    ((float) 1070),
    ((float) 1020), 
    ((float) 51),
    ((float) 102),
    ((float) 153),
    ((float) 204),
    ((float) 255),
    ((float) 306),
    ((float) 357),
    ((float) 408),
    ((float) 459),
    ((float) 510),
    ((float) 561),
    ((float) 612),
    ((float) 763),
    ((float) 814),
    ((float) 865),
    ((float) 916),
    ((float) 967),
    ((float) 1018),
    ((float) 1070),
    ((float) 1020), 
    ((float) 51),
    ((float) 102),
    ((float) 153),
    ((float) 204),
    ((float) 255),
    ((float) 306),
    ((float) 357),
    ((float) 408),
    ((float) 459),
    ((float) 510),
    ((float) 561),
    ((float) 612),
    ((float) 763),
    ((float) 814),
    ((float) 865),
    ((float) 916),
    ((float) 967),
    ((float) 1018),
    ((float) 1070),
    ((float) 1020), 
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024 / (float) 8),
    ((float) 1024),
    ((float) 2048),
    ((float) 3072),
    ((float) 4096),
    ((float) 1024),
    ((float) 2048),
    ((float) 3072),
    ((float) 4096),
    ((float) 1024),
    ((float) 2048),
    ((float) 3072),
    ((float) 4096),
    ((float) 1024),
    ((float) 2048),
    ((float) 3072),
    ((float) 4096),
    ((float) 51),
    ((float) 102),
    ((float) 153),
    ((float) 204),
    ((float) 255),
    ((float) 306),
    ((float) 357),
    ((float) 408),
    ((float) 459),
    ((float) 510),
    ((float) 561),
    ((float) 612),
    ((float) 763),
    ((float) 814),
    ((float) 865),
    ((float) 916),
    ((float) 967),
    ((float) 1018),
    ((float) 1070),
    ((float) 1020), 
    ((float) 51),
    ((float) 102),
    ((float) 153),
    ((float) 204),
    ((float) 255),
    ((float) 306),
    ((float) 357),
    ((float) 408),
    ((float) 459),
    ((float) 510),
    ((float) 561),
    ((float) 612),
    ((float) 763),
    ((float) 814),
    ((float) 865),
    ((float) 916),
    ((float) 967),
    ((float) 1018),
    ((float) 1070),
    ((float) 1020), 
    -0.1
};

int glob_parttype;
#define LISTTYPE_ALL	0
#define LISTTYPE_PART	1
#define LISTTYPE_WEAP	2
#define LISTTYPE_AMMO	3


/* Also sets the fuel we have ; but I digress */

void SetCargoWeight(MECH * mech)
{
    int pile[NUM_ITEMS];
    int weight = 0;		/* in 1/10 tons */
    float sw = 0.0;
    int i, j, k;
    char *t;
    int i1, i2, i3;

    t = silly_atr_get(mech->mynum, A_ECONPARTS);
    bzero(pile, sizeof(pile));
    while (*t) {
	if (*t == '[')
	    if ((sscanf(t, "[%d,%d,%d]", &i1, &i2, &i3)) == 3)
		pile[i1] += ((IsBomb(i1)) ? 4 : 1) * i3;
	t++;
    }
/*
    if (FlyingT(mech))
	for (i = 0; i < NUM_SECTIONS; i++)
	    for (j = 0; j < NUM_CRITICALS; j++) {
		if (IsBomb((k = GetPartType(mech, i, j))))
		    pile[k]++;
		else if (IsSpecial(k))
		    if (Special2I(k) == FUELTANK)
			pile[I2Special(FUELTANK)]++;
	    }
*/
    /* We've 'so-called' pile now */
    for (i = 0; i < NUM_ITEMS; i++)
	if (pile[i]) {
#if 0
	    if (IsWeapon(i))
		sw = ((float) ((float) ((float) 1024 / 10) * (float) MechWeapons[Weapon2I(i)].weight) / (float) MechWeapons[Weapon2I(i)].criticals);
	    else if (IsAmmo(i))
		sw = ((float) 1024 / (float) MechWeapons[Ammo2I(i)].ammoperton);
	    else if (IsBomb(i))
		sw = ((float) ((float) 1024 / 10) * (float) BombWeight(Bomb2I(i)));
	    else if (IsSpecial(i)) /* && i <= I2Special(LAMEQUIP) */
		sw = ((float) specialweight[Special2I(i)]);
	    else if (IsCargo(i))
		sw = ((float) cargoweight[Cargo2I(i)]);
	    else
		/* hmm.. tricky, suppose we'll make things light */
		sw = ((float) 1024 / 10);
#endif
	    sw = EconPartWeight(i, 0, NULL);
	    if (sw <= 0)
		sw = (1024 * 100);

	    weight += ((float) ((float) sw) * (float) pile[i]);
	}
    if (FlyingT(mech)) {
	AeroFuelMax(mech) =
	    AeroFuelOrig(mech) + 2000 * pile[I2Special(FUELTANK)];
	if (AeroFuel(mech) > AeroFuelOrig(mech))
	    weight += AeroFuel(mech) - AeroFuelOrig(mech);
    }
    SetCarriedCargo(mech, weight);
}

/* Returns 1 if calling function should return */

int loading_bay_whine(dbref player, dbref cargobay, MECH * mech)
{
    char *c;
    int i1, i2, i3 = 0;

    c = silly_atr_get(cargobay, A_MECHSKILLS);
    if (c && *c)
	if (sscanf(c, "%d %d %d", &i1, &i2, &i3) >= 2)
	    if (MechX(mech) != i1 || MechY(mech) != i2) {
		notify(player, "You're not where the cargo is!");
		if (i3)
		    notify(player,
			tprintf("Try looking around %d,%d instead.", i1,
			    i2));
		return 1;
	    }
    return 0;
}

void mech_Rfixstuff(dbref player, void *data, char *buffer)
{
    int loc = Location(player);
    int pile[NUM_ITEMS];
    char *t;
    int ol, nl, items = 0, kinds = 0;
    int i1, i2, id;

    if (!data)
	loc = atoi(buffer);
    bzero(pile, sizeof(pile));
    t = silly_atr_get(loc, A_ECONPARTS);
    ol = strlen(t);
    while (*t) {
	if (*t == '[')
	    if ((sscanf(t, "[%d,%d]", &i1, &i2)) == 2)
/*		if (!IsCrap(i1)) */ 
/* Removed IsCrap check that caused Endo/Ferro dissapearance - DJ 6/04/00 */
		    pile[i1] += i2;
	t++;
    }
    silly_atr_set(loc, A_ECONPARTS, "");
    for (id = 0; id < NUM_ITEMS; id++)
	if (pile[id] > 0 && get_parts_long_name(id)) {
	    econ_change_items(loc, id, pile[id]);
	    kinds++;
	    items += pile[id];
	    }
    t = silly_atr_get(loc, A_ECONPARTS);
    nl = strlen(t);
    notify(player,
	tprintf("Fixing done. Original length: %d. New length: %d.", ol,
	    nl));
    notify(player, tprintf("Items in new: %d. Unique items in new: %d.",
	    items, kinds));
}


/* HACK HACK HACK HACK */
int IsCargoAmmo(int id)
{
int cid;

if (!IsCargo(id))
    return 0;

cid = Cargo2I(id);

if ((cid > ARTEMIS_SSRM_AMMO && cid < SWARM_LRM_AMMO) || (cid > INFERNO_SRM_AMMO && cid < MRM_AMMO) || (cid > AMMO_ATMHE && cid < AMMO_ATM) || (cid > AMMO_LRM_STINGER))
    return 0; 

return 1;
}

void list_matching(dbref player, char *header, dbref loc, char *buf)
{
    int pile[NUM_ITEMS];
    int pile2[NUM_ITEMS];
    char *t, *ch;
    char buffer[1024];
    int i1, i2, id, filter;
    int x, i;
    float weight = 0.0;
    float sw = 0.0;
    coolmenu *c = NULL;
    int found = 0;

    bzero(pile, sizeof(pile));
    bzero(pile2, sizeof(pile2));
    CreateMenuEntry_Simple(&c, NULL, CM_ONE | CM_LINE);
    CreateMenuEntry_Simple(&c, header, CM_ONE | CM_CENTER);
    CreateMenuEntry_Simple(&c, NULL, CM_ONE | CM_LINE);
    /* Then, we go on a mad rampage ;-) */
    t = silly_atr_get(loc, A_ECONPARTS);
    while (*t) {
	if (*t == '[')
	    if ((sscanf(t, "[%d,%d]", &i1, &i2)) == 2)
		pile[i1] += i2;
	t++;
    }
    i = 0;
    if (buf)
	while (find_matching_long_part(buf, &i, &id))
	    pile2[id] = pile[id];
    for (i = 0; i < object_count; i++) { 
	UNPACK_PART(short_sorted[i]->index, id);
	if ((buf && (x = pile2[id])) || ((!buf && (x = pile[id])))) { 
#if 0
        if (IsWeapon(id))
            sw = ((float) ((float) ((float) 1024 / 10) * (float) MechWeapons[Weapon2I(id)].weight) / (float) MechWeapons[Weapon2I(id)].criticals);
        else if (IsAmmo(id))
            sw = ((float) ((float) 1024 / (float) MechWeapons[Ammo2I(id)].ammoperton));
        else if (IsBomb(id))
            sw = ((float)  ((float) 1024 / 10) * (float) BombWeight(Bomb2I(id)));
        else if (IsSpecial(id)) 
            sw = ((float) specialweight[Special2I(id)]);
        else if (IsCargo(id))
            sw = ((float) cargoweight[Cargo2I(id)]);
        else 
            sw = ((float) 1024 / 10);
#endif
	if (glob_parttype != LISTTYPE_ALL) {
	    filter = 1;
	    if (glob_parttype == LISTTYPE_AMMO)
		if (IsAmmo(id) || IsCargoAmmo(id))
		    filter = 0; 
	    if (glob_parttype == LISTTYPE_WEAP)
		if (IsWeapon(id))
		    filter = 0;
	    if (glob_parttype == LISTTYPE_PART)
		if ((IsCargo(id) && !IsCargoAmmo(id)) || IsSpecial(id) || IsActuator(id))
		    filter = 0;
	    if (filter)
		continue;
	    }

	sw = EconPartWeight(id, 0, NULL);
	if (sw <= 0)
	    sw = (1024 * 100);

        weight = ((float) ((float) sw) * (float) x); 
	memset(buffer, '\0', 1024);
	ch = get_parts_vlong_name(id);
	snprintf(buffer, sizeof(char) * 1024, "%s (%.2ft)", ch, (float) (weight / 1024));
        /* strcat(part_name_long(id, brand), tprintf(" (%.2ft)", (float) (weight / 1024))); */ 
        if (!ch) {
		SendError(tprintf("#%d in %d encountered odd thing: %d %d's.", player, loc, pile[id], id));
		continue;
	    }
	/* x = amount of things */
	CreateMenuEntry_Killer(&c, buffer, CM_TWO | CM_NUMBER | CM_NOTOG, 0, x, x);
	found++;
	}
    }
    if (!found)
	CreateMenuEntry_Simple(&c, "None", CM_ONE);
    CreateMenuEntry_Simple(&c, NULL, CM_ONE | CM_LINE);
    ShowCoolMenu(player, c);
    KillCoolMenu(c);
}

#define MY_DO_LIST(t) \
if (*buffer && glob_parttype == LISTTYPE_ALL) \
  list_matching(player, tprintf("Part listing for %s matching %s", Name(t), buffer), t, buffer); \
else \
  list_matching(player, tprintf("Part listing for %s", Name(t)), t, NULL)

#define SET_LISTTYPE(buffer) \
    if (strcmp(buffer, "weapons") == 0) glob_parttype = LISTTYPE_WEAP; \
    else if (strcmp(buffer, "ammo") == 0) glob_parttype = LISTTYPE_AMMO; \
    else if (strcmp(buffer, "parts") == 0) glob_parttype = LISTTYPE_PART; \
    else glob_parttype = LISTTYPE_ALL;

void mech_manifest(dbref player, void *data, char *buffer)
{
    while (isspace(*buffer))
	buffer++;
    SET_LISTTYPE(buffer)
    MY_DO_LIST(Location(player));
    glob_parttype = LISTTYPE_ALL;
}

void mech_stores(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_CONSISTENT);

    if (mech->mapindex > 1) {
	DOCHECK(Location(mech->mynum) != mech->mapindex || In_Character(Location(mech->mynum)), "You aren't inside hangar!");
    } else {
	DOCHECK(!Good_obj(Location(mech->mynum)) || !Hardcode(Location(mech->mynum)) || !getMech(Location(mech->mynum)), "Your not inside a carrier!");
    }

    if (loading_bay_whine(player, Location(mech->mynum), mech))
	return;
    while (isspace(*buffer))
	buffer++;
    SET_LISTTYPE(buffer)
    MY_DO_LIST(Location(mech->mynum));
    glob_parttype = LISTTYPE_ALL;
}

#ifdef ECON_ALLOW_MULTIPLE_LOAD_UNLOAD
#define silly_search(func) \
  if (!count) {  \
    i = -1 ; while (func(args[0], &i, &id)) \
      count++; \
    DOCHECK(argc<2, "Invalid number of arguments!"); \
    if (count > 0) sfun = func; }
#else
#define silly_search(func) \
  if (!count) {  \
    i = -1 ; while (func(args[0], &i, &id)) \
      count++; \
    DOCHECK(argc<2, "Invalid number of arguments!"); \
    DOCHECK(count > 1, "Too many matches!"); \
    if (count > 0) sfun = func; }
#endif

static void stuff_change_sub(dbref player, char *buffer, dbref loc1,
    dbref loc2, int mod, int mort)
{
    int i = -1, id;
    int count = 0;
    int argc;
    char *args[2];
    char *c;
    int num;
    int (*sfun) (char *, int *i, int *id) = NULL;
    int foo = 0;

    argc = mech_parseattributes(buffer, args, 2);
    DOCHECK(argc < 2, "Invalid number of arguments!");
    num = atoi(args[1]);
    DOCHECK(num <= 0, "Think I'm stupid, eh?");
    silly_search(find_matching_short_part);
    silly_search(find_matching_vlong_part);
    silly_search(find_matching_long_part);
    DOCHECK(count == 0, "Nothing matches that!");
    DOCHECK(!mort && count > 20 && player != GOD, tprintf (
	"Wizzes can't add more than 20 diff. objtypes at a time. ('%s' matches: %d)", args[0], count));
    if (mort) {
	DOCHECK(Location(player) != loc1, "You ain't in your 'mech!");
	DOCHECK(Location(loc1) != loc2, "You ain't in a hangar!");
    }
    i = -1;
#define MY_ECON_MODIFY(loc,num) \
      econ_change_items(loc, id, num); \
      SendEcon(tprintf("#%d %s %d %s %s #%d.", \
			    player, num>0 ? "added": "removed",  \
			    abs(num), (c=get_parts_long_name(id)), \
			    num>0 ? "to": "from", \
			    loc))
    while (sfun(args[0], &i, &id)) {
	if (mort) {
	    if (mod < 0)
		count = MIN(num, econ_find_items(loc1, id));
	    else
		count = MIN(num, econ_find_items(loc2, id));
	} else
	    count = num;
	foo += count;
	if (!count)
	    continue;
	MY_ECON_MODIFY(loc1, mod * count);
	if (count)
	    switch (mort) {
	    case 0:
		notify(player, tprintf("You %s %d %s%s.",
			mod > 0 ? "add" : "remove", count, c,
			count > 1 ? "s" : ""));
		break;
	    case 1:
		MY_ECON_MODIFY(loc2, (0 - mod) * count);
		notify(player, tprintf("You %s %d %s%s.",
			mod > 0 ? "load" : "unload", count, c,
			count > 1 ? "s" : ""));
		break;
	    }
    }
    DOCHECK(!foo, "Nothing matching that criteria was found!");
}

void mech_Raddstuff(dbref player, void *data, char *buffer)
{
    stuff_change_sub(player, buffer, Location(player), -1, 1, 0);
}

void mech_Rremovestuff(dbref player, void *data, char *buffer)
{
    stuff_change_sub(player, buffer, Location(player), -1, -1, 0);
}

void mech_loadcargo(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_CONSISTENT);
    DOCHECK(fabs(MechSpeed(mech)) > MP1, "You're moving too fast!");
    if (mech->mapindex > 1) {
	DOCHECK(Location(mech->mynum) != mech->mapindex || In_Character(Location(mech->mynum)), "You aren't inside hangar!");
    } else {
	DOCHECK(!Good_obj(Location(mech->mynum)) || !Hardcode(Location(mech->mynum)) || !getMech(Location(mech->mynum)), "Your not inside a carrier!");
    }
    DOCHECK(!(MechSpecials(mech) & CARGO_TECH),
	 "Your Toy aint prepared for what you're asking it!");
    if (loading_bay_whine(player, Location(mech->mynum), mech))
	return;
    stuff_change_sub(player, buffer, mech->mynum, mech->mapindex > 1 ? mech->mapindex : Location(mech->mynum), 1, 1);
    correct_speed(mech);
}

void mech_unloadcargo(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_CONSISTENT);
    stuff_change_sub(player, buffer, mech->mynum, mech->mapindex > 1 ? mech->mapindex : Location(mech->mynum), -1, 1);
    correct_speed(mech);
}

void mech_Rresetstuff(dbref player, void *data, char *buffer)
{
    dbref it;

    notify(player, "Inventory cleaned!");
    it = (isRoom(player) ? player : Location(player));
    silly_atr_set(it, A_ECONPARTS, "");
    SendEcon(tprintf("#%d reset #%d's stuff.", player, it));
}
