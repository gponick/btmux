SUBDIRS=event.do src.do misc.do tree.do

all: 
	(cd event ; $(GNUMAKE))
	(cd misc ; $(GNUMAKE))
	(cd tree ; $(GNUMAKE))
	(CDPATH= ; cd src ; $(GNUMAKE))

include mux.general

ndall:
	(cd event ; $(GNUMAKE))
	(cd misc ; $(GNUMAKE))
	(cd tree ; $(GNUMAKE))
	(CDPATH= ; cd src ; $(GNUMAKE) ndall)

depend:	
	(cd event ; $(GNUMAKE) depend)
	(cd misc ; $(GNUMAKE) depend)
	(cd tree ; $(GNUMAKE) depend)
	(CDPATH= ; cd src ; $(GNUMAKE) depend)
	(cd src/hcode ; $(GNUMAKE) depend)
	(cd src/hcode/btech ; $(GNUMAKE) depend)
	(cd src/hcode/fantasy ; $(GNUMAKE) depend)

clean:
	(cd event ; $(GNUMAKE) clean)
	(cd misc ; $(GNUMAKE) clean)
	(cd tree ; $(GNUMAKE) clean)
	(CDPATH= ; cd src ; $(GNUMAKE) clean)

commit:
	cvsmapfs -pw > .cvsmapfs
	cvs -q commit -m'auto-update' .cvsmapfs
	(CDPATH= ; cd src ; cvs -q commit -m'auto-update' changes.txt buildnum.data)
	cvs com
	cvsmapfs -cw < .cvsmapfs

update:
	cvs update
	cvsmapfs -cw < .cvsmapfs
# DO NOT DELETE
