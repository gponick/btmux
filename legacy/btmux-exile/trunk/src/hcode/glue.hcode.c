#include "db.h"
#include "externs.h"
#ifdef BT_ENABLED
#include "mech.h"
#include "macros.h"
#include "p.glue.h"
#endif

void set_attr_internal(dbref player, dbref thing, int attrnum,
    char *attrtext, int key);


int bt_get_attr(char *tbuf, int obj, char *name)
{
    ATTR *a;
    int ao, af;

    if (!name)
	return 0;
    if (!(a = atr_str(name)))
	return 0;
    atr_get_str(tbuf, obj, a->number, &ao, &af);
    if (*tbuf)
	return 1;
    return 0;
}

char *silly_atr_get(int id, int flag)
{
    int i, j;
    static char buf[LBUF_SIZE];

    atr_get_str(buf, id, flag, &i, &j);
    return buf;
#if 0				/* This would waste memory, so.. :P */
    return atr_pget(id, flag, &i, &j);
#endif
}

void silly_atr_set(int id, int flag, char *dat)
{
    atr_add_raw(id, flag, dat);
}

void bt_set_attr(dbref obj, char *attri, char *value)
{
    int attr;

    ATTR *atr;

    atr = atr_str(attri);
    attr = atr ? atr->number : mkattr(attri);
    set_attr_internal(GOD, obj, attr, value, SET_QUIET);
}

void KillText(char **mapt)
{
    int i;

    for (i = 0; mapt[i]; i++)
	free(mapt[i]);
    free(mapt);
}

void ShowText(char **mapt, dbref player)
{
    int i;

    for (i = 0; mapt[i]; i++)
	notify(player, mapt[i]);
}


int BOUNDED(int min, int val, int max)
{
    if (val < min)
	return min;
    if (val > max)
	return max;
    return val;
}

int MAX(int v1, int v2)
{
    if (v1 > v2)
	return v1;
    return v2;
}

int MIN(int v1, int v2)
{
    if (v1 < v2)
	return v1;
    return v2;
}

float FBOUNDED(float min, float val, float max)
{
    if (val < min)
	return min;
    if (val > max)
	return max;
    return val;
}

float FMAX(float v1, float v2)
{
    if (v1 > v2)
	return v1;
    return v2;
}

float FMIN(float v1, float v2)
{
    if (v1 < v2)
	return v1;
    return v2;
}

int silly_parseattributes(char *buffer, char **args, int max)
{
    char bufferi[LBUF_SIZE], foobuff[LBUF_SIZE];
    char *a, *b;
    int count = 0;
    char *parsed = buffer;
    int num_args = 0;

    memset(args, 0, sizeof(char *) * max);

    b = bufferi;
    for (a = buffer; *a && a; a++)
	if (*a == '=') {
	    *(b++) = ' ';
	    *(b++) = '=';
	    *(b++) = ' ';
	} else
	    *(b++) = *a;
    *b = 0;
    /* Got da silly string in bufferi variable */

    while ((count < max) && parsed) {
	if (!count) {
	    /* first time through */
	    parsed = strtok(bufferi, " \t");
	} else {
	    parsed = strtok(NULL, " \t");
	}
	args[count] = parsed;	/* Set the args pointer */
	if (parsed)
	    num_args++;		/* Actual count of arguments */
	count++;		/* Loop to make sure we don't overrun our */
	/* buffer */
    }
    /* Hrm. Now all we gotta do is append -rest- of data to end of _last_ arg */
    if (args[max - 1] && args[max - 1][0]) {
	strcpy(foobuff, args[max - 1]);
	while ((parsed = strtok(NULL, " \t")))
	    sprintf(foobuff + strlen(foobuff), " %s", parsed);
	args[max - 1] = foobuff;
    }
    return num_args;
}

int mech_parseattributes(char *buffer, char **args, int maxargs)
{
    int count = 0;
    char *parsed = buffer;
    int num_args = 0;

    memset(args, 0, sizeof(char *) * maxargs);

    while ((count < maxargs) && parsed) {
	parsed = strtok(!count ? buffer : NULL, " \t");
	args[count] = parsed;	/* Set the args pointer */
	if (parsed)
	    num_args++;		/* Actual count of arguments */
	count++;		/* Loop to make sure we don't overrun our */
	/* buffer */
    }
    return num_args;
}
