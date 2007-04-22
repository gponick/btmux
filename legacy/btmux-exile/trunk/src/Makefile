# Generated automatically from Makefile.in by configure.
# Makefile for TinyMUX 1.2
#
# Search for the text 'CONFIGURATION SECTION' and make any changes needed
# there.

SHELL=/bin/sh
srcdir = .
BIN = ../game/bin

# CPP = gcc -E	# This is broken in autoconf.  Sigh.
CPP = $(CC) -E
# LIBS = -liberty -lbsd -lgdbm
#LIBS = -lbsd -lgdbm -L/usr/lib -L.. -Lhcode
LIBS = -lcrypt -lgdbm -L. -L/usr/lib -L.. -Lhcode -lhcode -lbtech -lhcode -L../tree -ltree -ldbi

# After we finish beta testing, we'll take away the debugging -g flag.
# If you don't use GCC, don't use -g. Add -pg for profiling (gprof netmux
# gmon.out)
#OPTIM = -O
#OPTIM = -g -O2
#OPTIM = -g -pg -O
#OPTIM = -g -O

# The first set of definitions is for disk-based caching. If you wish to be
# memory based, comment the first set and uncomment the second set. Being
# disk-based usually uses less memory, but more disk-space. Being memory
# entails using more memory, less disk-space, and some gain in performance.
BASED =
UDB_S = udb_ocache.c udb_ochunk.c udb_obj.c udb_misc.c
UDB_O = udb_ocache.o udb_ochunk.o udb_obj.o udb_misc.o
UDB_INC	= udb.h udb_defs.h

#BASED = -DMEMORY_BASED
#UDB_S =
#UDB_O =
#UDB_INC = 

# Uncomment these definitions to use radix tree string compression. This
# will result in less memory usage in both memory and disk based schemes, 
# and reduced disk space as well when disk based. You MUST carefully read 
# README.COMPRESSION and follow the instructions exactly before you define 
# these, or you may cause damage to your database.
# NOTE: You should have a fairly large database before you begin to use
#       radix compression. 
#COMPRESS_DEFS = -DRADIX_COMPRESSION 
#COMPRESS_LIB = -L./radixlib -lcompress

# If we want to use the port concentrator, comment out the first set and
# uncomment the second set of definitions. There is no need to use the
# concentrator unless you run into problems with too many players being
# online at once.
CONC_S =
CONC_O =
CONC_DEFS =

#CONC_S = newconc.c
#CONC_S = newconc.o
#CONC_DEFS = -DCONCENTRATE

# Malloc debugging options. The first set is with debugging off.
# If you wish to use malloc debugging, comment the first set and uncomment
# the second set. This should only be turned on if you're looking for memory
# leaks or corruption, as it slows down the MUX considerably.
MALLOC_S =
MALLOC_O =
MALLOC_DEFS =

#MALLOC_S = malloc.c mcheck.c mstats.c mtrace.c
#MALLOC_O = malloc.o mcheck.o mstats.o mtrace.o
#MALLOC_DEFS = -DMCHECK -DMTRACE_FILE=\"mtrace.out\"

# Standalone cache code.
SA_UDB_S = sa-udb_ocache.c sa-udb_ochunk.c sa-udb_obj.c sa-udb_misc.c
SA_UDB_O = sa-udb_ocache.o sa-udb_ochunk.o sa-udb_obj.o sa-udb_misc.o
SA_UDB_INC = udb.h udb_defs.h

# Source common to netmux and standalone code
COM_SRC	=
COM_OBJ	=

# Everything needed to use the database in standalone mode.
SA_SRC	= sa-db.c sa-db_rw.c sa-boolexp.c sa-unparse.c \
	  sa-pred.c sa-stringu.c sa-conf.c sa-log.c sa-object.c \
	  sa-vattr.c sa-htab.c sa-player_c.c sa-flags.c \
	  sa-udb_ocache.c sa-udb_ochunk.c sa-udb_obj.c sa-udb_misc.c
SA_OBJ	= sa-db.o sa-db_rw.o sa-boolexp.o sa-unparse.o \
	  sa-pred.o sa-stringu.o sa-conf.o sa-log.o sa-object.o \
	  sa-vattr.o sa-htab.o sa-player_c.o sa-flags.o

