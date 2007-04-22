# Makefile for TinyMUX 2.4
#
# Search for the text 'CONFIGURATION SECTION' and make any changes needed
# there.

SHELL=/bin/sh
srcdir = .

BIN = ../game/bin
CC = gcc
CXX = g++
#CXXCPP = g++ -E	# This is broken in autoconf.  Sigh.
CXXCPP = $(CXX) -E
LIBS = -lm -lnsl -lresolv -lcrypt -lpython2.4

.SUFFIXES: .cpp

# After we finish beta testing, we'll take away the debugging -g flag.
# If you don't use GCC, don't use -g. Add -pg for profiling (gprof netmux
# gmon.out)
#
#OPTIM = -O
#OPTIM = -g -pg -O
OPTIM = -g -O -Werror 

# By default, REALITY_LVLS is not enabled.  If you wish to use REALITY_LVLS,
# run configure with --enable-realitylvls.  Please read the docs/REALITY.
#
# NOTE: To ensure a clean build, please 'make clean' first.
#
REALITY_LVLS = 
REALITY_SRC = 
REALITY_OBJ = 

# By default, WOD_REALMS is not enabled.  If you wish to use WOD_REALMS,
# run configure with --enable-wodrealms.  Please read the docs/REALMS.
#
# NOTE: To ensure a clean build, please 'make clean' first.
#
WOD_REALMS = 

# Disk-based caching is the default.  If you wish to use memory-based, run
# configure with --enable-memorybased.  Disk-based usually uses less memory,
# but more disk space.  Memory-based usually uses more memory, less
# disk-space, and some gain in performance for a very narrow set of
# operations.
#
# NOTE: To ensure a clean build, please 'make clean' first.
#
MEMORY_BASED = 

# Query Slave. By default, this is off. This can be turned on by running
# configure with --enable-sqlquery. The Query Slave is how to support
# queries to foriegn data sources such as SQL servers.
#
QUERY_SLAVE = 

# Distribution source files
D_SRC	= _build.cpp alloc.cpp attrcache.cpp boolexp.cpp bsd.cpp command.cpp \
	comsys.cpp conf.cpp cque.cpp create.cpp db.cpp db_rw.cpp eval.cpp \
	file_c.cpp flags.cpp funceval.cpp functions.cpp funmath.cpp game.cpp \
	help.cpp htab.cpp local.cpp log.cpp look.cpp mail.cpp match.cpp \
	mguests.cpp move.cpp muxcli.cpp netcommon.cpp object.cpp \
	predicates.cpp player.cpp player_c.cpp powers.cpp quota.cpp rob.cpp \
	pcre.cpp set.cpp sha1.cpp speech.cpp sqlshared.cpp stringutil.cpp \
	strtod.cpp svdrand.cpp svdhash.cpp svdreport.cpp timer.cpp \
	timeutil.cpp unparse.cpp vattr.cpp walkdb.cpp wild.cpp wiz.cpp
D_OBJ	= _build.o alloc.o attrcache.o boolexp.o bsd.o command.o comsys.o \
	conf.o cque.o create.o db.o db_rw.o eval.o file_c.o flags.o \
	funceval.o functions.o funmath.o game.o help.o htab.o local.o log.o \
	look.o mail.o match.o mguests.o move.o muxcli.o netcommon.o object.o \
	predicates.o player.o player_c.o powers.o quota.o rob.o pcre.o set.o \
	sha1.o speech.o sqlshared.o stringutil.o strtod.o svdrand.o \
	svdhash.o svdreport.o timer.o timeutil.o unparse.o vattr.o walkdb.o \
	wild.o wiz.o

