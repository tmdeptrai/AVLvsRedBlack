

#include <string.h>
#include "tree-avl.h"
#include <stdbool.h>
#include "min-max.h"

/*--------------------------------------------------------------------*/
Tree
tree_new ()
{
  return NULL;
}

void
tree_delete (Tree tree, void (*delete) (void *))
{
  if (tree)
    {
      tree_delete (tree->left, delete);
      tree_delete (tree->right, delete);
      if (delete)
        delete (tree->data);
      free (tree);
    }
}

Tree
tree_create (const void *data, size_t size)
{
  Tree tree = malloc(sizeof(*tree) - sizeof(tree->data) + size); //Allocate more data for the parent and the balance
  if (tree)
    {

      tree->left = NULL;
      tree->right = NULL;

      //NEW: Parent node and balance
      tree->parent = NULL;
      tree->balance = 0;
      
      memcpy (tree->data, data, size);
    }

  return tree;
}

Tree
tree_get_left (Tree tree)
{
  if (tree)
    return tree->left;
  else
    return NULL;
}

Tree
tree_get_right (Tree tree)
{
  if (tree)
    return tree->right;
  else
    return NULL;
}

void *
tree_get_data (Tree tree)
{
  if (tree)
    return tree->data;
  else
    return NULL;
}

bool
tree_set_left (Tree tree, Tree left)
{
  if (tree)
    {
      tree->left = left;

      //NEW: set the parent pointer for the child
      if (left) left->parent=tree;

      return true;
    }
  else
    return false;
}

bool
tree_set_right (Tree tree, Tree right)
{
  if (tree)
    {
      tree->right = right;

      //NEW: set the parent pointer for the child
      if (right) right->parent=tree;

      return true;
    }
  else
    return false;
}

bool
tree_set_data (Tree tree, const void *data, size_t
size)
{
  if (tree)
    {
      memcpy (tree->data, data, size);
      return true;
    }
  else
    return false;
}

void
tree_pre_order (Tree tree,
                void (*func) (void *, void *),
                void *extra_data)
{
  if (tree)
    {
      func (tree->data, extra_data);
      tree_pre_order (tree->left, func, extra_data);
      tree_pre_order (tree->right, func, extra_data);
    }
}

void
tree_in_order (Tree tree,
               void (*func) (void *, void *),
               void *extra_data)
{
  if (tree)
    {
      tree_in_order (tree->left, func, extra_data);
      func (tree->data, extra_data);
      tree_in_order (tree->right, func, extra_data);
    }
}

void
tree_post_order (Tree tree,
                 void (*func) (void *, void *),
                 void *extra_data)
{
  if (tree)
    {
      tree_post_order (tree->left, func, extra_data);
      tree_post_order (tree->right, func, extra_data);
      func (tree->data, extra_data);
    }
}

size_t
tree_height (Tree tree)
{
  if (tree)
    return 1 + MAX (tree_height (tree->left),
tree_height (tree->right));
  else
    return 0;
}

size_t
tree_size (Tree tree)
{
  if (tree)
    return 1 + tree_size (tree->left) + tree_size
(tree->right);
  else
    return 0;
}

void *
tree_search (Tree tree,
             const void *data,
             int (*compare) (const void *, const void
*))
{
  if (tree)
    {
      switch (compare (data, tree->data))
        {
        case -1:
          return tree_search (tree->left, data, compare);
        case 0:
          return tree->data;
        case 1:
          return tree_search (tree->right, data, compare);
        default:
            return NULL; // RAJOUTE CAR WARNING !!!
        }
    }
  else
    return NULL;
}

static void
set (void *data,void *array)
{
  static size_t size;
  static size_t offset;

  if (data)
    {
      memcpy (array + offset, data, size);
      offset += size;
    }
  else
    {
      offset = 0;
      size = *(size_t *) array;
    }
}

int
tree_sort (void *array,
           size_t length,
           size_t size,
           int (*compare) (const void *, const void *))
{
  size_t i;
  Tree tree = tree_new ();
  void *pointer;

  pointer = array;
  for (i = 0; i < length; i++)
    {
      if (tree_insert_sorted (&tree, pointer, size,
compare))
        pointer += size;
      else
        {
          tree_delete (tree, NULL);
          return false;
        }
    }
  set (NULL, &size);
  tree_in_order (tree, set, array);
  tree_delete (tree, NULL);
  return true;
}



// ========================== ALL OF MY WORK ARE BELOW ========================================
//NEW: Recomputing balance for a node = height left - height right
static void recompute_balance(Tree tree)
{
  if (!tree) return;
  int hl = (int)tree_height(tree->left);
  int hr = (int)tree_height(tree->right);
  tree->balance = hl-hr;
}

/* rotate left:
    A                    B
   / \                 /   \
  a   B     ->        A     c
     / \             / \
    b   c           a   b
    
*/
static Tree rotate_left(Tree A) {
  Tree B = A->right;
  Tree b = B->left;

  // Perform rotation
  tree_set_left(B, A);
  tree_set_right(A, b);

  // Update parents
  B->parent = A->parent;

  // Recalculate balance factors for the affected nodes
  recompute_balance(A);
  recompute_balance(B);

  return B; // Return new root of this subtree
}

