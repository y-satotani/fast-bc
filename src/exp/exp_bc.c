
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <igraph.h>
#include "bcdec/arg_dist.h"
#include "bcdec/update_on_insert.h"
#include "bcdec/update_on_delete.h"
#include "bcdec/partial_bc.h"

void make_graph(igraph_t* G, char* name, int n, int k);
void choice_noadjacent_pair(igraph_t* G, int* u, int* v);
double l2_norm(igraph_vector_t* v1, igraph_vector_t* v2);

int main(int argc, char* argv[]) {
  // Initialization
  if(argc < 5) return 1;
  igraph_t G;
  char* name = argv[1];
  int n = atoi(argv[2]), k = atoi(argv[3]);
  char* mode = argv[4];
  make_graph(&G, name, n, k);

  // Make argumented distance and pair dependency
  igraph_matrix_t D, Sigma, Delta;
  igraph_vector_t delta, delta_update;
  arg_dist(&D, &Sigma, &G);
  igraph_vector_init(&delta, igraph_vcount(&G));
  igraph_vector_init(&delta_update, 0);
  igraph_matrix_init(&Delta, igraph_vcount(&G), igraph_vcount(&G));
  partial_bc_bfs(&G, &Delta, igraph_vss_all());

  // Choose edge to insert/delete then update
  igraph_integer_t u, v;
  if(strcmp(mode, "insert") == 0) {
    choice_noadjacent_pair(&G, &u, &v);
    igraph_add_edge(&G, u, v);
  } else if(strcmp(mode, "delete") == 0) {
    igraph_integer_t eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&G)-1);
    igraph_edge(&G, eid, &u, &v);
    igraph_delete_edges(&G, igraph_ess_1(eid));
  } else assert(0);

  // Calculate bc and update
  igraph_vector_t B;
  clock_t start, end;
  double time_update;
  igraph_vector_init(&B, igraph_vcount(&G));
  start = clock();
  for(igraph_integer_t source = 0; source < igraph_vcount(&G); source++)
    if(MATRIX(D, source, u) != MATRIX(D, source, v))
      igraph_vector_push_back(&delta_update, source);
  partial_bc_bfs(&G, &Delta, igraph_vss_vector(&delta_update));
  igraph_matrix_colsum(&Delta, &B);
  igraph_vector_scale(&B, 0.5);
  if(strcmp(mode, "insert") == 0) {
    update_on_insert(&D, &Sigma, u, v);
  } else if(strcmp(mode, "delete") == 0) {
    update_on_delete(&D, &Sigma, u, v);
  } else assert(0);
  end = clock();
  time_update = (double)(end - start) / CLOCKS_PER_SEC;

  // Calculate training data
  igraph_vector_t Btrain;
  double time_igraph;
  igraph_vector_init(&Btrain, igraph_vcount(&G));
  start = clock();
  igraph_betweenness(&G, &Btrain, igraph_vss_all(), 0, 0, 1);
  end = clock();
  time_igraph = (double)(end - start) / CLOCKS_PER_SEC;

  printf("%s,%d,%d,%s,%e,%f,%f,%f\n",
         name, n, k, mode, l2_norm(&B, &Btrain),
         time_update, time_igraph,
         (double)igraph_vector_size(&delta_update) / igraph_vcount(&G));

  igraph_vector_destroy(&Btrain);
  igraph_vector_destroy(&B);
  igraph_vector_destroy(&delta);
  igraph_vector_destroy(&delta_update);
  igraph_matrix_destroy(&D);
  igraph_matrix_destroy(&Sigma);
  igraph_matrix_destroy(&Delta);
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
  else {
    FILE* fp = fopen(name, "r");
    igraph_read_graph_edgelist(G, fp, 0, 0);
    fclose(fp);
  }
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

double l2_norm(igraph_vector_t* v1, igraph_vector_t* v2) {
  int i;
  double sub, sum = 0;
  for(i = 0; i < igraph_vector_size(v1); i++) {
    sub = igraph_vector_e(v1, i) - igraph_vector_e(v2, i);
    sum += sub * sub;
  }
  return sqrt(sum);
}
