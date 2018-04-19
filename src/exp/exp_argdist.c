
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <igraph.h>
#include "bcdec/arg_dist.h"
#include "bcdec/update_on_insert.h"
#include "bcdec/update_on_delete.h"

void make_graph(igraph_t* G, char* name, int n, int k);
void choice_noadjacent_pair(igraph_t* G, int* u, int* v);

int main(int argc, char* argv[]) {
  // Initialization
  if(argc < 5) return 1;
  igraph_t G;
  char* name = argv[1];
  int n = atoi(argv[2]), k = atoi(argv[3]);
  char* mode = argv[4];
  make_graph(&G, name, n, k);

  // Make argumented distance
  igraph_matrix_t D, Sigma;
  arg_dist(&D, &Sigma, &G);

  // Choose edge to insert/delete then update
  igraph_integer_t u, v;
  if(strcmp(mode, "insert")) {
    choice_noadjacent_pair(&G, &u, &v);
    igraph_add_edge(&G, u, v);
  } else if(strcmp(mode, "delete")) {
    igraph_integer_t eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&G)-1);
    igraph_edge(&G, eid, &u, &v);
    igraph_delete_edges(&G, igraph_ess_1(eid));
  } else assert(0);

  // Update
  clock_t start, end;
  double time_update;
  if(strcmp(mode, "insert")) {
    start = clock();
    update_on_insert(&D, &Sigma, u, v);
    end = clock();
  } else if(strcmp(mode, "delete")) {
    start = clock();
    update_on_delete(&D, &Sigma, u, v);
    end = clock();
  } else assert(0);
  time_update = (double)(end - start) / CLOCKS_PER_SEC;

  // Calculate training data
  igraph_matrix_t Dtrain;
  double time_igraph;
  igraph_matrix_init(&Dtrain, igraph_vcount(&G), igraph_vcount(&G));
  start = clock();
  igraph_shortest_paths(&G, &Dtrain,
                        igraph_vss_all(), igraph_vss_all(), IGRAPH_ALL);
  end = clock();
  time_igraph = (double)(end - start) / CLOCKS_PER_SEC;

  printf("%s,%d,%d,%s,%d,%f,%f\n",
         name, n, k, mode,
         igraph_matrix_all_e(&D, &Dtrain),
         time_update, time_igraph);

  igraph_matrix_destroy(&D);
  igraph_matrix_destroy(&Sigma);
  igraph_matrix_destroy(&Dtrain);
  igraph_destroy(&G);

  return 0;
}

void make_graph(igraph_t* G, char* name, int n, int k) {
  assert(k % 2 == 0);
  if(strcmp(name, "RRG") == 0)
    igraph_k_regular_game(G, n, k, 0, 0);
  else if(strcmp(name, "ER") == 0)
    igraph_erdos_renyi_game(G, IGRAPH_ERDOS_RENYI_GNM, n, n*k/2, 0, 0);
  else if(strcmp(name, "BA") == 0)
    igraph_barabasi_game(G, n, 1, k/2, 0, 1, 1, 0, IGRAPH_BARABASI_BAG, 0);
  else
    assert(0);
}

void choice_noadjacent_pair(igraph_t* G,
                            igraph_integer_t* u, igraph_integer_t* v) {
  igraph_vector_int_t ulist, vlist;
  igraph_integer_t w, x, eid;
  igraph_vector_int_init(&ulist, 0);
  igraph_vector_int_init(&vlist, 0);
  for(w = 0; w < igraph_vcount(G); w++)
    for(x = w+1; x < igraph_vcount(G); x++) {
      igraph_get_eid(G, &eid, w, x, 0, 0);
      if(eid < 0) {
        igraph_vector_int_push_back(&ulist, w);
        igraph_vector_int_push_back(&vlist, x);
      }
    }
  eid = igraph_rng_get_integer
    (igraph_rng_default(), 0, igraph_vector_int_size(&ulist)-1);
  *u = igraph_vector_int_e(&ulist, eid);
  *v = igraph_vector_int_e(&vlist, eid);
  igraph_vector_int_destroy(&ulist);
  igraph_vector_int_destroy(&vlist);
}
