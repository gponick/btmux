
/*
 * bsd.c 
 */

/*
 * $Id: bsd.c,v 1.5 2005/07/18 17:03:29 av1-op Exp $ 
 */
#include "copyright.h"
#include "config.h"

#ifdef VMS
#include "multinet_root:[multinet.include.sys]file.h"
#include "multinet_root:[multinet.include.sys]ioctl.h"
#include "multinet_root:[multinet.include]errno.h"
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#endif
#include <signal.h>
#include <errno.h>

#include "mudconf.h"
#include "db.h"
#include "file_c.h"
#include "externs.h"
#include "interface.h"
#include "flags.h"
#include "powers.h"
#include "alloc.h"
#include "command.h"
#include "slave.h"
#include "attrs.h"
#include <errno.h>

#ifdef __CYGWIN__
#undef WEXITSTATUS
#define WEXITSTATUS(stat) (((*((int *) &(stat))) >> 8) & 0xff)
#endif

#ifdef SQL_SUPPORT
#ifdef NO_SQLSLAVE
#include <dbi/dbi.h>
#endif
#include "sqlslave.h"
#endif

#if ARBITRARY_LOGFILES_MODE==2
#include "fileslave.h"
#endif

#ifdef SOLARIS
extern const int _sys_nsig;

#define NSIG _sys_nsig
#endif

#ifdef CONCENTRATE
extern struct descriptor_data *ccontrol;
extern void FDECL(send_killconcid, (DESC *));
extern long NDECL(make_concid);

#endif

extern void NDECL(dispatch);
extern void NDECL(dump_restart_db);
extern void FDECL(dump_database_internal, (int));
extern char *silly_atr_get(dbref, int);
extern void send_channel(char *chan, char *str);
extern void ChangeSpecialObjects(int i);

int sock;
int ndescriptors = 0;
int maxd = 0;

DESC *descriptor_list = NULL;
pid_t slave_pid;
int slave_socket = -1;

#ifdef SQL_SUPPORT
#ifndef NO_SQLSLAVE
pid_t sqlslave_pid;
int sqlslave_socket = -1;
#else
int query_number = 0;
#endif
#endif

#if ARBITRARY_LOGFILES_MODE==2
pid_t fileslave_pid;
int fileslave_socket = -1;
#endif

DESC *FDECL(initializesock, (int, struct sockaddr_in *));
DESC *FDECL(new_connection, (int));
int FDECL(process_output, (DESC *));
int FDECL(process_input, (DESC *));

void set_lastsite(DESC * d, char *lastsite)
{
    char buf[LBUF_SIZE];

    if (d->player) {
	if (!lastsite)
	    lastsite = silly_atr_get(d->player, A_LASTSITE);
	strcpy(buf, lastsite);
	atr_add_raw(d->player, A_LASTSITE, buf);
    }
}

#ifdef SQL_SUPPORT
/* Rewritting the sqlslave stuff and instead of having the seperate
 * sqlslave to handle the queries, going to do everything within the
 * mux. Made this as a compile time flag NO_SQLSLAVE.
 *
 * I know this will put more of a load on the mux but right now I'm
 * going for stability over performance.
 * 
 * If anyone fixes the socket stuff, just go ahead and delete this
 * Dany (07/2005)
 */

/*
 * get a result from the SQL slave
 */
#ifndef NO_SQLSLAVE 
static int get_sqlslave_result()
{
	dbref thing;
	int attr, len;
	static char response[SQLQUERY_MAX_STRING], result[SQLQUERY_MAX_STRING], preserve[SQLQUERY_MAX_STRING];
	char *argv[3];

	if (sqlslave_socket == -1)
		    return -100;

	memset(response, '\0', SQLQUERY_MAX_STRING);
	memset(result, '\0', SQLQUERY_MAX_STRING);
	memset(preserve, '\0', SQLQUERY_MAX_STRING);

	if (read(sqlslave_socket, &thing, sizeof(dbref)) <= 0) {
		    if (errno == EAGAIN || errno == EWOULDBLOCK)
			            return -1;
		        close(sqlslave_socket);
			    sqlslave_socket = -1;
			        return -1;
				    }
	if (read(sqlslave_socket, &attr, sizeof(int)) <= 0)
		    return -2;
	if (read(sqlslave_socket, &len, sizeof(int)) <= 0)
		    return -3;
	if (len > 0)
		    if (read(sqlslave_socket, response, len) <= 0)
			            return -4;
	if (read(sqlslave_socket, &len, sizeof(int)) <= 0)
		    return -5;
	if (len > 0)
		    if (read(sqlslave_socket, result, len) <= 0)
			            return -6;
	if (read(sqlslave_socket, &len, sizeof(int)) <= 0)
		    return -7;
	if (len > 0)
		    if (read(sqlslave_socket, preserve, len) <= 0)
			            return -8;

	argv[0] = response;
	argv[1] = result;
	argv[2] = preserve;
	did_it(GOD, thing, 0, NULL, 0, NULL, attr, argv, 3);
	return 0;
}
#endif
/*
 * Simple wrappers to get various mudconf pointers.
 * DISCLAIMER : This code was written under the influence while juggling the rest of the mechanics :P
 * I know this is 'VERY' uglee. 
 * TODO : Create a multi-dimensional (char *) array either globally or part of mudconf. 1 Dimension is A-E.
 * 2nd dimension is the HOSTNAME,etc... slots. This method needs a 2nd array of (int *) for the init val's.
 * 2nd solution is to make a struct of 1 (int *) and X (char *) and make an array of the struct. But,
 * do we really need to unprettify code for the one (int *) and have a minor structure extern'ed all over
 * the handfull of spots it's needed? To be decided. (dun dun dundun)
 * 
 * If anyone does the above TODO, get a patch in and delete these comments :D
 */

int *sqldb_slotinit(char db_slot)
{
switch (db_slot) {
    case 'A':
    case 'a':
        return &mudconf.sqlDB_init_A;
    case 'B':
    case 'b':
        return &mudconf.sqlDB_init_B;
    case 'C':
    case 'c':
        return &mudconf.sqlDB_init_C;
    case 'D':
    case 'd':
        return &mudconf.sqlDB_init_D;
    case 'E':
    case 'e':
        return &mudconf.sqlDB_init_E;
    default:
        return NULL;
    }
}

char *sqldb_slotval(char db_slot, int which)
{
switch (db_slot) {
    case 'A':
    case 'a':
        switch (which) {
            case SQLDB_SLOT_HOSTNAME:
                return mudconf.sqlDB_hostname_A;
            case SQLDB_SLOT_DBNAME:
                return mudconf.sqlDB_dbname_A;
            case SQLDB_SLOT_USERNAME:
                return mudconf.sqlDB_username_A;
            case SQLDB_SLOT_PASSWORD:
                return mudconf.sqlDB_password_A;
            case SQLDB_SLOT_DBTYPE:
                return mudconf.sqlDB_type_A;
            case SQLDB_SLOT_SOCKET:
                return mudconf.sqlDB_socket_A;
            case SQLDB_SLOT_PORT:
                return mudconf.sqlDB_port_A;
            }
    case 'B':
    case 'b':
        switch (which) {
            case SQLDB_SLOT_HOSTNAME:
                return mudconf.sqlDB_hostname_B;
            case SQLDB_SLOT_DBNAME:
                return mudconf.sqlDB_dbname_B;
            case SQLDB_SLOT_USERNAME:
                return mudconf.sqlDB_username_B;
            case SQLDB_SLOT_PASSWORD:
                return mudconf.sqlDB_password_B;
            case SQLDB_SLOT_DBTYPE:
                return mudconf.sqlDB_type_B;
            case SQLDB_SLOT_SOCKET:
                return mudconf.sqlDB_socket_B;
            case SQLDB_SLOT_PORT:
                return mudconf.sqlDB_port_B;
            }
    case 'C':
    case 'c':
        switch (which) {
            case SQLDB_SLOT_HOSTNAME:
                return mudconf.sqlDB_hostname_C;
            case SQLDB_SLOT_DBNAME:
                return mudconf.sqlDB_dbname_C;
            case SQLDB_SLOT_USERNAME:
                return mudconf.sqlDB_username_C;
            case SQLDB_SLOT_PASSWORD:
                return mudconf.sqlDB_password_C;
            case SQLDB_SLOT_DBTYPE:
                return mudconf.sqlDB_type_C;
            case SQLDB_SLOT_SOCKET:
                return mudconf.sqlDB_socket_C;
            case SQLDB_SLOT_PORT:
                return mudconf.sqlDB_port_C;
            }
    case 'D':
    case 'd':
        switch (which) {
            case SQLDB_SLOT_HOSTNAME:
                return mudconf.sqlDB_hostname_D;
            case SQLDB_SLOT_DBNAME:
                return mudconf.sqlDB_dbname_D;
            case SQLDB_SLOT_USERNAME:
                return mudconf.sqlDB_username_D;
            case SQLDB_SLOT_PASSWORD:
                return mudconf.sqlDB_password_D;
            case SQLDB_SLOT_DBTYPE:
                return mudconf.sqlDB_type_D;
            case SQLDB_SLOT_SOCKET:
                return mudconf.sqlDB_socket_D;
            case SQLDB_SLOT_PORT:
                return mudconf.sqlDB_port_D;
            }
    case 'E':
    case 'e':
        switch (which) {
            case SQLDB_SLOT_HOSTNAME:
                return mudconf.sqlDB_hostname_E;
            case SQLDB_SLOT_DBNAME:
                return mudconf.sqlDB_dbname_E;
            case SQLDB_SLOT_USERNAME:
                return mudconf.sqlDB_username_E;
            case SQLDB_SLOT_PASSWORD:
                return mudconf.sqlDB_password_E;
            case SQLDB_SLOT_DBTYPE:
                return mudconf.sqlDB_type_E;
            case SQLDB_SLOT_SOCKET:
                return mudconf.sqlDB_socket_E;
            case SQLDB_SLOT_PORT:
                return mudconf.sqlDB_port_E;
            }
    default:
        return NULL;
    }
}

