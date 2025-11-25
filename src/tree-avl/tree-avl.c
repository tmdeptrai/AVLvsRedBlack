#include <string.h>
#include "tree-avl.h"
#include <stdbool.h>

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

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
  Tree tree = malloc(sizeof(*tree) - sizeof(tree->data) + size);
  if (tree)
  {
    tree->left = NULL;
    tree->right = NULL;
    tree->parent = NULL;
    tree->height = 1; // New node has height 1
    memcpy(tree->data, data, size);
  }
  return tree;
}

Tree tree_get_left(Tree tree)
{
  return tree ? tree->left : NULL;
}

Tree tree_get_right(Tree tree)
{
  return tree ? tree->right : NULL;
}

void *tree_get_data(Tree tree)
{
  return tree ? tree->data : NULL;
}

bool tree_set_left(Tree tree, Tree left)
{
  if (tree)
  {
    tree->left = left;
    if (left)
      left->parent = tree;
    return true;
  }
  return false;
}

bool tree_set_right(Tree tree, Tree right)
{
  if (tree)
  {
    tree->right = right;
    if (right)
      right->parent = tree;
    return true;
  }
  return false;
}

bool tree_set_data(Tree tree, const void *data, size_t size)
{
  if (tree)
  {
    memcpy(tree->data, data, size);
    return true;
  }
  return false;
}

void tree_pre_order(Tree tree, void (*func)(void *, void *), void *extra_data)
{
  if (tree)
  {
    func(tree->data, extra_data);
    tree_pre_order(tree->left, func, extra_data);
    tree_pre_order(tree->right, func, extra_data);
  }
}

void tree_in_order(Tree tree, void (*func)(void *, void *), void *extra_data)
{
  if (tree)
  {
    tree_in_order(tree->left, func, extra_data);
    func(tree->data, extra_data);
    tree_in_order(tree->right, func, extra_data);
  }
}

void tree_post_order(Tree tree, void (*func)(void *, void *), void *extra_data)
{
  if (tree)
  {
    tree_post_order(tree->left, func, extra_data);
    tree_post_order(tree->right, func, extra_data);
    func(tree->data, extra_data);
  }
}

size_t tree_height(Tree tree)
{
  return tree ? tree->height : 0;
}

size_t tree_size(Tree tree)
{
  if (tree)
    return 1 + tree_size(tree->left) + tree_size(tree->right);
  else
    return 0;
}

void *tree_search(Tree tree, const void *data,
                  int (*compare)(const void *, const void *))
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
      return NULL;
    }
  }
  return NULL;
}

