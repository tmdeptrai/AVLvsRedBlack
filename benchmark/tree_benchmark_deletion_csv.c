#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Pull in AVL tree implementation with unique symbol names. */
#define Tree AvlTree
#define _TreeNode _AvlTreeNode
#define tree_new avl_tree_new
#define tree_delete avl_tree_delete
#define tree_create avl_tree_create
#define tree_get_left avl_tree_get_left
#define tree_get_right avl_tree_get_right
#define tree_get_data avl_tree_get_data
#define tree_set_left avl_tree_set_left
#define tree_set_right avl_tree_set_right
#define tree_set_data avl_tree_set_data
#define tree_pre_order avl_tree_pre_order
#define tree_in_order avl_tree_in_order
#define tree_post_order avl_tree_post_order
#define tree_height avl_tree_height
#define tree_size avl_tree_size
#define tree_insert_sorted avl_tree_insert_sorted
#define tree_search avl_tree_search
#define tree_sort avl_tree_sort
#define tree_remove_sorted avl_tree_remove_sorted
#define recompute_balance avl_recompute_balance
#define rotate_left avl_rotate_left
#define rotate_right avl_rotate_right
#define min_value_node avl_min_value_node
#define set avl_set
#include "../src/tree-avl/tree-avl.c"
#undef set
#undef min_value_node
#undef rotate_right
#undef rotate_left
#undef recompute_balance
#undef tree_remove_sorted
#undef tree_sort
#undef tree_search
#undef tree_insert_sorted
#undef tree_size
#undef tree_height
#undef tree_post_order
#undef tree_in_order
#undef tree_pre_order
#undef tree_set_data
#undef tree_set_right
#undef tree_set_left
#undef tree_get_data
#undef tree_get_right
#undef tree_get_left
#undef tree_create
#undef tree_delete
#undef tree_new
#undef _TreeNode
#undef Tree

/* Pull in Red-Black tree implementation with a second prefix. */
#define Tree RbtTree
#define _TreeNode _RbtTreeNode
#define Color RbtColor
#define tree_new rbt_tree_new
#define tree_delete rbt_tree_delete
#define tree_create rbt_tree_create
#define tree_get_left rbt_tree_get_left
#define tree_get_right rbt_tree_get_right
#define tree_get_data rbt_tree_get_data
#define tree_set_left rbt_tree_set_left
#define tree_set_right rbt_tree_set_right
#define tree_set_data rbt_tree_set_data
#define tree_pre_order rbt_tree_pre_order
#define tree_in_order rbt_tree_in_order
#define tree_post_order rbt_tree_post_order
#define tree_height rbt_tree_height
#define tree_size rbt_tree_size
#define tree_insert_sorted rbt_tree_insert_sorted
#define tree_search rbt_tree_search
#define tree_sort rbt_tree_sort
#define tree_remove_sorted rbt_tree_remove_sorted
#define tree_search_node rbt_tree_search_node
#define set rbt_set
#define rotate_left rbt_rotate_left
#define rotate_right rbt_rotate_right
#define tree_insert_fixup rbt_tree_insert_fixup
#define transplant rbt_transplant
#define min_value_node rbt_min_value_node
#define delete_fixup rbt_delete_fixup
#include "../src/tree-rbt/tree-rbt.c"
#undef delete_fixup
#undef min_value_node
#undef transplant
#undef tree_insert_fixup
#undef rotate_right
#undef rotate_left
#undef set
#undef tree_search_node
#undef tree_remove_sorted
#undef tree_sort
#undef tree_search
#undef tree_insert_sorted
#undef tree_size
#undef tree_height
#undef tree_post_order
#undef tree_in_order
#undef tree_pre_order
#undef tree_set_data
#undef tree_set_right
#undef tree_set_left
#undef tree_get_data
#undef tree_get_right
#undef tree_get_left
#undef tree_create
#undef tree_delete
#undef tree_new
#undef Color
#undef _TreeNode
#undef Tree

typedef bool (*InsertFn)(void **root, const void *data, size_t size,
                         int (*compare)(const void *, const void *));
