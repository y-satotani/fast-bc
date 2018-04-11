
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <igraph.h>
#include "distance_update.h"
#include "bc_algorithms.h"

char graph_random[] = "random", graph_scale_free[] = "scale-free";

int main(int argc, char* argv[]) {
  if(argc < 5) {
    printf("usage : %s (random|scale-free) n (p|m) seed\n", argv[0]);
    printf("output: n,p-or-m,type,seed,m,err-dm,err-es,time-brandes,time-dm,time-es\n");
    return 1;
  }
  unsigned long seed = strtoul(argv[4], NULL, 0);
  igraph_rng_seed(igraph_rng_default(), seed);

  igraph_t G;
  char* graph_type = argv[1];
  igraph_integer_t n = atoi(argv[2]);
  igraph_real_t p_or_m;

  if(strcmp(graph_type, graph_random) == 0) {
    p_or_m = atof(argv[3]);
    igraph_erdos_renyi_game(&G, IGRAPH_ERDOS_RENYI_GNP,
                            n, p_or_m, 0, 0);
  } else {
    p_or_m = atoi(argv[3]);
    igraph_barabasi_game(&G, n, 1, (igraph_integer_t)p_or_m,
                         0, 0, 1, 0, IGRAPH_BARABASI_PSUMTREE, 0);
  }

  igraph_matrix_t D2, S2, D3, S3;
  igraph_vector_ptr_t E3;
  arg_apsp(&D2, &S2, 0, &G);
  arg_apsp(&D3, &S3, &E3, &G);

  // decrement
  igraph_integer_t eid = igraph_rng_get_integer
    (igraph_rng_default(), 0, igraph_ecount(&G)-1);
  igraph_integer_t u, v;
  igraph_edge(&G, eid, &u, &v);
  igraph_delete_edges(&G, igraph_ess_1(eid));

  igraph_vector_t B1, B2, B3, Errvec;
  double time_brandes, time_distance_matrix, time_edge_set;
  clock_t start, end;
  // 1. igraph
  igraph_vector_init(&B1, n);
  start = clock();
  bc_brandes(&B1, &G);
  end = clock();
  time_brandes = (double)(end - start) / CLOCKS_PER_SEC;

  // 2. proposed (distance matrix)
  igraph_vector_init(&B2, n);
  start = clock();
  update_on_delete(&D2, &S2, 0, 0, 0, u, v);
  bc_from_arg_apsp(&B2, &D2, &S2);
  end = clock();
  time_distance_matrix = (double)(end - start) / CLOCKS_PER_SEC;

  // 3. proposed (edge set)
  igraph_matrix_t B3mat;
  igraph_vector_t B3update, two;
  igraph_matrix_init(&B3mat, n, n);
  igraph_vector_init(&B3update, n);
  igraph_vector_fill(&B3update, 1);
  igraph_vector_init(&two, n);
  igraph_vector_fill(&two, 2);
  bc_from_edges_mat(&B3mat, &D3, &S3, &E3, &B3update);
  igraph_vector_init(&B3, n);
  start = clock();
  update_on_delete(&D3, &S3, &E3, &B3update, &G, u, v);
  bc_from_edges_mat(&B3mat, &D3, &S3, &E3, &B3update);
  igraph_matrix_colsum(&B3mat, &B3);
  igraph_vector_div(&B3, &two);
  end = clock();
  time_edge_set = (double)(end - start) / CLOCKS_PER_SEC;
  igraph_matrix_destroy(&B3mat);
  igraph_vector_destroy(&B3update);
  igraph_vector_destroy(&two);

  // error as L2 norm
  igraph_vector_init(&Errvec, n);
  igraph_vector_update(&Errvec, &B1);
  igraph_vector_sub(&Errvec, &B2);
  igraph_vector_mul(&Errvec, &Errvec);
  double err_distance_matrix = igraph_vector_sum(&Errvec);
  igraph_vector_update(&Errvec, &B1);
  igraph_vector_sub(&Errvec, &B3);
  igraph_vector_mul(&Errvec, &Errvec);
  double err_edge_set = igraph_vector_sum(&Errvec);

  // n,p-or-m,type,seed,m,err-dm,err-es,time-brandes,time-dm,time-es
  printf("%d,%f,%s,%lu,%d,%e,%e,%f,%f,%f\n",
         n, p_or_m, graph_type, seed, igraph_ecount(&G),
         err_distance_matrix, err_edge_set,
         time_brandes, time_distance_matrix, time_edge_set);

  igraph_destroy(&G);
  igraph_vector_destroy(&B1);
  igraph_vector_destroy(&B2);
  igraph_vector_destroy(&B3);
  igraph_vector_destroy(&Errvec);
  igraph_matrix_destroy(&D2);
  igraph_matrix_destroy(&S2);
  igraph_matrix_destroy(&D3);
  igraph_matrix_destroy(&S3);
  igraph_vector_ptr_free_all(&E3);
  igraph_vector_ptr_destroy(&E3);
  return 0;
}