void set(void *data, void *array)
{
  size_t size;
  size_t offset;
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

int tree_sort(void *array, size_t length, size_t size,
              int (*compare)(const void *, const void *))
{
  size_t i;
  Tree tree = tree_new();
  void *pointer;
  pointer = array;
  for (i = 0; i < length; i++)
  {
    if (tree_insert_sorted(&tree, pointer, size, compare))
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

// ========================== AVL OPERATIONS ========================================

// O(1) - Get height from stored value
int get_height(Tree node)
{
  return node ? node->height : 0;
}

// O(1) - Calculate balance factor
int get_balance(Tree node)
{
  return node ? get_height(node->left) - get_height(node->right) : 0;
}

// O(1) - Update height based on children
void update_height(Tree node)
{
  if (node)
  {
    node->height = 1 + MAX(get_height(node->left), get_height(node->right));
  }
}

// Rotate left - O(1)
Tree rotate_left(Tree A)
{
  Tree B = A->right;
  Tree b = B->left;

  // Perform rotation
  B->left = A;
  A->right = b;

  // Update parents
  B->parent = A->parent;
  A->parent = B;
  if (b)
    b->parent = A;

  // Update heights (order matters!)
  update_height(A);
  update_height(B);

  return B;
}

// Rotate right - O(1)
Tree rotate_right(Tree B)
{
  Tree A = B->left;
  Tree b = A->right;

  // Perform rotation
  A->right = B;
  B->left = b;

  // Update parents
  A->parent = B->parent;
  B->parent = A;
  if (b)
    b->parent = B;

  // Update heights (order matters!)
  update_height(B);
  update_height(A);

  return A;
}

// Insert with AVL balancing - O(log n)
bool tree_insert_sorted(Tree *ptree, const void *data, size_t size,
                        int (*compare)(const void *, const void *))
{
  if (!*ptree)
  {
    Tree new_node = tree_create(data, size);
    if (!new_node)
    {
      return false;
    }
    *ptree = new_node;
    return true;
  }

  Tree root = *ptree;
  int cmp = compare(data, root->data);

  if (cmp < 0)
  {
    if (!tree_insert_sorted(&root->left, data, size, compare))
    {
      return false;
    }
    if (root->left)
      root->left->parent = root;
  }
  else
  {
    if (!tree_insert_sorted(&root->right, data, size, compare))
    {
      return false;
    }
    if (root->right)
      root->right->parent = root;
  }

  // Update height - O(1)
  update_height(root);

  // Get balance factor - O(1)
  int balance = get_balance(root);

  // Left Heavy
  if (balance > 1)
  {
    // Left-Right case
    if (get_balance(root->left) < 0)
    {
      root->left = rotate_left(root->left);
    }
    // Left-Left case
    *ptree = rotate_right(root);
  }
  // Right Heavy
  else if (balance < -1)
  {
    // Right-Left case
    if (get_balance(root->right) > 0)
    {
      root->right = rotate_right(root->right);
    }
    // Right-Right case
    *ptree = rotate_left(root);
  }
  else
  {
    *ptree = root;
  }

  return true;
}

// Delete with AVL balancing - O(log n)
bool tree_remove_sorted(Tree *ptree, const void *data,
                        int (*compare)(const void *, const void *))
{
  if (!ptree || !*ptree)
  {
    return false;
  }

  Tree root = *ptree;
  int cmp = compare(data, root->data);

  if (cmp < 0)
  {
    if (!tree_remove_sorted(&root->left, data, compare))
    {
      return false;
    }
  }
  else if (cmp > 0)
  {
    if (!tree_remove_sorted(&root->right, data, compare))
    {
      return false;
    }
  }
  else
  {
    // Node found, delete it
    if (!root->left || !root->right)
    {
      // 0 or 1 child
      Tree child = root->left ? root->left : root->right;

      if (child)
      {
        child->parent = root->parent;
      }

      *ptree = child;
      free(root);
      return true;
    }
    else
    {
      // 2 children: find inorder successor
      Tree succ = root->right;
      while (succ->left)
      {
        succ = succ->left;
      }

      // Copy successor's data
      memcpy(root->data, succ->data, sizeof(int)); // Adjust size if needed

      // Delete successor
      tree_remove_sorted(&root->right, succ->data, compare);
    }
  }

  if (*ptree == NULL)
  {
    return true;
  }

  root = *ptree;

  // Update height - O(1)
  update_height(root);

  // Get balance - O(1)
  int balance = get_balance(root);

  // Left Heavy
  if (balance > 1)
  {
    if (get_balance(root->left) >= 0)
    {
      *ptree = rotate_right(root);
    }
    else
    {
      root->left = rotate_left(root->left);
      *ptree = rotate_right(root);
    }
  }
  // Right Heavy
  else if (balance < -1)
  {
    if (get_balance(root->right) <= 0)
    {
      *ptree = rotate_left(root);
    }
    else
    {
      root->right = rotate_right(root->right);
      *ptree = rotate_left(root);
    }
  }

  // Update parent pointers
  if ((*ptree)->left)
    (*ptree)->left->parent = *ptree;
  if ((*ptree)->right)
    (*ptree)->right->parent = *ptree;

  return true;
}