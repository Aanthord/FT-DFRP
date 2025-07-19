/*
 * FT-DFRP: Fault Recovery System
 *
 * Dual Licensed:
 * 1. AGPL-3.0 for research/academic use
 * 2. Commercial license: contact michael.doran.808@gmail.com
 *
 * Copyright (C) 2025 Michael Doran
 */

#include "parity_types.h"
#include "distribution_policy.h"
#include "routing.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void recover_parity_tag(const char *tag) {
    printf("[RECOVERY] Triggering recovery for parity: %s\n", tag);

    // Step 1: Find all surviving holders
    int *holders = find_nodes_with_parity(tag);
    int count = 0;
    while (holders[count] != -1) count++;
    if (count == 0) {
        printf("[ERROR] No surviving copies for parity '%s'\n", tag);
        return;
    }

    // Step 2: Build minimal recovery tree
    parity_tree_evaluation_t *tree = build_recovery_tree_from_holders(holders, count);

    // Step 3: Evaluate tree for optimal new placement
    int *recovery_targets = evaluate_recovery_tree_efficient(tree);

    // Step 4: Copy parity to targets
    for (int i = 0; i < tree->policy->min_replicas; i++) {
        int target = recovery_targets[i];
        assign_parity_tag(target, tag);
        printf("[RECOVERY] Restored parity '%s' to node %d\n", tag, target);
    }

    // Step 5: Broadcast update
    for (int i = 0; i < tree->policy->min_replicas; i++) {
        announce_parity_holdings(recovery_targets[i]);
    }

    free(holders);
    free(recovery_targets);
    free(tree);
}

int* find_nodes_with_parity(const char *tag) {
    int *results = malloc(sizeof(int) * total_nodes);
    int count = 0;
    for (int i = 0; i < total_nodes; i++) {
        for (int j = 0; j < network[i].parity_count; j++) {
            if (strcmp(network[i].parity_tags[j], tag) == 0) {
                results[count++] = i;
                break;
            }
        }
    }
    results[count] = -1;
    return results;
}

parity_tree_evaluation_t* build_recovery_tree_from_holders(int *holders, int count) {
    parity_tree_evaluation_t *tree = malloc(sizeof(parity_tree_evaluation_t));
    tree->height = log2(count);
    tree->fanout = 2;
    tree->tree_nodes = malloc(sizeof(parity_node_t*) * count);
    for (int i = 0; i < count; i++) {
        tree->tree_nodes[i] = &network[holders[i]];
    }
    tree->eval_function = NULL;
    tree->policy = &default_williams_policy;
    return tree;
}

int* evaluate_recovery_tree_efficient(parity_tree_evaluation_t *tree) {
    int *targets = malloc(sizeof(int) * tree->policy->min_replicas);
    for (int i = 0; i < tree->policy->min_replicas; i++) {
        targets[i] = tree->tree_nodes[i % tree->fanout]->id;
    }
    return targets;
}

void assign_parity_tag(int node_id, const char *tag) {
    TorusNode *node = &network[node_id];
    if (node->parity_count < MAX_PARITY_TAGS) {
        node->parity_tags[node->parity_count] = strdup(tag);
        node->parity_count++;
    }
}
