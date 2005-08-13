#CC     = gcc
#CFLAGS = -g -O2
LIBN   = libtree.a
OBJS   = tree.o mux_tree.o

all:	$(LIBN)

include ../mux.general

clean:
	rm -f *.o *.a

libtree.a: $(OBJS)
	ar rclu $(LIBN) $(OBJS)
	ranlib $(LIBN)
