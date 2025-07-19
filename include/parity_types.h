#ifndef PARITY_TYPES_H
#define PARITY_TYPES_H

#include <stdint.h>
#include <time.h>
#include "fractal.h"

typedef struct {
    int node_id;
    char parity_tags[MAX_PARITY_TAGS][64];
    int parity_count;
    double load_factor;
    time_t timestamp;
    char signature[MAX_HASH_SIZE];
} parity_announcement_t;

typedef struct {
    char tag[64];
    int holder_nodes[MAX_PARITY_TAGS];
    int replica_count;
    double distribution_score;
    time_t last_updated;
} parity_distribution_entry_t;

typedef struct TorusNode {
    int id;
    double density;
    double coherence;
    int neighbors[MAX_NEIGHBORS];
    int neighbor_count;
    char *parity_tags[MAX_PARITY_TAGS];
    int parity_count;
    char hash[MAX_HASH_SIZE];
    double vector[VECTOR_DIM];

    // Parity broadcast
    parity_announcement_t *known_parity_map;
    int map_size;
    time_t last_announcement;
    int replication_factor;

#ifdef ENABLE_FHE
    fhe_ciphertext_t encrypted_density;
#endif
} TorusNode;

#endif // PARITY_TYPES_H