# Distribution source files
D_SRC	= create.c game.c help.c look.c match.c move.c player.c predicates.c \
	  rob.c set.c speech.c wiz.c walkdb.c timer.c boolexp.c log.c cque.c \
	  unparse.c eval.c command.c wild.c netcommon.c functions.c vattr.c \
	  db.c db_rw.c stringutil.c object.c conf.c flags.c htab.c \
	  compat.c file_c.c player_c.c bsd.c alloc.c commac.c comsys.c mail.c \
          macro.c mguests.c powers.c \
          funceval.c regexp.c strhandler.c
D_OBJ	= create.o game.o help.o look.o match.o move.o player.o predicates.o \
	  rob.o set.o speech.o wiz.o walkdb.o timer.o boolexp.o log.o cque.o \
	  unparse.o eval.o command.o wild.o netcommon.o functions.o vattr.o \
	  db.o db_rw.o stringutil.o object.o conf.o flags.o htab.o \
          compat.o file_c.o player_c.o bsd.o alloc.o comsys.o commac.o mail.o \
          macro.o mguests.o powers.o \
          funceval.o regexp.o strhandler.o
D_INC	= copyright.h flags.h help.h htab.h interface.h match.h functions.h \
	  command.h config.h db.h externs.h mudconf.h misc.h \
	  vattr.h file_c.h alloc.h attrs.h commac.h comsys.h macro.h \
          mguests.h powers.h slave.h regexp.h sqlslave.h fileslave.h

# Version number routine
VER_SRC	= version.c
VER_OBJ	= version.o
VER_INC	= patchlevel.h
VER_FLG	= -DMUX_BUILD_DATE="\"`date`\"" \
	  -DMUX_BUILD_NUM="\"`sh ./buildnum.sh`\""

# ===================== CONFIGURATION SECTION ====================
#
# Select the correct C compiler.  Whatever you choose, it must be able
# to grok ANSI C (function prototypes)
#
#-----CC or GCC (must be able to grok function prototypes)
#
DEFS =
#
#-----CC on a NeXT system, really weird derivative of GCC
#
#DEFS = -DNEXT -DNEED_STRDUP
#
#-----GCC if the libraries were built for a pcc-derived cc compiler
#     (most systems)
#
#DEFS = -fpcc-struct-return -Wall
#
#-----GCC with GCC-compatible libraries if you want verbose error messages
#
#DEFS = -Wall
#
#-----HP-UX C compiler
#
#DEFS = -w +Obb800 -Aa -D_INCLUDE_POSIX_SOURCE -D_INCLUDE_HPUX_SOURCE -D_INCLUDE_XOPEN_SOURCE
#
#-----MIPS C compiler (also DEC 3xxx, Prime EXL7xxx)
#
#DEFS = -signed
#
#-----Solaris.
#
#DEFS = -I/usr/ucbinclude -DSOLARIS
#

# Libraries.  Use the second line if you want to use the resolver to get
# hostnames and your libc doesn't use it already.  If you use it, you'd
# better have your nameserver working or things may hang for a while when
# people try to login from distant sites.  Use the third line if you're running
# on a SysV-ish system and BSD support isn't built in to the standard libc.
#
MORELIBS	= -lm
#MORELIBS	= -lm -lnsl -lsocket -L/usr/ucblib -lucb	# Mips
#MORELIBS	= -lm -L/usr/ucblib -lucb			# Solaris

# Select the networking support you have.  Currently only BSD networking
# is supported.
#
# ----- BSD networking
NET_S	= bsd.c
NET_O	= bsd.o

# ================== END OF CONFIGURATION SECTION =================

# Auxiliary source files: only used by offline utilities.
AUX_SRC	= dbconvert.c mkindx.c unsplit.c
LIB_SRC = 
LIB_INC = 
DEBUG	= $(OPTIM)

INCLUDES=-I../include -I. -Ihcode/include
ALLCFLAGS = $(CFLAGS) $(DEBUG) $(DEFS) $(COMPRESS_DEFS) $(BASED) $(MALLOC_DEFS) $(CONC_DEFS) $(MOREDEFS) $(RLIM_F) $(INCLUDES)

#Auxiliary files
A_INST	= README NOTES HIGHLIGHTS CREDITS CHANGES
A_TXT	= badsite.txt connect.txt create_reg.txt down.txt guest.txt help.txt \
	  motd.txt news.txt newuser.txt quit.txt register.txt wizhelp.txt \
	  wizmotd.txt full.txt plushelp.txt wiznews.txt