B_SRC	= python.cpp btech/coolmenu.cpp btech/debug.cpp btech/events.cpp btech/glue.hcode.cpp \
	btech/glue.cpp btech/glue.scode.cpp btech/mech.lostracer.cpp btech/spath.cpp \
	btech/event.cpp btech/myfifo.cpp btech/mux_tree.cpp \
	btech/tree.cpp btech/mechfile.cpp btech/aero.bomb.cpp btech/aero.move.cpp \
	btech/artillery.cpp btech/autopilot_ai.cpp btech/autopilot_autogun.cpp \
	btech/autopilot_commands.cpp btech/autopilot_core.cpp btech/autopilot_radio.cpp \
	btech/bsuit.cpp btech/btech.cpp btech/btechstats.cpp \
	btech/btspath.cpp btech/crit.cpp btech/ds.bay.cpp \
	btech/ds.turret.cpp btech/econ.cpp btech/econ_cmds.cpp \
	btech/eject.cpp btech/failures.cpp btech/hudinfo.cpp \
	btech/map.bits.cpp btech/map.coding.cpp btech/map.conditions.cpp \
	btech/map.dynamic.cpp btech/map.los.cpp btech/map.cpp \
	btech/map.obj.cpp btech/mech.advanced.cpp btech/mech.ammodump.cpp \
	btech/mech.avail.cpp btech/mech.bth.cpp btech/mech.build.cpp \
	btech/mech.c3.misc.cpp btech/mech.c3.cpp btech/mech.c3i.cpp \
	btech/mech.combat.misc.cpp btech/mech.combat.missile.cpp \
	btech/mech.combat.cpp btech/mech.consistency.cpp \
	btech/mech.contacts.cpp btech/mech.custom.cpp btech/mech.damage.cpp \
	btech/mech.ecm.cpp btech/mech.enhanced.criticals.cpp \
	btech/mech.events.cpp btech/mech.fire.cpp btech/mech.hitloc.cpp \
	btech/mech.ice.cpp btech/mech.lite.cpp btech/mech.los.cpp \
	btech/mech.maps.cpp btech/mech.mechref_ident.cpp \
	btech/mech.move.cpp btech/mech.notify.cpp btech/mech.ood.cpp \
	btech/mech.partnames.cpp btech/mech.physical.cpp \
	btech/mech.pickup.cpp btech/mech.restrict.cpp btech/mech.scan.cpp \
	btech/mech.sensor.functions.cpp btech/mech.sensor.cpp \
	btech/mech.spot.cpp btech/mech.startup.cpp btech/mech.tag.cpp \
	btech/mech.tech.commands.cpp btech/mech.tech.damages.cpp \
	btech/mech.tech.do.cpp btech/mech.tech.events.cpp \
	btech/mech.tech.cpp btech/mech.tech.repairs.cpp \
	btech/mech.tech.saverepair.cpp btech/mech.tic.cpp \
	btech/mech.update.cpp btech/mech.utils.cpp btech/mechrep.cpp \
	btech/mine.cpp btech/pcombat.cpp btech/scen.cpp \
	btech/template.cpp btech/mech.status.cpp btech/mech.stat.cpp \
	btech/rbtree.cpp btech/dllist.cpp

B_OBJ	= python.o btech/rbtree.o btech/coolmenu.o btech/debug.o btech/events.o btech/glue.hcode.o \
	btech/glue.o btech/glue.scode.o btech/mech.lostracer.o btech/spath.o \
	btech/event.o btech/myfifo.o btech/mux_tree.o \
	btech/tree.o btech/mechfile.o btech/aero.bomb.o btech/aero.move.o \
	btech/artillery.o btech/autopilot_ai.o btech/autopilot_autogun.o \
	btech/autopilot_commands.o btech/autopilot_core.o btech/autopilot_radio.o \
	btech/bsuit.o btech/btech.o btech/btechstats.o \
	btech/btspath.o btech/crit.o btech/ds.bay.o \
	btech/ds.turret.o btech/econ.o btech/econ_cmds.o \
	btech/eject.o btech/failures.o btech/hudinfo.o \
	btech/map.bits.o btech/map.coding.o btech/map.conditions.o \
	btech/map.dynamic.o btech/map.los.o btech/map.o \
	btech/map.obj.o btech/mech.advanced.o btech/mech.ammodump.o \
	btech/mech.avail.o btech/mech.bth.o btech/mech.build.o \
	btech/mech.c3.misc.o btech/mech.c3.o btech/mech.c3i.o \
	btech/mech.combat.misc.o btech/mech.combat.missile.o \
	btech/mech.combat.o btech/mech.consistency.o \
	btech/mech.contacts.o btech/mech.custom.o btech/mech.damage.o \
	btech/mech.ecm.o btech/mech.enhanced.criticals.o \
	btech/mech.events.o btech/mech.fire.o btech/mech.hitloc.o \
	btech/mech.ice.o btech/mech.lite.o btech/mech.los.o \
	btech/mech.maps.o btech/mech.mechref_ident.o \
	btech/mech.move.o btech/mech.notify.o btech/mech.ood.o \
	btech/mech.partnames.o btech/mech.physical.o \
	btech/mech.pickup.o btech/mech.restrict.o btech/mech.scan.o \
	btech/mech.sensor.functions.o btech/mech.sensor.o \
	btech/mech.spot.o btech/mech.startup.o btech/mech.tag.o \
	btech/mech.tech.commands.o btech/mech.tech.damages.o \
	btech/mech.tech.do.o btech/mech.tech.events.o \
	btech/mech.tech.o btech/mech.tech.repairs.o \
	btech/mech.tech.saverepair.o btech/mech.tic.o \
	btech/mech.update.o btech/mech.utils.o btech/mechrep.o \
	btech/mine.o btech/pcombat.o btech/scen.o \
	btech/template.o btech/mech.status.o btech/mech.stat.o \
	btech/dllist.o

# Version number routine
VER_SRC	= version.cpp
VER_OBJ	= version.o
VER_FLG	= -DMUX_BUILD_DATE="\"`date`\"" \
	  -DMUX_BUILD_NUM="\"`sh ./buildnum.sh`\""

