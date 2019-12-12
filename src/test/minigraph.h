
#ifndef _MINIGRAPH_H_
#define _MINIGRAPH_H_

#include <igraph/igraph.h>
#include <assert.h>

void make_test_graph(igraph_t* G, igraph_vector_t* weight);

void make_less_graph_and_edge(igraph_t* G,
                              igraph_integer_t* v,
                              igraph_integer_t* w,
                              igraph_real_t* c,
                              igraph_vector_t* weight);

void make_more_graph_and_edge(igraph_t* G,
                              igraph_integer_t* v,
                              igraph_integer_t* w,
                              igraph_real_t* c,
                              igraph_vector_t* weight);

int check_aug_dist_of_minigraph(igraph_matrix_t* D,
                                igraph_matrix_int_t* Sigma);

int check_betweenness_of_minigraph(igraph_vector_t* vec);

int check_aug_dist_of_less(igraph_matrix_t* D,
                           igraph_matrix_int_t* Sigma);

int check_aug_dist_of_more(igraph_matrix_t* D,
                           igraph_matrix_int_t* Sigma);

#endif
