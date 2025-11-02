// Save this as benchmark/BENCHMARK_DUONG.c

// FIX for CLOCK_MONOTONIC (must be at the very top)
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * =========================================================================
 * IMPORT AVL IMPLEMENTATION
 * Rename all functions and structs with an 'avl_' prefix
 * =========================================================================
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
// We include the .c file directly to apply the macros
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
 * =========================================================================
 * IMPORT RBT IMPLEMENTATION
 * Rename all functions and structs with an 'rbt_' prefix
 * =========================================================================
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
// Include the .c file for the RBT
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


// --- CONFIGURATION ---
#define N_START 50
#define N_MAX 1000
#define N_INCREMENT 50
#define NUM_TRIALS 50 // Average 50 runs
#define OUTPUT_FILE "benchmark_results.csv"

// --- HELPER FUNCTIONS ---

// Integer comparison function
int cmpInt(const void *a, const void *b) {
    return (*(const int *)a - *(const int *)b);
}

// Fisher-Yates shuffle
void shuffle(int *array, size_t n) {
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

// Helper to get high-resolution time in milliseconds
double get_time_ms(const struct timespec *start, const struct timespec *end) {
    time_t secs = end->tv_sec - start->tv_sec;
    long nsecs = end->tv_nsec - start->tv_nsec;
    if (nsecs < 0) {
        --secs;
        nsecs += 1000000000L;
    }
    return (double)secs * 1000.0 + (double)nsecs / 1e6;
}

// --- MAIN BENCHMARK PROGRAM ---

int main() {
    FILE *csv_file = fopen(OUTPUT_FILE, "w");
    if (csv_file == NULL) {
        perror("Error opening output file");
        return 1;
    }
    
    struct timespec start_ts, end_ts;
    srand((unsigned int)time(NULL));

    fprintf(csv_file, "N,AVL_Insert_Time,RBT_Insert_Time,AVL_Search_Time,RBT_Search_Time,AVL_Delete_Time,RBT_Delete_Time\n");
    printf("N, AVL_Insert (ms), RBT_Insert (ms), AVL_Search (ms), RBT_Search (ms), AVL_Delete (ms), RBT_Delete (ms)\n");

    for (int N = N_START; N <= N_MAX; N += N_INCREMENT) {
        
        double total_avl_insert = 0, total_rbt_insert = 0;
        double total_avl_search = 0, total_rbt_search = 0;
        double total_avl_delete = 0, total_rbt_delete = 0;

        for (int t = 0; t < NUM_TRIALS; t++) {
            
            int *data = malloc(N * sizeof(int));
            int *shuffled_data = malloc(N * sizeof(int));
            for (int i = 0; i < N; i++) {
                data[i] = rand();
                shuffled_data[i] = data[i];
            }
            shuffle(shuffled_data, N);

            // --- 1. AVL Test ---
            AvlTree avl_tree = avl_tree_new();
            clock_gettime(CLOCK_MONOTONIC, &start_ts);
            for (int i = 0; i < N; i++) {
                avl_tree_insert_sorted(&avl_tree, &data[i], sizeof(int), cmpInt);
            }
            clock_gettime(CLOCK_MONOTONIC, &end_ts);
            total_avl_insert += get_time_ms(&start_ts, &end_ts);

            clock_gettime(CLOCK_MONOTONIC, &start_ts);
            for (int i = 0; i < N; i++) {
                avl_tree_search(avl_tree, &data[i], cmpInt);
            }
            clock_gettime(CLOCK_MONOTONIC, &end_ts);
            total_avl_search += get_time_ms(&start_ts, &end_ts);

            clock_gettime(CLOCK_MONOTONIC, &start_ts);
            for (int i = 0; i < N; i++) {
                avl_tree_remove_sorted(&avl_tree, &shuffled_data[i], cmpInt);
            }
            clock_gettime(CLOCK_MONOTONIC, &end_ts);
            total_avl_delete += get_time_ms(&start_ts, &end_ts);
            avl_tree_delete(avl_tree, NULL);

            // --- 2. RBT Test ---
            RbtTree rbt_tree = rbt_tree_new();
            clock_gettime(CLOCK_MONOTONIC, &start_ts);
            for (int i = 0; i < N; i++) {
                rbt_tree_insert_sorted(&rbt_tree, &data[i], sizeof(int), cmpInt);
            }
            clock_gettime(CLOCK_MONOTONIC, &end_ts);
            total_rbt_insert += get_time_ms(&start_ts, &end_ts);

            clock_gettime(CLOCK_MONOTONIC, &start_ts);
            for (int i = 0; i < N; i++) {
                rbt_tree_search(rbt_tree, &data[i], cmpInt);
            }
            clock_gettime(CLOCK_MONOTONIC, &end_ts);
            total_rbt_search += get_time_ms(&start_ts, &end_ts);

            clock_gettime(CLOCK_MONOTONIC, &start_ts);
            for (int i = 0; i < N; i++) {
                rbt_tree_remove_sorted(&rbt_tree, &shuffled_data[i], cmpInt);
            }
            clock_gettime(CLOCK_MONOTONIC, &end_ts);
            total_rbt_delete += get_time_ms(&start_ts, &end_ts);
            rbt_tree_delete(rbt_tree, NULL);

            free(data);
            free(shuffled_data);
        }

        // Calculate and print the final averages (in milliseconds)
        double final_avl_insert = total_avl_insert / NUM_TRIALS;
        double final_rbt_insert = total_rbt_insert / NUM_TRIALS;
        double final_avl_search = total_avl_search / NUM_TRIALS;
        double final_rbt_search = total_rbt_search / NUM_TRIALS;
        double final_avl_delete = total_avl_delete / NUM_TRIALS;
        double final_rbt_delete = total_rbt_delete / NUM_TRIALS;

        fprintf(csv_file, "%d,%f,%f,%f,%f,%f,%f\n", N,
                final_avl_insert, final_rbt_insert,
                final_avl_search, final_rbt_search,
                final_avl_delete, final_rbt_delete);
        
        printf("%d, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n", N,
               final_avl_insert, final_rbt_insert,
               final_avl_search, final_rbt_search,
               final_avl_delete, final_rbt_delete);
    }

    fclose(csv_file);
    printf("\nBenchmark complete. Results saved to %s\n", OUTPUT_FILE);
    return 0;
}