A_SH	= db_load db_unload db_check buildnum.sh
A_RT	= Startmux mux.config
A_CONF	= netmux.conf alias.conf compat.conf
A_MISC	= buildnum.data
A_VMS	= vms_dbm.c vms_dbm.h startmux.com compile.com link.com
A_AUTO	= configure configure.in autoconf.h.in Makefile.in
AUXIL	= $(A_INST) $(A_TXT) $(A_SH) $(A_CONF) $(A_MISC) $(A_RT) $(A_VMS) \
	  $(A_AUTO)

#Installation files
INSTALL_PROG	= Startmux netmux dbconvert mkindx db_load db_unload db_check $(R_SERV)
INSTALL_TARG	= $(INSTALL_PROG) *.txt *.conf
REINSTALL_TARG	= $(INSTALL_PROG) help.txt wizhelp.txt alias.conf compat.conf

#compiliation source files
ALLSRC	= $(D_SRC) $(UDB_SRC) $(UDBA_SRC) $(UDBO_SRC) $(VER_SRC) \
	  $(COM_SRC) $(SA_SRC) $(AUX_SRC) $(LIB_SRC)
ALLINC	= $(D_INC) $(UDB_INC) $(VER_INC) $(NET_INC) $(LIB_INC)
SRC	= $(D_SRC) $(COM_SRC) $(UDB_S) $(MALLOC_S) $(CONC_S)
OBJ	= $(D_OBJ) $(COM_OBJ) $(UDB_O) $(MALLOC_O) $(CONC_O)

# Files in the standard distribution
DIST	= $(ALLSRC) $(ALLINC) $(AUXIL)

OUT	= netmux mkindx dbconvert slave unsplit sqlslave fileslave

.c.o:
	$(CC) $(ALLCFLAGS) -c $<

# paths is likely to remain broken


ndall: $(OUT)


include ../mux.general

#conc: portconc.o
#	$(CC) $(ALLCFLAGS) $(LIBS) -o conc portconc.o

slave: slave.o
	$(CC) $(ALLCFLAGS) $(LIBS) -o slave slave.o

sqlslave: sqlslave.o
	$(CC) $(ALLCFLAGS) $(LIBS) -o sqlslave sqlslave.o -L. -ldbi

fileslave: fileslave.o
	$(CC) $(ALLCFLAGS) $(LIBS) -o fileslave fileslave.o

#STATIC=--static
STATIC=

mkindx: mkindx.o
	$(CC) $(STATIC) $(ALLCFLAGS) -o mkindx mkindx.o

unsplit: unsplit.o
	$(CC) $(STATIC) $(ALLCFLAGS) -o unsplit unsplit.o

dbconvert: dbconvert.o $(SA_OBJ) $(SA_UDB_O) $(ALLOC_O)
	$(CC) $(STATIC) $(ALLCFLAGS) -o dbconvert dbconvert.o $(SA_OBJ) $(COM_OBJ) $(SA_UDB_O) $(ALLOC_O) $(LIBS) $(COMPRESS_LIB) $(MORELIBS)

saber:	$(SRC)
	#load $(SRC)

saber-o: $(OBJ)
	#load $(OBJ) $(VER_OBJ) $(LIBS) $(MORELIBS)

redo_libhcode:
	(cd hcode ; $(GNUMAKE))

netmux: $(OBJ) redo_libhcode
	$(CC) $(ALLCFLAGS) $(VER_FLG) -c $(VER_SRC)
	-mv -f netmux netmux~
	$(CC) $(STATIC) $(ALLCFLAGS) -o netmux $(OBJ) $(VER_OBJ) $(EVENTOBJ) $(MYFIFOOBJ) $(LIBS) $(COMPRESS_LIB) $(MORELIBS)

# add -lfantasy if FT_ENABLED

index: help.indx news.indx wizhelp.indx

help.indx: help.txt
	./mkindx help.txt help.indx

news.indx: news.txt
	./mkindx news.txt news.indx

wizhelp.indx: wizhelp.txt
	./mkindx wizhelp.txt wizhelp.indx

verify:
	runtest --tool mux --src ./testsuite

realclean:
	-rm -f *.o a.out core gmon.out mux.*log mux.*sum $(OUT) netmux~

clean:
	-rm -f *.o a.out core gmon.out mux.*log mux.*sum sqlslave fileslave slave conc $(OUT)
	(cd hcode ; make clean)

indent:
	indent -bad -bap -cdb -nsob -fca -sc -br -ce -ss -npcs -nbc -lp -i8 -ip0 -cp5 -ncs -npsl -di1 **/*.c

