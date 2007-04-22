// vattr.h -- Definitions for user-defined attributes.
//
// $Id: vattr.h,v 1.3 2004/07/07 17:48:01 sdennis Exp $
//

extern ATTR *vattr_rename_LEN(char *, int, char *, int);
extern ATTR *vattr_find_LEN(const char *pAttrName, size_t nAttrName);
extern ATTR *vattr_alloc_LEN(char *pAttrName, size_t nAttrName, int flags);
extern ATTR *vattr_define_LEN(char *pAttrName, size_t nAttrName, int number, int flags);
extern void  vattr_delete_LEN(char *, int);
extern ATTR *vattr_first(void);
extern ATTR *vattr_next(ATTR *);
extern void  list_vhashstats(dbref);
