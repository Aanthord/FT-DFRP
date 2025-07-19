/*
 * FT-DFRP: Fractal Toroidal Density Field Routing Protocol - ANN Implementation
 *
 * Dual Licensed:
 * 1. AGPL-3.0 for research/academic use: https://www.gnu.org/licenses/agpl-3.0.html
 * 2. Commercial license available - contact michael.doran.808@gmail.com for terms
 *
 * Copyright (C) 2025 Michael Doran
 */

#include "ann.h"
#include <string.h>

similarity_heap_t* create_similarity_heap(int capacity) {
    similarity_heap_t *heap = (similarity_heap_t*)malloc(sizeof(similarity_heap_t));
    heap->results = (similarity_result_t*)malloc(sizeof(similarity_result_t) * capacity);
    heap->count = 0;
    heap->capacity = capacity;
    return heap;
}

void heap_insert(similarity_heap_t *heap, int node_id, double similarity, double combined_score) {
    if (heap->count < heap->capacity) {
        heap->results[heap->count++] = (similarity_result_t){ node_id, similarity, combined_score };
    } else {
        int min_idx = 0;
        for (int i = 1; i < heap->count; i++) {
            if (heap->results[i].combined_score < heap->results[min_idx].combined_score) {
                min_idx = i;
            }
        }
        if (combined_score > heap->results[min_idx].combined_score) {
            heap->results[min_idx] = (similarity_result_t){ node_id, similarity, combined_score };
        }
    }
}

void heap_free(similarity_heap_t *heap) {
    if (heap) {
        free(heap->results);
        free(heap);
    }
}

similarity_result_t* find_k_nearest(TorusNode *network, int total_nodes, int query_node, int k) {
    similarity_heap_t *heap = create_similarity_heap(k);
    TorusNode *query = &network[query_node];

    for (int i = 0; i < total_nodes; i++) {
        if (i == query_node) continue;
        double similarity = cosine_similarity(query->vector, network[i].vector, VECTOR_DIM);
        double score = similarity * query->coherence + network[i].density;
        heap_insert(heap, i, similarity, score);
    }

    return heap->results;
}

void inject_vector(TorusNode *node, const double *vector, int dim) {
    if (!node->vector) node->vector = (double*)malloc(sizeof(double) * dim);
    memcpy(node->vector, vector, sizeof(double) * dim);
    node->density = 1.0;  // assume injected vectors are dense
}

void randomize_vector(TorusNode *node, int dim, double range) {
    if (!node->vector) node->vector = (double*)malloc(sizeof(double) * dim);
    for (int i = 0; i < dim; i++) {
        node->vector[i] = ((double)rand() / RAND_MAX) * range - (range / 2.0);
    }
    vector_normalize(node->vector, dim);
    node->density = 1.0;
}

void evolve_vector(TorusNode *node, double learning_rate, const double *target) {
    if (!node->vector) return;
    for (int i = 0; i < VECTOR_DIM; i++) {
        node->vector[i] += learning_rate * (target[i] - node->vector[i]);
    }
    vector_normalize(node->vector, VECTOR_DIM);
}
