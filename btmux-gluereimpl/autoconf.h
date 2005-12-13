/* autoconf.h.  Generated by configure.  */
/* autoconf.h.in -- System-dependent configuration information */

#ifndef AUTOCONF_H
#define AUTOCONF_H

#include "copyright.h"

/* ---------------------------------------------------------------------------
 * Configuration section:
 *
 * These defines are written by the configure script.
 * Change them if need be
 */

/* Define if we have stdlib.h et al */
#define STDC_HEADERS 1
/* Define if we have unistd.h */
#define HAVE_UNISTD_H 1
/* Define if we have memory.h and need it to get memcmp et al */
/* #undef NEED_MEMORY_H */
/* Decl for pid_t */
/* #undef pid_t */
/* signal() return type */
#define RETSIGTYPE void
/* Define if struct tm is not in time.h */
/* #undef TM_IN_SYS_TIME */
/* Define if struct tm has a timezone member */
#define HAVE_TM_ZONE 1
/* Define if tzname[] exists */
/* #undef HAVE_TZNAME */
/* Define if setrlimit exists */
#define HAVE_SETRLIMIT 1
/* Define if getrusage exists */
#define HAVE_GETRUSAGE 1
/* Define if getdtablesize exists */
#define HAVE_GETDTABLESIZE 1
/* Define if getpagesize exists */
#define HAVE_GETPAGESIZE 1
/* Define if gettimeofday exists */
#define HAVE_GETTIMEOFDAY 1
/* Define if usleep exists */
#define HAVE_USLEEP 1
/* Define if nanosleep exists */
#define HAVE_NANOSLEEP 1
/* Define if setitimer exists */
#define HAVE_SETITIMER 1
/* Define if sys_siglist[] exists */
/* #undef SYS_SIGLIST_DECLARED */
/* Define if sys_signame[] exists */
/* #undef HAVE_SYS_SIGNAME */
/* Define if index/rindex/mem??? are defined in string.h */
/* #undef INDEX_IN_STRING_H */
/* Define if malloc/realloc/free are defined in stdlib.h */
#define MALLOC_IN_STDLIB_H 1
/* Define if calling signal with SIGCHLD when handling SIGCHLD blows chow */
/* #undef SIGNAL_SIGCHLD_BRAINDAMAGE */
/* Define if errno.h exists */
#define HAVE_ERRNO_H 1
/* Define if malloc.h exists */
#define HAVE_MALLOC_H 1
/* Define if sys/wait.h exists */
#define HAVE_SYS_WAIT_H 1
/* Define if sys/select.h exists */
#define HAVE_SYS_SELECT_H 1
/* Define if sys/rusage.h exists */
/* #undef HAVE_SYS_RUSAGE_H */
/* Define if Big Endian */
/* #undef WORDS_BIGENDIAN */
/* Define if Little Endian */
#define WORDS_LITTLEENDIAN 1
/* Define if Unknown Endian */
/* #undef WORDS_UNKNOWN */
/* Define if const is broken */
/* #undef const */
/* sizeof(short) */
#define SIZEOF_SHORT 2
/* sizeof(unsigned short) */
#define SIZEOF_UNSIGNED_SHORT 2
/* sizeof(int) */
#define SIZEOF_INT 4
/* sizeof(unsigned int) */
#define SIZEOF_UNSIGNED_INT 4
/* sizeof(long) */
#define SIZEOF_LONG 4
/* sizeof(unsigned long) */
#define SIZEOF_UNSIGNED_LONG 4
/* Define if unaligned short access is allowed. */
#define CAN_UNALIGN_SHORT 1
/* Define if unaligned int access is allowed. */
#define CAN_UNALIGN_INT 1
/* Define if unaligned long access is allowed. */
#define CAN_UNALIGN_LONG 1
/* Define if unaligned long long access is allowed. */
#define CAN_UNALIGN_LONGLONG 1
/* Define if inline keyword is broken or nonstandard */
/* #undef inline */
/* Define if we need to redef index/bcopy et al to their SYSV counterparts */
/* #undef NEED_INDEX_DCL */
/* Define if we need to declare malloc et al */
/* #undef NEED_MALLOC_DCL */
/* Define if you need to declare vsprintf yourself */
/* #undef NEED_VSPRINTF_DCL */
/* Define if you need to declare sys_errlist yourself */
/* #undef NEED_SYS_ERRLIST_DCL */
/* Define if you need to declare _sys_errlist yourself */
/* #undef NEED_SYS__ERRLIST_DCL */
/* Define if you need to declare sprintf yourself */
/* #undef NEED_SPRINTF_DCL */
/* Define if you need to declare getrlimit yourself */
/* #undef NEED_GETRLIMIT_DCL */
/* Define if you need to declare getrusage yourself */
/* #undef NEED_GETRUSAGE_DCL */
/* Define if struct linger is defined */
#define HAVE_LINGER 1
/* Define if signal handlers have a struct sigcontext as their third arg */
#define HAVE_STRUCT_SIGCONTEXT 1
/* Define if stdio.h defines lots of extra functions */
#define EXTENDED_STDIO_DCLS 1
/* Define if sys/socket.h defines lots of extra functions */
#define EXTENDED_SOCKET_DCLS 1
/* Define if socklen_t is defined */
#define SOCKLEN_T_DCL 1
/* Define if we may safely include both time.h and sys/time.h */
#define TIME_WITH_SYS_TIME 1
/* Define if sys/time.h exists */
#define HAVE_SYS_TIME_H 1
/* Define if you need to declare gettimeofday yourself */
/* #undef NEED_GETTIMEOFDAY_DCL */
/* Define if you need to declare getpagesize yourself */
/* #undef NEED_GETPAGESIZE_DCL */
/* Define if you have IEEE floating-point formatted numbers */
#define HAVE_IEEE_FP_FORMAT 1
/* Define if your IEEE floating-point library can generate NaN */
#define HAVE_IEEE_FP_SNAN 1
/* Define if your platform computes the integer quotient as the smallest */
/* integer greater than or or equal to the algebraic quotient. For       */
/* example, -9/5 gives -1                                                */
#define SMALLEST_INT_GTE_NEG_QUOTIENT 1
/* Define if the character special file /dev/urandom is present */
#define HAVE_DEV_URANDOM 1
/* Define if fpu_config.h is available. */
#define HAVE_FPU_CONTROL_H 1
/* Define if ieeefp.h is available. */
/* #undef HAVE_IEEEFP_H */
/* Define if ieeefp.h is useable. */
/* #undef IEEEFP_H_USEABLE */
/* Define if fenv.h is available. */
#define HAVE_FENV_H 1
/* Define if fegetprec is available. */
/* #undef HAVE_FEGETPREC */
/* Define if fesetprec is available. */
/* #undef HAVE_FESETPREC */
/* Define if your system has the in_addr_t type */
#define HAVE_IN_ADDR_T 1
/* Define if fcntl.h exists */
#define HAVE_FCNTL_H 1
/* Define if crypt library exists */
#define HAVE_LIBCRYPT 1
/* Define if crypt exists */
#define HAVE_CRYPT 1
/* Define if pread exists */
#define HAVE_PREAD 1
/* Define if pwrite exists */
#define HAVE_PWRITE 1

