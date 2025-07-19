#ifndef FRACTAL_H
#define FRACTAL_H

#include "parity_types.h"

#define VECTOR_DIM 8
#define MAX_NEIGHBORS 16
#define MAX_PARITY_TAGS 32
#define MAX_HASH_SIZE 65

extern int total_nodes;
extern TorusNode *network;

void run_cli(int argc, char **argv);

#endif // FRACTAL_H
