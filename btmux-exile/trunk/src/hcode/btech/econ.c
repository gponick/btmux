/* Idea:
   Store the parts in an attribute on object

   Format:
   [id,brand,count]{,[id,brand,count],..}
 */

#include "mech.h"

extern char *silly_atr_get(int id, int flag);
extern void silly_atr_set(int id, int flag, char *dat);

/* entry = pointer to [ */
static void remove_entry(char *alku, char *entry)
{
    char *j;

    if (!(j = strstr(entry, "]")))
	return;
    j++;
    if (*j)
	strcpy(entry, j + 1);
    else {
	if (entry == alku)
	    strcpy(alku, "");
	else
	    strcpy(entry - 1, "");
    }
}


static void add_entry(char *to, char *data)
{
    if (*to)
	sprintf(to + strlen(to), ",[%s]", data);
    else
	sprintf(to + strlen(to), "[%s]", data);
}

static char *find_entry(char *s, int i)
{
    char buf[MBUF_SIZE];

    sprintf(buf, "[%d,", i);
    return strstr(s, buf);
}

extern char *get_parts_short_name(int, int);

void econ_change_items(dbref d, int id, int num)
{
    char *t, *u;
    int base = 0, i1, i2;

    if (!Good_obj(d))
	return;
    t = silly_atr_get(d, A_ECONPARTS);
/*    SendDebug(tprintf("ECONPARTS : %s", t)); */
    if ((u = find_entry(t, id))) {
	if (sscanf(u, "[%d,%d]", &i1, &i2) == 2)
	    base += i2;
	remove_entry(t, u);
    }
    base += num;
    if (base <= 0) {
	if (u)
	    silly_atr_set(d, A_ECONPARTS, t);
/*	SendDebug(tprintf("econ_change return : base %d num %d id %d brand %d", base, num, id, brand)); */
	return;
    }
    if (!(IsActuator(id)))
	add_entry(t, tprintf("%d,%d", id, base));
    silly_atr_set(d, A_ECONPARTS, t);
    if (IsActuator(id))
	econ_change_items(d, Cargo(S_ACTUATOR), base);
/*    SendDebug("Success?"); */
    /* Successfully changed */
}

int econ_find_items(dbref d, int id)
{
    char *t, *u;
    int i1, i2;

    if (!Good_obj(d))
	return 0;
    t = silly_atr_get(d, A_ECONPARTS);
    if ((u = find_entry(t, id)))
	if (sscanf(u, "[%d,%d]", &i1, &i2) == 2)
	    return i2;
    return 0;
}

void econ_set_items(dbref d, int id, int num)
{
    int i;

    if (!Good_obj(d))
	return;
    i = econ_find_items(d, id);
    if (i != num)
	econ_change_items(d, id, num - i);
}