#ifdef NO_SQLSLAVE
/* Function to log the sql stuff */
int sql_log(char *mesg)
{
    FILE *logfile;
    struct tm *tp;
    time_t now;

    /* Open Log File */
    logfile = fopen("sql.log", "a");
    /* Get Current Time */
    time((time_t *) (&now));
    tp = localtime((time_t *) (&now));
    /* Print Message */
    fprintf(logfile, "%d%02d%02d.%02d%02d%02d : %s\n",
        tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday,
        tp->tm_hour, tp->tm_min, tp->tm_sec,
        mesg);
    /* Close File */
    fclose(logfile);

    return 1;
}

/* Function to output the sql stuff */
int sql_output(dbref thing, int attr, char *resp, char *res, char *pres) 
{
    char *argv[3];

    argv[0] = resp;
    argv[1] = res;
    argv[2] = pres;
    did_it(GOD, thing, 0, NULL, 0, NULL, attr, argv, 3);
    
    return 1;
}

/* One massive function that basicly replaces the functionality 
 * of sqlslave. Use in replace of sqlslave_doquery */
void sql_doquery(char db_slot, dbref thing, int attr, char *pres, char *qry)
{
    static char dbtype[SQL_OPTION_MAX + 1];
    static char host[SQL_OPTION_MAX + 1];
    static char username[SQL_OPTION_MAX + 1];
    static char password[SQL_OPTION_MAX + 1];
    static char dbname[SQL_OPTION_MAX + 1];
    static char unix_socket[SQLQUERY_MAX_STRING + 1];
    static char port[SQL_OPTION_MAX + 1];

    static dbi_conn conn;
    static dbi_result dbiresult;

    static int err;

    static unsigned int rows, fields, i, ii, type, writ, tmp;
    static long field_int;
    static double field_dec;
    static const char *field_str;
    static time_t field_time;

    static char response[SQLQUERY_MAX_STRING + 1];
    static char result[SQLQUERY_MAX_STRING + 1];

    static char log_buffer[SQLQUERY_MAX_STRING];

    /* Count the Query */
    query_number++;

    /* Reset the Query Count so we dont get obscene query numbers */
    if (query_number > 1000) {
        query_number = 1;
        sql_log("Reset Query Count");
    }

    /* Log that we got a query */
    snprintf(log_buffer, LBUF_SIZE, "Received Query : Designating as Query # %d",
        query_number);
    sql_log(log_buffer);

    /* Setup the SQL Connection */
    memset(dbtype, '\0', SQL_OPTION_MAX + 1);
    memset(host, '\0', SQL_OPTION_MAX + 1);
    memset(username, '\0', SQL_OPTION_MAX + 1);
    memset(password, '\0', SQL_OPTION_MAX + 1);
    memset(dbname, '\0', SQL_OPTION_MAX + 1);
    memset(port, '\0', SQL_OPTION_MAX + 1);
    memset(unix_socket, '\0', SQLQUERY_MAX_STRING + 1);
    memset(response, '\0', SQLQUERY_MAX_STRING + 1);
    memset(result, '\0', SQLQUERY_MAX_STRING + 1);

    /* Getting values for connection */
    snprintf(dbtype, SBUF_SIZE, sqldb_slotval(db_slot, SQLDB_SLOT_DBTYPE));
    snprintf(host, SBUF_SIZE, sqldb_slotval(db_slot, SQLDB_SLOT_HOSTNAME));
    snprintf(username, SBUF_SIZE, sqldb_slotval(db_slot, SQLDB_SLOT_USERNAME));
    snprintf(password, SBUF_SIZE, sqldb_slotval(db_slot, SQLDB_SLOT_PASSWORD));
    snprintf(dbname, SBUF_SIZE, sqldb_slotval(db_slot, SQLDB_SLOT_DBNAME));
    snprintf(port, SBUF_SIZE, sqldb_slotval(db_slot, SQLDB_SLOT_PORT));
    snprintf(unix_socket, LBUF_SIZE, sqldb_slotval(db_slot, SQLDB_SLOT_SOCKET));

    /* Initialize */
    if (dbi_initialize(NULL) == -1) {
        if (dbi_initialize("./bin") == -1) {
            snprintf(log_buffer, LBUF_SIZE, "Error Loading libdbi drivers for Query # %d",
                    query_number);
            sql_log(log_buffer);
            snprintf(response, LBUF_SIZE, "Error Loading libdbi drivers");
            sql_output(thing, attr,
                    response,
                    NULL, pres);
        }
    }

    snprintf(log_buffer, LBUF_SIZE, "Initialized Drivers for Query # %d",
        query_number);
    sql_log(log_buffer);

    /* Make the connection */
    if (strcmp(dbtype, "mysql") != 0) {
        snprintf(log_buffer, LBUF_SIZE, "Bad DB Type for DB Slot %c and Query # %d",
            db_slot, query_number);
        sql_log(log_buffer);
        sql_output(thing, attr, 
            "Only MySQL is supported right now due to lazy driver access",
            NULL, pres);
        return;
    }
    if ((conn = dbi_conn_new(dbtype)) == NULL) {
        snprintf(log_buffer, LBUF_SIZE, "Unable to create connection to server for"
            " Query # %d", query_number);
        sql_log(log_buffer);
        sql_output(thing, attr, 
            "Unable to create connection to server",
            NULL, pres);
        snprintf(log_buffer, LBUF_SIZE, "Server Settings for Query # "
            "%d : %s %s %s %s %s %s %s",
            query_number, dbtype, host, username, password, dbname, unix_socket, port);
        sql_log(log_buffer);
        return;
    }

    snprintf(log_buffer, LBUF_SIZE, "Created Connection for Query # %d",
        query_number);
    sql_log(log_buffer);

    dbi_conn_set_option(conn, "host", host);
    dbi_conn_set_option(conn, "username", username);
    dbi_conn_set_option(conn, "password", password);
    dbi_conn_set_option(conn, "dbname", dbname);
    dbi_conn_set_option(conn, "mysql_unix_socket", unix_socket);
    dbi_conn_set_option(conn, "port", port);

    /* Heres were we actually try and connect */
    if (dbi_conn_connect(conn) != 0) {
        /* Check the connection for errors*/
        const char *tmp;
        err = dbi_conn_error(conn, &tmp);
        if (tmp != NULL) {
            snprintf(log_buffer, LBUF_SIZE, "Error Connecting for Query"
                " # %d -> %d %s", query_number, err, tmp);
            sql_log(log_buffer);
            snprintf(response, LBUF_SIZE, "Error Connecting : %s", tmp);
            sql_output(thing, attr, 
                response,
                NULL, pres);
        } else {
            snprintf(log_buffer, LBUF_SIZE, "Unknown Error Connecting for Query"
                " # %d", query_number);
            sql_log(log_buffer);
            snprintf(response, LBUF_SIZE, "Unknown Error Connecting");
            sql_output(thing, attr, 
                response,
                NULL, pres);
        }
        snprintf(log_buffer, LBUF_SIZE, "Unable to establish connection to server for"
            " Query # %d", query_number);
        sql_log(log_buffer);
        return;
    }

    /* Print that we made the connection */
    snprintf(log_buffer, LBUF_SIZE, "Connecting to Server for Query # %d : %s %s %s %s %s %s",
        query_number, host, username, password, dbname, unix_socket, port);
    sql_log(log_buffer);

    /* Now perform the query */
    dbiresult = dbi_conn_query(conn, qry);
    if (dbiresult == NULL) {

        const char *tmp;
        if ((err = dbi_conn_error(conn, &tmp)) != -1) {
            snprintf(log_buffer, LBUF_SIZE, "Sending Query Error for Query"
                    " # %d -> %s : %s", query_number, qry, tmp);
            sql_log(log_buffer);
            snprintf(response, LBUF_SIZE, "Sending Query Error : %s", tmp);
            sql_output(thing, attr, 
                response,
                NULL, pres);
        } else {
            snprintf(log_buffer, LBUF_SIZE, "Error Sending Query # %d -> %s : %s",
                query_number, qry, tmp);
            sql_log(log_buffer);
            snprintf(response, LBUF_SIZE, "Unknown Error Sending Query");
            sql_output(thing, attr, 
                response,
                NULL, pres);
        }
        dbi_conn_close(conn);
        return;

    }

    /* Now to print the results what not */
    snprintf(log_buffer, LBUF_SIZE, "Creating Output for Query # %d -> %s",
        query_number, qry);
    sql_log(log_buffer);

/*****************************************************************************/
    /* Get the # of rows and fields */
    rows = dbi_result_get_numrows(dbiresult);
    fields = dbi_result_get_numfields(dbiresult);

    /* Uhoh nothing returned ? */
    if (rows == 0 || fields == 0) {
        const char *tmp;

        if (dbi_conn_error(conn, &tmp) != -1 ) {
            snprintf(response, LBUF_SIZE, "Success");
            snprintf(log_buffer, LBUF_SIZE, "Number of Rows %d and Fields %d for Query # %d",
                rows, fields, query_number);
            sql_log(log_buffer);
        } else {
            snprintf(response, LBUF_SIZE, tmp);
            snprintf(log_buffer, LBUF_SIZE, "Error Creating Output for Query # %d",
                query_number);
            sql_log(log_buffer);
        }

    } else {
        snprintf(response, LBUF_SIZE, "Success");
        snprintf(log_buffer, LBUF_SIZE, "Number of Rows %d and Fields %d for Query # %d",
            rows, fields, query_number);
        sql_log(log_buffer);
    }

    writ = tmp = 0;

    /* Loop through the rows and pull out info for each one */
    for (i = 1; i <= rows; ++i) {
        if (dbi_result_seek_row(dbiresult, i) == 0)
            break;

        /* Loop through the fields and pull out each one */
        for (ii = 1; ii <= fields; ii++) {
            
            /* Now look at the type of value returned so we know how
             * to return it */

            type = dbi_result_get_field_type_idx(dbiresult, ii);
            switch (type) {
                
                /* Integer */
                case DBI_TYPE_INTEGER:
                    field_int = dbi_result_get_long_idx(dbiresult, ii);
                    tmp = snprintf(result + writ, SQLQUERY_MAX_STRING - writ, 
                        (ii == fields ? "%ld" : "%ld:"), field_int);
                    if (tmp < 0) {
                        snprintf(log_buffer, LBUF_SIZE,
                            "Error while creating output for Query # %d - Bad Integer Value",
                            query_number);
                        sql_log(log_buffer);
                        sql_output(thing, attr, 
                            "Error while creating output - Bad Integer Value",
                            NULL, pres);
                        dbi_result_free(dbiresult);
                        dbi_conn_close(conn);
                        return;
                    }
                    writ += tmp;
                    break;

                /* Float/Decimal */
                case DBI_TYPE_DECIMAL:
                    field_dec = dbi_result_get_double_idx(dbiresult, ii);
                    tmp = snprintf(result + writ, SQLQUERY_MAX_STRING - writ, 
                        (ii == fields ? "%f" : "%f:"), field_dec);
                    if (tmp < 0) {
                        snprintf(log_buffer, LBUF_SIZE,
                            "Error while creating output for Query # %d - Bad Float Value",
                            query_number);
                        sql_log(log_buffer);
                        sql_output(thing, attr, 
                            "Error while creating output - Bad Float Value",
                            NULL, pres);
                        dbi_result_free(dbiresult);
                        dbi_conn_close(conn);
                        return;
                    }
                    writ += tmp;
                    break;

                /* String */
                case DBI_TYPE_STRING:
                    field_str = dbi_result_get_string_idx(dbiresult, ii);
                    tmp = snprintf(result + writ, SQLQUERY_MAX_STRING - writ, 
                        (ii == fields ? "%s" : "%s:"), field_str);
                    if (tmp < 0) {
                        snprintf(log_buffer, LBUF_SIZE,
                            "Error while creating output for Query # %d - Bad String Value",
                            query_number);
                        sql_log(log_buffer);
                        sql_output(thing, attr, 
                            "Error while creating output - Bad String Value",
                            NULL, pres);
                        dbi_result_free(dbiresult);
                        dbi_conn_close(conn);
                        return;
                    }
                    writ += tmp;
                    break;

                /* Binary */
                case DBI_TYPE_BINARY:
                    {
                        int len;
                        const unsigned char *bindata; /* Sorry. MUX we can only pump 
                                                         out text anyhow, might as well 
                                                         convert to it */
                        char tempdata[SQLQUERY_MAX_STRING];

                        len = dbi_result_get_field_size_idx(dbiresult, ii);
                        bindata = dbi_result_get_binary_idx(dbiresult, ii);
                        if (len > SQLQUERY_MAX_STRING - 1)
                            len = SQLQUERY_MAX_STRING - 1;
                    
                        memset(tempdata, '\0', SQLQUERY_MAX_STRING);
                        memcpy(tempdata, bindata, len);

                        tmp = snprintf(result + writ, SQLQUERY_MAX_STRING - writ, 
                            (ii == fields ? "%s" : "%s:"), tempdata);
                        if (tmp < 0) {
                            snprintf(log_buffer, LBUF_SIZE,
                                "Error while creating output for Query # %d - Bad Binary Value",
                                query_number);
                            sql_log(log_buffer);
                            sql_output(thing, attr, 
                                "Error while creating output - Bad Binary Value",
                                NULL, pres);
                            dbi_result_free(dbiresult);
                            dbi_conn_close(conn);
                            return;
                        }
                        writ += tmp;
                    }
                    break;

                /* Datetime */
                case DBI_TYPE_DATETIME:
                    {
                        char timetmp[50];

                        memset(timetmp, '\0', 50);
                        field_time = dbi_result_get_datetime_idx(dbiresult, ii);
                        snprintf(timetmp, 50, "%d", (int) field_time);
                        /* timetmp[strlen(timetmp) - 1] = '\0'; */
                        tmp = snprintf(result + writ, SQLQUERY_MAX_STRING - writ, 
                            (ii == fields ? "%s" : "%s:"), timetmp);
                        if (tmp < 0) {
                            snprintf(log_buffer, LBUF_SIZE,
                                "Error while creating output for Query # %d - Bad DateTime Value",
                                query_number);
                            sql_log(log_buffer);
                            sql_output(thing, attr, 
                                "Error while creating output - Bad DateTime Value",
                                NULL, pres);
                            dbi_result_free(dbiresult);
                            dbi_conn_close(conn);
                            return;
                        }
                        writ += tmp;
                    }
                    break;

            /* End of Switch */ 
            }
        /* End of For loop for fields */
        }
        
        /* But in spacers between the rows */
        if (i != rows) {
            tmp = snprintf(result + writ, SQLQUERY_MAX_STRING - writ, "|");
            writ += tmp;
        }

    /* End of Row For Loop */
    }

/*****************************************************************************/

    /* Ok we done now the end print that we finished */
    snprintf(log_buffer, LBUF_SIZE, "Done with Query # %d -> %s -> %s",
            query_number, qry, result);
    sql_log(log_buffer);

    sql_output(thing, attr,
            response,
            result, pres);

    dbi_result_free(dbiresult);
    dbi_conn_close(conn);

    return;
}

