

#include <string.h>
#include "tree-rbt.h"
#include <stdbool.h>
#include "min-max.h"

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
  Tree tree = malloc(sizeof(*tree) - sizeof(tree->data) + size); // Allocate more data for the parent and the balance
  if (tree)
  {

    tree->left = NULL;
    tree->right = NULL;

    // NEW: Parent node and balance
    tree->parent = NULL;
    tree->color = RED;

    memcpy(tree->data, data, size);
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

    // NEW: set the parent pointer for the child
    if (left)
      left->parent = tree;

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

    // NEW: set the parent pointer for the child
    if (right)
      right->parent = tree;

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

size_t
tree_size(Tree tree)
{
  if (tree)
    return 1 + tree_size(tree->left) + tree_size(tree->right);
  else
    return 0;
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

static Tree tree_search_node(Tree tree,
                             const void *data,
                             int (*compare)(const void *, const void *))
{
  if (!tree)
    return NULL;

  int cmp = compare(data, tree->data);

  if (cmp < 0)
    return tree_search_node(tree->left, data, compare);
  else if (cmp > 0)
    return tree_search_node(tree->right, data, compare);
  else
    return tree; // CORRECT: Return the entire node pointer
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

// ========================== ALL OF MY WORK ARE BELOW ========================================
/* rotate left:
    A                    B
   / \                 /   \
  a   B     ->        A     c
     / \             / \
    b   c           a   b

*/
static void rotate_left(Tree *root, Tree x)
{
  Tree y = x->right;
  x->right = y->left;
  if (y->left != NULL)
  {
    y->left->parent = x;
  }
  y->parent = x->parent;
  if (x->parent == NULL)
  {
    *root = y;
  }
  else if (x == x->parent->left)
  {
    x->parent->left = y;
  }
  else
  {
    x->parent->right = y;
  }
  y->left = x;
  x->parent = y;
}

/* rotate right:
        B                A
       / \             /   \
      A   c   ->      a     B
     / \                   / \
    a   b                 b   c
*/
static void rotate_right(Tree *root, Tree x)
{
  Tree y = x->left;
  x->left = y->right;
  if (y->right != NULL)
  {
    y->right->parent = x;
  }
  y->parent = x->parent;
  if (x->parent == NULL)
  {
    *root = y;
  }
  else if (x == x->parent->right)
  {
    x->parent->right = y;
  }
  else
  {
    x->parent->left = y;
  }
  y->right = x;
  x->parent = y;
}

static void tree_insert_fixup(Tree *root, Tree z);

bool tree_insert_sorted(Tree *ptree,
                        const void *data,
                        size_t size,
                        int (*compare)(const void *, const void *))
{
  // Step 1: Standard BST insert
  Tree z = tree_create(data, size);
  if (!z)
    return false;

  Tree y = NULL;
  Tree x = *ptree;

  while (x != NULL)
  {
    y = x;
    if (compare(z->data, x->data) < 0)
    {
      x = x->left;
    }
    else
    {
      x = x->right;
    }
  }

  z->parent = y;
  if (y == NULL)
  {
    *ptree = z; // Tree was empty
  }
  else if (compare(z->data, y->data) < 0)
  {
    y->left = z;
  }
  else
  {
    y->right = z;
  }

  // Step 2: Call the fix-up function to restore properties
  tree_insert_fixup(ptree, z);

  return true;
}
/*
static void tree_insert_fixup(Tree *root, Tree z)
{
  // Loop as long as the parent of z is RED
  while (z->parent && z->parent->color == RED)
  {
    // Case 1: Parent is a LEFT child
    if (z->parent == z->parent->parent->left)
    {
      Tree y = z->parent->parent->right; // The "uncle"
      if (y && y->color == RED)
      {
        // Case 1a: Uncle is RED -> Recolor and move up
        z->parent->color = BLACK;
        y->color = BLACK;
        z->parent->parent->color = RED;
        z = z->parent->parent;
      }
      else
      {
        // Case 1b: Uncle is BLACK (or NULL)
        if (z == z->parent->right)
        {
          // Case 1b-i: z is a right child -> Left rotate on parent
          z = z->parent;
          rotate_left(root, z);
        }
        // Case 1b-ii: z is a left child -> Recolor and right rotate on grandparent
        z->parent->color = BLACK;
        z->parent->parent->color = RED;
        rotate_right(root, z);
      }
    }
    else
    {
      // Case 2: Parent is a RIGHT child (symmetric to Case 1)
      Tree y = z->parent->parent->left; // The "uncle"
      if (y && y->color == RED)
      {
        // Case 2a: Uncle is RED -> Recolor and move up
        z->parent->color = BLACK;
        y->color = BLACK;
        z->parent->parent->color = RED;
        z = z->parent->parent;
      }
      else
      {
        // Case 2b: Uncle is BLACK (or NULL)
        if (z == z->parent->left)
        {
          // Case 2b-i: z is a left child -> Right rotate on parent
          z = z->parent;
          rotate_right(root, z);
        }
        // Case 2b-ii: z is a right child -> Recolor and left rotate on grandparent
        z->parent->color = BLACK;
        z->parent->parent->color = RED;
        rotate_left(root, z);
      }
    }
  }
  // Final Step: Ensure the root of the entire tree is BLACK
  (*root)->color = BLACK;
}
*/

static void tree_insert_fixup(Tree *root, Tree z)
{
  while (z != *root && z->parent->color == RED)
  {
    // The grandparent must exist because the parent is RED, and the root is always BLACK.
    if (z->parent == z->parent->parent->left)
    {
      Tree y = z->parent->parent->right; // The Uncle
      if (y && y->color == RED)
      {
        // Case 1: Uncle is RED -> Recolor and move up
        z->parent->color = BLACK;
        y->color = BLACK;
        z->parent->parent->color = RED;
        z = z->parent->parent;
      }
      else
      {
        // Case 2: Uncle is BLACK -> Rotations needed
        if (z == z->parent->right)
        {
          // This handles the Left-Right case by transforming it
          // into a Left-Left case for the next iteration.
          z = z->parent;
          rotate_left(root, z);
        }
        // This now handles the clean Left-Left case.
        z->parent->color = BLACK;
        z->parent->parent->color = RED;
        rotate_right(root, z->parent->parent);
      }
    }
    else
    {
      // Symmetric case for when the parent is a right child
      Tree y = z->parent->parent->left; // The Uncle
      if (y && y->color == RED)
      {
        // Case 1
        z->parent->color = BLACK;
        y->color = BLACK;
        z->parent->parent->color = RED;
        z = z->parent->parent;
      }
      else
      {
        // Case 2
        if (z == z->parent->left)
        {
          // Right-Left case -> transform to Right-Right
          z = z->parent;
          rotate_right(root, z);
        }
        // Right-Right case
        z->parent->color = BLACK;
        z->parent->parent->color = RED;
        rotate_left(root, z->parent->parent);
      }
    }
  }
  // Ensure the root of the entire tree is always BLACK.
  (*root)->color = BLACK;
}

static void transplant(Tree *root, Tree u, Tree v)
{
  if (u->parent == NULL)
  {
    *root = v;
  }
  else if (u == u->parent->left)
  {
    u->parent->left = v;
  }
  else
  {
    u->parent->right = v;
  }
  if (v != NULL)
  {
    v->parent = u->parent;
  }
}


static Tree min_value_node(Tree node)
{
  Tree current = node;
  while (current && current->left != NULL)
    current = current->left;
  return current;
}

static void delete_fixup(Tree *root, Tree x, Tree x_parent)
{
  Tree w; // Sibling

  while (x != *root && (x == NULL || x->color == BLACK))
  {
    if (x_parent->left == x)
    { // Is x a left child?
      w = x_parent->right;
      if (w->color == RED)
      { // Case 1
        w->color = BLACK;
        x_parent->color = RED;
        rotate_left(root, x_parent);
        w = x_parent->right;
      }
      if ((w->left == NULL || w->left->color == BLACK) &&
          (w->right == NULL || w->right->color == BLACK))
      { // Case 2
        w->color = RED;
        x = x_parent;
        x_parent = x->parent; // <<< --- FIX: UPDATE THE PARENT POINTER
      }
      else
      {
        if (w->right == NULL || w->right->color == BLACK)
        { // Case 3
          if (w->left)
            w->left->color = BLACK;
          w->color = RED;
          rotate_right(root, w);
          w = x_parent->right;
        }
        // Case 4
        w->color = x_parent->color;
        x_parent->color = BLACK;
        if (w->right)
          w->right->color = BLACK;
        rotate_left(root, x_parent);
        x = *root;
      }
    }
    else
    { // x is a right child
      w = x_parent->left;
      if (w->color == RED)
      { // Case 1
        w->color = BLACK;
        x_parent->color = RED;
        rotate_right(root, x_parent);
        w = x_parent->left;
      }
      if ((w->left == NULL || w->left->color == BLACK) &&
          (w->right == NULL || w->right->color == BLACK))
      { // Case 2
        w->color = RED;
        x = x_parent;
        x_parent = x->parent; // <<< --- FIX: UPDATE THE PARENT POINTER
      }
      else
      {
        if (w->left == NULL || w->left->color == BLACK)
        { // Case 3
          if (w->right)
            w->right->color = BLACK;
          w->color = RED;
          rotate_left(root, w);
          w = x_parent->left;
        }
        // Case 4
        w->color = x_parent->color;
        x_parent->color = BLACK;
        if (w->left)
          w->left->color = BLACK;
        rotate_right(root, x_parent);
        x = *root;
      }
    }
  }
  if (x)
    x->color = BLACK;
}

// In tree_remove_sorted, you MUST find the parent of x before the fixup call
bool tree_remove_sorted(Tree *ptree,
                        const void *data,
                        int (*compare)(const void *, const void *))
{
  Tree z = tree_search_node(*ptree, data, compare);
  if (z == NULL)
    return false;

  Tree y = z;
  Tree x;
  Tree x_parent; // We need to track the parent of x
  Color y_original_color = y->color;

  if (z->left == NULL)
  {
    x = z->right;
    x_parent = z->parent;
    transplant(ptree, z, z->right);
  }
  else if (z->right == NULL)
  {
    x = z->left;
    x_parent = z->parent;
    transplant(ptree, z, z->left);
  }
  else
  {
    y = min_value_node(z->right);
    y_original_color = y->color;
    x = y->right;

    if (y->parent == z)
    {
      x_parent = y;
    }
    else
    {
      x_parent = y->parent;
      transplant(ptree, y, y->right);
      y->right = z->right;
      y->right->parent = y;
    }
    transplant(ptree, z, y);
    y->left = z->left;
    y->left->parent = y;
    y->color = z->color;
  }

  free(z);

  if (y_original_color == BLACK)
  {
    delete_fixup(ptree, x, x_parent);
  }

  return true;
}