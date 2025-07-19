/*
 * FT-DFRP: CLI Command Layer
 *
 * Dual Licensed:
 * 1. AGPL-3.0 for research/academic use
 * 2. Commercial license: contact michael.doran.808@gmail.com
 *
 * Copyright (C) 2025 Michael Doran
 */

#include "fractal.h"
#include "ann.h"
#include "memory_guard.h"
#include "parity_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void run_cli(int argc, char **argv) {
    if (argc < 2) {
        printf("[USAGE] fractal <command> [args]\n");
        return;
    }

    if (strcmp(argv[1], "injectvec") == 0 && argc >= 11) {
        int id = atoi(argv[2]);
        double vec[VECTOR_DIM];
        for (int i = 0; i < VECTOR_DIM; i++) {
            vec[i] = atof(argv[3 + i]);
        }
        inject_vector(&network[id], vec, VECTOR_DIM);
        printf("[OK] Vector injected into node %d\n", id);
    } 
    else if (strcmp(argv[1], "findnearest") == 0 && argc == 4) {
        int id = atoi(argv[2]);
        int k = atoi(argv[3]);
        similarity_result_t *res = find_k_nearest(network, total_nodes, id, k);
        printf("[RESULT] Nearest to %d:\n", id);
        for (int i = 0; i < k; i++) {
            printf("  #%d -> Node %d | Similarity: %.4f | Score: %.4f\n",
                   i, res[i].node_id, res[i].similarity, res[i].combined_score);
        }
        free(res);
    }
    else if (strcmp(argv[1], "announce") == 0 && argc == 3) {
        int id = atoi(argv[2]);
        announce_parity_holdings(id);
    } 
    else if (strcmp(argv[1], "recovery") == 0 && argc == 3) {
        recover_parity_tag(argv[2]);
    } 
    else if (strcmp(argv[1], "testann") == 0) {
        run_ann_tests();
    } 
    else if (strcmp(argv[1], "checkmem") == 0) {
        print_memory_report();
    } 
    else if (strcmp(argv[1], "detectleaks") == 0) {
        detect_memory_leaks();
    } 
    else {
        printf("[ERROR] Unknown command '%s'\n", argv[1]);
    }
}

int main(int argc, char **argv) {
    run_cli(argc, argv);
    return 0;
}