/* Ok person chose to use sqlslave */
#else
/*
 * Internal call to send a query down to the libdbi SQL client.
 * It's a bit spamy but I wanted it to support open ended string lengths
 * so as to cap the data send/receives at whichever internal buffers were at hand,
 * instead of hardwiring sizes into a static structure.
 * To make this code cleaner....
 * TODO : Create a static transmit structure with all string sizes defined as LBUF
 * for the large potential ones, and SBUF/MBUF for the smaller ones liek username, PW,
 * etc... When transmitting, just loop through the proper location's instead of
 * calling each, and use strlen of course to define send sizes.
 *
 * How important? <shrug>
 */

void sqlslave_doquery(char db_slot, dbref thing, int attr, char *pres, char *qry)
{
	int len;
	char *val;

	if (write(sqlslave_socket, &thing, sizeof(dbref)) <= 0) {
		    close(sqlslave_socket);
		        sqlslave_socket = -1;
			    return;
			        }
	if (write(sqlslave_socket, &attr, sizeof(int)) <= 0)
		    return;

	val = sqldb_slotval(db_slot, SQLDB_SLOT_DBTYPE);
	len = strlen(val);
	if (write(sqlslave_socket, &len, sizeof(int)) <= 0)
		    return;
	if (len > 0)
		    if (write(sqlslave_socket, val, len) <= 0)
			            return;

	val = sqldb_slotval(db_slot, SQLDB_SLOT_HOSTNAME);
	len = strlen(val);
	if (write(sqlslave_socket, &len, sizeof(int)) <= 0)
		    return;
	if (len > 0)
		    if (write(sqlslave_socket, val, len) <= 0)
			            return;

	val = sqldb_slotval(db_slot, SQLDB_SLOT_USERNAME);
	len = strlen(val);
	if (write(sqlslave_socket, &len, sizeof(int)) <= 0)
		    return;
	if (len > 0)
		    if (write(sqlslave_socket, val, len) <= 0)
			            return;

	val = sqldb_slotval(db_slot, SQLDB_SLOT_PASSWORD);
	len = strlen(val);
	if (write(sqlslave_socket, &len, sizeof(int)) <= 0)
		    return;
	if (len > 0)
		    if (write(sqlslave_socket, val, len) <= 0)
			            return;

	val = sqldb_slotval(db_slot, SQLDB_SLOT_DBNAME);
	len = strlen(val);
	if (write(sqlslave_socket, &len, sizeof(int)) <= 0)
		    return;
	if (len > 0)
		    if (write(sqlslave_socket, val, len) <= 0)
			            return;

	val = sqldb_slotval(db_slot, SQLDB_SLOT_SOCKET);
	len = strlen(val);
	if (write(sqlslave_socket, &len, sizeof(int)) <= 0)
		    return;
	if (len > 0)
		    if (write(sqlslave_socket, val, len) <= 0)
			            return;

	val = sqldb_slotval(db_slot, SQLDB_SLOT_PORT);
	len = strlen(val);
	if (write(sqlslave_socket, &len, sizeof(int)) <= 0)
		    return;
	if (len > 0)
		    if (write(sqlslave_socket, val, len) <= 0)
			            return;

	len = strlen(pres);
	if (len >= SQLQUERY_MAX_STRING)
		    len = SQLQUERY_MAX_STRING;
	if (write(sqlslave_socket, &len, sizeof(int)) <= 0)
		    return;
	if (len > 0)
		    if (write(sqlslave_socket, pres, len) <= 0)
			            return;

	len = strlen(qry);
	if (len >= SQLQUERY_MAX_STRING)
		    len = SQLQUERY_MAX_STRING;
	if (write(sqlslave_socket, &len, sizeof(int)) <= 0)
		    return;
	if (len > 0)
		    if (write(sqlslave_socket, qry, len) <= 0)
			            return;
	return;
}

