#include "mech.h"
#include "create.h"
#include "mech.events.h"
#include "p.mech.utils.h"
#include "p.mech.los.h"

#ifdef C3_SUPPORT

/* Rockin' C3 code of ours ;) */

static int **temp_c3_network = NULL;
static int *sorted_targets = NULL;
static MECH **temp_mechs;
static int temp_c3_size = 0;
static int temp_c3_target_count;

#define HAS_C3_SLAVE   1
#define HAS_C3_MASTER  2
#define FRIENDLY   3
#define UNFRIENDLY 4
#define UNSEEN_F   5
#define UNSEEN_U   6

#define MechSeesMech(map,i,j) \
        (map->LOSinfo[i][j] & (MECHLOSFLAG_SEESP|MECHLOSFLAG_SEESS))

/* All with 0 are potential targets ; if mechsOnMap
   is nice, and we figger we might want to get to know 'em, we
   mark 'em with 3/4 (potential target) */

#define C3MEMBER(j) \
        (temp_c3_network[j][j] && temp_c3_network[j][j] <= HAS_C3_MASTER)
#define C3PTARGET(j) \
        temp_c3_network[j][j]
#define C3TARGET(j) \
        (temp_c3_network[j][j] && temp_c3_network[j][j] < UNSEEN_F)

static void initialize_target_network(MECH * mech, MAP * map)
{
    int i, j;
    MECH *m;
    MECH *t;

    for (i = 0; i < map->first_free; i++) {
	if (temp_c3_network[i][i])
	    continue;
	if ((map->mechsOnMap[i]) <= 0)
	    continue;
	if (!(m = temp_mechs[i]))
	    continue;
	if (MechTeam(m) != MechTeam(mech))
	    temp_c3_network[i][i] = UNSEEN_U;
	else
	    temp_c3_network[i][i] = UNSEEN_F;
    }
    for (i = 0; i < map->first_free; i++)
	if (C3PTARGET(i) && i != mech->mapnumber) {
	    if (!(t = temp_mechs[i]))
		continue;
	    for (j = 0; j < map->first_free; j++)
		if (C3MEMBER(j)) {
		    if (!(m = temp_mechs[j]))
			continue;
		    if (MechSeesMech(map, j, i)) {
			temp_c3_network[j][i] = 1;
			if (temp_c3_network[i][i] >= UNSEEN_F)
			    temp_c3_network[i][i] -= 2;
		    }
		}
	}
}

#define Disturbed(mech)   ((MechStatus((mech)) & ECM_DISTURBANCE))
#define C3Destroyed(mech) (MechCritStatus((mech)) & C3_DESTROYED)

#define IsMaster(mech) \
(MechSpecials(mech) & C3_MASTER_TECH && !C3Destroyed(mech))

#define IsUMaster(mech) \
(IsMaster(mech) && !Disturbed(mech))

#define IsSlave(mech) \
(MechSpecials(mech) & C3_SLAVE_TECH && !C3Destroyed(mech))

#define IsUSlave(mech) \
(IsSlave(mech) && !Disturbed(mech))

#define IsUC3(mech) \
((MechSpecials(mech) & (C3_MASTER_TECH|C3_SLAVE_TECH)) && !C3Destroyed(mech) && !Disturbed(mech))

#define C3_STUFF(mech,a) \
temp_c3_network[a][a] = \
IsMaster(mech) ? 2 : IsSlave(mech) ? 1 : 0

static void initialize_slave_network(MECH * mech, MAP * map)
{
    int i;
    MECH *mas;

    if (!IsUMaster(mech))
	return;
    for (i = 0; i < map->first_free; i++)
	if (map->mechsOnMap[i] > 0 && map->mechsOnMap[i] != mech->mynum) {
	    if (temp_c3_network[i][i])
		continue;
	    if (!(mas = temp_mechs[i]))
		continue;
	    if (mas->mapindex != mech->mapindex)
		return;
	    if (!IsUC3(mas))
		continue;
	    if (MechC3Master(mas) != mech->mynum)
		continue;
	    if (MechTeam(mas) != MechTeam(mech))
		continue;
	    if (!Started(mas))
		continue;
	    C3_STUFF(mas, i);
	    initialize_slave_network(mas, map);
	}
}

