#include "config.h"

#include "sqlslave.h"

#include <dbi/dbi.h>
#include <sys/poll.h>
#include <errno.h>
#include <signal.h>

static dbi_conn conn;
static dbi_result dbiresult;

static dbref trig_obj;
static int trig_attr;
static char dbtype[SQL_OPTION_MAX + 1];
static char host[SQL_OPTION_MAX + 1];
static char username[SQL_OPTION_MAX + 1];
static char password[SQL_OPTION_MAX + 1];
static char dbname[SQL_OPTION_MAX + 1];
static char unix_socket[SQLQUERY_MAX_STRING + 1];
static char port[SQL_OPTION_MAX + 1];
static char sqlquery[SQLQUERY_MAX_STRING + 1];
static char preserve[SQLQUERY_MAX_STRING + 1];
static char response[SQLQUERY_MAX_STRING + 1];
static char result[SQLQUERY_MAX_STRING + 1];

pid_t parent_pid;

static char log_buffer[SQLQUERY_MAX_STRING];
int query_number = 0;

int log_sqlslave(char *mesg) {

    FILE *logfile;
    struct tm *tp;
    time_t now;

    /* Open Log File */
    logfile = fopen("sqlslave.log", "a");
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

int ingest_packet()
{
    static int len;

    snprintf(log_buffer, SQLQUERY_MAX_STRING, "Ingesting Packet for Query # %d",
            query_number);
    log_sqlslave(log_buffer);

    memset(dbtype, '\0', SQL_OPTION_MAX + 1);
    memset(host, '\0', SQL_OPTION_MAX + 1);
    memset(username, '\0', SQL_OPTION_MAX + 1);
    memset(password, '\0', SQL_OPTION_MAX + 1);
    memset(dbname, '\0', SQL_OPTION_MAX + 1);
    memset(port, '\0', SQL_OPTION_MAX + 1);
    memset(unix_socket, '\0', SQLQUERY_MAX_STRING + 1);
    memset(preserve, '\0', SQLQUERY_MAX_STRING + 1);
    memset(sqlquery, '\0', SQLQUERY_MAX_STRING + 1);

    if (read(STDIN_FILENO, &trig_obj, sizeof(dbref)) <= 0)
        return -2;

    if (read(STDIN_FILENO, &trig_attr, sizeof(int)) <= 0)
        return -4;

    if (read(STDIN_FILENO, &len, sizeof(int)) <= 0)
        return -5;
    if (len > 0)
        if (read(STDIN_FILENO, dbtype, len) <= 0)
	        return -6;

    if (read(STDIN_FILENO, &len, sizeof(int)) <= 0)
        return -7;
    if (len > 0)
        if (read(STDIN_FILENO, host, len) <= 0)
	        return -8;

    if (read(STDIN_FILENO, &len, sizeof(int)) <= 0)
        return -9;
    if (len > 0)
        if (read(STDIN_FILENO, username, len) <= 0)
	        return -10;

    if (read(STDIN_FILENO, &len, sizeof(int)) <= 0)
        return -11;
    if (len > 0)
        if (read(STDIN_FILENO, password, len) <= 0)
	        return -12;

    if (read(STDIN_FILENO, &len, sizeof(int)) <= 0)
        return -13;
    if (len > 0)
        if (read(STDIN_FILENO, dbname, len) <= 0)
	        return -14;

    if (read(STDIN_FILENO, &len, sizeof(int)) <= 0)
        return -15;
    if (len > 0)
        if (read(STDIN_FILENO, unix_socket, len) <= 0)
	        return -16;

    if (read(STDIN_FILENO, &len, sizeof(int)) <= 0)
        return -17;
    if (len > 0)
        if (read(STDIN_FILENO, port, len) <= 0)
	        return -18;

    if (read(STDIN_FILENO, &len, sizeof(int)) <= 0)
        return -19;
    if (len > 0)
        if (read(STDIN_FILENO, preserve, len) <= 0)
	        return -20;

    if (read(STDIN_FILENO, &len, sizeof(int)) <= 0)
        return -21;
    if (len > 0)
        if (read(STDIN_FILENO, sqlquery, len) <= 0)
	        return -22;

    snprintf(log_buffer, SQLQUERY_MAX_STRING, "Packet Ingested for Query # %d",
            query_number);
    log_sqlslave(log_buffer);

    return 0;
}

int output_response(char *resp, char *res)
{
    static int len, len_write;

    if ((len_write = write(STDOUT_FILENO, &trig_obj, sizeof(dbref))) <= 0)
        return -2;

    if ((len_write = write(STDOUT_FILENO, &trig_attr, sizeof(int))) <= 0)
        return -4;

    len = strlen(resp);
    if ((len_write = write(STDOUT_FILENO, &len, sizeof(int))) <= 0)
        return -5;

    if (len > 0)
        if ((len_write = write(STDOUT_FILENO, resp, len)) <= 0)
	        return -6;

    if (res == NULL) {
        len = 0;
        if ((len_write = write(STDOUT_FILENO, &len, sizeof(int))) <= 0)
	        return -7;
    } else {
	    len = strlen(res);
	    if ((len_write = write(STDOUT_FILENO, &len, sizeof(int))) <= 0)
	        return -8;
	    if (len > 0)
	        if ((len_write = write(STDOUT_FILENO, res, len)) <= 0)
	            return -9;
	}

    len = strlen(preserve);
    if ((len_write = write(STDOUT_FILENO, &len, sizeof(int))) <= 0)
        return -10;
    if (len > 0)
        if ((len_write = write(STDOUT_FILENO, preserve, len)) <= 0)
	        return -11;

    return 0;
}

int make_connection()
{

    if (strcmp(dbtype, "mysql") != 0)
        return -1;
    if ((conn = dbi_conn_new(dbtype)) == NULL)
        return -2;

    dbi_conn_set_option(conn, "host", host);
    dbi_conn_set_option(conn, "username", username);
    dbi_conn_set_option(conn, "password", password);
    dbi_conn_set_option(conn, "dbname", dbname);
    dbi_conn_set_option(conn, "mysql_unix_socket", unix_socket);
    dbi_conn_set_option(conn, "port", port);

    if (dbi_conn_connect(conn) != 0)
        return -3;

    snprintf(log_buffer, SQLQUERY_MAX_STRING, "Connecting to DB %s %s %s %s %s %s",
        host, username, password, dbname, unix_socket, port);
    log_sqlslave(log_buffer);

    return 0;
}

int spew_results()
{
static unsigned int rows, fields, i, ii, type, writ, tmp;
static long long field_int;
static double field_dec;
static const char *field_str;
static time_t field_time;

rows = dbi_result_get_numrows(dbiresult);
fields = dbi_result_get_numfields(dbiresult);

memset(response, '\0', SQLQUERY_MAX_STRING + 1);
memset(result, '\0', SQLQUERY_MAX_STRING + 1);

if (rows == 0 || fields == 0) {
    const char *tmp;

    if (dbi_conn_error(conn, &tmp) != -1 ) 
	snprintf(response, SQLQUERY_MAX_STRING, "Success");
    else
	snprintf(response, SQLQUERY_MAX_STRING, tmp);
    return 0;
    } else {
	snprintf(response, SQLQUERY_MAX_STRING, "Success");
    }

writ = tmp = 0;
for (i = 1; i <= rows; ++i) {
   if (dbi_result_seek_row(dbiresult, i) == 0)
	break;
   for (ii = 1; ii <= fields; ii++) {
	switch (type = dbi_result_get_field_type_idx(dbiresult, ii)) {
	    case DBI_TYPE_INTEGER:
		field_int = dbi_result_get_longlong_idx(dbiresult, ii);
		tmp = snprintf(result + writ, SQLQUERY_MAX_STRING - writ, (ii == fields ? "%lld" : "%lld:"), field_int);
		if (tmp < 0)
		    return -1;
		writ += tmp;
		break;
	    case DBI_TYPE_DECIMAL:
		field_dec = dbi_result_get_double_idx(dbiresult, ii);
		tmp = snprintf(result + writ, SQLQUERY_MAX_STRING - writ, (ii == fields ? "%f" : "%f:"), field_dec);
		if (tmp < 0)
		    return -1;
		writ += tmp;
		break;
	    case DBI_TYPE_STRING:
		field_str = dbi_result_get_string_idx(dbiresult, ii);
		tmp = snprintf(result + writ, SQLQUERY_MAX_STRING - writ, (ii == fields ? "%s" : "%s:"), field_str);
		if (tmp < 0)
		    return -1;
		writ += tmp;
		break;
	    case DBI_TYPE_BINARY:
		{
		int len;
		const unsigned char *bindata; /* Sorry. MUX we can only pump out text anyhow, might as well convert to it */
		char tempdata[SQLQUERY_MAX_STRING];

		len = dbi_result_get_field_size_idx(dbiresult, ii);
		bindata = dbi_result_get_binary_idx(dbiresult, ii);
		if (len > SQLQUERY_MAX_STRING - 1)
		    len = SQLQUERY_MAX_STRING - 1;
		memset(tempdata, '\0', SQLQUERY_MAX_STRING);
		memcpy(tempdata, bindata, len);

		tmp = snprintf(result + writ, SQLQUERY_MAX_STRING - writ, (ii == fields ? "%s" : "%s:"), tempdata);
		if (tmp < 0)
		    return -1;
		writ += tmp;
		}
		break;
	    case DBI_TYPE_DATETIME:
		{
		char timetmp[50];

		memset(timetmp, '\0', 50);
		field_time = dbi_result_get_datetime_idx(dbiresult, ii);
		snprintf(timetmp, 50, "%d", (int) field_time);
/* timetmp[strlen(timetmp) - 1] = '\0'; */
		tmp = snprintf(result + writ, SQLQUERY_MAX_STRING - writ, (ii == fields ? "%s" : "%s:"), timetmp);
		if (tmp < 0)
		    return -1;
		writ += tmp;
		}
		break;
	    }
	}
    if (i != rows) {
	tmp = snprintf(result + writ, SQLQUERY_MAX_STRING - writ, "|");
	writ += tmp;
	}
}
return 0;
}

void call_query()
{
    static int err;

    query_number++;
    if (query_number > 1000) {
        query_number = 1;
        log_sqlslave("Reset Query Count");
    }
    snprintf(log_buffer, SQLQUERY_MAX_STRING, "Received a Query : Designating as Query # %d",
            query_number);
    log_sqlslave(log_buffer);

    if (ingest_packet() < 0)
        return;
    if ((err = make_connection()) < 0) {
        switch (err) {
	        case -1:
	            output_response("Only MySQL is supported right now due "
                    "to lazy driver access", NULL);
	            break;
	        case -2:
	        case -3:
	        {
	            const char *tmp;
	            err = dbi_conn_error(conn, &tmp);

                snprintf(log_buffer, SQLQUERY_MAX_STRING, "Error Connecting for Query"
                    " # %d-> %s", query_number, tmp);
                log_sqlslave(log_buffer);

	            snprintf(response, SQLQUERY_MAX_STRING, "%s", tmp);

	            if (tmp != NULL)
		            output_response(response, NULL);
	            else
		            output_response("Unknown error while connecting", NULL);
	        }
	    }
    return;
    }

    snprintf(log_buffer, SQLQUERY_MAX_STRING, "Query # %d -> %s",
        query_number, sqlquery);
    log_sqlslave(log_buffer);

    dbiresult = dbi_conn_query(conn, sqlquery);
    if (dbiresult == NULL) {
        const char *tmp;

        if ((err = dbi_conn_error(conn, &tmp)) != -1) {
            snprintf(log_buffer, SQLQUERY_MAX_STRING, "Sending Query Error for Query"
                    " # %d -> %s : %s", query_number, sqlquery, tmp);
            log_sqlslave(log_buffer);
	        snprintf(response, SQLQUERY_MAX_STRING, "%s", tmp);
	        output_response(response, NULL);
	    } else {
            snprintf(log_buffer, SQLQUERY_MAX_STRING, "Error Sending Query # %d -> %s : %s",
                query_number, sqlquery, tmp);
            log_sqlslave(log_buffer);
	        output_response("Error while sending query", NULL);
	    }
        dbi_conn_close(conn);
        return;
    }

    snprintf(log_buffer, SQLQUERY_MAX_STRING, "Creating Output for Query # %d -> %s",
            query_number, sqlquery);
    log_sqlslave(log_buffer);

    if (spew_results() != 0) {
        snprintf(log_buffer, SQLQUERY_MAX_STRING, "Error While Creating Output"
                " for Query # %d -> %s", query_number, sqlquery);
        log_sqlslave(log_buffer);
        output_response("Error while creating output", NULL);
        dbi_result_free(dbiresult);
        dbi_conn_close(conn);
        return;
    }

    snprintf(log_buffer, SQLQUERY_MAX_STRING, "Done with Query # %d -> %s",
            query_number, sqlquery);
    log_sqlslave(log_buffer);

    dbi_result_free(dbiresult);
    dbi_conn_close(conn);
    output_response(response, result);
    return;
}

void EZexit(int num)
{
    log_sqlslave("Exiting SQLSlave");
    dbi_shutdown();
    exit(num);
}

int main(argc, argv)
int argc;
char **argv;
{
    struct pollfd item;

    log_sqlslave("Started SQLSlave");
#ifdef SNPRINTF_LONGLONG_SUPPORT
    log_sqlslave("LONG LONG Support Enabled");
#endif

    parent_pid = getppid();
    if (parent_pid == 1)
        EZexit(1);

    signal(SIGPIPE, SIG_DFL);

    if (dbi_initialize(NULL) == -1)
        if (dbi_initialize("./bin") == -1)
	        EZexit(5);

    item.fd = STDIN_FILENO;
    item.events = (POLLIN|POLLERR|POLLHUP|POLLNVAL);
    item.revents = 0;

    while (1) {
        if (parent_pid != getppid())
	        EZexit(2);

        switch (poll(&item, 1, -1)) {
	        case -1:
	            if (errno == EINTR)
		            continue;
	            EZexit(3);
	        case 0:
	            continue;
	        default:
	            if (item.revents & (POLLERR|POLLNVAL|POLLHUP))
		            EZexit(4);
	            if (!(item.revents & POLLIN))
		            continue;
	            call_query();
	            continue;
        }
    }
    EZexit(0);
}