/*
 * Boot and/or Restart the SQL Slave
 */

void boot_sqlslave()
{
    int sv[2];
    int i;
    int maxfds;

#ifdef HAVE_GETDTABLESIZE
    maxfds = getdtablesize();
#else
    maxfds = sysconf(_SC_OPEN_MAX);
#endif

    if (sqlslave_socket != -1) {
        close(sqlslave_socket);
        sqlslave_socket = -1;
    }

    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) < 0) {
        return;
    }
    /*
     * set to nonblocking
     */
    if (fcntl(sv[0], F_SETFL, FNDELAY) == -1) {
        close(sv[0]);
        close(sv[1]);
        return;
    }
    sqlslave_pid = vfork();
    switch (sqlslave_pid) {
    case -1:
        close(sv[0]);
        close(sv[1]);
        return;
    case 0:                     /*
                                   * * child
                                 */
        close(sv[0]);
        close(0);
        close(1);
        if (dup2(sv[1], 0) == -1) {
            _exit(1);
        }
        if (dup2(sv[1], 1) == -1) {
            _exit(1);
        }
        for (i = 3; i < maxfds; ++i) {
            close(i);
        }
        execlp("bin/sqlslave", "sqlslave", NULL);
        _exit(1);
    }
    close(sv[1]);

    if (fcntl(sv[0], F_SETFL, FNDELAY) == -1) {
        close(sv[0]);
        return;
    }
    sqlslave_socket = sv[0];
}
#endif
#endif

#if ARBITRARY_LOGFILES_MODE==2
void boot_fileslave()
{
    int sv[2];
    int i;
    int maxfds;

#ifdef HAVE_GETDTABLESIZE
    maxfds = getdtablesize();
#else
    maxfds = sysconf(_SC_OPEN_MAX);
#endif

    if (fileslave_socket != -1) {
        close(fileslave_socket);
        fileslave_socket = -1;
    }

    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) < 0) {
        return;
    }
    /*
     * set to nonblocking
     */
    if (fcntl(sv[0], F_SETFL, FNDELAY) == -1) {
        close(sv[0]);
        close(sv[1]);
        return;
    }
    fileslave_pid = vfork();
    switch (fileslave_pid) {
    case -1:
        close(sv[0]);
        close(sv[1]);
        return;
    case 0:                     /*
                                   * * child
                                 */
        close(sv[0]);
        close(0);
        close(1);
        if (dup2(sv[1], 0) == -1) {
            _exit(1);
        }
        if (dup2(sv[1], 1) == -1) {
            _exit(1);
        }
        for (i = 3; i < maxfds; ++i) {
            close(i);
        }
        execlp("bin/fileslave", "fileslave", NULL);
        _exit(1);
    }
    close(sv[1]);

    if (fcntl(sv[0], F_SETFL, FNDELAY) == -1) {
        close(sv[0]);
        return;
    }
    fileslave_socket = sv[0];
}

static int get_fileslave_result()
{
dbref thing;
int len;
static char response[FILESLAVE_MAX_STRING + 1];

memset(response, '\0', FILESLAVE_MAX_STRING + 1);

if (fileslave_socket == -1)
    return -100;

if (read(fileslave_socket, &thing, sizeof(dbref)) <=0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
        return -1;
    close(fileslave_socket);
    fileslave_socket = -1;
    return -1;
    }
if (read(fileslave_socket, &len, sizeof(int)) <= 0)
    return -2;
if (len > 0)
    if (read(fileslave_socket, response, len) <= 0)
        return -3;
notify(thing, response);
return 0;
}

void fileslave_dolog(dbref thing, const char *fname, const char *fdata)
{
int len;

if (fname[0] == '\0') {
    notify(thing, "Log to which file?");
    return;
    }
if (fdata[0] == '\0') {
    notify(thing, "Log what to the file?");
    return;
    }

len = strlen(fname);
if (write(fileslave_socket, &len, sizeof(int)) <= 0) {
    close(fileslave_socket);
    fileslave_socket = -1;
    return;
    }
if (write(fileslave_socket, fname, len) <= 0)
    return;

len = strlen(fdata);
if (write(fileslave_socket, &len, sizeof(int)) <= 0)
    return;
if (write(fileslave_socket, fdata, len) <= 0)
    return;

if (write(fileslave_socket, &thing, sizeof(dbref)) <=0)
    return;
notify(thing, "Log packet sent.");
return;
}
#endif

/*
 * get a result from the slave 
 */
static int get_slave_result()
{
    char *buf;
    char *token;
    char *os;
    char *userid;
    char *host;
    int local_port, remote_port;
    char *p;
    DESC *d;
    int len;

    buf = alloc_lbuf("slave_buf");

    len = read(slave_socket, buf, LBUF_SIZE - 1);
    if (len < 0) {
	if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
	    free_lbuf(buf);
	    return (-1);
	}
	close(slave_socket);
	slave_socket = -1;
	free_lbuf(buf);
	return (-1);
    } else if (len == 0) {
	free_lbuf(buf);
	return (-1);
    }
    buf[len] = '\0';

    token = alloc_lbuf("slave_token");
    os = alloc_lbuf("slave_os");
    userid = alloc_lbuf("slave_userid");
    host = alloc_lbuf("slave_host");

    if (sscanf(buf, "%s %s", host, token) != 2) {
	free_lbuf(buf);
	free_lbuf(token);
	free_lbuf(os);
	free_lbuf(userid);
	free_lbuf(host);
	return (0);
    }
    p = strchr(buf, '\n');
    *p = '\0';
    for (d = descriptor_list; d; d = d->next) {
	if (strcmp(d->addr, host))
	    continue;
	if (d->flags & DS_IDENTIFIED)
	    continue;
	if (mudconf.use_hostname) {
	    StringCopyTrunc(d->addr, token, 50);
	    d->addr[50] = '\0';
	    if (d->player) {
		if (d->username[0])
		    set_lastsite(d, tprintf("%s@%s", d->username,
			    d->addr));
		else
		    set_lastsite(d, d->addr);

	    }
	}
    }

    if (sscanf(p + 1, "%s %d , %d : %s : %s : %s", host, &remote_port,
	    &local_port, token, os, userid) != 6) {
	free_lbuf(buf);
	free_lbuf(token);
	free_lbuf(os);
	free_lbuf(userid);
	free_lbuf(host);
	return (0);
    }
    for (d = descriptor_list; d; d = d->next) {
	if (ntohs((d->address).sin_port) != remote_port)
	    continue;
	if (d->flags & DS_IDENTIFIED)
	    continue;
	StringCopyTrunc(d->username, userid, 10);
	d->username[10] = '\0';
	set_lastsite(d, tprintf("%s@%s", d->username, d->addr));
	free_lbuf(buf);
	free_lbuf(token);
	free_lbuf(os);
	free_lbuf(userid);
	free_lbuf(host);
	return (0);
    }
    free_lbuf(buf);
    free_lbuf(token);
    free_lbuf(os);
    free_lbuf(userid);
    free_lbuf(host);
    return (0);
}

