
#ifndef _ARG_DIST_BC_H_
#define _ARG_DIST_BC_H_

#include <igraph.h>

void arg_dist_bc(igraph_vector_t* B,
                 igraph_matrix_t* D, igraph_matrix_t* Sigma);
double arg_dist_bc1(igraph_matrix_t* D, igraph_matrix_t* Sigma,
                    long int source);

#endif // _ARG_DIST_BC_H_
