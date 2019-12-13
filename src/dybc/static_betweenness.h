
#ifndef _STATIC_BETWEENNESS_H_
#define _STATIC_BETWEENNESS_H_

#include <igraph/igraph.h>

void betweenness_with_redundant_information(igraph_t* G,
                                            igraph_matrix_t* D,
                                            igraph_matrix_int_t* S,
                                            igraph_vector_t* B,
                                            igraph_vector_t* weight);

#endif // _STATIC_BETWEENNESS_H_
