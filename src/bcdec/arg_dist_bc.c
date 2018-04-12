
#include "arg_dist_bc.h"

void arg_dist_bc(igraph_vector_t* B, igraph_matrix_t* D, igraph_matrix_t* S) {
  long int N = igraph_vector_size(B), i;
  for(i = 0; i < N; i++) {
    igraph_vector_set(B, i, arg_dist_bc1(D, S, i));
  }
}

double arg_dist_bc1(igraph_matrix_t* D, igraph_matrix_t* S, long int i) {
  long int s, t, N = igraph_matrix_nrow(D);
  double b = 0;
  for(s = 0; s < N; s++) {
    if(i == s) continue;
    for(t = s+1; t < N; t++) {
      if(i == t) continue;
      if((int)igraph_matrix_e(D, s, i)+(int)igraph_matrix_e(D, i, t)==
         (int)igraph_matrix_e(D, s, t))
        b += igraph_matrix_e(S, s, i) * igraph_matrix_e(S, i, t) /
          igraph_matrix_e(S, s, t);
    }
  }
  return b;
}
