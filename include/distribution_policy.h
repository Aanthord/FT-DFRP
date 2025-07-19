#ifndef DISTRIBUTION_POLICY_H
#define DISTRIBUTION_POLICY_H

#include "parity_types.h"

typedef struct {
    double rtt_weight;
    double load_balance_weight;
    double knn_similarity_weight;
    double centrality_weight;
    int min_replicas;
    int max_replicas;
    int tree_evaluation_depth;
} williams_distribution_policy_t;

// Default policy instance
extern williams_distribution_policy_t default_williams_policy;

#endif // DISTRIBUTION_POLICY_H