void boot_slave()
{
    int sv[2];
    int i;
    int maxfds;

#ifdef HAVE_GETDTABLESIZE
    maxfds = getdtablesize();
#else
    maxfds = sysconf(_SC_OPEN_MAX);
#endif

    if (slave_socket != -1) {
	close(slave_socket);
	slave_socket = -1;
    }
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) < 0) {
	return;
    }
    /*
     * set to nonblocking 
     */
    if (fcntl(sv[0], F_SETFL, FNDELAY) == -1) {
	close(sv[0]);
	close(sv[1]);
	return;
    }
    slave_pid = vfork();
    switch (slave_pid) {
    case -1:
	close(sv[0]);
	close(sv[1]);
	return;

    case 0:			/*
				   * * child  
				 */
	close(sv[0]);
	close(0);
	close(1);
	if (dup2(sv[1], 0) == -1) {
	    _exit(1);
	}
	if (dup2(sv[1], 1) == -1) {
	    _exit(1);
	}
	for (i = 3; i < maxfds; ++i) {
	    close(i);
	}
	execlp("bin/slave", "slave", NULL);
	_exit(1);
    }
    close(sv[1]);

    if (fcntl(sv[0], F_SETFL, FNDELAY) == -1) {
	close(sv[0]);
	return;
    }
    slave_socket = sv[0];
}

int make_socket(port)
int port;
{
    int s, opt;
    struct sockaddr_in server;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
	log_perror("NET", "FAIL", NULL, "creating master socket");
	exit(3);
    }
    opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
	    sizeof(opt)) < 0) {
	log_perror("NET", "FAIL", NULL, "setsockopt");
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    if (!mudstate.restarting)
	if (bind(s, (struct sockaddr *) &server, sizeof(server))) {
	    log_perror("NET", "FAIL", NULL, "bind");
	    close(s);
	    exit(4);
	}
    listen(s, 5);
    return s;
}

static int eradicate_broken_fd(void)
{
    struct stat statbuf;
    DESC *d;

    DESC_ITER_ALL(d) {
	if (fstat(d->descriptor, &statbuf) < 0) {
	    /* An invalid player connection... eject, eject, eject. */
	    STARTLOG(LOG_PROBLEMS, "ERR", "EBADF") {
		log_text("Broken descriptor ");
		log_number(d->descriptor);
		log_text(" for player ");
		log_name(d->player);
		ENDLOG;
	    }
	    shutdownsock(d, R_SOCKDIED);
	}
    }
    if (slave_socket != -1 && fstat(slave_socket, &statbuf) < 0) {
	STARTLOG(LOG_PROBLEMS, "ERR", "EBADF") {
	    log_text("Broken descriptor for DNS slave: ");
	    log_number(slave_socket);
	    ENDLOG;
	}
	boot_slave();
    }
#ifdef SQL_SUPPORT
#ifndef NO_SQLSLAVE
    if (sqlslave_socket != -1 && fstat(sqlslave_socket, &statbuf) < 0) {
        STARTLOG(LOG_PROBLEMS, "ERR", "EBADF") {
            log_text("Broken descriptor for SQL slave: ");
            log_number(sqlslave_socket);
            ENDLOG;
        }
        boot_sqlslave();
    }
#endif
#endif
#if ARBITRARY_LOGFILES_MODE==2
    if (fileslave_socket != -1 && fstat(fileslave_socket, &statbuf) < 0) {
        STARTLOG(LOG_PROBLEMS, "ERR", "EBADF") {
            log_text("Broken descriptor for file slave: ");
            log_number(fileslave_socket);
            ENDLOG;
        }
        boot_fileslave();
    }
#endif
    if (sock != -1 && fstat(sock, &statbuf) < 0) {
	STARTLOG(LOG_PROBLEMS, "ERR", "EBADF") {
	    log_text("Broken descriptor for our main port: ");
	    log_number(slave_socket);
	    ENDLOG;
	}
	sock = -1;
	return -1;
    }
    return 0;
}

#ifndef HAVE_GETTIMEOFDAY
#define get_tod(x)	{ (x)->tv_sec = time(NULL); (x)->tv_usec = 0; }
#else
#define get_tod(x)	gettimeofday(x, (struct timezone *)0)
#endif

void shovechars(port)
int port;
{
    fd_set input_set, output_set;
    struct timeval last_slice, current_time, next_slice, timeout,
	slice_timeout;
    int found, check;
    DESC *d, *dnext, *newd;
    int avail_descriptors, maxfds;

#define CheckInput(x)	FD_ISSET(x, &input_set)
#define CheckOutput(x)	FD_ISSET(x, &output_set)

    mudstate.debug_cmd = (char *) "< shovechars >";
    if (!mudstate.restarting) {
	sock = make_socket(port);
    }
    if (!mudstate.restarting)
	maxd = sock + 1;

    get_tod(&last_slice);

#ifdef HAVE_GETDTABLESIZE
    maxfds = getdtablesize();
#else
    maxfds = sysconf(_SC_OPEN_MAX);
#endif

    avail_descriptors = maxfds - 7;

    while (mudstate.shutdown_flag == 0) {
	get_tod(&current_time);
	last_slice = update_quotas(last_slice, current_time);

	process_commands();
	if (mudstate.shutdown_flag)
	    break;

	/*
	 * test for events 
	 */

	dispatch();

	/*
	 * any queued robot commands waiting? 
	 */

	timeout.tv_sec = que_next();
	timeout.tv_usec = 0;
	next_slice = msec_add(last_slice, mudconf.timeslice);
	slice_timeout = timeval_sub(next_slice, current_time);

	FD_ZERO(&input_set);
	FD_ZERO(&output_set);

	/*
	 * Listen for new connections if there are free descriptors 
	 */
	#define FIXMAXD(a) if (a+1>maxd) maxd=a+1

	if (ndescriptors < avail_descriptors) {
	    FD_SET(sock, &input_set);
	    FIXMAXD(sock);
	}
	/*
	 * Listen for replies from the slave socket 
	 */

	if (slave_socket != -1) {
	    FD_SET(slave_socket, &input_set);
	    FIXMAXD(slave_socket);
	}
#ifdef SQL_SUPPORT
#ifndef NO_SQLSLAVE
        if (sqlslave_socket != -1) {
	    FD_SET(sqlslave_socket, &input_set);
	    FIXMAXD(sqlslave_socket);
	    }
#endif
#endif

#if ARBITRARY_LOGFILES_MODE==2
        if (fileslave_socket != -1) {
	    FD_SET(fileslave_socket, &input_set);
	    FIXMAXD(fileslave_socket);
	}
#endif
	/*
	 * Mark sockets that we want to test for change in status 
	 */

	DESC_ITER_ALL(d) {
	    if (!d->input_head)
		FD_SET(d->descriptor, &input_set);
	    if (d->output_head)
		FD_SET(d->descriptor, &output_set);
	    FIXMAXD(d->descriptor);
	}

	/*
	 * Wait for something to happen 
	 */
	found =
	    select(maxd, &input_set, &output_set, (fd_set *) NULL,
	    &timeout);

	if (found < 0) {
	    if (errno == EBADF) {
		/* One of the connection sockets went kablowey
		 * underneath us. This is not good. Figure out which
		 * one it is and rip it out. */
		log_perror("NET", "FAIL", "checking for activity",
			   "select");
		if (eradicate_broken_fd() < 0)
		    break;
	    } else if (errno != EINTR) {
		log_perror("NET", "FAIL", "checking for activity",
			   "select");
	    }
	    continue;
	}
	/*
	 * if !found then time for robot commands 
	 */

	if (!found) {
	    if (mudconf.queue_chunk)
		do_top(mudconf.queue_chunk);
	    continue;
	} else {
	    do_top(mudconf.active_q_chunk);
	}

	/*
	 * Get usernames and hostnames 
	 */

	if (slave_socket != -1 && FD_ISSET(slave_socket, &input_set)) {
	    while (get_slave_result() == 0);
	}
#ifdef SQL_SUPPORT
#ifndef NO_SQLSLAVE
       if (sqlslave_socket != -1 && FD_ISSET(sqlslave_socket, &input_set)) {
            while (get_sqlslave_result() == 0);
        }
#endif
#endif

#if ARBITRARY_LOGFILES_MODE==2
        if (fileslave_socket != -1 && FD_ISSET(fileslave_socket, &input_set)) {
            while (get_fileslave_result() == 0);
        }
#endif
	/*
	 * Check for new connection requests 
	 */

	if (CheckInput(sock)) {
	    newd = new_connection(sock);
	    if (!newd) {
		check = (errno && (errno != EINTR) && (errno != EMFILE) &&
		    (errno != ENFILE));
		if (check) {
		    log_perror("NET", "FAIL", NULL, "new_connection");
		}
	    } else {
		if (newd->descriptor >= maxd)
		    maxd = newd->descriptor + 1;
	    }
	}
	/*
	 * Check for activity on user sockets 
	 */

	DESC_SAFEITER_ALL(d, dnext) {

	    /*
	     * Process input from sockets with pending input 
	     */

	    if (CheckInput(d->descriptor)) {

		/*
		 * Undo autodark 
		 */

		if (d->flags & DS_AUTODARK) {
		    d->flags &= ~DS_AUTODARK;
		    s_Flags(d->player, Flags(d->player) & ~DARK);
		}
		/*
		 * Process received data 
		 */
#ifdef CONCENTRATE
		if (!(d->cstatus & C_REMOTE))
#endif
		    if (!process_input(d)) {
			shutdownsock(d, R_SOCKDIED);
			continue;
		    }
	    }
	    /*
	     * Process output for sockets with pending output 
	     */

	    if (CheckOutput(d->descriptor)) {
		if (!process_output(d)) {
#ifdef CONCENTRATE
		    if (!(d->cstatus & C_CCONTROL))
#endif
			shutdownsock(d, R_SOCKDIED);
		}
	    }
	}
    }
}

