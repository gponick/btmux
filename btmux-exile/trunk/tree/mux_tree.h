#ifndef MUX_TREE_H
#define MUX_TREE_H

#include "tree.h"

typedef int muxkey_t;
typedef unsigned char dtype_t;
typedef unsigned short dsize_t;

#define NodeKey(n)  n->key
#define NodeData(n) n->data
#define NodeSize(n) n->size
#define NodeType(n) n->type

#define MT_TYPE_LIST 127

typedef struct rbtc_list_type {
    dtype_t type;
    dsize_t size;
    void *data;
    struct rbtc_list_type *next;
} ListEntry;

typedef struct rbtc_node_type {
    muxkey_t key;
    dtype_t type;
    dsize_t size;
    void *data;
} Node;

typedef tree *Tree;

#include "p.mux_tree.h"

Node *FindNode(Tree tree, muxkey_t key);
#endif				/* MUX_TREE_H */
