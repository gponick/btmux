#include <stdio.h>

#undef FILES_COMPRESSED_BY_DEFAULT

FILE *my_open_file(char *name, char *mode, int *openway)
{
    FILE *f;
    char buf[512];
    char buf2[512];

    if (!strcmp(mode, "w")) {
#ifdef FILES_COMPRESSED_BY_DEFAULT

/*       dup2(2, 1); */
	sprintf(buf, "nice gzip -c > %s.gz", name);
	if (!(f = popen(buf, mode)))
	    return NULL;
	*openway = 1;
	return f;
#else
	if (!(f = fopen(name, mode)))
	    return NULL;
	*openway = 0;
	return f;
#endif
    }
    if ((f = fopen(name, mode))) {
	*openway = 0;
	return f;
    }
    sprintf(buf, "%s.gz", name);
    if ((f = fopen(buf, mode)))
	fclose(f);
    else
	return NULL;
    sprintf(buf2, "nice gzip -dc < %s", buf);

/*   dup2(2, 1); */
    if ((f = popen(buf2, mode))) {
	*openway = 1;
	return f;
    }
    return NULL;
}

void my_close_file(FILE * f, int *openway)
{
    if (!f)
	return;
    if (*openway) {
	pclose(f);

/*       close(1); */
    } else
	fclose(f);
}
