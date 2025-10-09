#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "tree-avl.h"
#include <stddef.h>

void monPrintF (void * a, void * b){
    printf("Valeur du noeud : %d\n", *(int*)a);
}

/**
 * Test réalisés pour les arbres binaires
 * Affichage des résultats des tris
 */
void testArbresBinaires(void){
    int i = 5, j = 10, k = 15, m = 20;
    size_t sizeInt = sizeof(int);

    Tree racine = tree_create(&i, sizeInt);
    Tree fils1 = tree_create(&j, sizeInt);
    Tree fils2 = tree_create(&k, sizeInt);

    tree_set_left(racine, fils1);
    tree_set_right(racine, fils2);

    Tree fils1fils1 = tree_create(&m, sizeInt);
    Tree fils2fils1 = tree_create(&i, sizeInt);

    tree_set_left(fils1, fils1fils1);
    tree_set_right(fils1, fils2fils1);

    printf("Tri pre-order : \n");
    tree_pre_order(racine, monPrintF, NULL);

    printf("\nTri post-order : \n");
    tree_post_order(racine, monPrintF, NULL);

    printf("\nTri in-order : \n");
    tree_in_order(racine, monPrintF, NULL);
    
    // Les données sont sur le stack (variables locales i, j, k, m).
    // Pas besoin d'appeler free donc pointeur nul.
    tree_delete(racine, 0);
}

/// ------------------ AVL HELPERS ------------------

void printIntWithBalance(void *data, void *extra_data) {
    (void) extra_data;
    Tree node = (Tree)((char *)data - offsetof(struct _TreeNode, data));
    int value = *(int*)data;
    int balance = node->balance;

    printf("Valeur : %2d (Balance: %2d)\n", value, balance);
}

int cmpInt(const void *a, const void *b) {
    int x = *(int*)a, y = *(int*)b;
    return (x > y) - (x < y);
}

void printStr(void *data, void *extra_data) {
    (void) extra_data;
    Tree node = (Tree)((char *)data - offsetof(struct _TreeNode, data));

    char *value = *(char **)data;
    int balance = node->balance;

    printf("Valeur du noeud : %-10s (Balance: %2d)\n", value, balance);
}

int cmpStr(const void *a, const void *b) {
    return strcmp((char*)a, (char*)b);
}

typedef struct {
    char *mot;
    char *definition;
} Entry;

void printEntry(void *data, void *extra_data) {
    (void) extra_data;
    Tree node = (Tree)((char *)data - offsetof(struct _TreeNode, data));

    // Access the data and balance
    Entry *e = (Entry *)data;
    int balance = node->balance;
    printf("{%-7s : %-18s} (Balance: %2d)\n", e->mot, e->definition, balance);
}


int cmpEntry(const void *a, const void *b) {
    Entry *ea = (Entry*)a;
    Entry *eb = (Entry*)b;
    return strcmp(ea->mot, eb->mot);
}

void freeEntry(Entry *e) {
    free(e->mot);
    free(e->definition);
}

/// ------------------ AVL TESTS ------------------

void testAVLInt(void) {
    Tree root = NULL;
    size_t sz = sizeof(int);

    int values[] = { 30, 20, 40, 10, 25, 50, 5 };
    int n = sizeof(values) / sizeof(values[0]);

    printf("\n===== Test AVL avec int =====\n");
    for (int i = 0; i < n; i++) {
        printf(">Insertion (%d)\n", values[i]);
        assert(tree_insert_sorted(&root, &values[i], sz, cmpInt));
        printf("Post-order après insertion :\n");
        tree_post_order(root, printIntWithBalance, NULL);
        printf("\n");
    }

    int values_to_delete[] = {30,5,25};
    n = sizeof(values_to_delete) / sizeof(values_to_delete[0]);
    for (int i = 0; i < n; i++) {
        printf(">Deletion (%d)\n", values_to_delete[i]);
        assert(tree_remove_sorted(&root, &values_to_delete[i], cmpInt));
        printf("Post-order après suppression :\n");
        tree_post_order(root, printIntWithBalance, NULL);
        printf("\n");

    }

    tree_delete(root, NULL);
}

void testAVLStr(void) {
    Tree root = NULL;
    size_t sz = sizeof(char*);

    char *words[] = { "pomme", "banane", "cerise", "datte", "abricot" };
    int n = sizeof(words) / sizeof(words[0]);

    printf("\n===== Test AVL avec chaînes =====\n");
    for (int i = 0; i < n; i++) {
        char *dup = strdup(words[i]);  // allocate copy on heap
        printf(">Insertion (%s)\n", dup);
        assert(tree_insert_sorted(&root, &dup, sz, cmpStr));
        printf("Post-order après insertion :\n");
        tree_post_order(root, printStr, NULL);
        printf("\n");
    }

    tree_delete(root, 0);  // free each strdup’ed string
}


void testAVLEntry(void) {
    Tree root = NULL;
    size_t sz = sizeof(Entry);

    Entry entries[] = {
        {"chien", "animal domestique"},
        {"chat", "animal mignon"},
        {"maison", "lieu d'habitation"},
        {"arbre", "plante ligneuse"},
        {"voiture", "moyen de transport"}
    };
    int n = sizeof(entries) / sizeof(entries[0]);

    printf("\n===== Test AVL avec structures (mot, definition) =====\n");
    for (int i = 0; i < n; i++) {
        Entry e;
        e.mot = strdup(entries[i].mot);
        e.definition = strdup(entries[i].definition);

        printf(">Insertion (%s)\n", e.mot);
        assert(tree_insert_sorted(&root, &e, sz, cmpEntry));
        printf("Post-order après insertion :\n");
        tree_post_order(root, printEntry, NULL);
        printf("\n");
    }

    // free function to release strdup’ed fields
    tree_delete(root, (void(*)(void*)) freeEntry);
}


/// ------------------ MAIN ------------------

int main(void) {
    // testArbresBinaires();
    testAVLInt();           // AVL with integers
    testAVLStr();           // AVL with strings
    testAVLEntry();         // AVL with structs

    printf("\nTous les tests sont terminés avec succès.\n");
    return EXIT_SUCCESS;
}