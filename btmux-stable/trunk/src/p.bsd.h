
/*
   p.bsd.h

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at Thu Mar 11 17:43:39 CET 1999 from bsd.c */

#include "config.h"

#ifndef _P_BSD_H
#define _P_BSD_H

/* bsd.c */
void set_lastsite(DESC * d, char *lastsite);
void boot_slave(void);
int make_socket(int port);
void shovechars(int port);
DESC *new_connection(int sock);
void shutdownsock(DESC * d, int reason);
void make_nonblocking(int s);
DESC *initializesock(int s, struct sockaddr_in *a);
int process_output(DESC * d);
int fatal_bug(void);
int process_input(DESC * d);
void close_sockets(int emergency, char *message);
void emergency_shutdown(void);
void set_signals(void);
void log_signal(const char *signame);
void log_commands(int sig);
#ifdef SQL_SUPPORT
int *sqldb_slotinit(char db_slot);
char *sqldb_slotval(char db_slot, int which);
#ifdef NO_SQLSLAVE
void sql_doquery(char db_slot, dbref thing, int attr, char *pres, char *qry);
int sql_log(char *mesg);
int sql_output(dbref thing, int attr, char *resp, char *res, char *pres);
#else
static int get_fileslave_result();
void sqlslave_doquery(char db_slot, dbref thing, int attr, char *pres, char *qry);
void boot_sqlslave(void);
#endif
#endif
#if ARBITRARY_LOGFILES_MODE==2 
void fileslave_dolog(dbref thing, const char *fname, const char *fdata);
#endif
#endif				/* _P_BSD_H */
