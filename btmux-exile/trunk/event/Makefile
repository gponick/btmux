#event_test: event.c event_test.c
#	$(CC) $(CFLAGS) event_test.c event.c -o event_test

all:	event.o

include ../mux.general

clean:
	rm -f event.o

event.o: event.c event.h
