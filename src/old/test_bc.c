
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <igraph.h>
#include "distance_update.h"
#include "bc_algorithms.h"

void make_test_graph(igraph_t* G) {
  igraph_empty(G, 6, IGRAPH_UNDIRECTED);
  //E = [(0, 1), (0, 2), (2, 3), (2, 4), (4, 5)]
  igraph_add_edge(G, 0, 1);
  igraph_add_edge(G, 0, 2);
  igraph_add_edge(G, 2, 3);
  igraph_add_edge(G, 2, 4);
  igraph_add_edge(G, 4, 5);
}

int main(int argc, char* argv[]) {
  igraph_t G;
  igraph_matrix_t D, S;
  igraph_vector_ptr_t E;
  int n = 30, k = 3;

  //igraph_k_regular_game(&G, n, k, 0, 0);
  igraph_barabasi_game(&G, n, 1, k,
                       0, 0, 1, 0, IGRAPH_BARABASI_PSUMTREE, 0);
  arg_apsp(&D, &S, &E, &G);

  igraph_vector_t B1, B2, B3, Errvec;
  // 1. igraph
  igraph_vector_init(&B1, n);
  bc_brandes(&B1, &G);
  // 2. proposed (argumented apsp)
  igraph_vector_init(&B2, n);
  bc_from_arg_apsp(&B2, &D, &S);
  // 3. proposed (from edges)
  igraph_vector_init(&B3, n);
  bc_from_edges(&B3, &D, &S, &E);
  // error as L2 norm
  igraph_vector_init(&Errvec, n);
  igraph_vector_update(&Errvec, &B1);
  igraph_vector_sub(&Errvec, &B2);
  igraph_vector_mul(&Errvec, &Errvec);
  double Err1 = igraph_vector_sum(&Errvec);
  igraph_vector_update(&Errvec, &B1);
  igraph_vector_sub(&Errvec, &B3);
  igraph_vector_mul(&Errvec, &Errvec);
  double Err2 = igraph_vector_sum(&Errvec);
  printf("error1: %f error2: %f\n", Err1, Err2);

  igraph_vector_destroy(&B1);
  igraph_vector_destroy(&B2);
  igraph_vector_destroy(&B3);
  igraph_vector_destroy(&Errvec);

  igraph_destroy(&G);
  igraph_matrix_destroy(&D);
  igraph_matrix_destroy(&S);
  igraph_vector_ptr_free_all(&E);
  igraph_vector_ptr_destroy(&E);
  return 0;
}