static void initialize_master_network(MECH * mech, MAP * map)
{
    MECH *mas;
    int base;

    if (MechC3Master(mech) <= 0)
	return;
    if (!(mas = FindObjectsData(MechC3Master(mech))))
	return;
    if (mas->mapindex != mech->mapindex)
	return;
    base = mas->mapnumber;
    if (temp_c3_network[base][base])
	return;
    if (!IsUMaster(mas))
	return;
    if (!Started(mas))
	return;
    C3_STUFF(mas, mas->mapnumber);
    initialize_slave_network(mas, map);
    initialize_master_network(mas, map);
}

static void deinitialize_c3_networks()
{
    int i;

    for (i = 0; i < temp_c3_size; i++)
	free((void *) temp_c3_network[i]);
    free((void *) temp_c3_network);
    free((void *) sorted_targets);
    free((void *) temp_mechs);
    temp_c3_size = 0;
}

static void initialize_c3_networks(MECH * mech)
{
    int i;
    int base = mech->mapnumber;
    MAP *map = FindObjectsData(mech->mapindex);
    MECH *mek;

    if (!map)
	return;
    if (temp_c3_size)
	deinitialize_c3_networks();
    temp_c3_size = map->first_free;
    Create(temp_c3_network, int *, map->first_free);
    Create(sorted_targets, int, map->first_free);

    Create(temp_mechs, MECH *, map->first_free);
    for (i = 0; i < map->first_free; i++)
	Create(temp_c3_network[i], int, map->first_free);

    for (i = 0; i < map->first_free; i++)
	if (map->mechsOnMap[i] > 0)
	    if ((mek = getMech(map->mechsOnMap[i])))
		temp_mechs[i] = mek;
    C3_STUFF(mech, base);
    initialize_master_network(mech, map);
    initialize_slave_network(mech, map);
    initialize_target_network(mech, map);
}

static int sort_id_order(MECH * m1, MECH * m2)
{
    if (MechID(m1)[0] < MechID(m2)[0])
	return 1;
    if (MechID(m1)[0] > MechID(m2)[0])
	return -1;
    if (MechID(m1)[1] < MechID(m2)[1])
	return 1;
    if (MechID(m1)[1] > MechID(m2)[1])
	return -1;
    return 0;
}

static void sort_targets_by_id(MAP * map)
{
    MECH *m1, *m2;
    int i, j, k;
    int count = 0;

    for (i = 0; i < map->first_free; i++)
	if (C3TARGET(i))
	    sorted_targets[count++] = i;
    for (i = 0; i < (count - 1); i++) {
	m1 = temp_mechs[sorted_targets[i]];
	for (j = i + 1; j < count; j++) {
	    if (map->mechsOnMap[(k = sorted_targets[j])] <= 0)
		continue;
	    m2 = temp_mechs[sorted_targets[j]];
	    if (sort_id_order(m1, m2) < 0) {
		sorted_targets[j] = sorted_targets[i];
		sorted_targets[i] = k;
		m1 = temp_mechs[sorted_targets[i]];
	    }
	}
    }
    temp_c3_target_count = count;
}

static void c3_print_dbrefs(MECH * mech, MAP * map, char *buf, int odd)
{
    int i, j, k;
    MECH *mek;
    int count = 0;

    for (i = 0; i < temp_c3_target_count; i++) {
	j = sorted_targets[i];
	k = temp_c3_network[j][j];

	if (k > 0 && k < FRIENDLY) {
	    count++;
	    if (!(odd == count % 2))
		continue;
	    mek = temp_mechs[j];
	    if (k == HAS_C3_MASTER)
		strcat(buf, "%ch");
	    sprintf(buf + strlen(buf), "%s  ", MechIDS(mek, 1));
	    if (k == HAS_C3_MASTER)
		strcat(buf, "%cn");
	}
    }
}

