
/*
 * commac.c 
 */


#include "copyright.h"
#include "autoconf.h"
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "db.h"
#include "interface.h"
#include "match.h"
#include "config.h"
#include "externs.h"

#include "commac.h"
#include "p.comsys.h"

extern void load_comsystem(FILE *);
extern void load_macros(FILE *);
extern void save_comsystem(FILE *);
extern void save_macros(FILE *);

static void load_old_comsys_and_macros(char *filename)
{
    FILE *fp;
    int i;
    char buffer[400];

    for (i = 0; i < NUM_COMMAC; i++)
	commac_table[i] = NULL;

    if (!(fp = fopen(filename, "r"))) {
	fprintf(stderr, "Error: Couldn't find %s.\n", filename);
	return;
    } else {
	fprintf(stderr, "LOADING: %s\n", filename);
	if (fscanf(fp, "*** Begin %s ***\n", buffer) == 1 &&
	    !strcmp(buffer, "COMMAC")) {
	    load_commac(fp);
	} else {
	    fprintf(stderr, "Error: Couldn't find Begin COMMAC in %s.",
		filename);
	    return;
	}

	if (fscanf(fp, "*** Begin %s ***\n", buffer) == 1 &&
	    !strcmp(buffer, "COMSYS")) {
	    load_comsystem(fp);
	} else {
	    fprintf(stderr, "Error: Couldn't find Begin COMSYS in %s.",
		filename);
	    return;
	}

	if (fscanf(fp, "*** Begin %s ***\n", buffer) == 1 &&
	    !strcmp(buffer, "MACRO")) {
	    load_macros(fp);
	} else {
	    fprintf(stderr, "Error: Couldn't find Begin MACRO in %s.",
		filename);
	    return;
	}

	fclose(fp);
	fprintf(stderr, "LOADING: %s (done)\n", filename);
    }
}

static void load_new_comsys_and_macros(char *prefix)
{
    FILE *fp;
    int i;
    char buffer[400];
    char file1[400];
    char file2[400];
    char file3[400];

    sprintf(file1, "%s.users.db", prefix);
    sprintf(file2, "%s.channels.db", prefix);
    sprintf(file3, "%s.macros.db", prefix);

    for (i = 0; i < NUM_COMMAC; i++)
        commac_table[i] = NULL;

    if (!(fp = fopen(file1, "r"))) {
        fprintf(stderr, "Error: Couldn't find %s.\n", file1);
    } else {
        fprintf(stderr, "LOADING: %s\n", file1);
        if (fscanf(fp, "*** Begin %s ***\n", buffer) == 1 &&
            !strcmp(buffer, "COMMAC")) {
            load_commac(fp);
        } else {
            fprintf(stderr, "Error: Couldn't find Begin COMMAC in %s.",
                file1);
        }
    }
    if (!feof(fp) && fgets(buffer, 400, fp))
        fprintf(stderr, "Garbage at end of file %s ignored\n", file1);
    fclose(fp);
    if (!(fp = fopen(file2, "r"))) {
        fprintf(stderr, "Error: Couldn't find %s.\n", file2);
    } else {
        fprintf(stderr, "LOADING: %s\n", file2);
        if (fscanf(fp, "*** Begin %s ***\n", buffer) == 1 &&
            !strcmp(buffer, "COMSYS")) {
            load_comsystem(fp);
        } else {
            fprintf(stderr, "Error: Couldn't find Begin COMSYS in %s.",
                file2);
        }
    }
    if (!feof(fp) && fgets(buffer, 400, fp))
        fprintf(stderr, "Garbage at end of file %s ignored\n", file2);
    fclose(fp);
    if (!(fp = fopen(file3, "r"))) {
        fprintf(stderr, "Error: Couldn't find %s.\n", file3);
    } else {
        fprintf(stderr, "LOADING: %s\n", file3);
        if (fscanf(fp, "*** Begin %s ***\n", buffer) == 1 &&
            !strcmp(buffer, "MACRO")) {
            load_macros(fp);
        } else {
            fprintf(stderr, "Error: Couldn't find Begin MACRO in %s.",
                file3);
        }
    }
    if (!feof(fp) && fgets(buffer, 400, fp))
        fprintf(stderr, "Garbage at end of file %s ignored\n", file3);
    fclose(fp);
    fprintf(stderr, "LOADING: %s (done)\n", prefix);
}

