#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * Import the AVL implementation under a dedicated prefix. The function renames
 * prevent clashes once we also include the red-black implementation below.
 */
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

/*
 * Import the red-black tree implementation, again shielding the symbols with
 * a prefix to avoid collisions.
 */
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

static double
elapsed_ms(const struct timespec *start, const struct timespec *end)
{
  time_t secs = end->tv_sec - start->tv_sec;
  long nsecs = end->tv_nsec - start->tv_nsec;
  if (nsecs < 0)
  {
    --secs;
    nsecs += 1000000000L;
  }
  return (double)secs * 1000.0 + (double)nsecs / 1e6;
}

static int
cmp_int(const void *a, const void *b)
{
  int ia = *(const int *)a;
  int ib = *(const int *)b;
  if (ia < ib)
    return -1;
  if (ia > ib)
    return 1;
  return 0;
}

static void
shuffle_ints(int *array, size_t length)
{
  if (length < 2)
    return;
  for (size_t i = length - 1; i > 0; --i)
  {
    size_t j = (size_t)(rand() % (int)(i + 1));
    int tmp = array[i];
    array[i] = array[j];
    array[j] = tmp;
  }
}

static char *
build_filename(const char *prefix, const char *suffix)
{
  size_t len = strlen(prefix) + strlen(suffix) + 2; /* underscore + NUL */
  char *path = malloc(len);
  if (!path)
    return NULL;
  snprintf(path, len, "%s_%s", prefix, suffix);
  return path;
}

static bool
write_csv(const char *filepath,
          const double *insert_times,
          const double *search_times,
          const double *delete_times,
          size_t count)
{
  FILE *fp = fopen(filepath, "w");
  if (!fp)
  {
    fprintf(stderr, "Unable to open %s for writing.\n", filepath);
    return false;
  }

  for (size_t i = 0; i < count; ++i)
  {
    if (fprintf(fp, "%.6f,%.6f,%.6f\n",
                insert_times[i],
                search_times[i],
                delete_times[i]) < 0)
    {
      fprintf(stderr, "Failed while writing to %s.\n", filepath);
      fclose(fp);
      return false;
    }
  }

  fclose(fp);
  return true;
}