/* rotate right:
        B                A
       / \             /   \
      A   c   ->      a     B
     / \                   / \
    a   b                 b   c
*/
static Tree rotate_right(Tree B) {
  Tree A = B->left;
  Tree b = A->right;

  // Perform rotation
  tree_set_right(A, B);
  tree_set_left(B, b);
  
  // Update parents
  A->parent = B->parent;

  // Recalculate balance factors for the affected nodes
  recompute_balance(B);
  recompute_balance(A);

  return A; // Return new root of this subtree
}

bool
tree_insert_sorted (Tree * ptree,
                    const void *data,
                    size_t size,
                    int (*compare) (const void *, const void *))
{
 if (!*ptree) {
        // Base case: insert new node here
        Tree new_node = tree_create(data, size);
        if (!new_node) {
            return false;
        }
        *ptree = new_node;
        return true;
    }

    Tree root = *ptree;
    int cmp = compare(data, root->data);

    if (cmp < 0) {
        // Go Left
        if (!tree_insert_sorted(&root->left, data, size, compare)) {
            return false;
        }
        if (root->left) root->left->parent = root;
    } else {
        // Go Right (handles equal values too)
        if (!tree_insert_sorted(&root->right, data, size, compare)) {
            return false;
        }
        if (root->right) root->right->parent = root;
    }

    // After insertion, update balance and rebalance if needed
    recompute_balance(root);
    
    // Left Heavy
    if (root->balance > 1) {
        // Left-Right case
        if (root->left && root->left->balance < 0) {
            root->left = rotate_left(root->left);
        }
        // Left-Left case (or after fixing LR)
        *ptree = rotate_right(root);
    }
    // Right Heavy
    else if (root->balance < -1) {
        // Right-Left case
        if (root->right && root->right->balance > 0) {
            root->right = rotate_right(root->right);
        }
        // Right-Right case (or after fixing RL)
        *ptree = rotate_left(root);
    } else {
        // No rotation needed, just update the pointer
        *ptree = root;
    }

    return true;
}

// helper: find minimum node in subtree
static Tree min_value_node(Tree node) {
    Tree current = node;
    while (current && current->left)
        current = current->left;
    return current;
}
bool tree_remove_sorted(Tree *ptree,
                        const void *data,
                        int (*compare)(const void *, const void *)) 
{
    // Base case: data not found in this branch
    if (!ptree || !*ptree) {
        return false;
    }

    Tree root = *ptree;
    int cmp = compare(data, root->data);

    if (cmp < 0) {
        // Recurse left
        if (!tree_remove_sorted(&root->left, data, compare)) {
            return false; // Node not found
        }
    } else if (cmp > 0) {
        // Recurse right
        if (!tree_remove_sorted(&root->right, data, compare)) {
            return false; // Node not found
        }
    } else {
        // Node found, start deletion logic
        Tree node_to_delete = root;

        if (!root->left || !root->right) {
            // Case 1 & 2: Node has 0 or 1 child
            Tree child = root->left ? root->left : root->right;
            
            if (child) {
                child->parent = root->parent;
            }
            
            *ptree = child; // Parent's pointer now points to the child (or NULL)
            free(node_to_delete);
            // The function will continue to the rebalancing part below
            
        } else {
            // Case 3: Node has 2 children
            // Find inorder successor (smallest node in the right subtree)
            Tree succ = root->right;
            while (succ->left) {
                succ = succ->left;
            }

            *(int*)root->data = *(int*)succ->data;

            // Recursively delete the successor node from the right subtree
            tree_remove_sorted(&root->right, succ->data, compare);
        }
    }

    // If the tree became empty after deletion ==> removing the last node
    if (*ptree == NULL) {
        return true;
    }
    
    root = *ptree; 

    // --- Update balance and rebalance the tree ---
    int leftH = root->left ? tree_height(root->left) + 1 : 0;
    int rightH = root->right ? tree_height(root->right) + 1 : 0;
    root->balance = leftH - rightH;
    
    // Left Heavy
    if (root->balance > 1) { 
        // Left-Left Case
        if (root->left && root->left->balance >= 0) {
            *ptree = rotate_right(root);
        } 
        // Left-Right Case
        else {
            root->left = rotate_left(root->left);
            *ptree = rotate_right(root);
        }
    } 
    // Right Heavy
    else if (root->balance < -1) {
        // Right-Right Case
        if (root->right && root->right->balance <= 0) {
            *ptree = rotate_left(root);
        }
        // Right-Left Case
        else {
            root->right = rotate_right(root->right);
            *ptree = rotate_left(root);
        }
    }
    
    // Update parent pointers after potential rotations
    if ((*ptree)->left) (*ptree)->left->parent = *ptree;
    if ((*ptree)->right) (*ptree)->right->parent = *ptree;

    return true;
}