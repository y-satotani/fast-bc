
#ifndef _INCREMENTAL_SHORTEST_PATH_H_
#define _INCREMENTAL_SHORTEST_PATH_H_

#include <igraph/igraph.h>
#include <igraph/igraph_types_internal.h>
#include "dybc_update_stats.h"

void update_sssp_inc_weighted(igraph_t* G,               // graph
                              igraph_inclist_t* preds,   // predecessors list
                              igraph_inclist_t* succs,   // successors list
                              igraph_matrix_t* D,        // distance matrix
                              igraph_matrix_int_t* S,    // geodesics matrix
                              igraph_integer_t u,        // endpoint
                              igraph_integer_t v,        // endpoint
                              igraph_integer_t source,   // source vertex
                              igraph_vector_t* weights,  // weight vector
                              igraph_real_t weight,      // weight of u-v
                              igraph_bool_t post_mod);   // post-mod. flag

void update_stsp_inc_weighted(igraph_t* G,               // graph
                              igraph_inclist_t* preds,   // predecessors list
                              igraph_inclist_t* succs,   // successors list
                              igraph_matrix_t* D,        // distance matrix
                              igraph_matrix_int_t* S,    // geodesics matrix
                              igraph_integer_t u,        // endpoint
                              igraph_integer_t v,        // endpoint
                              igraph_integer_t target,   // target vertex
                              igraph_vector_t* weights,  // weight vector
                              igraph_real_t weight,      // weight of u-v
                              igraph_bool_t post_mod);   // post-mod. flag

void update_sssp_inc_unweighted(igraph_t* G,             // graph
                                igraph_inclist_t* preds, // predecessors list
                                igraph_inclist_t* succs, // successors list
                                igraph_matrix_t* D,      // distance matrix
                                igraph_matrix_int_t* S,  // geodesics matrix
                                igraph_integer_t u,      // endpoint
                                igraph_integer_t v,      // endpoint
                                igraph_integer_t source, // source vertex
                                igraph_bool_t post_mod); // post-mod. flag

void update_stsp_inc_unweighted(igraph_t* G,             // graph
                                igraph_inclist_t* preds, // predecessors list
                                igraph_inclist_t* succs, // successors list
                                igraph_matrix_t* D,      // distance matrix
                                igraph_matrix_int_t* S,  // geodesics matrix
                                igraph_integer_t u,      // endpoint
                                igraph_integer_t v,      // endpoint
                                igraph_integer_t target, // target vertex
                                igraph_bool_t post_mod); // post-mod. flag

void affected_targets_inc(igraph_t* G,               // graph
                          igraph_inclist_t* inclist, // incident list
                          igraph_vector_int_t* out,  // result
                          igraph_matrix_t* D,        // distance matrix
                          igraph_integer_t u,        // endpoint to be added
                          igraph_integer_t v,        // endpoint to be added
                          igraph_integer_t source,   // source vertex
                          igraph_vector_t* weights,  // weight vector (opt)
                          igraph_real_t weight,      // weight of u-v (opt)
                          igraph_bool_t post_mod);   // post-mod. flag


void affected_sources_inc(igraph_t* G,               // graph
                          igraph_inclist_t* inclist, // incident list
                          igraph_vector_int_t* out,  // result
                          igraph_matrix_t* D,        // distance matrix
                          igraph_integer_t u,        // endpoint to be added
                          igraph_integer_t v,        // endpoint to be added
                          igraph_integer_t target,   // target vertex
                          igraph_vector_t* weights,  // weight vector (opt)
                          igraph_real_t weight,      // weight of u-v (opt)
                          igraph_bool_t post_mod);   // post-mod. flag

void count_affected_vertices_path_inc(igraph_t* G,
                                      igraph_inclist_t* preds,
                                      igraph_inclist_t* succs,
                                      igraph_matrix_t* D,
                                      igraph_matrix_int_t* S,
                                      igraph_vector_t* B,
                                      igraph_integer_t u,
                                      igraph_integer_t v,
                                      igraph_vector_t* weights,
                                      igraph_real_t weight,
                                      dybc_update_stats_t* upd_stats,
                                      igraph_bool_t post_mod);

#endif // _INCREMENTAL_SHORTEST_PATH_H_