void load_comsys_and_macros(void)
{
    struct stat sb;
    char fname[500];

    sprintf(fname, "%s.users.db", mudconf.comsys_dbname);
    if (stat(fname, &sb) == 0 && sb.st_size)
        load_new_comsys_and_macros(mudconf.comsys_dbname);
    else {
        fprintf(stderr, "New comsys db not found, loading old format.\n");
        load_old_comsys_and_macros(mudconf.commac_db);
    }
}

void save_comsys_and_macros(char *templ)
{
    FILE *fp1, *fp2, *fp3;
    char file1[400], temp1[400];
    char file2[400], temp2[400];
    char file3[400], temp3[400];

    sprintf(file1, "%s.users.db", templ);
    sprintf(file2, "%s.channels.db", templ);
    sprintf(file3, "%s.macros.db", templ);

    sprintf(temp1, "%s.#%d#", file1, mudstate.epoch);
    sprintf(temp2, "%s.#%d#", file2, mudstate.epoch);
    sprintf(temp3, "%s.#%d#", file3, mudstate.epoch);

    if (!(fp1 = fopen(temp1, "w"))) {
        fprintf(stderr, "Unable to open %s for writing.\n", temp1);
        return;
    }
    if (!(fp2 = fopen(temp2, "w"))) {
        fprintf(stderr, "Unable to open %s for writing.\n", temp2);
        return;
    }
    if (!(fp3 = fopen(temp3, "w"))) {
        fprintf(stderr, "Unable to open %s for writing.\n", temp3);
        return;
    }

    fprintf(fp1, "*** Begin COMMAC ***\n");
    save_commac(fp1);

    fprintf(fp2, "*** Begin COMSYS ***\n");
    save_comsystem(fp2);

    fprintf(fp3, "*** Begin MACRO ***\n");
    save_macros(fp3);

    /* fclose() returns 0 on success, and we can't use '||' or '&&' here
     * because we want all fclose()'es to be processed even if an earlier
     * one failed. errno might be clobbered if one fails but the others
     * don't, but too bad.
     */
    if (fclose(fp1) + fclose(fp2) + fclose(fp3)) {
        send_channel("Wizard", tprintf("Comsystem dump failed: %s",
                strerror(errno)));
        unlink(temp1);
        unlink(temp2);
        unlink(temp3);
        return;
    }

    rename(temp1, file1);
    rename(temp2, file2);
    rename(temp3, file3);
}

void load_commac(FILE * fp)
{
    int i, j;
    char buffer[500];
    int np;
    struct commac *c;
    char *t;
    char in;

    fscanf(fp, "%d\n", &np);
    for (i = 0; i < np; i++) {
	c = create_new_commac();
	fscanf(fp, "%d %d %d %d %d %d %d %d\n", &(c->who),
	    &(c->numchannels), &(c->macros[0]), &(c->macros[1]),
	    &c->macros[2], &(c->macros[3]), &(c->macros[4]), &(c->curmac));
	c->maxchannels = c->numchannels;
	if (c->maxchannels > 0) {
	    c->alias = (char *) malloc(c->maxchannels * 6);
	    c->channels = 
		(char **) malloc(sizeof(char *) * c->maxchannels);

	    for (j = 0; j < c->numchannels; j++) {
		t = c->alias + j * 6;
		while ((in = fgetc(fp)) != ' ')
		    *t++ = in;
		*t = 0;
#if 0
                if (fgets(buffer, sizeof(buffer), fp))
                    buffer[MIN(strlen(buffer), sizeof(buffer)) - 1] = '\0';

		c->channels[j] = strdup(buffer);
#else
                fscanf(fp, "%[^\n]\n", buffer);
                                                                                                                                                      
                c->channels[j] = (char *) malloc(strlen(buffer) + 1);
                StringCopy(c->channels[j], buffer);
#endif
	    }
	    sort_com_aliases(c);
	} else {
	    c->alias = NULL;
	    c->channels = NULL;
	}
	if ((Typeof(c->who) == TYPE_PLAYER) || (!God(Owner(c->who))) ||
	    ((!Going(c->who))))
	    add_commac(c);
	purge_commac();
    }
}