typedef bool (*RemoveFn)(void **root, const void *data,
                         int (*compare)(const void *, const void *));
typedef void (*DestroyFn)(void *root, void (*delete_cb)(void *));

typedef struct
{
  const char *name;
  InsertFn insert;
  RemoveFn remove;
  DestroyFn destroy;
} TreeDriver;

static double
elapsed_ms (const struct timespec *start, const struct timespec *end)
{
  time_t secs = end->tv_sec - start->tv_sec;
  long nsecs = end->tv_nsec - start->tv_nsec;
  if (nsecs < 0)
    {
      --secs;
      nsecs += 1000000000L;
    }
  return (double) secs * 1000.0 + (double) nsecs / 1e6;
}

static int
cmp_int (const void *a, const void *b)
{
  int ia = *(const int *) a;
  int ib = *(const int *) b;
  if (ia < ib)
    return -1;
  if (ia > ib)
    return 1;
  return 0;
}

static void
shuffle_ints (int *array, size_t length)
{
  if (length < 2)
    return;
  for (size_t i = length - 1; i > 0; --i)
    {
      size_t j = (size_t) (rand () % (int) (i + 1));
      int tmp = array[i];
      array[i] = array[j];
      array[j] = tmp;
    }
}

static bool
write_csv (const char *path, const double *sizes, const double *times, size_t count)
{
  FILE *fp = fopen (path, "w");
  if (!fp)
    {
      fprintf (stderr, "Unable to open %s for writing.\n", path);
      return false;
    }

  for (size_t i = 0; i < count; ++i)
    {
      if (fprintf (fp, "%.0f,%.6f\n", sizes[i], times[i]) < 0)
        {
          fprintf (stderr, "Failed while writing %s.\n", path);
          fclose (fp);
          return false;
        }
    }

  fclose (fp);
  return true;
}

static bool
benchmark_tree (const TreeDriver *driver,
                size_t n_start,
                size_t n_end,
                size_t delete_count,
                int *values,
                double *sizes,
                double *times)
{
  size_t index = 0;

  for (size_t n = n_start; n <= n_end; ++n)
    {
      void *root = NULL;

      for (size_t i = 0; i < n; ++i)
        values[i] = (int) i;

      shuffle_ints (values, n);
      for (size_t i = 0; i < n; ++i)
        {
          if (!driver->insert (&root, &values[i], sizeof (int), cmp_int))
            {
              fprintf (stderr, "%s insert failed while building tree of size %zu\n",
                       driver->name, n);
              driver->destroy (root, NULL);
              return false;
            }
        }

      /* Select 50 keys, measure total time to remove them. */
      for (size_t i = 0; i < n; ++i)
        values[i] = (int) i;
      shuffle_ints (values, n);

      struct timespec start, end;
      clock_gettime (CLOCK_MONOTONIC, &start);
      for (size_t i = 0; i < delete_count; ++i)
        {
          if (!driver->remove (&root, &values[i], cmp_int))
            {
              fprintf (stderr, "%s removal failed for key %d in tree size %zu\n",
                       driver->name, values[i], n);
              driver->destroy (root, NULL);
              return false;
            }
        }
      clock_gettime (CLOCK_MONOTONIC, &end);

      sizes[index] = (double) n;
      times[index] = elapsed_ms (&start, &end);
      ++index;

      driver->destroy (root, NULL);
    }

  return true;
}