DESC *new_connection(sock)
int sock;
{
    int newsock;
    char *buff, *buff1, *cmdsave;
    DESC *d;
    struct sockaddr_in addr;
    int addr_len, len;
    char *buf;


    cmdsave = mudstate.debug_cmd;
    mudstate.debug_cmd = (char *) "< new_connection >";
    addr_len = sizeof(struct sockaddr);

    newsock = accept(sock, (struct sockaddr *) &addr, &addr_len);
    if (newsock < 0)
	return 0;

    if (site_check(addr.sin_addr, mudstate.access_list) == H_FORBIDDEN) {
	STARTLOG(LOG_NET | LOG_SECURITY, "NET", "SITE") {
	    buff = alloc_mbuf("new_connection.LOG.badsite");
	    sprintf(buff, "[%d/%s] Connection refused.  (Remote port %d)",
		newsock, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	    log_text(buff);
	    free_mbuf(buff);
	    ENDLOG;
	}
	fcache_rawdump(newsock, FC_CONN_SITE);

	shutdown(newsock, 2);
	close(newsock);
	errno = 0;
	d = NULL;
    } else {
	buff = alloc_mbuf("new_connection.address");
	buf = alloc_lbuf("new_connection.write");
	StringCopy(buff, inet_ntoa(addr.sin_addr));

	/*
	 * Ask slave process for host and username 
	 */
	if ((slave_socket != -1) && mudconf.use_hostname) {
	    sprintf(buf, "%s\n%s,%d,%d\n", inet_ntoa(addr.sin_addr),
		inet_ntoa(addr.sin_addr), ntohs(addr.sin_port),
		mudconf.port);
	    len = strlen(buf);
	    if (WRITE(slave_socket, buf, len) < 0) {
		close(slave_socket);
		slave_socket = -1;
	    }
	}
	free_lbuf(buf);
	STARTLOG(LOG_NET, "NET", "CONN") {
	    buff1 = alloc_mbuf("new_connection.LOG.open");
	    sprintf(buff1, "[%d/%s] Connection opened (remote port %d)",
		newsock, buff, ntohs(addr.sin_port));
	    log_text(buff1);
	    free_mbuf(buff1);
	    ENDLOG;
	}
	d = initializesock(newsock, &addr);

	mudstate.debug_cmd = cmdsave;
	free_mbuf(buff);
    }
    mudstate.debug_cmd = cmdsave;
    return (d);
}

/*
 * Disconnect reasons that get written to the logfile 
 */

static const char *disc_reasons[] = {
    "Unspecified",
    "Quit",
    "Inactivity Timeout",
    "Booted",
    "Remote Close or Net Failure",
    "Game Shutdown",
    "Login Retry Limit",
    "Logins Disabled",
    "Logout (Connection Not Dropped)",
    "Too Many Connected Players"
};

/*
 * Disconnect reasons that get fed to A_ADISCONNECT via announce_disconnect 
 */

static const char *disc_messages[] = {
    "unknown",
    "quit",
    "timeout",
    "boot",
    "netdeath",
    "shutdown",
    "badlogin",
    "nologins",
    "logout"
};

void shutdownsock(d, reason)
DESC *d;
int reason;
{
    char *buff, *buff2;
    time_t now;
    int i, num;
    DESC *dtemp;

    if ((reason == R_LOGOUT) &&
	(site_check((d->address).sin_addr,
		mudstate.access_list) == H_FORBIDDEN))
	reason = R_QUIT;

    if (d->flags & DS_CONNECTED) {

	/*
	 * Do the disconnect stuff if we aren't doing a LOGOUT * * *
	 * * * * (which keeps the connection open so the player can *
	 * * connect * * * * to a different character). 
	 */

	if (reason != R_LOGOUT) {
	    fcache_dump(d, FC_QUIT);
	    STARTLOG(LOG_NET | LOG_LOGIN, "NET", "DISC") {
		buff = alloc_mbuf("shutdownsock.LOG.disconn");
		sprintf(buff, "[%d/%s] Logout by ", d->descriptor,
		    d->addr);
		log_text(buff);
		log_name(d->player);
		sprintf(buff, " <Reason: %s>", disc_reasons[reason]);
		log_text(buff);
		free_mbuf(buff);
		ENDLOG;
	    }
	} else {
	    STARTLOG(LOG_NET | LOG_LOGIN, "NET", "LOGO") {
		buff = alloc_mbuf("shutdownsock.LOG.logout");
		sprintf(buff, "[%d/%s] Logout by ", d->descriptor,
		    d->addr);
		log_text(buff);
		log_name(d->player);
		sprintf(buff, " <Reason: %s>", disc_reasons[reason]);
		log_text(buff);
		free_mbuf(buff);
		ENDLOG;
	    }
	}

	/*
	 * If requested, write an accounting record of the form: * *
	 * * * * * Plyr# Flags Cmds ConnTime Loc Money [Site]
	 * <DiscRsn>  * *  * Name 
	 */

	STARTLOG(LOG_ACCOUNTING, "DIS", "ACCT") {
	    now = mudstate.now - d->connected_at;
	    buff = alloc_lbuf("shutdownsock.LOG.accnt");
	    buff2 =
		decode_flags(GOD, Flags(d->player), Flags2(d->player),
		Flags3(d->player));
	    sprintf(buff, "%d %s %d %d %d %d [%s] <%s> %s", d->player,
		buff2, d->command_count, (int) now, Location(d->player),
		Pennies(d->player), d->addr, disc_reasons[reason],
		Name(d->player));
	    log_text(buff);
	    free_lbuf(buff);
	    free_sbuf(buff2);
	    ENDLOG;
	} announce_disconnect(d->player, d, disc_messages[reason]);
    } else {
	if (reason == R_LOGOUT)
	    reason = R_QUIT;
	STARTLOG(LOG_SECURITY | LOG_NET, "NET", "DISC") {
	    buff = alloc_mbuf("shutdownsock.LOG.neverconn");
	    sprintf(buff,
		"[%d/%s] Connection closed, never connected. <Reason: %s>",
		d->descriptor, d->addr, disc_reasons[reason]);
	    log_text(buff);
	    free_mbuf(buff);
	    ENDLOG;
	}
    }
    process_output(d);
    clearstrings(d);
    if (reason == R_LOGOUT) {
	d->flags &= ~DS_CONNECTED;
	d->connected_at = mudstate.now;
	d->retries_left = mudconf.retry_limit;
	d->command_count = 0;
	d->timeout = mudconf.idle_timeout;
	d->player = 0;
	d->doing[0] = '\0';
	d->hudkey[0] = '\0';
	d->quota = mudconf.cmd_quota_max;
	d->last_time = 0;
	d->host_info =
	    site_check((d->address).sin_addr,
	    mudstate.access_list) | site_check((d->address).sin_addr,
	    mudstate.suspect_list);
	d->input_tot = d->input_size;
	d->output_tot = 0;
	welcome_user(d);
    } else {
#ifdef CONCENTRATE
	if (!(d->cstatus & C_REMOTE)) {
	    if (d->cstatus & C_CCONTROL) {
		register struct descriptor_data *k;

		for (k = descriptor_list; k; k = k->next)
		    if (k->parent == d)
			shutdownsock(k, R_QUIT);
	    }
#endif
	    shutdown(d->descriptor, 2);
	    close(d->descriptor);
#ifdef CONCENTRATE
	} else {
	    register struct descriptor_data *k;

	    for (k = descriptor_list; k; k = k->next)
		if (d->parent == k)
		    send_killconcid(d);
	}
#endif
	freeqs(d);
	*d->prev = d->next;
	if (d->next)
	    d->next->prev = d->prev;

	/*
	 * Is this desc still in interactive mode? 
	 */
	if (d->program_data != NULL) {
	    num = 0;
	    DESC_ITER_PLAYER(d->player, dtemp) num++;

	    if (num == 0) {
		for (i = 0; i < MAX_GLOBAL_REGS; i++) {
		    free_lbuf(d->program_data->wait_regs[i]);
		}
		free(d->program_data);
	    }
	}

	free_desc(d);
#ifdef CONCENTRATE
	if (!(d->cstatus & C_REMOTE))
#endif
	    ndescriptors--;
    }
}

void make_nonblocking(s)
int s;
{
#ifdef HAVE_LINGER
    struct linger ling;
#endif

#ifdef FNDELAY
    if (fcntl(s, F_SETFL, FNDELAY) == -1) {
	log_perror("NET", "FAIL", "make_nonblocking", "fcntl");
    }
#else
    if (fcntl(s, F_SETFL, O_NDELAY) == -1) {
	log_perror("NET", "FAIL", "make_nonblocking", "fcntl");
    }
#endif
#ifdef HAVE_LINGER
    ling.l_onoff = 0;
    ling.l_linger = 0;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &ling,
	    sizeof(ling)) < 0) {
	log_perror("NET", "FAIL", "linger", "setsockopt");
    }
