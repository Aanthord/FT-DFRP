/*
 * FT-DFRP: Fractal Toroidal Density Field Routing Protocol - Hybrid Routing Logic
 *
 * Dual Licensed:
 * 1. AGPL-3.0 for research/academic use: https://www.gnu.org/licenses/agpl-3.0.html
 * 2. Commercial license available - contact michael.doran.808@gmail.com for terms
 *
 * Copyright (C) 2025 Michael Doran
 */

#include "fractal.h"
#include <math.h>

int compute_hybrid_next_hop(int current_id, const double *target_vector, routing_config_t *config) {
    TorusNode *current = &network[current_id];
    int best_id = -1;
    double best_score = -INFINITY;

    for (int i = 0; i < current->neighbor_count; i++) {
        int neighbor_id = current->neighbors[i];
        TorusNode *neighbor = &network[neighbor_id];

        double density = config->use_fhe ?
            fhe_decrypt(neighbor->encrypted_density) : neighbor->density;

        double similarity = target_vector ?
            cosine_similarity(neighbor->vector, target_vector, VECTOR_DIM) : 0.0;

        double coherence = neighbor->coherence;

        double score = config->density_weight * density +
                       config->similarity_weight * similarity +
                       config->coherence_weight * coherence;

        if (score > best_score) {
            best_score = score;
            best_id = neighbor_id;
        }
    }

    return best_id;
}

int compute_parity_aware_route(int current_id, const char *parity_tag, routing_config_t *config) {
    int *holders = find_nodes_with_parity(parity_tag);
    if (!holders || holders[0] == -1) return compute_hybrid_next_hop(current_id, NULL, config);

    TorusNode *current = &network[current_id];
    int best_id = -1;
    double best_score = -INFINITY;

    for (int i = 0; i < current->neighbor_count; i++) {
        int neighbor_id = current->neighbors[i];
        double min_dist = INFINITY;

        for (int j = 0; holders[j] != -1; j++) {
            double dist = calculate_network_distance(neighbor_id, holders[j]);
            if (dist < min_dist) min_dist = dist;
        }

        double hybrid_score = compute_node_hybrid_score(neighbor_id, config);
        double parity_score = 1.0 / (1.0 + min_dist);

        double score = config->parity_weight * parity_score +
                       (1.0 - config->parity_weight) * hybrid_score;

        if (score > best_score) {
            best_score = score;
            best_id = neighbor_id;
        }
    }

    return best_id;
}

double compute_node_hybrid_score(int node_id, routing_config_t *config) {
    TorusNode *node = &network[node_id];
    double density = config->use_fhe ? fhe_decrypt(node->encrypted_density) : node->density;
    double similarity = cosine_similarity(node->vector, global_query_vector, VECTOR_DIM);
    double coherence = node->coherence;

    return config->density_weight * density +
           config->similarity_weight * similarity +
           config->coherence_weight * coherence;
}
