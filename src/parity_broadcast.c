/*
 * FT-DFRP: Parity Broadcast and Distribution System
 * 
 * Dual Licensed:
 * 1. AGPL-3.0 for research/academic use: https://www.gnu.org/licenses/agpl-3.0.html
 * 2. Commercial license available - contact michael.doran.808@gmail.com for terms
 * 
 * Copyright (C) 2025 Michael Doran
 */

#include "fractal.h"
#include "parity_types.h"
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void sign_announcement(parity_announcement_t *a) {
    snprintf(a->signature, MAX_HASH_SIZE, "SIG-%d-%ld", a->node_id, a->timestamp);
}

void update_parity_knowledge_map(int node_id, parity_announcement_t *a) {
    TorusNode *n = &network[node_id];
    if (n->map_size < MAX_PARITY_TAGS) {
        n->known_parity_map[n->map_size++] = *a;
    }
}

void announce_parity_holdings(int node_id) {
    TorusNode *node = &network[node_id];
    parity_announcement_t a;
    a.node_id = node_id;
    a.parity_count = node->parity_count;
    a.load_factor = calculate_node_load(node_id);
    a.timestamp = get_current_timestamp();
    for (int i = 0; i < a.parity_count; i++) {
        strncpy(a.parity_tags[i], node->parity_tags[i], 63);
    }
    sign_announcement(&a);
    MPI_Bcast(&a, sizeof(parity_announcement_t), MPI_BYTE, node_id, MPI_COMM_WORLD);
    update_parity_knowledge_map(node_id, &a);
}

void build_announcement(int node_id, parity_announcement_t *a) {
    TorusNode *n = &network[node_id];
    a->node_id = node_id;
    a->parity_count = n->parity_count;
    a->load_factor = calculate_node_load(node_id);
    a->timestamp = get_current_timestamp();
    for (int i = 0; i < n->parity_count; i++) {
        strncpy(a->parity_tags[i], n->parity_tags[i], 63);
    }
    sign_announcement(a);
}

void send_announcement_to_neighbor(int neighbor_id, parity_announcement_t *a) {
    MPI_Send(a, sizeof(parity_announcement_t), MPI_BYTE, neighbor_id, 0, MPI_COMM_WORLD);
}

void gossip_parity_announcement(int node_id) {
    TorusNode *n = &network[node_id];
    parity_announcement_t a;
    build_announcement(node_id, &a);
    int gossip_targets = (n->neighbor_count < 3) ? n->neighbor_count : 3;
    for (int i = 0; i < gossip_targets; i++) {
        int target = n->neighbors[rand() % n->neighbor_count];
        send_announcement_to_neighbor(target, &a);
    }
}
