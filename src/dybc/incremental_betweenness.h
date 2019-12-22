#ifndef _DYNAMIC_INCREMENTAL_BETWEENNESS_H_
#define _DYNAMIC_INCREMENTAL_BETWEENNESS_H_

#include <igraph/igraph.h>

void update_betw_inc_weighted(igraph_t* G,                  // graph
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
                              igraph_real_t factor,         // multiplier
                              igraph_bool_t is_post_upd);   // post-update flag

void update_betw_inc_unweighted(igraph_t* G,                  // graph
                                igraph_inclist_t* preds,      // predecessors
                                igraph_matrix_t* D,           // distance
                                igraph_matrix_int_t* S,       // geodesics
                                igraph_vector_t* B,           // betweenness
                                igraph_integer_t u,           // endpoint
                                igraph_integer_t v,           // endpoint
                                igraph_integer_t source,      // source
                                igraph_vector_int_t* targets, // targets
                                igraph_real_t factor,         // multiplier
                                igraph_bool_t is_post_upd);   // post-update flag

void update_betw_inc_weighted_statistics(igraph_t* G,
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
                                         igraph_bool_t is_post_upd,
                                         igraph_vector_int_t* traversed);

void update_betw_inc_unweighted_statistics(igraph_t* G,
                                           igraph_inclist_t* preds,
                                           igraph_matrix_t* D,
                                           igraph_matrix_int_t* S,
                                           igraph_vector_t* B,
                                           igraph_integer_t u,
                                           igraph_integer_t v,
                                           igraph_integer_t source,
                                           igraph_vector_int_t* targets,
                                           igraph_real_t factor,
                                           igraph_bool_t is_post_upd,
                                           igraph_vector_int_t* traversed);

#endif // _DYNAMIC_INCREMENTAL_BETWEENNESS_H_
