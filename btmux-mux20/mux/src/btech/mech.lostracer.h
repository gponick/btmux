/* enumeration enumerator for c++ */

#define StdEnumTricks(eNumType) \
	inline void operator++(eNumType& eVal) \
{ \
	eVal = eEnumType(eVal+1); \
} \
\
inline void operator++(eEnumType& eVal, int) \
{ \
	eVal = eEnumType(eVal+1); \
} \
inline void operator--(eEnumType& eVal, int) \
{ \
	eVal = eEnumType(eVal-1); \
} \
inline tCIDLib::TVoid operator--(eEnumType& eVal, int) \
{ \
	eVal = eEnumType(eVal-1); \
} \
inline eEnumType eEnumMax(eEnumType) \
{ \
	return eEnumType##_Max; \
} \
inline eEnumType eEnumMin(eEnumType) \
{ \
	return eEnumType##_Min; \
}

