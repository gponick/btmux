#include <stdio.h>
#include <string.h>

#include "mech.h"
#include "htab.h"
#include "extern.h"
#include "create.h"
#include "mech.partnames.h"

void list_hashstat(dbref player, const char *tab_name, HASHTAB * htab);

/* Main idea: 
   Keep 2 sorted tables, one of shortform -> index
   longform  -> index
   vlongform -> index
   Other
   index -> {short,long,vlong} form

   Index = ID + NUM_ITEMS * brand
 */

static PN *index_sorted[NUM_ITEMS];

/* Sorted: short -> index, long -> index */
PN **short_sorted = NULL;
PN **long_sorted = NULL;
PN **vlong_sorted = NULL;
int object_count;

static void insert_sorted_brandname(int ind, PN * e)
{
    int i, j;

#define UGLY_SORT(b,a) \
  for (i = 0 ; i < ind; i++) if (strcmp(e->a, b[i]->a)<0) break; \
    for (j = ind ; j > i ; j--) b[j] = b[j-1]; b[i] = e
    UGLY_SORT(short_sorted, shorty);
    UGLY_SORT(long_sorted, longy);
    UGLY_SORT(vlong_sorted, vlongy);
}

extern char *part_figure_out_name(int i);
extern char *part_figure_out_sname(int i);
extern char *part_figure_out_shname(int i);
extern char *my_shortform(char *);

static int create_brandname(int id)
{
    char buf[MBUF_SIZE];
    char *c;
    PN *p;

    Create(p, PN, 1);
#define SILLINESS(fun,val) \
  if (!(c=fun(id))) \
    { free ((void *) p->val); free((void *) p); return 0; } \
    strcpy(buf, c); \
  p->val = strdup(buf)
    SILLINESS(part_figure_out_name, vlongy);
    SILLINESS(part_figure_out_sname, longy);
    SILLINESS(part_figure_out_shname, shorty);
    PACK_PART(p->index, id);
    index_sorted[id] = p;
    return 1;
}

static HASHTAB short_hash, vlong_hash;

void list_phashstats(dbref player)
{
    list_hashstat(player, "Part:Short", &short_hash);
    list_hashstat(player, "Part:VLong", &vlong_hash);
}

void initialize_partname_tables()
{
    int i, c = 0, n;
    char tmpbuf[MBUF_SIZE];
    char *tmpc1, *tmpc2;

    bzero(index_sorted, sizeof(index_sorted));
    for (i = 0; i < NUM_ITEMS; i++)
	c += create_brandname(i);
    Create(short_sorted, PN *, c);
    Create(long_sorted, PN *, c);
    Create(vlong_sorted, PN *, c);
    /* bubble-sort 'em and insert to array */
    i = 0;
    for (n = 0; n < NUM_ITEMS; n++)
	if (index_sorted[n])
	    insert_sorted_brandname(i++, index_sorted[n]);
    hashinit(&short_hash, 20 * HASH_FACTOR);
    hashinit(&vlong_hash, 20 * HASH_FACTOR);
#define DASH(fromval,tohash) \
  for (tmpc1 = short_sorted[i]->fromval, tmpc2 = tmpbuf ; *tmpc1 ; tmpc1++, tmpc2++) \
    *tmpc2 = ToLower(*tmpc1); \
  *tmpc2 = 0; \
  hashadd(tmpbuf, (int *) (i+1), &tohash);
    for (i = 0; i < c; i++) {
	DASH(shorty, short_hash);
/*       DASH(longy, long_hash); */
	DASH(vlongy, vlong_hash);
    }
    object_count = c;
}

#define SILLY_GET(fun,value) \
char * fun (int i) { if (!(index_sorted[i])) \
{ SendError(tprintf("No partname index for %d", i)); return NULL; }\
return index_sorted[i]->value; }

SILLY_GET(get_parts_short_name, shorty);
SILLY_GET(get_parts_long_name, longy);
SILLY_GET(get_parts_vlong_name, vlongy);

#define wildcard_match quick_wild
extern int wildcard_match(char *, char *);

int find_matching_vlong_part(char *wc, int *ind, int *id)
{
    PN *p;
    char *tmpc1, *tmpc2;
    char tmpbuf[MBUF_SIZE];
    int *i;

    if (ind && *ind >= 0)
	return 0;
    for (tmpc1 = wc, tmpc2 = tmpbuf; *tmpc1; tmpc1++, tmpc2++)
	*tmpc2 = ToLower(*tmpc1);
    *tmpc2 = 0;
    if ((i = hashfind(tmpbuf, &vlong_hash)))
	if ((p = short_sorted[((int) i) - 1])) {
	    if (ind)
		*ind = ((int) i);
	    UNPACK_PART(p->index, *id);
	    return 1;
	}
    return 0;
}

int find_matching_long_part(char *wc, int *i, int *id)
{
    PN *p;

    for ((*i)++; *i < object_count; (*i)++)
	if (wildcard_match(wc, (p = long_sorted[*i])->longy)) {
	    UNPACK_PART(p->index, *id);
	    return 1;
	}
    return 0;
}

int find_matching_short_part(char *wc, int *ind, int *id)
{
    PN *p;
    char *tmpc1, *tmpc2;
    char tmpbuf[MBUF_SIZE];
    int *i;

    if (*ind >= 0)
	return 0;
    for (tmpc1 = wc, tmpc2 = tmpbuf; *tmpc1; tmpc1++, tmpc2++)
	*tmpc2 = ToLower(*tmpc1);
    *tmpc2 = 0;
    if ((i = hashfind(tmpbuf, &short_hash)))
	if ((p = short_sorted[((int) i) - 1])) {
	    *ind = ((int) i);
	    UNPACK_PART(p->index, *id);
	    return 1;
	}
    return 0;
}

void ListForms(dbref player, void *data, char *buffer)
{
    int i;

    notify(player, "Listing of forms:");
    for (i = 0; i < object_count; i++)
	notify(player, tprintf("%3d %-20s %-25s %s", i,
		short_sorted[i]->shorty, short_sorted[i]->longy,
		short_sorted[i]->vlongy));
}
