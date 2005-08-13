INCLUDES=-I.. -I../../include -Iinclude -Ibtech -DBT_ENABLED

# I'm not really satisfied yet ; needing to include the 'btech' dir is..
# unclean.

# defines:
#   BT_SUPPORT - enables mech-related things

COMMON_OBJS = coolmenu.o debug.o events.o glue.o glue.hcode.o glue.scode.o mech.lostracer.o mechfile.o spath.o

OBJS = $(COMMON_OBJS)

TARGET=libhcode.a

all:	$(TARGET)
	(CDPATH=;cd btech ; $(GNUMAKE))

#	(cd fantasy ; $(GNUMAKE))
# technically, we _should_ do this

$(TARGET): $(OBJS)
	-rm -f $(TARGET) 
	ar rclu $(TARGET) $(OBJS)

#	ranlib $(TARGET) 


include ../../mux.general

CFLAGS=$(MUXCFLAGS) $(INCLUDES)

clean:
	-rm -f $(OBJS) $(TARGET)
	(CDPATH=;cd btech ; make clean)

#	(cd fantasy ; make clean)

protos:
	-rm -f include/p.*.h
	$(PROTOMAKER) "$(INCLUDES)" $(wildcard *.c)
	mv p.*.h include
