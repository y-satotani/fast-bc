
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <igraph.h>
#include "distance_update.h"

int main(int argc, char* argv[]) {
  igraph_t G;
  igraph_matrix_t D, S;
  igraph_vector_ptr_t E;
  int n = 30, k = 2;
  int s;

  //igraph_k_regular_game(&G, n, k, 0, 0);
  igraph_barabasi_game(&G, n, 1, k,
                       0, 0, 1, 0, IGRAPH_BARABASI_PSUMTREE, 0);
  arg_apsp(&D, &S, &E, &G);
  for(s = 0; s < n; s++) {
    int ei;
    igraph_vector_t* edges = (igraph_vector_t*)
      igraph_vector_ptr_e(&E, s);
    printf("A: ");
    for(ei = 0; ei < igraph_vector_size(edges); ei++) {
      int d = (igraph_integer_t)igraph_vector_e(edges, ei) / (n*n);
      int e = (igraph_integer_t)igraph_vector_e(edges, ei) % (n*n);
      int i = e / n;
      int j = e % n;
      printf("(%d %d %d) ", d, i, j);
    }
    printf("\n");
  }

  // decrement
  igraph_integer_t eid = igraph_rng_get_integer
    (igraph_rng_default(), 0, igraph_ecount(&G)-1);
  igraph_integer_t u, v;
  igraph_edge(&G, eid, &u, &v);
  igraph_delete_edges(&G, igraph_ess_1(eid));
  update_on_delete(&D, &S, &E, 0, &G, u, v);

  // testing
  igraph_matrix_t Dtrain, Strain;
  igraph_vector_ptr_t Etrain;
  igraph_vector_t Etestvec;
  igraph_vector_init(&Etestvec, 0);
  arg_apsp(&Dtrain, &Strain, &Etrain, &G);
  for(s = 0; s < n; s++) {
    int ei;
    igraph_vector_t* edges = (igraph_vector_t*)
      igraph_vector_ptr_e(&E, s);
    igraph_vector_t* edgestrain = (igraph_vector_t*)
      igraph_vector_ptr_e(&Etrain, s);
    printf("E: ");
    for(ei = 0; ei < igraph_vector_size(edges); ei++) {
      int d = (igraph_integer_t)igraph_vector_e(edges, ei) / (n*n);
      int e = (igraph_integer_t)igraph_vector_e(edges, ei) % (n*n);
      int i = e / n;
      int j = e % n;
      printf("(%d %d %d) ", d, i, j);
    }
    printf("\n");
    printf("T: ");
    for(ei = 0; ei < igraph_vector_size(edgestrain); ei++) {
      int d = (igraph_integer_t)igraph_vector_e(edgestrain, ei) / (n*n);
      int e = (igraph_integer_t)igraph_vector_e(edgestrain, ei) % (n*n);
      int i = e / n;
      int j = e % n;
      printf("(%d %d %d) ", d, i, j);
    }
    printf("\n");
    igraph_bool_t test;
    test = igraph_vector_all_e(igraph_vector_ptr_e(&E, s),
                               igraph_vector_ptr_e(&Etrain, s));
    printf("test: %d\n\n", test);
    igraph_vector_push_back(&Etestvec, test);
  }
  printf("remove edge: {%d, %d}\n", u, v);
  printf("distance matrix: %d\n", igraph_matrix_all_e(&D, &Dtrain));
  printf("pathnum matrix: %d\n", igraph_matrix_all_e(&S, &Strain));
  printf("edge set: %d\n", (int)igraph_vector_prod(&Etestvec));

  igraph_destroy(&G);
  igraph_matrix_destroy(&D);
  igraph_matrix_destroy(&S);
  igraph_vector_ptr_free_all(&E);
  igraph_vector_ptr_destroy(&E);
  igraph_matrix_destroy(&Dtrain);
  igraph_matrix_destroy(&Strain);
  igraph_vector_ptr_free_all(&Etrain);
  igraph_vector_ptr_destroy(&Etrain);
  igraph_vector_destroy(&Etestvec);
  return 0;
}
