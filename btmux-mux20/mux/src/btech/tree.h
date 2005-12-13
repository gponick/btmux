
/* tree.h - declare structures used by tree.c
 * vix 27jun86 [broken out of tree.c]
 * vix 22jan93 [revisited; uses RCS, ANSI, POSIX; has bug fixes]
 *
 * $Id: tree.h,v 1.1 2005/06/13 20:50:53 murrayma Exp $
 */


#ifndef	_TREE_FLAG
#define	_TREE_FLAG


#ifdef __STDC__
typedef void *tree_t;

#ifndef __P
#define __P(x) x
#endif
#else
typedef char *tree_t;

#ifndef __P
#define	__P(x) ()
#endif
#endif


typedef struct tree_s {
    struct tree_s *tree_l, *tree_r;
    short tree_b;
    tree_t tree_p;
} tree;


void tree_init __P((tree **));
void * tree_srch __P((tree **, int (*)(void *, void *), void *));
void tree_add __P((tree **, int (*)(void *, void *), void *, void (*)(void *)));
int tree_delete __P((tree **, int (*)(void *, void *), void *, void (*)(void *)));
int tree_trav __P((tree **, int (*)(void *)));
void tree_mung __P((tree **, void (*)(void *)));


#undef __P


#endif				/* _TREE_FLAG */
