INCLUDES=-I../.. -I../include -I../../../include -DBT_ENABLED=1

OBJS=aero.bomb.o aero.move.o artillery.o autogun.o autopilot.o autopilot_command.o autopilot_commands.o bsuit.o btech.o btechstats.o btspath.o crit.o ds.bay.o ds.turret.o econ.o econ_cmds.o eject.o map.o map.bits.o map.coding.o map.conditions.o map.dynamic.o map.obj.o mech.advanced.o mech.avail.o mech.build.o mech.combat.o mech.consistency.o mech.custom.o mech.c3.o mech.contacts.o mech.ecm.o mech.events.o mech.hitloc.o mech.ice.o mech.los.o  mech.maps.o mech.mechref_ident.o mech.move.o mech.notify.o mech.ood.o mech.partnames.o mech.physical.o mech.pickup.o mech.restrict.o mech.scan.o mech.sensor.o mech.sensor.functions.o mech.startup.o mech.status.o mech.tech.o mech.tech.commands.o mech.tech.damages.o mech.tech.do.o mech.tech.events.o mech.tech.repairs.o mech.tech.saverepair.o mech.tic.o mech.update.o mech.utils.o mechrep.o mine.o pcombat.o template.o mech.stat.o scen.o ai.o mech.lite.o  hudinfo.o

DYNVER=1.0.0

all: static #dynamic

#dynamic: libbtech.so.$(DYNVER) -- Leftover from dynamically linked  libbtech
static: libbtech.a

libbtech.so.$(DYNVER): $(OBJS)
	$(CC) -shared -o $@ $(OBJS)
	rm -f libbtech.so ; ln -s $@ libbtech.so
	rm -f ../libbtech* ; ln -s btech/libbtech.so .. ; ln -s btech/libbtech.so.$(DYNVER) ..

libbtech.a: $(OBJS)
	-rm -f $@
	rm -f ../libbtech* ; ln -s btech/libbtech.a ..
	ar rclu $@ $(OBJS)

include ../../../mux.general

CFLAGS=$(MUXCFLAGS) $(INCLUDES)

clean:
	-rm -f $(OBJS) $(TARGET)

