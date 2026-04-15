#ifndef BST_H_
#define BST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ERR_MESG(x) { perror(x); exit(1); }
#define DATA_AT(tree, i) ((char*)(((tree)->data)) + (i)*((tree)->element_size))

typedef void* DATA;

typedef struct node{
    //int index;
    int left, right;
} BST_NODE;

typedef struct {
    size_t element_size;
    int capacity, no_elements, free_list_index;
    void* data;
    BST_NODE* bst;
    int (*comparator)(void*,void*);
}BST;

/* Main operations: see BST.c for what each function returns */

int InitBST(BST*, size_t element_size, int capacity, int (*comparator)(void*,void*)); // returns 0 in case of successful initilization
int Insert(BST *, int root, DATA d); // returns the root index of the tree
int Search(BST *, int root, DATA d); // returns the index of d in the BST is case not in the bst returns -1
int SearchMaxLessThan(BST *tree, int root, DATA d);
int FindMax(BST *tree, int root);    // returns node of max element
int RandomNodePicker(BST *tree, int root);
int Delete(BST *, int root, DATA d); // returns the root
int ReadTree(BST*, void* data_array, int array_len); // reads from an array to build a BST and reutrns the root index

void* InorderIterative(BST *tree, int root);
//extern void PrintBST(BST*, BST_NODE root);
//extern void PrintpsBST(BST*, BST_NODE root);

#endif // BST_H_