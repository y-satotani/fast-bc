
#include <assert.h>
#include <igraph/igraph.h>
#include <math.h>

#include "dybc/incremental_betweenness.h"
#include "dybc/dynamic_betweenness.h"
#include "dybc/static_betweenness.h"
#include "minigraph.h"

int main(int argc, char* argv[]) {
  unsigned long int seed = 0;
  if(argc >= 2) seed = atoll(argv[1]);
  igraph_rng_seed(igraph_rng_default(), seed);

  igraph_t G;
  igraph_integer_t eid;
  igraph_integer_t u, v;
  igraph_real_t weight;
  igraph_vector_t weights;
  igraph_erdos_renyi_game(&G, IGRAPH_ERDOS_RENYI_GNM, 100, 400, 0, 0);

  igraph_t C;
  igraph_complementer(&C, &G, 0);
  eid = igraph_rng_get_integer
    (igraph_rng_default(), 0, igraph_ecount(&C)-1);
  igraph_edge(&C, eid, &u, &v);
  igraph_destroy(&C);

  igraph_vector_init(&weights, igraph_ecount(&G));
  for(eid = 0; eid < igraph_ecount(&G); eid++) {
    weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
    igraph_vector_set(&weights, eid, weight);
  }
  weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
  //make_less_graph_and_edge(&G, &u, &v, &weight, &weights);

  igraph_matrix_t D;
  igraph_matrix_int_t S;
  igraph_vector_t B;
  igraph_matrix_init(&D, igraph_vcount(&G), igraph_vcount(&G));
  igraph_matrix_int_init(&S, igraph_vcount(&G), igraph_vcount(&G));
  igraph_vector_init(&B, igraph_vcount(&G));
  betweenness_with_redundant_information(&G, &D, &S, &B, &weights);

  igraph_inclist_t inclist;
  igraph_inclist_init(&G, &inclist, IGRAPH_ALL);

  igraph_vector_int_t sources, targets;
  igraph_vector_ptr_t targets_of_s, sources_of_t;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);
  affected_sources_inc(&G, &inclist, &sources, &D, u, v, v, &weights, weight);
  affected_targets_inc(&G, &inclist, &targets, &D, u, v, u, &weights, weight);
  igraph_vector_ptr_init(&targets_of_s, igraph_vector_int_size(&sources));
  igraph_vector_ptr_init(&sources_of_t, igraph_vector_int_size(&targets));
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    igraph_vector_int_t *ts
      = (igraph_vector_int_t*) malloc(sizeof(igraph_vector_int_t));
    igraph_vector_int_init(ts, 0);
    affected_targets_inc(&G, &inclist, ts, &D, u, v, s, &weights, weight);
    igraph_vector_ptr_set(&targets_of_s, si, ts);
  }
  for(igraph_integer_t ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    igraph_vector_int_t *ss
      = (igraph_vector_int_t*) malloc(sizeof(igraph_vector_int_t));
    igraph_vector_int_init(ss, 0);
    affected_sources_inc(&G, &inclist, ss, &D, u, v, t, &weights, weight);
    igraph_vector_ptr_set(&sources_of_t, ti, ss);
  }

  // decrease betweenness
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    igraph_vector_int_t *ts
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&targets_of_s, si);
    // factor is -2 for undirected and -1 for directed
    update_deps_inc_weighted(&G, &inclist, &D, &S, &B, u, v,
                             s, ts, &weights, weight, -2);
  }
  /*
  for(igraph_integer_t ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    igraph_vector_int_t *ss
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&sources_of_t, ti);
    update_deps_inc_weighted(&G, &inclist, &D, &S, &B, u, v,
                             t, ss, &weights, weight, -1);
  }
  */

  igraph_add_edge(&G, u, v);
  igraph_vector_push_back(&weights, weight);
  eid = igraph_ecount(&G) - 1;
  igraph_vector_int_push_back(igraph_inclist_get(&inclist, u), eid);
  igraph_vector_int_push_back(igraph_inclist_get(&inclist, v), eid);

  for(int ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    update_sssp_inc_weighted
      (&G, &inclist, &inclist, &D, &S, v, u, t, &weights, weight);
  }
  for(int si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    update_sssp_inc_weighted
      (&G, &inclist, &inclist, &D, &S, u, v, s, &weights, weight);
  }

  igraph_matrix_t D_true;
  igraph_matrix_int_t S_true;
  igraph_vector_t B_true;
  igraph_vector_t B_grand;
  igraph_matrix_init(&D_true, igraph_vcount(&G), igraph_vcount(&G));
  igraph_matrix_int_init(&S_true, igraph_vcount(&G), igraph_vcount(&G));
  igraph_vector_init(&B_true, igraph_vcount(&G));
  igraph_vector_init(&B_grand, igraph_vcount(&G));
  betweenness_with_redundant_information(&G, &D_true, &S_true, &B_true, &weights);
  igraph_betweenness(&G, &B_grand, igraph_vss_all(), 0, &weights, 0);
  igraph_vector_scale(&B_grand, 2);

  // increase betweenness
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    igraph_vector_int_t *ts
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&targets_of_s, si);
    // factor is 2 for undirected and 1 for directed
    update_deps_inc_weighted(&G, &inclist, &D, &S, &B, u, v,
                             s, ts, &weights, weight, 2);
  }
  /*
  for(igraph_integer_t ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    igraph_vector_int_t *ss
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&sources_of_t, ti);
    update_deps_inc_weighted(&G, &inclist, &D, &S, &B, u, v,
                             t, ss, &weights, weight, 1);
  }
  */

  for(int i = 0; i < igraph_vcount(&G); i++) {
    printf("%d %d %f %f %f\n", seed, i, VECTOR(B)[i], VECTOR(B_true)[i], VECTOR(B_grand)[i]);
  }

  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&S);
  igraph_vector_destroy(&B);
  igraph_matrix_destroy(&D_true);
  igraph_matrix_int_destroy(&S_true);
  igraph_vector_destroy(&B_true);

  for(igraph_integer_t ti = 0; ti < igraph_vector_ptr_size(&sources_of_t); ti++) {
    igraph_vector_int_t *ss = igraph_vector_ptr_e(&sources_of_t, ti);
    igraph_vector_int_destroy(ss);
    free(ss);
  }
  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);

  igraph_inclist_destroy(&inclist);
  igraph_vector_destroy(&weights);
  igraph_destroy(&G);
  return 0;
}

