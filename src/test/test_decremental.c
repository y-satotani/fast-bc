
#include <assert.h>
#include <igraph/igraph.h>
#include <math.h>

#include "dybc/decremental_betweenness.h"
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
  igraph_erdos_renyi_game(&G, IGRAPH_ERDOS_RENYI_GNM, 10, 25, 0, 0);

  igraph_vector_init(&weights, igraph_ecount(&G));
  for(eid = 0; eid < igraph_ecount(&G); eid++) {
    weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
    igraph_vector_set(&weights, eid, weight);
  }
  eid = igraph_rng_get_integer
    (igraph_rng_default(), 0, igraph_ecount(&G)-1);
  igraph_edge(&G, eid, &u, &v);
  weight = igraph_vector_e(&weights, eid);
  //make_more_graph_and_edge(&G, &u, &v, &weight, &weights);

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
  affected_sources_dec(&G, &inclist, &sources, &D, u, v, v, &weights, weight);
  affected_targets_dec(&G, &inclist, &targets, &D, u, v, u, &weights, weight);
  igraph_vector_ptr_init(&targets_of_s, igraph_vector_int_size(&sources));
  igraph_vector_ptr_init(&sources_of_t, igraph_vector_int_size(&targets));
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    igraph_vector_int_t *ts
      = (igraph_vector_int_t*) malloc(sizeof(igraph_vector_int_t));
    igraph_vector_int_init(ts, 0);
    affected_targets_dec(&G, &inclist, ts, &D, u, v, s, &weights, weight);
    igraph_vector_ptr_set(&targets_of_s, si, ts);
  }
  for(igraph_integer_t ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    igraph_vector_int_t *ss
      = (igraph_vector_int_t*) malloc(sizeof(igraph_vector_int_t));
    igraph_vector_int_init(ss, 0);
    affected_sources_dec(&G, &inclist, ss, &D, u, v, t, &weights, weight);
    igraph_vector_ptr_set(&sources_of_t, ti, ss);
  }

  // decrease betweenness
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    igraph_vector_int_t *ts
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&targets_of_s, si);
    // factor is -2 for undirected and -1 for directed
    update_deps_dec_weighted(&G, &inclist, &D, &S, &B, u, v,
                             s, ts, &weights, weight, -2);
  }
  /*
  for(igraph_integer_t ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    igraph_vector_int_t *ss
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&sources_of_t, ti);
    update_deps_dec_weighted(&G, &inclist, &D, &S, &B, u, v,
                             t, ss, &weights, weight, -1);
  }
  */
  igraph_get_eid(&G, &eid, u, v, 0, 1);
  igraph_delete_edges(&G, igraph_ess_1(eid));
  igraph_vector_remove(&weights, eid);
  // apply modification to inclist
  /*
  long int del_idx;
  igraph_vector_int_t* neis;
  neis = igraph_inclist_get(&inclist, u);
  for(del_idx = 0; del_idx < igraph_vector_int_size(neis); del_idx++)
    if(igraph_vector_int_e(neis, del_idx) == eid) break;
  igraph_vector_int_remove(neis, del_idx);
  neis = igraph_inclist_get(&inclist, v);
  for(del_idx = 0; del_idx < igraph_vector_int_size(neis); del_idx++)
    if(igraph_vector_int_e(neis, del_idx) == eid) break;
  igraph_vector_int_remove(neis, del_idx);
  */
  for(long int vid = 0; vid < igraph_vcount(&G); vid++) {
    igraph_vector_int_t* neis = igraph_inclist_get(&inclist, vid);
    for(long int ni = igraph_vector_int_size(neis)-1; ni >= 0; ni--) {
      if(igraph_vector_int_e(neis, ni) == eid) {
        igraph_vector_int_remove(neis, ni);
      } else if(igraph_vector_int_e(neis, ni) > eid) {
        VECTOR(*neis)[ni]--;
      }
    }
  }

  for(int ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    update_sssp_dec_weighted
      (&G, &inclist, &inclist, &D, &S, v, u, t, &weights, weight);
  }
  for(int si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    update_sssp_dec_weighted
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
    update_deps_dec_weighted(&G, &inclist, &D, &S, &B, u, v,
                             s, ts, &weights, weight, 2);
  }
  /*
  for(igraph_integer_t ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    igraph_vector_int_t *ss
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&sources_of_t, ti);
    update_deps_dec_weighted(&G, &inclist, &D, &S, &B, u, v,
                             t, ss, &weights, weight, 1);
  }
  */
  for(int i = 0; i < igraph_vcount(&G); i++) {
    printf("%d %d %f %f %f\n", seed, i, VECTOR(B)[i], VECTOR(B_true)[i], VECTOR(B_grand)[i]);
  }
/*
  for(int i = 0; i < igraph_vcount(&G); i++) {
    for(int j = 0; j < igraph_vcount(&G); j++) {
      printf("%d %d %f %f %d %d\n", i, j, MATRIX(D, i, j), MATRIX(D_true, i, j), MATRIX(S, i, j), MATRIX(S_true, i, j));
    }
  }
printf("%d %d\n", igraph_matrix_all_e(&D, &D_true), igraph_matrix_int_all_e(&S, &S_true));
*/
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