#endif
}

extern int fcache_conn_c;

DESC *initializesock(s, a)
int s;
struct sockaddr_in *a;
{
    DESC *d;

    ndescriptors++;
    d = alloc_desc("init_sock");
    d->descriptor = s;
    if (fcache_conn_c)
	d->logo = rand() % fcache_conn_c;
#ifdef CONCENTRATE
    d->concid = make_concid();
    d->cstatus = 0;
    d->parent = 0;
#endif
    d->flags = 0;
    d->connected_at = mudstate.now;
    d->retries_left = mudconf.retry_limit;
    d->command_count = 0;
    d->timeout = mudconf.idle_timeout;
    d->host_info =
	site_check((*a).sin_addr,
	mudstate.access_list) | site_check((*a).sin_addr,
	mudstate.suspect_list);
    d->player = 0;		/*
				 * be sure #0 isn't wizard.  Shouldn't be. 
				 */

    d->addr[0] = '\0';
    d->doing[0] = '\0';
    d->hudkey[0] = '\0';
    d->username[0] = '\0';
    make_nonblocking(s);
    d->output_prefix = NULL;
    d->output_suffix = NULL;
    d->output_size = 0;
    d->output_tot = 0;
    d->output_lost = 0;
    d->output_head = NULL;
    d->output_tail = NULL;
    d->input_head = NULL;
    d->input_tail = NULL;
    d->input_size = 0;
    d->input_tot = 0;
    d->input_lost = 0;
    d->raw_input = NULL;
    d->raw_input_at = NULL;
    d->quota = mudconf.cmd_quota_max;
    d->program_data = NULL;
    d->last_time = 0;
    d->address = *a;		/*
				 * added 5/3/90 SCG 
				 */
    if (descriptor_list)
	descriptor_list->prev = &d->next;
    d->hashnext = NULL;
    d->next = descriptor_list;
    d->prev = &descriptor_list;
    StringCopyTrunc(d->addr, inet_ntoa(a->sin_addr), 50);
    descriptor_list = d;
    welcome_user(d);
    return d;
}

int process_output(d)
DESC *d;
{
    TBLOCK *tb, *save;
    int cnt;
    char *cmdsave;

    cmdsave = mudstate.debug_cmd;
    mudstate.debug_cmd = (char *) "< process_output >";

    tb = d->output_head;

#ifdef CONCENTRATE
    if (d->cstatus & C_REMOTE) {
	static char buf[10];
	static char obuf[2048];
	int buflen, k, j;

	sprintf(buf, "%d ", d->concid);
	buflen = strlen(buf);

	bcopy(buf, obuf, buflen);
	j = buflen;

	while (tb != NULL) {
	    for (k = 0; k < tb->hdr.nchars; k++) {
		obuf[j++] = tb->hdr.start[k];
		if (tb->hdr.start[k] == '\n') {
		    if (d->parent)
			queue_write(d->parent, obuf, j);
		    bcopy(buf, obuf, buflen);
		    j = buflen;
		}
	    }
	    d->output_size -= tb->hdr.nchars;
	    save = tb;
	    tb = tb->hdr.nxt;
	    free(save);
	    d->output_head = tb;
	    if (tb == NULL)
		d->output_tail = NULL;
	}

	if (j > buflen)
	    queue_write(d, obuf + buflen, j - buflen);

	return 1;
    } else {
#endif
	while (tb != NULL) {
	    while (tb->hdr.nchars > 0) {
		cnt = WRITE(d->descriptor, tb->hdr.start, tb->hdr.nchars);
		if (cnt < 0) {
		    mudstate.debug_cmd = cmdsave;
		    if (errno == EWOULDBLOCK || errno == EINTR)
			return 1;
		    send_channel("MUXConnections",
			tprintf("Debug: Got %d - Errno:%d[%s]", cnt, errno,
			    strerror(errno)));
		    return 0;
		}
		d->output_size -= cnt;
		tb->hdr.nchars -= cnt;
		tb->hdr.start += cnt;
	    }
	    save = tb;
	    tb = tb->hdr.nxt;
	    free(save);
	    d->output_head = tb;
	    if (tb == NULL)
		d->output_tail = NULL;
	}
#ifdef CONCENTRATE
    }
#endif

    mudstate.debug_cmd = cmdsave;
    return 1;
}

extern int event_tick;
int last_bug2 = -1;
int last_bug = -1;
int last_bugc = 0;		/* If 4+ bugs per second, and/or 3 bugs per 2sec, *bang* */

int fatal_bug()
{
    if (event_tick != last_bug) {
	last_bug2 = last_bug;
	last_bug = event_tick;
	last_bugc = 1;
	return 0;
    }
    last_bugc++;
    if (last_bugc == 4)
	return 1;
    if (last_bug2 == (last_bug - 1) && last_bugc == 2)
	return 1;
    return 0;
}

int process_input(d)
DESC *d;
{
    static char buf[LBUF_SIZE];
    int got, in, lost;
    char *p, *pend, *q, *qend;
    char *cmdsave;
    int cnt = 0;

    cmdsave = mudstate.debug_cmd;
    mudstate.debug_cmd = (char *) "< process_input >";

    do {
	got = in = READ(d->descriptor, buf, (sizeof buf - 1));
	if (got <= 0 && errno != EINTR) {
	    mudstate.debug_cmd = cmdsave;
	    if (errno == EAGAIN) {
		if (!fatal_bug()) {
		    send_channel("MUXConnections",
			tprintf
			("Debug: Bugged for %d (#%d) ; kludge saved the day [%d]",
			    d->descriptor, d->player, cnt));
		    return 1;	/* Should be impossible, but happens. Why? God knows */
		}
	    }
	    send_channel("MUXConnections",
		tprintf("Debug: Got %d - Errno %d[%s]", got, errno,
		    strerror(errno)));
	    return 0;
	}
	if (cnt++ > 10)
	    return 0;		/* Silent disconnection */
    }
    while (got <= 0 && errno == EINTR);
    if (got == LBUF_SIZE)
	got = LBUF_SIZE - 1;
    buf[got] = 0;
    if (!d->raw_input) {
	d->raw_input = (CBLK *) alloc_lbuf("process_input.raw");
	d->raw_input_at = d->raw_input->cmd;
    }
    p = d->raw_input_at;
    pend = d->raw_input->cmd + LBUF_SIZE - sizeof(CBLKHDR) - 1;
    lost = 0;
    for (q = buf, qend = buf + got; q < qend; q++) {
	if (*q == '\n') {
	    *p = '\0';
	    if (p > d->raw_input->cmd) {
		save_command(d, d->raw_input);
		d->raw_input = (CBLK *) alloc_lbuf("process_input.raw");

		p = d->raw_input_at = d->raw_input->cmd;
		pend = d->raw_input->cmd + LBUF_SIZE - sizeof(CBLKHDR) - 1;
	    } else {
		in -= 1;	/*
				 * for newline 
				 */
	    }
	} else if ((*q == '\b') || (*q == 127)) {
	    if (*q == 127)
		queue_string(d, "\b \b");
	    else
		queue_string(d, " \b");
	    in -= 2;
	    if (p > d->raw_input->cmd)
		p--;
	    if (p < d->raw_input_at)
		(d->raw_input_at)--;
	} else if (p < pend && ((isascii(*q) && isprint(*q)) || *q < 0)) {
	    *p++ = *q;
	} else {
	    in--;
	    if (p >= pend)
		lost++;
	}
    }
    if (p > d->raw_input->cmd) {
	d->raw_input_at = p;
    } else {
	free_lbuf(d->raw_input);
	d->raw_input = NULL;
	d->raw_input_at = NULL;
    }
    d->input_tot += got;
    d->input_size += in;
    d->input_lost += lost;
    mudstate.debug_cmd = cmdsave;
    return 1;
}

