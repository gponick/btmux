#ifndef MINE_H
#define MINE_H

#define MINE_LOW  1
#define MINE_HIGH 5
#define MINE_STANDARD 1
#define MINE_INFERNO  2
#define MINE_COMMAND  3
#define MINE_VIBRA    4
#define MINE_TRIGGER  5		/* Same as vibra, except shows _no_ message,
				   and doesn't get destroyed */
#define MINE_STRIGGER 6		/* Same as trigger, except doesn't broadcast anywhere */
#define VIBRO(a)      (a == MINE_VIBRA || a == MINE_TRIGGER || a == MINE_STRIGGER)

#endif				/* MINE_H */
