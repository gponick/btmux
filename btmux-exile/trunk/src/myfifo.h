#ifndef MYFIFO_H
#define MYFIFO_H

typedef struct myfifo_entry_struct {
    void *data;
    struct myfifo_entry_struct *next;
    struct myfifo_entry_struct *prev;
} myfifo_e;

typedef struct myfifo_struct {
    myfifo_e *first;		/* First entry (last put in) */
    myfifo_e *last;		/* Last entry (first to get out) */
    int count;			/* Number of entries in the queue */
} myfifo;

/* myfifo.c */
int myfifo_length(myfifo ** foo);
void *myfifo_pop(myfifo ** foo);
void myfifo_push(myfifo ** foo, void *data);
void myfifo_trav(myfifo ** foo, void (*func) ());
void myfifo_trav_r(myfifo ** foo, void (*func) ());

#endif				/* MYFIFO_H */