#define DEFINE_COLLECT_FUNCTIONS(tag, tree_type, insert_fn, search_fn, remove_fn, delete_fn) \
  static bool collect_timings_##tag(const int *insert_order,                                 \
                                    const int *search_order,                                 \
                                    const int *delete_order,                                 \
                                    size_t count,                                            \
                                    double *insert_times,                                    \
                                    double *search_times,                                    \
                                    double *delete_times)                                    \
  {                                                                                          \
    tree_type root = NULL;                                                                   \
    struct timespec start, end;                                                              \
                                                                                             \
    for (size_t i = 0; i < count; ++i)                                                       \
    {                                                                                        \
      clock_gettime(CLOCK_MONOTONIC, &start);                                                \
      if (!insert_fn(&root, &insert_order[i], sizeof(int), cmp_int))                         \
      {                                                                                      \
        fprintf(stderr, #tag " insert failed at index %zu\n", i);                            \
        delete_fn(root, NULL);                                                               \
        return false;                                                                        \
      }                                                                                      \
      clock_gettime(CLOCK_MONOTONIC, &end);                                                  \
      insert_times[i] = elapsed_ms(&start, &end);                                            \
    }                                                                                        \
                                                                                             \
    for (size_t i = 0; i < count; ++i)                                                       \
    {                                                                                        \
      clock_gettime(CLOCK_MONOTONIC, &start);                                                \
      void *found = search_fn(root, &search_order[i], cmp_int);                              \
      clock_gettime(CLOCK_MONOTONIC, &end);                                                  \
      search_times[i] = elapsed_ms(&start, &end);                                            \
      if (!found)                                                                            \
      {                                                                                      \
        fprintf(stderr, #tag " search missed key %d\n", search_order[i]);                    \
        delete_fn(root, NULL);                                                               \
        return false;                                                                        \
      }                                                                                      \
    }                                                                                        \
                                                                                             \
    for (size_t i = 0; i < count; ++i)                                                       \
    {                                                                                        \
      clock_gettime(CLOCK_MONOTONIC, &start);                                                \
      if (!remove_fn(&root, &delete_order[i], cmp_int))                                      \
      {                                                                                      \
        fprintf(stderr, #tag " delete failed for key %d\n", delete_order[i]);                \
        delete_fn(root, NULL);                                                               \
        return false;                                                                        \
      }                                                                                      \
      clock_gettime(CLOCK_MONOTONIC, &end);                                                  \
      delete_times[i] = elapsed_ms(&start, &end);                                            \
    }                                                                                        \
                                                                                             \
    delete_fn(root, NULL);                                                                   \
    return true;                                                                             \
  }

DEFINE_COLLECT_FUNCTIONS(avl, AvlTree, avl_tree_insert_sorted, avl_tree_search,
                         avl_tree_remove_sorted, avl_tree_delete)
DEFINE_COLLECT_FUNCTIONS(rbt, RbtTree, rbt_tree_insert_sorted, rbt_tree_search,
                         rbt_tree_remove_sorted, rbt_tree_delete)

int main(int argc, char **argv)
{
  size_t count = 1000;
  unsigned int seed = (unsigned int)time(NULL);
  const char *prefix = "exec_time";

  if (argc > 1)
  {
    char *endptr = NULL;
    errno = 0;
    unsigned long parsed = strtoul(argv[1], &endptr, 10);
    if (errno != 0 || endptr == argv[1] || *endptr != '\0')
    {
      fprintf(stderr, "Invalid item count: %s\n", argv[1]);
      return EXIT_FAILURE;
    }
    if (parsed == 0)
    {
      fprintf(stderr, "Item count must be greater than zero.\n");
      return EXIT_FAILURE;
    }
    count = (size_t)parsed;
  }

  if (argc > 2)
  {
    char *endptr = NULL;
    errno = 0;
    unsigned long parsed = strtoul(argv[2], &endptr, 10);
    if (errno != 0 || endptr == argv[2] || *endptr != '\0')
    {
      fprintf(stderr, "Invalid seed: %s\n", argv[2]);
      return EXIT_FAILURE;
    }
    seed = (unsigned int)parsed;
  }

  if (argc > 3)
  {
    prefix = argv[3];
  }

  srand(seed);

  int *base = malloc(sizeof(int) * count);
  int *insert_order = malloc(sizeof(int) * count);
  int *search_order = malloc(sizeof(int) * count);
  int *delete_order = malloc(sizeof(int) * count);

  double *avl_insert_times = malloc(sizeof(double) * count);
  double *avl_search_times = malloc(sizeof(double) * count);
  double *avl_delete_times = malloc(sizeof(double) * count);

  double *rbt_insert_times = malloc(sizeof(double) * count);
  double *rbt_search_times = malloc(sizeof(double) * count);
  double *rbt_delete_times = malloc(sizeof(double) * count);

  if (!base || !insert_order || !search_order || !delete_order ||
      !avl_insert_times || !avl_search_times || !avl_delete_times ||
      !rbt_insert_times || !rbt_search_times || !rbt_delete_times)
  {
    fprintf(stderr, "Allocation failure for %zu elements.\n", count);
    free(base);
    free(insert_order);
    free(search_order);
    free(delete_order);
    free(avl_insert_times);
    free(avl_search_times);
    free(avl_delete_times);
    free(rbt_insert_times);
    free(rbt_search_times);
    free(rbt_delete_times);
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < count; ++i)
  {
    base[i] = (int)i;
  }

  memcpy(insert_order, base, sizeof(int) * count);
  memcpy(search_order, base, sizeof(int) * count);
  memcpy(delete_order, base, sizeof(int) * count);

  shuffle_ints(insert_order, count);
  shuffle_ints(search_order, count);
  shuffle_ints(delete_order, count);

  bool ok = collect_timings_avl(insert_order, search_order, delete_order, count,
                                avl_insert_times, avl_search_times, avl_delete_times);
  if (!ok)
  {
    fprintf(stderr, "Failed while benchmarking AVL tree.\n");
    goto cleanup;
  }

  ok = collect_timings_rbt(insert_order, search_order, delete_order, count,
                           rbt_insert_times, rbt_search_times, rbt_delete_times);
  if (!ok)
  {
    fprintf(stderr, "Failed while benchmarking red-black tree.\n");
    goto cleanup;
  }

  char *avl_filename = build_filename(prefix, "avl.csv");
  char *rbt_filename = build_filename(prefix, "rbt.csv");
  if (!avl_filename || !rbt_filename)
  {
    fprintf(stderr, "Failed to build CSV file names.\n");
    free(avl_filename);
    free(rbt_filename);
    goto cleanup;
  }

  ok = write_csv(avl_filename, avl_insert_times, avl_search_times, avl_delete_times, count);
  if (!ok)
  {
    fprintf(stderr, "Unable to create %s.\n", avl_filename);
    free(avl_filename);
    free(rbt_filename);
    goto cleanup;
  }

  ok = write_csv(rbt_filename, rbt_insert_times, rbt_search_times, rbt_delete_times, count);
  if (!ok)
  {
    fprintf(stderr, "Unable to create %s.\n", rbt_filename);
    free(avl_filename);
    free(rbt_filename);
    goto cleanup;
  }

  printf("Wrote AVL timings to %s\n", avl_filename);
  printf("Wrote Red-Black timings to %s\n", rbt_filename);

  free(avl_filename);
  free(rbt_filename);

cleanup:
  free(base);
  free(insert_order);
  free(search_order);
  free(delete_order);
  free(avl_insert_times);
  free(avl_search_times);
  free(avl_delete_times);
  free(rbt_insert_times);
  free(rbt_search_times);
  free(rbt_delete_times);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}