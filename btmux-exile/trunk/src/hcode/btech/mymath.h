#ifndef MYMATH_H
#define MYMATH_H

#ifdef fcos
#undef fcos
#endif
#define fcos cos
#ifdef fsin
#undef fsin
#endif
#define fsin sin
#ifdef fatan
#undef fatan
#endif
#define fatan atan
#ifdef MAX
#undef MAX
#endif
#ifdef MIN
#undef MIN
#endif
#define TWOPIOVER360 0.0174533
#define PI 3.141592654

#endif				/* MYMATH_H */
