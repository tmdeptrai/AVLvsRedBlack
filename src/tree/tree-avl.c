

#include <string.h>
#include "tree-avl.h"
#include <stdbool.h>
#include "min-max.h"

static void rotate_left (Tree *root, Tree node); 
static void rotate_right (Tree *root, Tree node); 
static void balance_after_insert (Tree *root, Tree node);

/*--------------------------------------------------------------------*/
Tree tree_new()
{
    return NULL;
}

void tree_delete(Tree tree, void (*delete)(void *))
{
    if (tree)
    {
        tree_delete(tree->left, delete);
        tree_delete(tree->right, delete);
        if (delete)
            delete(tree->data);
        free(tree);
    }
}


Tree tree_create(const void *data, size_t size)
{
    Tree tree = NULL;
    size_t payload = size ? size : 1; 
    size_t total_size = sizeof *tree + payload - sizeof tree->data; 
    tree = malloc(total_size); 
    if (tree)
    {
        tree->parent = NULL;
        tree->left = NULL;
        tree->right = NULL;
        tree->balance = 0;
        if (size)
            memcpy(tree->data, data, size);
        else
            tree->data[0] = 0;
    }

    return tree;
}


Tree tree_get_left(Tree tree)
{
    if (tree)
        return tree->left;
    else
        return NULL;
}

Tree tree_get_right(Tree tree)
{
    if (tree)
        return tree->right;
    else
        return NULL;
}

void *
tree_get_data(Tree tree)
{
    if (tree)
        return tree->data;
    else
        return NULL;
}

bool tree_set_left(Tree tree, Tree left)
{
    if (tree)
    {
        tree->left = left;
        return true;
    }
    else
        return false;
}

bool tree_set_right(Tree tree, Tree right)
{
    if (tree)
    {
        tree->right = right;
        return true;
    }
    else
        return false;
}

bool tree_set_data(Tree tree, const void *data, size_t size)
{
    if (tree)
    {
        memcpy(tree->data, data, size);
        return true;
    }
    else
        return false;
}

void tree_pre_order(Tree tree,
                    void (*func)(void *, void *),
                    void *extra_data)
{
    if (tree)
    {
        func(tree->data, extra_data);
        tree_pre_order(tree->left, func, extra_data);
        tree_pre_order(tree->right, func, extra_data);
    }
}


void tree_in_order(Tree tree,
                   void (*func)(void *, void *),
                   void *extra_data)
{
    if (tree)
    {
        tree_in_order(tree->left, func, extra_data);
        func(tree->data, extra_data);
        tree_in_order(tree->right, func, extra_data);
    }
}

void tree_post_order(Tree tree,
                     void (*func)(void *, void *),
                     void *extra_data)
{
    if (tree)
    {
        tree_post_order(tree->left, func, extra_data);
        tree_post_order(tree->right, func, extra_data);
        func(tree->data, extra_data);
    }
}


size_t
tree_height(Tree tree)
{
    if (tree)
        return 1 + MAX(tree_height(tree->left),
                       tree_height(tree->right));
    else
        return 0;
}

/* assume we have:
a
 \
  b
   \
    c
*/
static void rotate_left(Tree *root, Tree node)
{
    if (!node || !node->right)
        return;

    Tree pivot = node->right;      // b
    Tree pivot_left = pivot->left; // null

    pivot->parent = node->parent; // null
    if (!pivot->parent)
        *root = pivot;
    else if (pivot->parent->left == node)
        pivot->parent->left = pivot;
    else
        pivot->parent->right = pivot;
    
    node->right = pivot_left;
    if (pivot_left)
        pivot_left->parent = node;

    pivot->left = node; 
    node->parent = pivot;

    node->balance = tree_height(node->left) - tree_height(node->right);
    pivot->balance = tree_height(pivot->left) - tree_height(pivot->right);
}


static void rotate_right(Tree *root, Tree node) {
    if (!node || !node->left)
        return;
    
    Tree pivot = node->left;
    Tree pivot_right = pivot->right;

    pivot->parent = node->parent;
    if (!pivot->parent)
        *root = pivot;
    else if (pivot->parent->left == node)
        pivot->parent->left = pivot;
    else
        pivot->parent->right = pivot;

    node->left = pivot_right; 
    if (pivot_right)
        pivot_right->parent = node;

    pivot->right = node; 
    node->parent = pivot;

    node->balance = tree_height(node->left) - tree_height(node->right);
    pivot->balance = tree_height(pivot->left) - tree_height(pivot->right);
}


static void balance_after_insert(Tree *root, Tree node) {
    Tree parent = node ? node->parent : NULL; 

    while (parent) {
        if (node == parent->left)
            parent->balance--;
        else
            parent->balance++;

        if (parent->balance == 0)
            break;

        if (parent->balance == -2) {
            if (parent->left && parent->left->balance > 0)
                rotate_left(root, parent->left);
            rotate_right(root, parent);
            parent = parent->parent;
            break;
        }

        if (parent->balance == 2) {
            if (parent->right && parent->right->balance < 0)
                rotate_right(root, parent->right);
            rotate_left(root, parent);
            parent = parent->parent;
            break;
        }

        node = parent;
        parent = parent->parent;
    }
}


size_t
tree_size(Tree tree)
{
    if (tree)
        return 1 + tree_size(tree->left) + tree_size(tree->right);
    else
        return 0;
}


bool tree_insert_sorted(Tree *ptree,
                        const void *data,
                        size_t size,
                        int (*compare)(const void *, const void *))
{
    if (!ptree)
        return false;

    Tree parent = NULL;
    Tree current = *ptree;
    int cmp = 0;

    while (current)
    {
        parent = current;
        cmp = compare(data, current->data);
        if (cmp < 0)
            current = current->left;
        else
            current = current->right;
    }

    Tree new_node = tree_create(data, size);
    if (!new_node)
        return false;

    new_node->parent = parent;

    if (!parent)
        *ptree = new_node;
    else if (cmp < 0)
        parent->left = new_node;
    else
        parent->right = new_node;

    balance_after_insert(ptree, new_node);

    return true;
}


void *
tree_search(Tree tree,
            const void *data,
            int (*compare)(const void *, const void
                                             *))
{
    if (tree)
    {
        switch (compare(data, tree->data))
        {
        case -1:
            return tree_search(tree->left, data, compare);
        case 0:
            return tree->data;
        case 1:
            return tree_search(tree->right, data, compare);
        default:
            return NULL; // RAJOUTE CAR WARNING !!!
        }
    }
    else
        return NULL;
}


static void
set(void *data, void *array)
{
    static size_t size;
    static size_t offset;

    if (data)
    {
        memcpy(array + offset, data, size);
        offset += size;
    }
    else
    {
        offset = 0;
        size = *(size_t *)array;
    }
}


int tree_sort(void *array,
              size_t length,
              size_t size,
              int (*compare)(const void *, const void *))
{
    size_t i;
    Tree tree = tree_new();
    void *pointer;

    pointer = array;
    for (i = 0; i < length; i++)
    {
        if (tree_insert_sorted(&tree, pointer, size,
                               compare))
            pointer += size;
        else
        {
            tree_delete(tree, NULL);
            return false;
        }
    }
    set(NULL, &size);
    tree_in_order(tree, set, array);
    tree_delete(tree, NULL);
    return true;
}