static void c3_show_target(dbref player, MECH * mech, MECH * target)
{
    char buf[MBUF_SIZE];
    int tnum = target->mapnumber;
    int i, j, k;

    if (MechTeam(mech) == MechTeam(target))
	strcpy(buf, "");
    else {
	if (!Started(target))
	    strcpy(buf, "%ch%cy");
	else
	    strcpy(buf, "%ch%cr");
    }
    sprintf(buf + strlen(buf), "%s %-19s %3d %3d %3d %5.1f %3d %c%c%c%c ",
	MechIDS(target, MechTeam(target) == MechTeam(mech)),
	silly_atr_get(target->mynum, A_MECHNAME), MechX(target),
	MechY(target), MechZ(target), MechSpeed(target),
	MechFacing(target),
	MechSwarmTarget(target) >
	0 ? 'W' : Towed(target) ? 'T' : Fallen(target) ? 'F' :
	Standing(target) ? 'f' : ' ', Destroyed(target) ? 'D' : ' ',
	Jumping(target) ? 'J' : OODing(target) ? 'O' : ' ',
	Started(target) ? ' ' : Starting(target) ? 's' : 'S');
    /* Then, the sighting data. This is .. tricky */
    /* Basic idea: 'x x x   x'.. */
    for (i = 0; i < temp_c3_target_count; i++) {
	j = sorted_targets[i];
	k = temp_c3_network[j][j];
	if (k > 0 && k < FRIENDLY) {
	    if (!temp_c3_network[j][tnum])
		strcat(buf, "  ");
	    else {
		if (MechTeam(target) != MechTeam(mech)) {
		    if (j == mech->mapnumber)
			strcat(buf, "* ");
		    else
			strcat(buf, ". ");
		} else {
		    if (j == mech->mapnumber)
			strcat(buf, "X ");
		    else
			strcat(buf, "x ");
		}

	    }
	}
    }
    if (MechTeam(mech) != MechTeam(target))
	strcat(buf, "%cn");
    notify(player, buf);
}

void mech_c3_targets(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data, *tmpm;
    MAP *map;
    int i;
    char buf[MBUF_SIZE];
    char buf2[MBUF_SIZE];

    cch(MECH_USUAL);
    DOCHECK(!IsC3(mech), "You don't have C3 master/slave installed!");
    DOCHECK(Disturbed(mech), "Your C3 system seems to be inoperational.");
    map = getMap(mech->mapindex);
    initialize_c3_networks(mech);
    sort_targets_by_id(map);
    sprintf(buf,
	"%%cbC3 Target Report%%c %%ch%%cy________________________________%%c ");
    sprintf(buf2,
	"%%c%%cgID Name                X   Y   Z   S     H   S%%c      ");
    c3_print_dbrefs(mech, map, buf, 1);
    c3_print_dbrefs(mech, map, buf2, 0);
    notify(player, buf);
    notify(player, buf2);
    for (i = 0; i < temp_c3_target_count; i++)
	if ((tmpm = temp_mechs[sorted_targets[i]]) != mech)
	    c3_show_target(player, mech, tmpm);
    notify(player, "End of C3 Report");
    deinitialize_c3_networks();
}

static int count_c3_slaves(MECH * mech)
{
    MAP *map;
    int i, id;
    MECH *m;
    int count = 0;

    map = FindObjectsData(mech->mapindex);
    if (!map)
        if (IsC3i(mech))
	    return MAX_C3I_UNITS;
	else
	    return MAX_C3_SLAVES;
    for (i = 0; i < map->first_free; i++)
	if ((id = map->mechsOnMap[i]) > 0)
	    if ((m = FindObjectsData(id)))
		if (MechC3Master(m) == mech->mynum)
		    count++;
    return count;
}

