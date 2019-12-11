
#ifndef _DECREMENTAL_BETWEENNESS_H_
#define _DECREMENTAL_BETWEENNESS_H_

#include <igraph/igraph.h>
#include <igraph/igraph_types_internal.h>

void update_sssp_dec_weighted(igraph_t* G,               // graph
                              igraph_inclist_t* preds,   // predecessors list
                              igraph_inclist_t* succs,   // successors list
                              igraph_matrix_t* D,        // distance matrix
                              igraph_matrix_int_t* S,    // geodesics matrix
                              igraph_integer_t u,        // endpoint
                              igraph_integer_t v,        // endpoint
                              igraph_integer_t source,   // source vertex
                              igraph_vector_t* weights,  // weight vector
                              igraph_real_t weight);     // weight of u-v

void update_stsp_dec_weighted(igraph_t* G,               // graph
                              igraph_inclist_t* preds,   // predecessors list
                              igraph_inclist_t* succs,   // successors list
                              igraph_matrix_t* D,        // distance matrix
                              igraph_matrix_int_t* S,    // geodesics matrix
                              igraph_integer_t u,        // endpoint
                              igraph_integer_t v,        // endpoint
                              igraph_integer_t target,   // target vertex
                              igraph_vector_t* weights,  // weight vector
                              igraph_real_t weight);     // weight of u-v

void affected_targets_dec(igraph_t* G,               // graph
                          igraph_inclist_t* inclist, // incident list
                          igraph_vector_int_t* out,  // result
                          igraph_matrix_t* D,        // distance matrix
                          igraph_integer_t u,        // endpoint to be added
                          igraph_integer_t v,        // endpoint to be added
                          igraph_integer_t source,   // source vertex
                          igraph_vector_t* weights,  // weight vector (opt)
                          igraph_real_t weight);     // weight of u-v (opt)

void affected_sources_dec(igraph_t* G,               // graph
                          igraph_inclist_t* inclist, // incident list
                          igraph_vector_int_t* out,  // result
                          igraph_matrix_t* D,        // distance matrix
                          igraph_integer_t u,        // endpoint to be added
                          igraph_integer_t v,        // endpoint to be added
                          igraph_integer_t target,   // target vertex
                          igraph_vector_t* weights,  // weight vector (opt)
                          igraph_real_t weight);     // weight of u-v (opt)

#endif // _DECREMENTAL_BETWEENNESS_H_
