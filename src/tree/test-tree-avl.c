#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "tree-avl.h"

static int compare_ints(const void *a, const void *b);
static void print_traversals(const char *label, Tree root);
static void collect_values(void *data, void *extra);
static Tree rebuild_without(Tree root, int value);

void monPrintF (void * a, void * b){
    (void) b; 
    printf("Valeur du noeud : %d\n", *(int*)a);
}

/**
 * Test réalisés pour les arbres binaires
 * Affichage des résultats des tris
 */
void testArbresBinaires(void){
    int initial_values[] = {5, 10, 15, 20}; 
    size_t initial_count = sizeof initial_values / sizeof *initial_values; 

    Tree racine = tree_new(); 
    for (size_t idx = 0; idx < initial_count; ++idx) 
    {
        bool inserted = tree_insert_sorted(&racine, &initial_values[idx], sizeof(int), compare_ints); 
        assert(inserted);
    }

    print_traversals("Arbre initial :", racine);

    int nouveau = 12; 
    bool inserted = tree_insert_sorted(&racine, &nouveau, sizeof nouveau, compare_ints); 
    assert(inserted); 
    print_traversals("Après insertion et équilibrage :", racine); 

    int supprime = 10;
    racine = rebuild_without(racine, supprime); 
    print_traversals("Après suppression et rééquilibrage :", racine); 

    int recherche = 15;
    int *found = tree_search(racine, &recherche, compare_ints); 
    printf("Recherche de %d : %s\n", recherche, found ? "trouvé" : "absent"); 

    tree_delete(racine, NULL);
}

static int compare_ints(const void *a, const void *b)
{
    int left = *(const int *)a; 
    int right = *(const int *)b;
    if (left < right) 
        return -1; 
    if (left > right) 
        return 1;
    return 0;
}

static void print_traversals(const char *label, Tree root)
{
    printf("%s\n", label); 
    printf("Pre-order :\n");
    tree_pre_order(root, monPrintF, NULL);
    printf("\nPost-order :\n");
    tree_post_order(root, monPrintF, NULL);
    printf("\nIn-order :\n");
    tree_in_order(root, monPrintF, NULL);
    printf("\n"); 
}

struct collect_ctx
{
    int *buffer;
    size_t index; 
};

static void collect_values(void *data, void *extra)
{
    struct collect_ctx *ctx = extra; 
    ctx->buffer[ctx->index++] = *(int *)data;
}

static Tree rebuild_without(Tree root, int value)
{
    size_t count = tree_size(root);
    if (!count)
        return root; 

    int *buffer = malloc(count * sizeof *buffer);
    if (!buffer)
        return root;

    struct collect_ctx ctx = { buffer, 0 }; 
    tree_in_order(root, collect_values, &ctx); 

    Tree new_root = tree_new();
    for (size_t i = 0; i < ctx.index; ++i) 
    {
        if (buffer[i] == value) 
            continue; 
        bool inserted = tree_insert_sorted(&new_root, &buffer[i], sizeof buffer[i], compare_ints);
        assert(inserted); 
    }

    free(buffer);
    tree_delete(root, NULL);
    return new_root;
}



int main(){

    testArbresBinaires();

    return EXIT_SUCCESS;
}