void c3_send(MECH * mech, char *msg)
{
    int i;
    const char *c = GetMechID(mech);

    for (i = 0; i < temp_c3_size; i++)
	if (C3MEMBER(i))
	    mech_notify(temp_mechs[i], MECHSTARTED,
		tprintf("%%chC3/%s: %s%%c",
		    temp_mechs[i] == mech ? "You" : c, msg));
}

void mech_c3_message(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUAL);
    DOCHECK(!IsC3(mech), "You don't have C3 master/slave installed!");
    DOCHECK(Disturbed(mech), "Your C3 system seems to be inoperational.");
    initialize_c3_networks(mech);
    skipws(buffer);
    DOCHECK(!*buffer, "What do you want to send on the C3 Network?");
    c3_send(mech, buffer);
    deinitialize_c3_networks();
}

void mech_c3_set_master(dbref player, void *data, char *buffer)
{
    char *args[2];
    MECH *mech = (MECH *) data, *mas;
    dbref target;

    cch(MECH_USUAL);
    DOCHECK(!IsC3(mech), "You don't have C3 master/slave installed!");
    DOCHECK(Disturbed(mech), "Your C3 system seems to be inoperational.");
    DOCHECK(mech_parseattributes(buffer, args, 2) != 1,
	"Invalid number of arguments to function!");
    if (!strcmp(args[0], "-")) {
	if (MechC3Master(mech) > 0) {
	    MechC3Master(mech) = -1;
	}
	mech_notify(mech, MECHALL, "You disconnect from the C3 network.");
	return;
    }
    target = FindTargetDBREFFromMapNumber(mech, args[0]);
    DOCHECK(!(mas = getMech(target)), "Invalid C3 master unit selected!");
    DOCHECK(MechTeam(mas) != MechTeam(mech),
	"Error: You can't use people of other teams as C3 masters.");
    DOCHECK(mas == mech, "You happily network with yourself.");
    DOCHECK(count_c3_slaves(mas) >= MAX_C3_SLAVES && !(MechStatus2(mas) & NOC3LIMIT),
	"Error: Too many units contacted to the C3 Master unit.");
    DOCHECK(!IsMaster(mas),
	tprintf("%s isn't equipped with C3 Master unit!",
	    GetMechToMechID(mech, mas)));
    DOCHECK(Destroyed(mas), "The C3 Master is destroyed!");
    DOCHECK(!Started(mas), "The C3 Master is destroyed!");
    mech_notify(mech, MECHALL, tprintf("You connect to %s's data feed.",
	    GetMechToMechID(mech, mas)));
    mech_notify(mas, MECHALL, tprintf("%s connects to your data feed.",
	    GetMechToMechID(mas, mech)));
    MechC3Master(mech) = target;
}

float FindC3Range(MECH * mech, MECH * target, float range)
{
    float ra, r;
    int i;
    MECH *tempmech;
    MAP *map;
    
    map = getMap(mech->mapindex);
    if (!map)
	return range; 
    if (!IsUC3(mech))
	return range;
    if (!target)
	return range;
    if (MechStatus2(target) & ECM_PROTECTED)
	return range; 
    initialize_c3_networks(mech);
    ra = range;
    for (i = 0; i < temp_c3_size; i++)
	if (C3MEMBER(i))
		{
		tempmech = temp_mechs[i];
		if (!(map->LOSinfo[mech->mapnumber][tempmech->mapnumber] & MECHLOSFLAG_ECM))
		    if ((r = FaMechRange(tempmech, target)) < ra)
			if (InLineOfSight(tempmech, target, MechX(target), MechY(target), r))
			    ra = r;
		}
    deinitialize_c3_networks();
    return ra;
}

#endif
