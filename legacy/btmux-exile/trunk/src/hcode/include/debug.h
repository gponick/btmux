#ifndef DEBUG_H
#define DEBUG_H
#ifndef DOCHECK
#define DOCHECK(a,b) if (a) { notify(player, b); return; }
#endif
void debug_allocfree(dbref key, void **data, int selector);
void debug_list(dbref player, void *data, char *buffer);
void debug_savedb(dbref player, void *data, char *buffer);
void debug_loaddb(dbref player, void *data, char *buffer);
void debug_shutdown(dbref player, void *data, char *buffer);
#endif				/* DEBUG_H */
