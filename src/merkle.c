/*
 * FT-DFRP: Merkle Tree Integrity System
 *
 * Dual Licensed:
 * 1. AGPL-3.0 for research/academic use
 * 2. Commercial license: contact michael.doran.808@gmail.com
 *
 * Copyright (C) 2025 Michael Doran
 */

#include "merkle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

static void compute_hash(const char *input, char *output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)input, strlen(input), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\0';
}

static merkle_node_t* build_tree(char **hashes, int count) {
    if (count == 1) {
        merkle_node_t *leaf = malloc(sizeof(merkle_node_t));
        strcpy(leaf->hash, hashes[0]);
        leaf->left = leaf->right = NULL;
        leaf->is_leaf = 1;
        leaf->node_id = -1;
        return leaf;
    }

    int mid = count / 2;
    merkle_node_t *left = build_tree(hashes, mid);
    merkle_node_t *right = build_tree(hashes + mid, count - mid);

    merkle_node_t *parent = malloc(sizeof(merkle_node_t));
    char concat[2 * MAX_HASH_SIZE];
    snprintf(concat, sizeof(concat), "%s%s", left->hash, right->hash);
    compute_hash(concat, parent->hash);
    parent->left = left;
    parent->right = right;
    parent->is_leaf = 0;
    parent->node_id = -1;

    return parent;
}

merkle_tree_t* build_network_merkle_tree() {
    char **leaf_hashes = malloc(sizeof(char*) * total_nodes);
    for (int i = 0; i < total_nodes; i++) {
        leaf_hashes[i] = malloc(MAX_HASH_SIZE);
        compute_hash(network[i].hash, leaf_hashes[i]);
    }

    merkle_tree_t *tree = malloc(sizeof(merkle_tree_t));
    tree->leaf_hashes = leaf_hashes;
    tree->leaf_count = total_nodes;
    tree->root = build_tree(leaf_hashes, total_nodes);
    strcpy(tree->global_root, tree->root->hash);
    return tree;
}

void export_merkle_journal(const char *filepath) {
    merkle_tree_t *tree = build_network_merkle_tree();
    FILE *f = fopen(filepath, "w");
    fprintf(f, "MERKLE_ROOT: %s\n", tree->global_root);
    for (int i = 0; i < tree->leaf_count; i++) {
        fprintf(f, "Node[%d]: %s\n", i, tree->leaf_hashes[i]);
    }
    fclose(f);
}

int verify_merkle_path(int node_id, const char *expected_hash) {
    char computed[MAX_HASH_SIZE];
    compute_hash(network[node_id].hash, computed);
    return strcmp(computed, expected_hash) == 0;
}

void update_merkle_tree_incremental(int node_id) {
    compute_hash(network[node_id].hash, network[node_id].hash);
}
