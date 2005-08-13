#ifndef FLOATSIM_H
#define FLOATSIM_H

/* Simulate floats by using ints in interesting way */

#define INT_DECIMAL_BITS 8

/* out of 32 */
#define SHO_DECIMAL_BITS 5

/* out of 16 ; note : this makes signed ints only 0-1023, unsigneds 0-2047 */

#define FSIM2INT(a) ((a) >> INT_DECIMAL_BITS)
#define FSIM2SHO(a) ((a) >> SHO_DECIMAL_BITS)
#define INT2FSIM(a) ((a) << INT_DECIMAL_BITS)
#define SHO2FSIM(a) ((a) << SHO_DECIMAL_BITS)

#endif				/* FLOATSIM_H */