/* ---------------------------------------------------------------------------
 * Setup section:
 *
 * Load system-dependent header files.
 */

#if !defined(STDC_HEADERS)
#error MUX requires standard headers.
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>

#ifdef NEED_MEMORY_H
#include <memory.h>
#endif

#include <string.h>

#ifdef NEED_INDEX_DCL
#define index           strchr
#define rindex          strrchr
#define bcopy(s,d,n)    memmove(d,s,n)
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#if defined(HAVE_SETRLIMIT) || defined(HAVE_GETRUSAGE)
#include <sys/resource.h>
#ifdef NEED_GETRUSAGE_DCL
extern int      getrusage(int, struct rusage *);
#endif
#ifdef NEED_GETRLIMIT_DCL
extern int      getrlimit(int, struct rlimit *);
extern int      setrlimit(int, struct rlimit *);
#endif
#endif

#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_GETTIMEOFDAY
#ifdef NEED_GETTIMEOFDAY_DCL
extern int gettimeofday(struct timeval *, struct timezone *);
#endif
#endif

#ifdef HAVE_GETDTABLESIZE
extern int getdtablesize(void);
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_GETPAGESIZE

#ifdef NEED_GETPAGESIZE_DECL
extern int getpagesize(void);
#endif // NEED_GETPAGESIZE_DECL

