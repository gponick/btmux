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
tree_t tree_srch __P((tree **, int (*)(), tree_t));
void tree_add __P((tree **, int (*)(), tree_t, void (*)()));
int tree_delete __P((tree **, int (*)(), tree_t, void (*)()));
int tree_trav __P((tree **, int (*)()));
void tree_mung __P((tree **, void (*)()));


#undef __P


#endif				/* _TREE_FLAG */