void close_sockets(emergency, message)
int emergency;
char *message;
{
    DESC *d, *dnext;

    DESC_SAFEITER_ALL(d, dnext) {
	if (emergency) {

	    WRITE(d->descriptor, message, strlen(message));
	    if (shutdown(d->descriptor, 2) < 0)
		log_perror("NET", "FAIL", NULL, "shutdown");
	    close(d->descriptor);
	} else {
	    queue_string(d, message);
	    queue_write(d, "\r\n", 2);
	    shutdownsock(d, R_GOING_DOWN);
	}
    }
    close(sock);
}

void NDECL(emergency_shutdown)
{
    close_sockets(1, (char *) "Going down - Bye");
}


/*
 * ---------------------------------------------------------------------------
 * * Signal handling routines.
 */

#ifndef SIGCHLD
#define SIGCHLD SIGCLD
#endif

#ifdef HAVE_STRUCT_SIGCONTEXT
static RETSIGTYPE sighandler(int, int, struct sigcontext);
#else
static RETSIGTYPE sighandler(int);
#endif

/* *INDENT-OFF* */

NAMETAB sigactions_nametab[] = {
{(char *)"exit",	3,	0,	SA_EXIT},
{(char *)"default",	1,	0,	SA_DFLT},
{ NULL,			0,	0,	0}};

/* *INDENT-ON* */








void NDECL(set_signals)
{
    signal(SIGALRM, sighandler);
    signal(SIGCHLD, sighandler);
    signal(SIGHUP, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGUSR1, sighandler);
    signal(SIGUSR2, sighandler);
    signal(SIGTRAP, sighandler);
#ifdef SIGXCPU
    signal(SIGXCPU, sighandler);
#endif

    signal(SIGILL, sighandler);
#ifdef __linux__
    signal(SIGFPE, SIG_IGN);
#else
    signal(SIGFPE, sighandler);
#endif
    signal(SIGSEGV, sighandler);
    signal(SIGABRT, sighandler);
#ifdef SIGFSZ
    signal(SIGXFSZ, sighandler);
#endif
#ifdef SIGEMT
    signal(SIGEMT, sighandler);
#endif
#ifdef SIGBUS
    signal(SIGBUS, sighandler);
#endif
#ifdef SIGSYS
    signal(SIGSYS, sighandler);
#endif

}

static void unset_signals()
{
    int i;

    for (i = 0; i < NSIG; i++)
	signal(i, SIG_DFL);
    abort();
}

static void check_panicking(sig)
int sig;
{
    int i;

    /*
     * If we are panicking, turn off signal catching and resignal 
     */

    if (mudstate.panicking) {
	for (i = 0; i < NSIG; i++)
	    signal(i, SIG_DFL);
	kill(getpid(), sig);
    }
    mudstate.panicking = 1;
}

void log_signal(signame)
const char *signame;
{
    STARTLOG(LOG_PROBLEMS, "SIG", "CATCH") {
	log_text((char *) "Caught signal ");
	log_text((char *) signame);
	ENDLOG;
    }
}


void log_commands(int sig)
{
}

#ifdef HAVE_STRUCT_SIGCONTEXT
static RETSIGTYPE sighandler(sig, code, scp)
int sig;
int code;
struct sigcontext *scp;

#else
static RETSIGTYPE sighandler(sig)
int sig;

#endif
{
#ifdef SYS_SIGLIST_DECLARED
#define signames sys_siglist
#else
    static const char *signames[] = {
	"SIGZERO", "SIGHUP", "SIGINT", "SIGQUIT",
	"SIGILL", "SIGTRAP", "SIGABRT", "SIGEMT",
	"SIGFPE", "SIGKILL", "SIGBUS", "SIGSEGV",
	"SIGSYS", "SIGPIPE", "SIGALRM", "SIGTERM",
	"SIGURG", "SIGSTOP", "SIGTSTP", "SIGCONT",
	"SIGCHLD", "SIGTTIN", "SIGTTOU", "SIGIO",
	"SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF",
	"SIGWINCH", "SIGLOST", "SIGUSR1", "SIGUSR2"
    };

#endif

    char buff[32];

#ifdef HAVE_UNION_WAIT
    union wait stat = {0};

#else
    int stat;

#endif

    switch (sig) {
    case SIGUSR1:
	do_restart(1, 1, 0);
	break;
    case SIGALRM:		/*
				 * Timer 
				 */
	mudstate.alarm_triggered = 1;
	break;
    case SIGCHLD:		/*
				 * Change in child status 
				 */
#ifndef SIGNAL_SIGCHLD_BRAINDAMAGE
	signal(SIGCHLD, sighandler);
#endif
#ifdef HAVE_WAIT3
	while (wait3(&stat, WNOHANG, NULL) > 0);
#else
	wait(&stat);
#endif
	/* Did the child exit? */

	if (WEXITSTATUS(stat) == 8)
	    exit(0);

	mudstate.dumping = 0;
	break;
    case SIGHUP:		/*
				 * Perform a database dump 
				 */
	log_signal(signames[sig]);
	mudstate.dump_counter = 0;
	break;
    case SIGINT:		/*
				 * Log + ignore 
				 */
	log_signal(signames[sig]);
	break;
    case SIGQUIT:		/*
				 * Normal shutdown 
				 */
    case SIGTERM:
#ifdef SIGXCPU
    case SIGXCPU:
#endif
	check_panicking(sig);
	log_signal(signames[sig]);
	log_commands(sig);
	sprintf(buff, "Caught signal %s, exiting.", signames[sig]);
	raw_broadcast(0, buff);
	dump_database_internal(DUMP_KILLED);
	exit(0);
	break;
    case SIGILL:		/*
				 * Panic save + restart 
				 */
    case SIGFPE:
    case SIGSEGV:
    case SIGTRAP:
#ifdef SIGXFSZ
    case SIGXFSZ:
#endif
#ifdef SIGEMT
    case SIGEMT:
#endif
#ifdef SIGBUS
    case SIGBUS:
#endif
#ifdef SIGSYS
    case SIGSYS:
#endif
	check_panicking(sig);
	log_signal(signames[sig]);
	report();
	if (mudconf.sig_action != SA_EXIT) {
	    char outdb[128];
	    char indb[128];

	    log_commands(sig);
	    raw_broadcast(0,
		"Game: Fatal signal %s caught, restarting with previous database.",
		signames[sig]);

	    /* Don't sync first. Using older db. */

	    CLOSE;
	    dump_database_internal(DUMP_CRASHED);
	    shutdown(slave_socket, 2);
	    kill(slave_pid, SIGKILL);
#ifdef SQL_SUPPORT
#ifndef NO_SQLSLAVE
	    shutdown(sqlslave_socket, 2);
	    kill(sqlslave_pid, SIGKILL);
#endif
#endif

#if ARBITRARY_LOGFILES_MODE==2
            shutdown(fileslave_socket, 2);
            kill(fileslave_pid, SIGKILL);
#endif
	    /*
	     * Try our best to dump a core first 
	     */
	    if (!fork()) {
		unset_signals();
		exit(1);
	    }
	    if (mudconf.compress_db) {
		sprintf(outdb, "%s.Z", mudconf.outdb);
		sprintf(indb, "%s.Z", mudconf.indb);
		rename(outdb, indb);
	    } else {
		rename(mudconf.outdb, mudconf.indb);
	    }
	    if (mudconf.have_specials)
		ChangeSpecialObjects(0);
	    alarm(0);
	    dump_restart_db();
	    execl("bin/netmux", "netmux", mudconf.config_file, NULL);
	    break;
	} else {
	    unset_signals();
	    signal(sig, SIG_DFL);
	    exit(1);
	}
    case SIGABRT:		/*
				 * Coredump. 
				 */
	check_panicking(sig);
	log_signal(signames[sig]);
	report();

	unset_signals();
	signal(sig, SIG_DFL);
	exit(1);

    }
    signal(sig, sighandler);
    mudstate.panicking = 0;
#ifdef VMS
    return 1;
#else
    return;
#endif
}

