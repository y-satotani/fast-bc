#ifndef _DYNAMIC_BETWEENNESS_H_
#define _DYNAMIC_BETWEENNESS_H_

#include <igraph/igraph.h>
#include "graph_info.h"
#include "update_info.h"
#include "dybc_update_stats.h"

void insert_edge(graph_info_t* graph, update_info_t* update);
void delete_edge(graph_info_t* graph, update_info_t* update);
void update_weight(graph_info_t* graph, update_info_t* update);
void decrease_weight(graph_info_t* graph, update_info_t* update);
void increase_weight(graph_info_t* graph, update_info_t* update);

void update_deps_weighted(igraph_t* G,                  // graph
                          igraph_inclist_t* preds,      // predecessors
                          igraph_matrix_t* D,           // distance
                          igraph_matrix_int_t* S,       // geodesics
                          igraph_vector_t* B,           // betweenness
                          igraph_integer_t u,           // endpoint
                          igraph_integer_t v,           // endpoint
                          igraph_integer_t source,      // source
                          igraph_vector_int_t* targets, // targets
                          igraph_vector_t* weights,     // weights
                          igraph_real_t weight,         // weight of u-v
                          igraph_real_t factor);        // multiplier

void update_deps_unweighted(igraph_t* G,                  // graph
                            igraph_inclist_t* preds,      // predecessors
                            igraph_matrix_t* D,           // distance
                            igraph_matrix_int_t* S,       // geodesics
                            igraph_vector_t* B,           // betweenness
                            igraph_integer_t u,           // endpoint
                            igraph_integer_t v,           // endpoint
                            igraph_integer_t source,      // source
                            igraph_vector_int_t* targets, // targets
                            igraph_real_t factor);        // multiplier

void update_deps_weighted_statistics(igraph_t* G,
                                     igraph_inclist_t* preds,
                                     igraph_matrix_t* D,
                                     igraph_matrix_int_t* S,
                                     igraph_vector_t* B,
                                     igraph_integer_t u,
                                     igraph_integer_t v,
                                     igraph_integer_t source,
                                     igraph_vector_int_t* targets,
                                     igraph_vector_t* weights,
                                     igraph_real_t weight,
                                     igraph_real_t factor,
                                     igraph_vector_int_t* traversed_vertices);

void update_deps_unweighted_statistics(igraph_t* G,
                                       igraph_inclist_t* preds,
                                       igraph_matrix_t* D,
                                       igraph_matrix_int_t* S,
                                       igraph_vector_t* B,
                                       igraph_integer_t u,
                                       igraph_integer_t v,
                                       igraph_integer_t source,
                                       igraph_vector_int_t* targets,
                                       igraph_real_t factor,
                                       igraph_vector_int_t* traversed_vertices);

void count_affected_vertices_betw(igraph_t* G,
                                  igraph_vector_int_t* aff_deps_before,
                                  igraph_vector_int_t* aff_deps_after,
                                  igraph_vector_int_t* sources,
                                  dybc_update_stats_t* upd_stats);

#endif // _DYNAMIC_BETWEENNESS_H_