int
main (int argc, char **argv)
{
  size_t n_start = 50;
  size_t n_end = 1049;
  size_t delete_count = 50;
  unsigned int seed = (unsigned int) time (NULL);
  const char *prefix = "";

  if (argc > 1)
    {
      char *endptr = NULL;
      errno = 0;
      unsigned long parsed = strtoul (argv[1], &endptr, 10);
      if (errno != 0 || endptr == argv[1] || *endptr != '\0')
        {
          fprintf (stderr, "Invalid start value: %s\n", argv[1]);
          return EXIT_FAILURE;
        }
      n_start = (size_t) parsed;
    }

  if (argc > 2)
    {
      char *endptr = NULL;
      errno = 0;
      unsigned long parsed = strtoul (argv[2], &endptr, 10);
      if (errno != 0 || endptr == argv[2] || *endptr != '\0')
        {
          fprintf (stderr, "Invalid end value: %s\n", argv[2]);
          return EXIT_FAILURE;
        }
      n_end = (size_t) parsed;
    }

  if (argc > 3)
    {
      char *endptr = NULL;
      errno = 0;
      unsigned long parsed = strtoul (argv[3], &endptr, 10);
      if (errno != 0 || endptr == argv[3] || *endptr != '\0')
        {
          fprintf (stderr, "Invalid delete count: %s\n", argv[3]);
          return EXIT_FAILURE;
        }
      delete_count = (size_t) parsed;
    }

  if (argc > 4)
    {
      char *endptr = NULL;
      errno = 0;
      unsigned long parsed = strtoul (argv[4], &endptr, 10);
      if (errno != 0 || endptr == argv[4] || *endptr != '\0')
        {
          fprintf (stderr, "Invalid seed: %s\n", argv[4]);
          return EXIT_FAILURE;
        }
      seed = (unsigned int) parsed;
    }

  if (argc > 5)
    {
      prefix = argv[5];
    }

  if (n_end < n_start)
    {
      fprintf (stderr, "End value must be >= start value.\n");
      return EXIT_FAILURE;
    }

  if (delete_count == 0 || delete_count > n_start)
    {
      fprintf (stderr, "Delete count must be between 1 and start tree size.\n");
      return EXIT_FAILURE;
    }

  srand (seed);

  size_t samples = n_end - n_start + 1;

  int *values = malloc (sizeof (int) * (n_end + 1));
  double *sizes = malloc (sizeof (double) * samples);
  double *avl_times = malloc (sizeof (double) * samples);
  double *rbt_times = malloc (sizeof (double) * samples);

  if (!values || !sizes || !avl_times || !rbt_times)
    {
      fprintf (stderr, "Allocation failure for samples=%zu.\n", samples);
      free (values);
      free (sizes);
      free (avl_times);
      free (rbt_times);
      return EXIT_FAILURE;
    }

  const TreeDriver avl_driver = {
    "AVL", (InsertFn) avl_tree_insert_sorted, (RemoveFn) avl_tree_remove_sorted,
    (DestroyFn) avl_tree_delete
  };
  const TreeDriver rbt_driver = {
    "Red-Black", (InsertFn) rbt_tree_insert_sorted, (RemoveFn) rbt_tree_remove_sorted,
    (DestroyFn) rbt_tree_delete
  };

  if (!benchmark_tree (&avl_driver, n_start, n_end, delete_count,
                       values, sizes, avl_times))
    {
      fprintf (stderr, "AVL benchmark failed.\n");
      goto cleanup;
    }

  if (!benchmark_tree (&rbt_driver, n_start, n_end, delete_count,
                       values, sizes, rbt_times))
    {
      fprintf (stderr, "Red-Black benchmark failed.\n");
      goto cleanup;
    }

  char avl_path[256];
  char rbt_path[256];
  snprintf (avl_path, sizeof (avl_path), "%savl_deletion_time.csv", prefix);
  snprintf (rbt_path, sizeof (rbt_path), "%srbt_deletion_time.csv", prefix);

  if (!write_csv (avl_path, sizes, avl_times, samples))
    {
      fprintf (stderr, "Could not write %s\n", avl_path);
      goto cleanup;
    }

  if (!write_csv (rbt_path, sizes, rbt_times, samples))
    {
      fprintf (stderr, "Could not write %s\n", rbt_path);
      goto cleanup;
    }

  printf ("Wrote AVL partial deletion timings to %s\n", avl_path);
  printf ("Wrote Red-Black partial deletion timings to %s\n", rbt_path);

cleanup:
  free (values);
  free (sizes);
  free (avl_times);
  free (rbt_times);
  return EXIT_SUCCESS;
}
