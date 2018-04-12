
#ifndef _ARGDIST_H_
#define _ARGDIST_H_

#include <igraph.h>

void arg_dist(igraph_matrix_t* D,
              igraph_matrix_t* Sigma,
              igraph_t* G);

void edge_set(igraph_vector_ptr_t* E,
              igraph_t* G);

#endif // _ARGDIST_H_
