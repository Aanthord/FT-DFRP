#ifndef MERKLE_H
#define MERKLE_H

#include "fractal.h"

typedef struct merkle_node {
    char hash[MAX_HASH_SIZE];
    struct merkle_node *left;
    struct merkle_node *right;
    int is_leaf;
    int node_id;
} merkle_node_t;

typedef struct {
    merkle_node_t *root;
    char **leaf_hashes;
    int leaf_count;
    char global_root[MAX_HASH_SIZE];
} merkle_tree_t;

merkle_tree_t* build_network_merkle_tree();
void export_merkle_journal(const char *filepath);
int verify_merkle_path(int node_id, const char *expected_hash);
void update_merkle_tree_incremental(int node_id);

#endif // MERKLE_H
