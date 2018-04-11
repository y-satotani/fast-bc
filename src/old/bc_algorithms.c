
#include "bc_algorithms.h"

void bc_from_arg_apsp(igraph_vector_t* B,
                     igraph_matrix_t* D,
                     igraph_matrix_t* S) {
  int N = igraph_vector_size(B), i, s, t;
  double b;
  for(i = 0; i < N; i++) {
    b = 0;
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
    igraph_vector_set(B, i, b);
  }
}

void bc_from_edges_mat(igraph_matrix_t* B,
                       igraph_matrix_t* D,
                       igraph_matrix_t* S,
                       igraph_vector_ptr_t* E,
                       igraph_vector_t* update_flag) {
  igraph_vector_t zero;
  int s, n = igraph_matrix_ncol(B);
  int ei, e, i, j;
  igraph_vector_init(&zero, n);
  for(s = 0; s < n; s++) {
    if(!igraph_vector_e(update_flag, s))
      continue;
    igraph_matrix_set_row(B, &zero, s);
    igraph_vector_t* edges = (igraph_vector_t*)
      igraph_vector_ptr_e(E, s);

    for(ei = igraph_vector_size(edges)-1; ei >= 0; ei--) {
      e = (igraph_integer_t) igraph_vector_e(edges, ei) % (n*n);
      i = e / n; j = e % n;
      if(i == s) continue;
      igraph_matrix_set
        (B, s, i,
         igraph_matrix_e(B, s, i) +
         igraph_matrix_e(S, s, i) / igraph_matrix_e(S, s, j) *
         (1 + igraph_matrix_e(B, s, j)));
    }
  }
  igraph_vector_destroy(&zero);
}

void bc_from_edges(igraph_vector_t* B,
                   igraph_matrix_t* D,
                   igraph_matrix_t* S,
                   igraph_vector_ptr_t* E) {
  int n = igraph_vector_size(B);
  igraph_matrix_t Bmat;
  igraph_matrix_init(&Bmat, n, n);
  igraph_vector_t update_flag, two;
  igraph_vector_init(&update_flag, n);
  igraph_vector_fill(&update_flag, 1);

  bc_from_edges_mat(&Bmat, D, S, E, &update_flag);
  igraph_matrix_colsum(&Bmat, B);
  igraph_vector_init(&two, n);
  igraph_vector_fill(&two, 2);
  igraph_vector_div(B, &two);

  igraph_matrix_destroy(&Bmat);
  igraph_vector_destroy(&update_flag);
  igraph_vector_destroy(&two);
}

void bc_brandes(igraph_vector_t* B, igraph_t* G) {
  igraph_betweenness(G, B, igraph_vss_all(), 0, 0, 1);
}
