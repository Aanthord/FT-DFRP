/*
 * FT-DFRP: Fractal Toroidal Density Field Routing Protocol - ANN Module
 *
 * Dual Licensed:
 * 1. AGPL-3.0 for research/academic use: https://www.gnu.org/licenses/agpl-3.0.html
 * 2. Commercial license available - contact michael.doran.808@gmail.com for terms
 *
 * Copyright (C) 2025 Michael Doran
 */

#ifndef ANN_H
#define ANN_H

#include <math.h>
#include <stdlib.h>

#define VECTOR_DIM 8
#define MAX_NEIGHBORS 16
#define MAX_PARITY_TAGS 32
#define MAX_HASH_SIZE 64
#define MAX_REPLICAS 8

typedef struct {
    double *data;
    int dim;
    int id;
} vector_t;

typedef struct {
    int node_id;
    double similarity;
    double combined_score;
} similarity_result_t;

typedef struct {
    similarity_result_t *results;
    int count;
    int capacity;
} similarity_heap_t;

typedef struct {
    int id;
    double density;
    double coherence;
    int neighbors[MAX_NEIGHBORS];
    int neighbor_count;
    char *parity_tags[MAX_PARITY_TAGS];
    int parity_count;
    char hash[MAX_HASH_SIZE];
    double *vector;
    int replication_factor;
} TorusNode;

// Core vector operations
static inline double cosine_similarity(const double *a, const double *b, int dim) {
    double dot = 0.0, norm_a = 0.0, norm_b = 0.0;
    for (int i = 0; i < dim; i++) {
        dot += a[i] * b[i];
        norm_a += a[i] * a[i];
        norm_b += b[i] * b[i];
    }
    return (norm_a == 0 || norm_b == 0) ? 0.0 : dot / (sqrt(norm_a) * sqrt(norm_b));
}

static inline double euclidean_distance(const double *a, const double *b, int dim) {
    double sum = 0.0;
    for (int i = 0; i < dim; i++) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

static inline void vector_normalize(double *vec, int dim) {
    double norm = 0.0;
    for (int i = 0; i < dim; i++) norm += vec[i] * vec[i];
    norm = sqrt(norm);
    for (int i = 0; i < dim; i++) vec[i] /= (norm + 1e-8);
}

static inline void vector_add_weighted(double *dest, const double *src, double weight, int dim) {
    for (int i = 0; i < dim; i++) dest[i] += weight * src[i];
}

// ANN search function declarations
similarity_heap_t* create_similarity_heap(int capacity);
void heap_insert(similarity_heap_t *heap, int node_id, double similarity, double combined_score);
void heap_free(similarity_heap_t *heap);
similarity_result_t* find_k_nearest(TorusNode *network, int total_nodes, int query_node, int k);

// Vector injection and management
void inject_vector(TorusNode *node, const double *vector, int dim);
void randomize_vector(TorusNode *node, int dim, double range);
void evolve_vector(TorusNode *node, double learning_rate, const double *target);

#endif // ANN_H
