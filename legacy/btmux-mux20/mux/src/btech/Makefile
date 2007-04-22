#!/usr/bin/make -f

CC             = gcc
MAKEDEPEND     = gcc -MM
CFLAGS		   = -g -Wall -Wno-unused
CPPFLAGS       = 

INCLUDES       = -Iinclude -Isrc -Iinclude -Ibtech -I..
LIBS           = -ldbi -levent -lnsl -lm -lcrypt -lbsd 
LDFLAGS		   = 

VERSION_FLAGS  = -DMUX_BUILD_DATE="\"`date`\""              \
                 -DMUX_BUILD_NUM="\"`sh src/buildnum.sh`\""
BUILDFLAGS     = -DMYFIFO -DBT_ENABLED
ALLFLAGS       = $(CFLAGS) $(CPPFLAGS) $(INCLUDES) $(LDFLAGS) $(BUILDFLAGS)

PROTOMAKER     = bin/protomaker.sh

MISC_OBJECTS   = misc/myfifo.o

EVENT_OBJECTS  = event/event.o

TREE_OBJECTS   = tree/mux_tree.o \
                 tree/tree.o     \

HCODE_OBJECTS =  coolmenu.o        \
                 debug.o           \
                 events.o          \
                 glue.hcode.o      \
                 glue.o            \
                 glue.scode.o      \
                 mech.lostracer.o  \
                 mechfile.o        \
                 spath.o           \

BTECH_OBJECTS  = btech/aero.bomb.o                \
                 btech/aero.move.o                \
                 btech/ai.o                       \
                 btech/artillery.o                \
                 btech/autogun.o                  \
                 btech/autopilot.o                \
                 btech/autopilot_command.o        \
                 btech/autopilot_commands.o       \
                 btech/bsuit.o                    \
                 btech/btech.o                    \
                 btech/btechstats.o               \
                 btech/btspath.o                  \
                 btech/crit.o                     \
                 btech/ds.bay.o                   \
                 btech/ds.turret.o                \
                 btech/econ.o                     \
                 btech/econ_cmds.o                \
                 btech/eject.o                    \
                 btech/failures.o                 \
                 btech/hudinfo.o                  \
                 btech/map.bits.o                 \
                 btech/map.coding.o               \
                 btech/map.conditions.o           \
                 btech/map.dynamic.o              \
                 btech/map.los.o                  \
                 btech/map.o                      \
                 btech/map.obj.o                  \
                 btech/mech.advanced.o            \
                 btech/mech.ammodump.o            \
                 btech/mech.avail.o               \
                 btech/mech.bth.o                 \
                 btech/mech.build.o               \
                 btech/mech.c3.misc.o             \
                 btech/mech.c3.o                  \
                 btech/mech.c3i.o                 \
                 btech/mech.combat.misc.o         \
                 btech/mech.combat.missile.o      \
                 btech/mech.combat.o              \
                 btech/mech.consistency.o         \
                 btech/mech.contacts.o            \
                 btech/mech.custom.o              \
                 btech/mech.damage.o              \
                 btech/mech.ecm.o                 \
                 btech/mech.enhanced.criticals.o  \
                 btech/mech.events.o              \
                 btech/mech.fire.o                \
                 btech/mech.hitloc.o              \
                 btech/mech.ice.o                 \
                 btech/mech.lite.o                \
                 btech/mech.los.o                 \
                 btech/mech.maps.o                \
                 btech/mech.mechref_ident.o       \
                 btech/mech.move.o                \
                 btech/mech.notify.o              \
                 btech/mech.ood.o                 \
                 btech/mech.partnames.o           \
                 btech/mech.physical.o            \
                 btech/mech.pickup.o              \
                 btech/mech.restrict.o            \
                 btech/mech.scan.o                \
                 btech/mech.sensor.functions.o    \
                 btech/mech.sensor.o              \
                 btech/mech.spot.o                \
                 btech/mech.startup.o             \
                 btech/mech.stat.o                \
                 btech/mech.status.o              \
                 btech/mech.tag.o                 \
                 btech/mech.tech.commands.o       \
                 btech/mech.tech.damages.o        \
                 btech/mech.tech.do.o             \
                 btech/mech.tech.events.o         \
                 btech/mech.tech.o                \
                 btech/mech.tech.repairs.o        \
                 btech/mech.tech.saverepair.o     \
                 btech/mech.tic.o                 \
                 btech/mech.update.o              \
                 btech/mech.utils.o               \
                 btech/mechrep.o                  \
                 btech/mine.o                     \
                 btech/pcombat.o                  \
                 btech/scen.o                     \
                 btech/template.o                 \

ALLOBJS = $(MISC_OBJECTS) $(EVENT_OBJECTS) $(TREE_OBJECTS) \
          $(HCODE_OBJECTS) $(BTECH_OBJECTS)

ALLSRCS = $(ALLOBJS:.o=.c)

TARGETS = src/netmux src/mkindx src/unsplit src/slave src/fileslave src/sqlslave

all: $(TARGETS)

# Install copies the 'game' directory to game.run, after trying to get all
# files from bk. If the bk command fails, we don't really care.
install: $(TARGETS)
	if [ ! -d game ]; then \
	    echo No game directory. Please acquire one from http://sourceforge.net/projects/btonline-btech.; \
	    exit 1; \
	fi
	if [ ! -d game.run ]; then \
	    cp -a game game.run || exit 1; \
	    chmod -R u+w game.run || exit 1; \
	fi
	if [ ! -d game.run/bin ]; then \
	    mkdir game.run/bin/; \
	fi
	mv -f $(TARGETS) game.run/bin/

update:
	svn update;

netmux src/netmux: $(ALLOBJS)
	$(CC) $(ALLFLAGS) -o src/netmux $(ALLOBJS) $(LIBS)

clean:
	rm -f $(ALLOBJS) $(TARGETS)

slave src/slave: src/slave.c
	$(CC) $(ALLFLAGS) -o src/slave src/slave.c

sqlslave src/sqlslave: src/sqlslave.c
	$(CC) $(ALLFLAGS) -o src/sqlslave src/sqlslave.c $(LIBS)

fileslave src/fileslave: src/fileslave.c
	$(CC) $(ALLFLAGS) -o src/fileslave src/fileslave.c $(LIBS)

mkindx src/mkindx: src/mkindx.c
	$(CC) $(ALLFLAGS) -o src/mkindx src/mkindx.c

unsplit src/unsplit: src/unsplit.c
	$(CC) $(ALLFLAGS) -o src/unsplit src/unsplit.c

dbconvert src/dbconvert:
	echo "dbconvert is no longer supported." && exit 1;

$(ALLOBJS): %.o: %.c
	$(CC) $(ALLFLAGS) -c $< -o $@

# Removed protomaker because it doesn't work and if
# we used it would wreck lots of things
#protos: $(PROTOMAKER)
#	$(PROTOMAKER) "$(INCLUDES)" $(ALLSRCS)

.depend:
	$(MAKEDEPEND) $(ALLFLAGS) $(ALLSRCS) > .depend

depend dep: .depend

.PHONY: clean protos

include .depend

