#include "BST.h"

static int AllocateExtraSpace(BST* tree)
{
    int new_capacity = 2* tree->capacity;
    tree->data = realloc(tree->data, (new_capacity)*(tree->element_size));
    tree->bst = realloc(tree->bst, (new_capacity)*(sizeof(BST_NODE)));

    // free list increase
    tree->bst[tree->capacity - 1].right = tree->capacity;
    for (int i = tree->free_list_index; i < new_capacity - 1; i++)
        tree->bst[i].right = i + 1;
    tree->bst[new_capacity - 1].right = -1;
    tree->free_list_index = tree->capacity;
    // now changing the capacity to new_capacity
    tree->capacity = new_capacity;
    return 1;
}
static int GetNewNode(BST *tree)
{
    if (tree->free_list_index == -1)
        AllocateExtraSpace(tree);

    int node = tree->free_list_index;
    tree->free_list_index = tree->bst[node].right;

    return node;
}

static void FreeUpNode(BST *tree, int node_index)
{
    tree->bst[node_index].right = tree->free_list_index;
    tree->free_list_index = node_index;
}

static int DetachSuccessor(BST *tree, int root)
{
    int parent = root;
    int child = tree->bst[parent].right;

    if (tree->bst[child].left == -1)
    {
        tree->bst[parent].right = tree->bst[child].right;
        return child;
    }

    while(tree->bst[child].left != -1)
    {
        parent = child;
        child = tree->bst[parent].left;
    }
    tree->bst[parent].left = tree->bst[child].right;
    return child;
}

int InitBST(BST* tree, size_t element_size, int capacity, int (*comparator)(void *, void *))
{
    tree->element_size = element_size;
    tree->capacity = capacity;
    tree->no_elements = 0;
    tree->free_list_index = 0;
    tree->comparator = comparator;

    tree->data = (void*)malloc(capacity*(tree->element_size));
    tree->bst = (BST_NODE*)malloc(capacity*sizeof(BST_NODE));

    //freelist initialization
    tree->free_list_index = 0;
    for (int i = 0; i < capacity - 1; i++)
        tree->bst[i].right = i + 1;
    tree->bst[capacity - 1].right = -1;
    return 0;
}

int Insert(BST *tree, int root, DATA d)
{
    int next;

    if (root == -1) {
        if (-1 == (next = GetNewNode(tree)))
            ERR_MESG("insert: out of memory");
        memcpy(DATA_AT(tree, next), d, tree->element_size);
        tree->bst[next].left = tree->bst[next].right = -1;
        tree->no_elements++;
        return next;
    }
    int cmp = tree->comparator(d, DATA_AT(tree, root));
    if (cmp < 0)
        tree->bst[root].left = Insert(tree, tree->bst[root].left, d);
    else if (cmp > 0)
        tree->bst[root].right = Insert(tree, tree->bst[root].right, d);
    return root;
}

int Search(BST *tree, int root, DATA d)
{
    if (root == -1) return -1;
    int cmp = tree->comparator(d, DATA_AT(tree, root));
    if (cmp < 0) 
        return Search(tree, tree->bst[root].left, d);
    else if (cmp > 0)
        return Search(tree, tree->bst[root].right, d);
    else return root;
}

int SearchMaxLessThan(BST *tree, int root, DATA d)
{
    if (root == -1) return -1;

    int cmp = tree->comparator(d, DATA_AT(tree, root));

    if (cmp <= 0)
    {
        return SearchMaxLessThan(tree, tree->bst[root].left, d);
    }
    else
    {
        int rightResult = SearchMaxLessThan(tree, tree->bst[root].right, d);
        return (rightResult != -1) ? rightResult : root;
    }
}

int RandomNodePicker(BST *tree, int root)
{
    int min = -1, max = 1, rand_int;
    //srandom((int) time(NULL));
    rand_int = (random() % (max - min + 1)) + min;
    if (root == -1) return -1;
    if (rand_int < 0) 
        return RandomNodePicker(tree, tree->bst[root].left);
    else if (rand_int > 0)
        return RandomNodePicker(tree, tree->bst[root].right);
    else return root;
}

int FindMax(BST *tree, int root)
{
    int max = root;

    if (root == -1) return -1;
    
    while(tree->bst[max].right != -1)
    {
        max = tree->bst[max].right;
    }
    return max;
}

int Delete(BST *tree, int root, DATA d)
{
    if (root == -1) return -1;

    int cmp = tree->comparator(d, DATA_AT(tree, root));
    if (cmp < 0)
        tree->bst[root].left = Delete(tree, tree->bst[root].left, d);
    else if (cmp > 0)
        tree->bst[root].right = Delete(tree, tree->bst[root].right, d);
    else {
        // TODO: complete this part
        if((tree->bst[root].left == -1) && (tree->bst[root].right == -1))
        {
            FreeUpNode(tree, root);
            tree->no_elements--;
            return -1;
        }
        else if(tree->bst[root].left == -1)
        {
            int temp = tree->bst[root].right;
            FreeUpNode(tree, root);
            tree->no_elements--;
            return temp;
        }
        else if(tree->bst[root].right == -1)
        {
            int temp = tree->bst[root].left;
            FreeUpNode(tree, root);
            tree->no_elements--;
            return temp;
        }
        else
        {
            int leftSub = tree->bst[root].left;
            int rightSub = tree->bst[root].right;
            int temp = DetachSuccessor(tree, root);
            tree->bst[temp].left = leftSub;
            if(temp != rightSub) tree->bst[temp].right = rightSub;
            FreeUpNode(tree, root);
            tree->no_elements--;
            return temp;
        }
    }

    return root;
}

int ReadTree(BST *tree, void *data_array, int array_len)
{
    int i;
    int root = -1;
    for(i=0;i<array_len;i++)
    {
        root = Insert(tree, root, (char*)data_array+i*tree->element_size);
    }
    return root;
}

void* InorderIterative(BST *tree, int root)
{
    int *stack = (int*)malloc(tree->capacity * sizeof(int));
    int top = -1;
    int current = root;

    // Allocate result array
    void *result = malloc(tree->no_elements * tree->element_size);
    int index = 0;

    while (current != -1 || top != -1)
    {
        // Traverse left
        while (current != -1)
        {
            stack[++top] = current;
            current = tree->bst[current].left;
        }

        // Pop
        current = stack[top--];

        // Copy data into result array
        memcpy((char*)result + (index)*tree->element_size,
               DATA_AT(tree, current),
               tree->element_size);

        (index)++;

        // Go right
        current = tree->bst[current].right;
    }

    free(stack);
    return result;
}

/*

void PrintBST(BST *, BST_NODE root)
{
}

void PrintpsBST(BST *, BST_NODE root)
{
}
*/
