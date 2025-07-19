/*
 * FT-DFRP: Main Fractal Engine (fractal.c)
 *
 * Dual Licensed:
 * 1. AGPL-3.0 for research/academic use
 * 2. Commercial license available - contact michael.doran.808@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <mpi.h>
#include "fractal.h"
#include "ann.h"
#include "memory_guard.h"
#include "routing.h"
#include "parity_types.h"
#include "parity_distribution.h"
#include "parity_broadcast.h"
#include "fhe_stub.h"
#include "merkle.h"

TorusNode *network;
int total_nodes;
int world_rank;
int world_size;
int running = 1;

pthread_t daemon_thread;

// Initialization routine for nodes
void initialize_network(int count, int dim) {
    total_nodes = count;
    network = SAFE_MALLOC(sizeof(TorusNode) * count);
    for (int i = 0; i < count; i++) {
        network[i].id = i;
        network[i].density = drand48();
        network[i].coherence = drand48();
        network[i].vector = SAFE_MALLOC(sizeof(double) * dim);
        randomize_vector(&network[i], dim, 1.0);
        network[i].neighbor_count = 0;
        network[i].parity_count = 0;
        network[i].replication_factor = 3;
        sprintf(network[i].hash, "node%dhash", i);
    }
}

void connect_neighbors(int id, int fanout) {
    TorusNode *node = &network[id];
    for (int i = 0; i < fanout; i++) {
        int neighbor = (id + i + 1) % total_nodes;
        node->neighbors[node->neighbor_count++] = neighbor;
    }
}

void graceful_shutdown() {
    running = 0;
    pthread_join(daemon_thread, NULL);
    for (int i = 0; i < total_nodes; i++) {
        SAFE_FREE(network[i].vector);
    }
    SAFE_FREE(network);
    print_memory_report();
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (argc < 2) {
        if (world_rank == 0) {
            fprintf(stderr, "Usage: %s <total_nodes>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    int dim = VECTOR_DIM;
    initialize_network(atoi(argv[1]), dim);
    for (int i = 0; i < total_nodes; i++) connect_neighbors(i, MAX_NEIGHBORS);

    pthread_create(&daemon_thread, NULL, parity_management_daemon, NULL);

    // Main loop placeholder (CLI or message queue)
    if (world_rank == 0) {
        printf("[FT-DFRP] Node initialized. Running CLI interface...\n");
        run_cli_interface();
    }

    graceful_shutdown();
    MPI_Finalize();
    return 0;
}
