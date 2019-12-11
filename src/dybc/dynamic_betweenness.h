
#ifndef _DYNAMIC_BETWEENNESS_H_
#define _DYNAMIC_BETWEENNESS_H_

#include <igraph/igraph.h>
#include "graph_info.h"
#include "update_info.h"

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



#endif // _DYNAMIC_BETWEENNESS_H_
