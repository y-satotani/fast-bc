
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <igraph.h>
#include "distance_update.h"

int test_edge_set(igraph_vector_ptr_t *E,
                  igraph_vector_ptr_t *Ed,
                  igraph_vector_ptr_t *Etrain,
                  igraph_vector_ptr_t *Edtrain) {
  int test_e = 1;
  int s, ei, d, n = igraph_vector_ptr_size(E);
  for(s = 0; s < n; s++) {
    if(igraph_vector_size(igraph_vector_ptr_e(Ed, s)) !=
       igraph_vector_size(igraph_vector_ptr_e(Edtrain, s))) {
      test_e = 0;
      printf("error 1\n");
      break;
    }
    if(!igraph_vector_all_e(igraph_vector_ptr_e(Ed, s),
                            igraph_vector_ptr_e(Edtrain, s))) {
      test_e = 0;
      printf("error 2\n");
      break;
    }
    igraph_vector_t *Es = (igraph_vector_t*)
      igraph_vector_ptr_e(E, s);
    igraph_vector_t *Estrain = (igraph_vector_t*)
      igraph_vector_ptr_e(Etrain, s);
    ei = 0;
    while(ei < igraph_vector_size(Es)) {
      d = igraph_vector_e(igraph_vector_ptr_e(Ed, s), ei);
      long int pos;
      igraph_vector_t bin1, bin2;
      igraph_vector_init(&bin1, 0);
      igraph_vector_init(&bin2, 0);
      while(ei < igraph_vector_size(Es) &&
            igraph_vector_e(igraph_vector_ptr_e(Ed, s), ei) == d) {
        igraph_real_t e1 = igraph_vector_e(Es, ei);
        igraph_real_t e2 = igraph_vector_e(Estrain, ei);
        igraph_vector_binsearch(&bin1, e1, &pos);
        igraph_vector_insert(&bin1, pos, e1);
        igraph_vector_binsearch(&bin2, e2, &pos);
        igraph_vector_insert(&bin2, pos, e2);
        ei++;
      }
      test_e = igraph_vector_all_e(&bin1, &bin2);
      igraph_vector_destroy(&bin1);
      igraph_vector_destroy(&bin2);
      if(!test_e) break;
    }
    if(!test_e) break;
  }
  return test_e;
}

int main(int argc, char* argv[]) {
  igraph_t G;
  igraph_matrix_t D, S;
  igraph_vector_ptr_t E;
  int n = 30, k = 3;
  int s;

  igraph_k_regular_game(&G, n, k, 0, 0);
  arg_apsp(&D, &S, &E, &G);

  // decrement
  igraph_integer_t eid = igraph_rng_get_integer
    (igraph_rng_default(), 0, igraph_ecount(&G)-1);
  igraph_integer_t u, v;
  igraph_edge(&G, eid, &u, &v);
  igraph_delete_edges(&G, igraph_ess_1(eid));
  update_on_delete(&D, &S, &E, &G, u, v);

  // testing
  igraph_matrix_t Dtrain, Strain;
  igraph_vector_ptr_t Etrain;
  igraph_vector_t Etestvec;
  igraph_vector_init(&Etestvec, 0);
  arg_apsp(&Dtrain, &Strain, &Etrain, &G);
  for(s = 0; s < n; s++) {
    igraph_bool_t test;
    test = igraph_vector_all_e(igraph_vector_ptr_e(&E, s),
                               igraph_vector_ptr_e(&Etrain, s));
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
