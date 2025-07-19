/*
 * FT-DFRP: Parity Distribution Engine
 *
 * Dual Licensed:
 * 1. AGPL‑3.0 for research/academic use
 * 2. Commercial license: contact michael.doran.808@gmail.com
 *
 * Copyright (C) 2025 Michael Doran
 */

#include "parity_types.h"
#include "distribution_policy.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// Builds the parity computation graph
static parity_computation_graph_t* build_parity_computation_graph() {
    parity_computation_graph_t *graph = malloc(sizeof(*graph));
    graph->node_count = total_nodes;
    graph->nodes = malloc(sizeof(parity_node_t) * total_nodes);
    graph->global_scores = malloc(sizeof(double) * total_nodes);
    graph->adjacency_matrix = NULL; // placeholder for RTT-based future
    for (int i = 0; i < total_nodes; i++) {
        TorusNode *n = &network[i];
        parity_node_t *p = &graph->nodes[i];
        p->node_id = n->id;
        p->rtt_latency = 1.0; // default, improve with measurement
        p->centrality_score = 1.0; // placeholder
        p->current_load = n->parity_count;
        p->last_access = n->last_announcement;
        graph->global_scores[i] = 0.0;
    }
    graph->tree_height = (int)log2(total_nodes);
    return graph;
}

// Constructs a Williams tree from the graph
static parity_tree_evaluation_t* construct_placement_tree(
        parity_computation_graph_t *graph,
        const williams_distribution_policy_t *policy) {
    parity_tree_evaluation_t *tree = malloc(sizeof(*tree));
    tree->height = graph->tree_height;
    tree->fanout = (int)sqrt(graph->node_count);
    if (tree->fanout < 2) tree->fanout = 2;
    tree->tree_nodes = malloc(sizeof(parity_node_t*) * graph->node_count);
    for (int i = 0; i < graph->node_count; i++) {
        tree->tree_nodes[i] = &graph->nodes[i];
    }
    tree->eval_function = calculate_williams_placement_score;
    tree->policy = (williams_distribution_policy_t*)policy;
    return tree;
}

// Recursively evaluates tree to compute scores
double evaluate_parity_placement_tree(
        parity_tree_evaluation_t *tree, int node_index) {
    if (tree->height == 0) {
        parity_node_t *node = tree->tree_nodes[node_index];
        return tree->policy->rtt_weight / (1 + node->rtt_latency)
             + tree->policy->load_balance_weight * (1.0 - node->current_load / (double)MAX_PARITY_TAGS)
             + tree->policy->knn_similarity_weight * node->centrality_score
             + tree->policy->centrality_weight * node->centrality_score;
    }
    double best = -INFINITY;
    int start = node_index * tree->fanout + 1;
    for (int i = 0; i < tree->fanout; i++) {
        int child = start + i;
        if (child < (int)pow(tree->fanout, tree->height + 1)) {
            double score = evaluate_parity_placement_tree(tree, child);
            if (score > best) best = score;
        }
    }
    return best;
}

// Identifies top-K candidate nodes for placement
static int* select_tree_optimal_nodes(
        parity_computation_graph_t *graph,
        double *scores,
        const williams_distribution_policy_t *policy) {

    int K = policy->min_replicas;
    int *selected = malloc(sizeof(int) * K);
    for (int i = 0; i < K; i++) {
        double best = -INFINITY;
        int best_idx = -1;
        for (int j = 0; j < graph->node_count; j++) {
            if (scores[j] > best) {
                best = scores[j];
                best_idx = j;
            }
        }
        selected[i] = graph->nodes[best_idx].node_id;
        scores[best_idx] = -INFINITY;
    }
    return selected;
}

// Main entry to distribute a parity bit
int* distribute_parity_with_tree_evaluation(
        const char *new_parity_tag,
        williams_distribution_policy_t *policy) {

    printf("[DISTRIBUTION] Placing parity '%s' …\n", new_parity_tag);

    // Build graph & placement tree
    parity_computation_graph_t *graph = build_parity_computation_graph();
    parity_tree_evaluation_t *tree = construct_placement_tree(graph, policy);

    // Compute scores
    double *scores = malloc(sizeof(double) * graph->node_count);
    for (int i = 0; i < graph->node_count; i++) {
        scores[i] = evaluate_parity_placement_tree(tree, i);
    }

    // Select nodes
    int *chosen = select_tree_optimal_nodes(graph, scores, policy);

    // Assign and broadcast
    for (int i = 0; i < policy->min_replicas; i++) {
        int nid = chosen[i];
        assign_parity_tag(nid, new_parity_tag);
        announce_parity_holdings(nid);
        printf("[DISTRIBUTION] Assigned parity '%s' to node %d\n", new_parity_tag, nid);
    }

    free(graph->nodes);
    free(graph);
    free(tree->tree_nodes);
    free(tree);
    free(scores);
    return chosen;
}
