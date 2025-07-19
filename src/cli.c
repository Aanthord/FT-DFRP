/*
 * FT-DFRP: Fractal Toroidal Density Field Routing Protocol - CLI Interface
 *
 * Dual Licensed:
 * 1. AGPL-3.0 for research/academic use: https://www.gnu.org/licenses/agpl-3.0.html
 * 2. Commercial license available - contact michael.doran.808@gmail.com for terms
 *
 * Copyright (C) 2025 Michael Doran
 */

#include "fractal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./fractal <command> [args]\n");
        return 1;
    }

    if (strcmp(argv[1], "injectvec") == 0 && argc >= 10) {
        int node_id = atoi(argv[2]);
        double vec[VECTOR_DIM];
        for (int i = 0; i < VECTOR_DIM; i++) vec[i] = atof(argv[3 + i]);
        inject_vector(&network[node_id], vec, VECTOR_DIM);
        printf("Injected vector into node %d\n", node_id);

    } else if (strcmp(argv[1], "findnearest") == 0 && argc == 4) {
        int node_id = atoi(argv[2]);
        int k = atoi(argv[3]);
        similarity_result_t *results = find_k_nearest(network, total_nodes, node_id, k);
        printf("Top %d nearest nodes to node %d:\n", k, node_id);
        for (int i = 0; i < k; i++) {
            printf("  Node %d: sim=%.4f score=%.4f\n",
                   results[i].node_id, results[i].similarity, results[i].combined_score);
        }
        free(results);

    } else if (strcmp(argv[1], "vectorstats") == 0 && argc == 3) {
        int node_id = atoi(argv[2]);
        TorusNode *node = &network[node_id];
        printf("Node %d: density=%.4f coherence=%.4f\n", node_id, node->density, node->coherence);
        printf("Vector: [");
        for (int i = 0; i < VECTOR_DIM; i++) {
            printf("%.3f%s", node->vector[i], (i < VECTOR_DIM-1) ? ", " : "]\n");
        }

    } else if (strcmp(argv[1], "evolveann") == 0 && argc == 4) {
        int node_id = atoi(argv[2]);
        double rate = atof(argv[3]);
        TorusNode *node = &network[node_id];
        evolve_vector(node, rate, global_query_vector);
        printf("Evolved vector for node %d\n", node_id);

    } else {
        printf("Unknown or invalid command\n");
    }

    return 0;
}