#else // HAVE_GETPAGESIZE

#ifdef _SC_PAGESIZE
#define getpagesize() sysconf(_SC_PAGESIZE)
#else // _SC_PAGESIZE

#include <sys/param.h>

#ifdef EXEC_PAGESIZE
#define getpagesize() EXEC_PAGESIZE
#else // EXEC_PAGESIZE
#ifdef NBPG
#ifndef CLSIZE
#define CLSIZE 1
#endif // CLSIZE
#define getpagesize() NBPG * CLSIZE
#else // NBPG
#ifdef PAGESIZE
#define getpagesize() PAGESIZE
#else // PAGESIZE
#ifdef NBPC
#define getpagesize() NBPC
#else // NBPC
#define getpagesize() 0
#endif // NBPC
#endif // PAGESIZE
#endif // NBPG
#endif // EXEC_PAGESIZE

#endif // _SC_PAGESIZE
#endif // HAVE_GETPAGESIZE

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

// Assure that malloc, realloc, and free are defined.
//
#if !defined(MALLOC_IN_STDLIB_H)
#if   defined(HAVE_MALLOC_H)
#include <malloc.h>
#elif defined(NEED_MALLOC_DCL)
extern char *malloc(int);
extern char *realloc(char *, int);
extern int   free(char *);
#endif
#endif

#ifdef NEED_SYS_ERRLIST_DCL
extern char *sys_errlist[];
#endif

#include <sys/types.h>
#include <stdio.h>
#include <sys/fcntl.h>

#ifdef NEED_SPRINTF_DCL
extern char *sprintf(char *, const char *, ...);
#endif

#ifndef EXTENDED_STDIO_DCLS
extern int    fprintf(FILE *, const char *, ...);
extern int    printf(const char *, ...);
extern int    sscanf(const char *, const char *, ...);
extern int    close(int);
extern int    fclose(FILE *);
extern int    fflush(FILE *);
extern int    fgetc(FILE *);
extern int    fputc(int, FILE *);
extern int    fputs(const char *, FILE *);
extern int    fread(void *, size_t, size_t, FILE *);
extern int    fseek(FILE *, long, int);
extern int    fwrite(void *, size_t, size_t, FILE *);
extern pid_t  getpid(void);
extern int    pclose(FILE *);
extern int    rename(char *, char *);
extern time_t time(time_t *);
extern int    ungetc(int, FILE *);
extern int    unlink(const char *);
#endif

#include <sys/socket.h>
#ifndef EXTENDED_SOCKET_DCLS
extern int    accept(int, struct sockaddr *, int *);
extern int    bind(int, struct sockaddr *, int);
extern int    listen(int, int);
extern int    sendto(int, void *, int, unsigned int,
                    struct sockaddr *, int);
extern int    setsockopt(int, int, int, void *, int);
extern int    shutdown(int, int);
extern int    socket(int, int, int);
extern int    select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
#endif

typedef int   dbref;
typedef int   FLAG;
typedef int   POWER;
typedef char  boolexp_type;
typedef char  IBUF[16];

#endif /* AUTOCONF_H */