# ===================== CONFIGURATION SECTION ====================
#
# Select the correct C compiler.  Whatever you choose, it must be able
# to grok ANSI C (function prototypes)
#
#-----CC or GCC (must be able to grok function prototypes)
#
DEFS = -fPIC -Ibtech -I. -I/usr/include/python2.4 
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

# Libraries.  Use the second line if you want to use the resolver to get
# hostnames and your libc doesn't use it already.  If you use it, you'd
# better have your nameserver working or things may hang for a while when
# people try to login from distant sites.  Use the third line if you're running
# on a SysV-ish system and BSD support isn't built in to the standard libc.
#
MORELIBS	= -lm
#MORELIBS	= -lm -lnsl -lsocket -L/usr/ucblib -lucb	# Mips

# Select the networking support you have.  Currently only BSD networking
# is supported.
#
# ----- BSD networking
NET_S	= bsd.cpp
NET_O	= bsd.o

# ================== END OF CONFIGURATION SECTION =================

# Auxiliary source files: only used by offline utilities.
#
AUX_SRC	= unsplit.cpp
LIB_SRC = 
LIB_INC = 
ALLCXXFLAGS = $(CXXFLAGS) $(OPTIM) $(DEFS) $(COMPRESS_DEFS) $(BASED) $(MALLOC_DEFS) $(MOREDEFS) $(RLIM_F) $(MEMORY_BASED) $(WOD_REALMS) $(REALITY_LVLS) $(QUERY_SLAVE)

#Auxiliary files
A_INST	= README NOTES HIGHLIGHTS CREDITS CHANGES
A_TXT	= badsite.txt connect.txt create_reg.txt down.txt guest.txt help.txt \
	  motd.txt news.txt newuser.txt quit.txt register.txt wizhelp.txt \
	  wizmotd.txt full.txt plushelp.txt wiznews.txt
A_SH	= db_load db_unload db_check buildnum.sh
A_RT	= Startmux mux.config
A_CONF	= netmux.conf alias.conf compat.conf
A_MISC	= buildnum.data
A_VMS	= vms_dbm.cpp vms_dbm.h startmux.com compile.com link.com
A_AUTO	= configure configure.in autoconf.h.in Makefile.in .depend
AUXIL	= $(A_INST) $(A_TXT) $(A_SH) $(A_CONF) $(A_MISC) $(A_RT) $(A_VMS) \
	  $(A_AUTO)

# Compiliation source files.
#
ALLSRC	= $(D_SRC) $(REALITY_SRC) $(VER_SRC) $(AUX_SRC) $(LIB_SRC) $(B_SRC)
SRC	= $(D_SRC) $(REALITY_SRC) $(MALLOC_S) $(B_SRC)
OBJ	= $(D_OBJ) $(REALITY_OBJ) $(MALLOC_O) $(B_OBJ)

.cpp.o:
	$(CXX) $(ALLCXXFLAGS) -c $<

all: netmux slave sqlslave links 

links: netmux slave sqlslave
	cd ../game/bin ; rm -f dbconvert ; ln -s ../../src/netmux dbconvert
	cd ../game/bin ; rm -f netmux ; ln -s ../../src/netmux netmux
	cd ../game/bin ; rm -f slave ; ln -s ../../src/slave slave
	cd ../game/bin ; rm -f sqlslave ; ln -s ../../src/sqlslave sqlslave

sqlslave: sqlslave.o sqlshared.o
	$(CXX) $(ALLCXXFLAGS) $(LIBS) -o sqlslave sqlslave.o sqlshared.o

slave: slave.o
	$(CXX) $(ALLCXXFLAGS) $(LIBS) -o slave slave.o

unsplit: unsplit.o
	$(CXX) $(ALLCXXFLAGS) -o unsplit unsplit.o

netmux: $(OBJ) $(VER_SRC)
	$(CXX) $(ALLCXXFLAGS) $(VER_FLG) -c $(VER_SRC)
	( if [ -f netmux ]; then mv -f netmux netmux~ ; fi )
	cp *.o btech
	$(CXX) $(ALLCXXFLAGS) -o netmux $(OBJ) $(VER_OBJ) $(LIBS) $(MORELIBS)

depend: $(ALLSRC) unsplit
	for i in $(D_SRC) $(SA_SRC) $(AUX_SRC) $(LIB_SRC) $(MALLOC_S) slave.cpp sqlslave.cpp sqlshared.cpp ; do $(CXXCPP) $(ALLCXXFLAGS) -M $$i; done | sed -e 's:/usr[^ ]*[ ]*::g' | ./unsplit > .depend~
	mv .depend~ .depend

realclean:
	-rm -f *.o a.out core gmon.out mux.*log mux.*sum netmux netmux~

clean:
	-rm -f *.o btech/*.o a.out core gmon.out mux.*log mux.*sum slave sqlslave netmux

include .depend