void purge_commac(void)
{
    struct commac *c;
    struct commac *d;
    int i;

#ifdef ABORT_PURGE_COMSYS
    return;
#endif				/*
				   * * ABORT_PURGE_COMSYS  
				 */

    for (i = 0; i < NUM_COMMAC; i++) {
	c = commac_table[i];
	while (c) {
	    d = c;
	    c = c->next;
	    if (d->numchannels == 0 && d->curmac == -1 &&
		d->macros[1] == -1 && d->macros[2] == -1 &&
		d->macros[3] == -1 && d->macros[4] == -1 &&
		d->macros[0] == -1) {
		del_commac(d->who);
		continue;
	    }

/*
 * if ((Typeof(d->who) != TYPE_PLAYER) && (God(Owner(d->who))) &&
 * * (Going(d->who))) 
 */
	    if (Typeof(d->who) == TYPE_PLAYER)
		continue;
	    if (God(Owner(d->who)) && Going(d->who)) {
		del_commac(d->who);
		continue;
	    }
	}
    }
}

void save_commac(FILE * fp)
{
    int np;
    struct commac *c;
    int i, j;

    purge_commac();
    np = 0;
    for (i = 0; i < NUM_COMMAC; i++) {
	c = commac_table[i];
	while (c) {
	    np++;
	    c = c->next;
	}
    }

    fprintf(fp, "%d\n", np);
    for (i = 0; i < NUM_COMMAC; i++) {
	c = commac_table[i];
	while (c) {
	    fprintf(fp, "%d %d %d %d %d %d %d %d\n", c->who,
		c->numchannels, c->macros[0], c->macros[1], c->macros[2],
		c->macros[3], c->macros[4], c->curmac);
	    for (j = 0; j < c->numchannels; j++) {
		fprintf(fp, "%s %s\n", c->alias + j * 6, c->channels[j]);
	    }
	    c = c->next;
	}
    }
}

struct commac *create_new_commac(void)
{
    struct commac *c;
    int i;

    c = (struct commac *) malloc(sizeof(struct commac));

    c->who = -1;
    c->numchannels = 0;
    c->maxchannels = 0;
    c->alias = NULL;
    c->channels = NULL;

    c->curmac = -1;
    for (i = 0; i < 5; i++)
	c->macros[i] = -1;

    c->next = NULL;
    return c;
}

struct commac *get_commac(which)
dbref which;
{
    struct commac *c;

    if (which < 0)
	return NULL;

    c = commac_table[which % NUM_COMMAC];

    while (c && (c->who != which))
	c = c->next;

    if (!c) {
	c = create_new_commac();
	c->who = which;
	add_commac(c);
    }
    return c;
}

void add_commac(struct commac *c)
{
    if (c->who < 0)
	return;

    c->next = commac_table[c->who % NUM_COMMAC];
    commac_table[c->who % NUM_COMMAC] = c;
}

void del_commac(dbref who)
{
    struct commac *c;
    struct commac *last;

    if (who < 0)
	return;

    c = commac_table[who % NUM_COMMAC];

    if (c == NULL)
	return;

    if (c->who == who) {
	commac_table[who % NUM_COMMAC] = c->next;
	destroy_commac(c);
	return;
    }
    last = c;
    c = c->next;
    while (c) {
	if (c->who == who) {
	    last->next = c->next;
	    destroy_commac(c);
	    return;
	}
	last = c;
	c = c->next;
    }
}

void destroy_commac(struct commac *c)
{
    int i;

    free(c->alias);
    for (i = 0; i < c->numchannels; i++)
	free(c->channels[i]);
    free(c->channels);
    free(c);
}

void sort_com_aliases(struct commac *c)
{
    int i;
    int cont;
    char buffer[10];
    char *s;

    cont = 1;
    while (cont) {
	cont = 0;
	for (i = 0; i < c->numchannels - 1; i++)
	    if (strcasecmp(c->alias + i * 6, c->alias + (i + 1) * 6) > 0) {
		StringCopy(buffer, c->alias + i * 6);
		StringCopy(c->alias + i * 6, c->alias + (i + 1) * 6);
		StringCopy(c->alias + (i + 1) * 6, buffer);
		s = c->channels[i];
		c->channels[i] = c->channels[i + 1];
		c->channels[i + 1] = s;
		cont = 1;
	    }
    }
}
