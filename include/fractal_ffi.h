#ifndef FRACTAL_FFI_H
#define FRACTAL_FFI_H

#ifdef __cplusplus
extern "C" {
#endif

// Network state functions
int ffi_add_node(int id, double density, double *vector);
int ffi_query_parity(const char *tag, int *result_nodes, int max_results);
int ffi_compute_route(int from, int to, int *path, int max_hops);

// Parity management
int ffi_announce_parity(int node_id);
int ffi_distribute_parity(const char *tag, int replicas);
char* ffi_get_parity_map();
int ffi_trigger_rebalance();

// ANN functions
double ffi_vector_similarity(int node_a, int node_b);
int ffi_find_k_nearest(int query_node, int k, int *results);

// State export
char* ffi_export_json_state();
int ffi_import_json_state(const char *json_str);
char* ffi_get_merkle_root();

#ifdef __cplusplus
}
#endif

#endif // FRACTAL_FFI_H
