#include <errno.h>
#include "autoconf.h"
#include "udb_defs.h"
#include "externs.h"

/*
 * Log database errors 
 */

void log_db_err(obj, attr, txt)
int obj, attr;
const char *txt;
{
    STARTLOG(LOG_ALWAYS, "DBM", "ERROR") {
	log_text((char *) "Could not ");
	log_text((char *) txt);
	log_text((char *) " object #");
	log_number(obj);
	if (attr != NOTHING) {
	    log_text((char *) " attr #");
	    log_number(attr);
	}
	ENDLOG;
}}

/*
 * print a series of warnings - do not exit
 */

/*
 * VARARGS 
 */
void logf(char *p, ...)
{
    va_list ap;

    va_start(ap, p);
    while (1) {
	if (p == (char *) 0)
	    break;

	if (p == (char *) -1)
	    p = (char *) strerror(errno);

	(void) fprintf(stderr, "%s", p);
	p = va_arg(ap, char *);
    }
    va_end(ap);
    (void) fflush(stderr);
}

/*
 * print a series of warnings - exit
 */

/*
 * VARARGS 
 */
void fatal(char *p, ...)
{
    va_list ap;

    va_start(ap, p);
    while (1) {
	if (p == (char *) 0)
	    break;

	if (p == (char *) -1)
	    p = (char *) strerror(errno);

	(void) fprintf(stderr, "%s", p);
	p = va_arg(ap, char *);
    }
    va_end(ap);
    (void) fflush(stderr);
    exit(1);
}
