#define FATAL(msgs...) \
{ fprintf(stderr, ##msgs); exit(1); }

#define TREAD(to,len,msg) \
if (feof(f)) FATAL("ERROR READING FILE (%s): EOF.\n", msg); \
if (fread(to,1,len,f) != len) \
FATAL("ERROR READING FILE (%s): NOT ENOUGH READ!\n", msg);

#define TSAVE(from,len,msg) \
if (fwrite(from, 1, len, tree_file) != len) \
FATAL("ERROR WRITING FILE (%s): NOT ENOUGH WRITTEN(?!?)\n", msg);

/* Main aim: Attempt to provide 1-1 interface when compared to my
   old rbtc code and the new AVL code */

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "mux_tree.h"

#define Create(var,typ,count) \
if (!(var = (typ *) calloc(sizeof (typ), count))) \
{ fprintf(stderr, "Error mallocating!\n"); exit(1); }

static int NodeCompare(Node * a, Node * b)
{
    if (a->key < b->key)
	return -1;
    return (a->key > b->key);
}

static void NodeDelete(Node * a)
{
    if (a->data)
	free((void *) a->data);
    free((void *) a);
}

void AddEntry(Tree * tree, muxkey_t key, dtype_t type, dsize_t size,
    void *data)
{
    Node *foo;

    if (!tree)
	return;
    Create(foo, Node, 1);
    foo->key = key;
    foo->data = data;
    foo->type = type;
    foo->size = size;
    tree_add(tree, NodeCompare, foo, NodeDelete);
}

Node *FindNode(Tree tree, muxkey_t key)
{
    Node foo;

    foo.key = key;
    return tree_srch(&tree, NodeCompare, &foo);
}

void DeleteEntry(Tree * tree, muxkey_t key)
{
    Node foo;

    if (FindNode(*tree, key)) {
	foo.key = key;
	tree_delete(tree, NodeCompare, &foo, NodeDelete);
    }
}

static FILE *tree_file;

void recursively_save_list(void *data)
{
    ListEntry *l = (ListEntry *) data;

    if (l->size) {
	TSAVE(l->data, l->size, "listdata");
	if (l->type == MT_TYPE_LIST)
	    recursively_save_list(l->data);
    }
    if (l->next) {
	dsize_t size = sizeof(ListEntry);

	TSAVE(&size, sizeof(size), "lsize");
	TSAVE(l->next, size, "nextlistdata");
	recursively_save_list(l->next);
    }
}

static int nodesave_count;

static int NodeSave(Node * n)
{
    TSAVE(&n->key, sizeof(n->key), "key");
    TSAVE(&n->type, sizeof(n->type), "type");
    TSAVE(&n->size, sizeof(n->size), "size");
    if (n->size > 0)
	TSAVE(n->data, n->size, "data");
    nodesave_count++;
    /* Kludge-ish: */
    if (n->type == MT_TYPE_LIST)
	recursively_save_list(n->data);
    return 1;
}

int SaveTree(FILE * f, Tree tree)
{
    muxkey_t key;

    nodesave_count = 0;
    tree_file = f;
    tree_trav(&tree, NodeSave);
    key = -1;
    fwrite(&key, sizeof(key), 1, tree_file);
    return nodesave_count;
}

void recursively_read_list(FILE * f, void *data)
{
    ListEntry *l = (ListEntry *) data;
    dsize_t size;

    if (l->size) {
	if (!(l->data = malloc(l->size))) {
	    printf("Error malloccing!\n");
	    exit(1);
	}
	TREAD(l->data, l->size, "listdata");
	/* Kludge to read lists properly */
	if (l->type == MT_TYPE_LIST)
	    /* Recursive lists _can_ be done, you know. */
	    recursively_read_list(f, data);
    } else
	l->data = NULL;
    if (l->next) {
	TREAD(&size, sizeof(size), "size");
	/* This COULD be prettified by removing recursion. 'todo' */
	if (!size) {
	    printf("Odd : Next listentry clear?\n");
	    exit(1);
	}
	if (!(l->next = malloc(size))) {
	    printf("Error malloccing!\n");
	    exit(1);
	}
	TREAD(l->next, size, "nextlistdata");
	recursively_read_list(f, l->next);
    }
}

static void MyLoadTree(FILE * f, Tree * tree)
{
    muxkey_t key;
    dtype_t type;
    dsize_t size;
    void *data;

    TREAD(&key, sizeof(key), "first key");
    while (key >= 0 && !feof(f)) {
	TREAD(&type, sizeof(type), "type");
	TREAD(&size, sizeof(size), "size");
	if (size) {
	    if (!(data = malloc(size))) {
		printf("Error malloccing!\n");
		exit(1);
	    }
	    TREAD(data, size, "data");
	    /* Kludge to read lists properly */
	    if (type == MT_TYPE_LIST)
		/* Recursive lists _can_ be done, you know. */
		recursively_read_list(f, data);
	} else
	    data = NULL;
	AddEntry(tree, key, type, size, data);
	TREAD(&key, sizeof(key), "new key");
    }
}


/* This is a _MONSTER_ :P */
void ClearTree(Tree * tree)
{
    tree_mung(tree, NodeDelete);
}

void LoadTree(FILE * f, Tree * tree)
{
    ClearTree(tree);
    MyLoadTree(f, tree);
}

void UpdateTree(FILE * f, Tree * tree)
{
    MyLoadTree(f, tree);
}

void GoThruTree(Tree tree, int (*func) (Node *))
{
    tree_trav(&tree, func);
}
