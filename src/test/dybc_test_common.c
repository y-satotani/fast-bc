
#include "dybc_test_common.h"

#include <stdio.h>
#include <igraph/igraph.h>
#include <igraph/igraph_math.h>

#include "dybc/static_betweenness.h"
#include "dybc/dynamic_betweenness.h"
#include "dybc/incremental_shortest_path.h"
#include "dybc/decremental_shortest_path.h"

int check_quantities(const char* test_name,
                     igraph_t* G,
                     igraph_matrix_t* D,
                     igraph_matrix_int_t* S,
                     igraph_vector_t* B,
                     igraph_vector_t* weights) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define s(a, b) (MATRIX(*S, (a), (b)))
#define B(a) (VECTOR(*B)[(a)])
#define dt(a, b) (MATRIX(D_true, (a), (b)))
#define st(a, b) (MATRIX(S_true, (a), (b)))
#define Bt(a) (VECTOR(B_true)[(a)])
#define Bg(a) (VECTOR(B_grand)[(a)])

  igraph_matrix_t D_true;
  igraph_matrix_int_t S_true;
  igraph_vector_t B_true;
  igraph_vector_t B_grand;

  igraph_matrix_init(&D_true, igraph_vcount(G), igraph_vcount(G));
  igraph_matrix_int_init(&S_true, igraph_vcount(G), igraph_vcount(G));
  igraph_vector_init(&B_true, igraph_vcount(G));
  igraph_vector_init(&B_grand, igraph_vcount(G));
  betweenness_with_redundant_information(G, &D_true, &S_true, &B_true, weights);
  igraph_betweenness
    (G, &B_grand, igraph_vss_all(), igraph_is_directed(G), weights, 0);
  if(!igraph_is_directed(G))
    igraph_vector_scale(&B_grand, 2);

  int dist_err = 0, sigma_err = 0, bet_err = 0;
  // check distance
  for(long int s = 0; s < igraph_vcount(G); s++) {
    for(long int t = 0; t < igraph_vcount(G); t++) {
      if(cmp(d(s, t), dt(s, t))) {
        if(!dist_err) {
          printf("distance error on %s:\n", test_name);
          dist_err = 1;
        }
        printf("%ld %ld (res:%f true:%f)\n", s, t, d(s, t), dt(s, t));
      }
    }
  }
  // check geodesics
  for(long int s = 0; s < igraph_vcount(G); s++) {
    for(long int t = 0; t < igraph_vcount(G); t++) {
      if(s(s, t) != st(s, t)) {
        if(!sigma_err) {
          printf("geodesics error on %s:\n", test_name);
          sigma_err = 1;
        }
        printf("%ld %ld (res:%d true:%d)\n", s, t, s(s, t), st(s, t));
      }
    }
  }
  // check betweenness
  for(long int x = 0; x < igraph_vcount(G); x++) {
    igraph_real_t diff1 = fabs(B(x) - Bt(x));
    igraph_real_t diff2 = fabs(B(x) - Bg(x));
    if(cmp(diff1, EPS) > 0 || cmp(diff2, EPS) > 0) {
      if(!bet_err) {
        printf("betweenness error on %s:\n", test_name);
        bet_err = 1;
      }
      printf("%ld (res:%g true:%g grand:%g)\n", x, B(x), Bt(x), Bg(x));
    }
  }

  igraph_matrix_destroy(&D_true);
  igraph_matrix_int_destroy(&S_true);
  igraph_vector_destroy(&B_true);
  igraph_vector_destroy(&B_grand);
#undef EPS
#undef cmp
#undef d
#undef dt
#undef s
#undef st
#undef B
#undef Bt
#undef Bg
  return !(dist_err | sigma_err | bet_err);
}

void incremental_update_weighted(igraph_t* G,
                                 igraph_matrix_t* D,
                                 igraph_matrix_int_t *S,
                                 igraph_vector_t* B,
                                 igraph_integer_t u,
                                 igraph_integer_t v,
                                 igraph_vector_t* weights,
                                 igraph_real_t weight) {
  igraph_inclist_t succs, preds;
  if(igraph_is_directed(G)) {
    igraph_inclist_init(G, &succs, IGRAPH_OUT);
    igraph_inclist_init(G, &preds, IGRAPH_IN);
  } else {
    igraph_inclist_init(G, &succs, IGRAPH_ALL);
    preds = succs;
  }

  igraph_vector_int_t targets_;
  igraph_vector_int_t sources, targets;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);
  igraph_vector_int_init(&targets_, 0);
  if(!igraph_is_directed(G))
    affected_sources_inc(G, &preds, &targets_, D, v, u, u, weights, weight, 0);
  else
    affected_targets_inc(G, &succs, &targets_, D, u, v, u, weights, weight, 0);
  affected_sources_inc(G, &preds, &sources, D, u, v, v, weights, weight, 0);

  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    // decrease betweenness
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_inc(G, &succs, &targets, D, u, v, s, weights, weight, 0);

    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    update_deps_weighted(G, &preds, D, S, B, u, v,
                         s, &targets, weights, weight, -factor, 0);

    // add edge
    igraph_add_edge(G, u, v);
    igraph_vector_push_back(weights, weight);
    igraph_integer_t eid = igraph_ecount(G) - 1;
    igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
    igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);

    update_sssp_inc_weighted
      (G, &preds, &succs, D, S, u, v, s, weights, weight, 0);

    // increase betweenness
    // factor is 2 for undirected and 1 for directed
    update_deps_weighted(G, &preds, D, S, B, u, v,
                         s, &targets, weights, weight, factor, 0);

    // cleanup
    igraph_delete_edges(G, igraph_ess_1(eid));
    igraph_vector_remove(weights, eid);
    igraph_vector_int_pop_back(igraph_inclist_get(&succs, u));
    igraph_vector_int_pop_back(igraph_inclist_get(&preds, v));
  }

  // add edge
  igraph_integer_t eid = igraph_ecount(G);
  igraph_add_edge(G, u, v);
  igraph_vector_push_back(weights, weight);
  igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
  igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);

  for(int ti = 0; ti < igraph_vector_int_size(&targets_); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets_, ti);
    if(!igraph_is_directed(G))
      update_sssp_inc_weighted
        (G, &succs, &preds, D, S, v, u, t, weights, weight, 0);
    else
      update_stsp_inc_weighted
        (G, &preds, &succs, D, S, u, v, t, weights, IGRAPH_INFINITY, 0);
  }

  // cleanup
  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
  igraph_vector_int_destroy(&targets_);
  igraph_inclist_destroy(&succs);
  if(igraph_is_directed(G))
    igraph_inclist_destroy(&preds);
}

void decremental_update_weighted(igraph_t* G,
                                 igraph_matrix_t* D,
                                 igraph_matrix_int_t *S,
                                 igraph_vector_t* B,
                                 igraph_integer_t u,
                                 igraph_integer_t v,
                                 igraph_vector_t* weights,
                                 igraph_real_t weight) {
  // move the edge to be deleted to back of list
  igraph_integer_t eid;
  igraph_get_eid(G, &eid, u, v, 0, 1);
  igraph_delete_edges(G, igraph_ess_1(eid));
  weight = igraph_vector_e(weights, eid);
  igraph_vector_remove(weights, eid);

  igraph_inclist_t succs, preds;
  if(igraph_is_directed(G)) {
    igraph_inclist_init(G, &succs, IGRAPH_OUT);
    igraph_inclist_init(G, &preds, IGRAPH_IN);
  } else {
    igraph_inclist_init(G, &succs, IGRAPH_ALL);
    preds = succs;
  }

  // then push the deleted edge
  eid = igraph_ecount(G);
  igraph_add_edge(G, u, v);
  igraph_vector_push_back(weights, weight);
  igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
  igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);

  // finding affected sources
  igraph_vector_int_t targets_;
  igraph_vector_int_t sources, targets;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);
  igraph_vector_int_init(&targets_, 0);
  if(!igraph_is_directed(G))
    affected_sources_dec(G, &preds, &targets_, D, v, u, u, weights, weight, 0);
  else
    affected_targets_dec(G, &succs, &targets_, D, u, v, u, weights, weight, 0);
  affected_sources_dec(G, &preds, &sources, D, u, v, v, weights, weight, 0);

  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_dec(G, &succs, &targets, D, u, v, s, weights, weight, 0);
    // decrease betweenness
    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    update_deps_weighted(G, &preds, D, S, B, u, v,
                         s, &targets, weights, weight, -factor, 0);

    // modify
    igraph_delete_edges(G, igraph_ess_1(eid));
    igraph_vector_pop_back(weights);
    igraph_vector_int_pop_back(igraph_inclist_get(&succs, u));
    igraph_vector_int_pop_back(igraph_inclist_get(&preds, v));

    // update sssp
    update_sssp_dec_weighted
      (G, &preds, &succs, D, S, u, v, s, weights, weight, 0);

    // increase betweenness
    // factor is 2 for undirected and 1 for directed
    update_deps_weighted(G, &preds, D, S, B, u, v,
                         s, &targets, weights, weight, factor, 0);

    // cleanup for next round
    igraph_add_edge(G, u, v);
    igraph_vector_push_back(weights, weight);
    igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
    igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);
  }

  // delete an edge
  igraph_delete_edges(G, igraph_ess_1(eid));
  igraph_vector_pop_back(weights);
  igraph_vector_int_pop_back(igraph_inclist_get(&succs, u));
  igraph_vector_int_pop_back(igraph_inclist_get(&preds, v));
  // update stsp
  for(int ti = 0; ti < igraph_vector_int_size(&targets_); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets_, ti);
    if(!igraph_is_directed(G))
      update_sssp_dec_weighted
        (G, &succs, &preds, D, S, v, u, t, weights, weight, 0);
    else
      update_stsp_dec_weighted
        (G, &preds, &succs, D, S, u, v, t, weights, IGRAPH_INFINITY, 0);
  }
  // cleanup
  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
  igraph_vector_int_destroy(&targets_);
  igraph_inclist_destroy(&succs);
  if(igraph_is_directed(G))
    igraph_inclist_destroy(&preds);
}

void incremental_update_unweighted(igraph_t* G,
                                   igraph_matrix_t* D,
                                   igraph_matrix_int_t *S,
                                   igraph_vector_t* B,
                                   igraph_integer_t u,
                                   igraph_integer_t v) {
  igraph_inclist_t succs, preds;
  if(igraph_is_directed(G)) {
    igraph_inclist_init(G, &succs, IGRAPH_OUT);
    igraph_inclist_init(G, &preds, IGRAPH_IN);
  } else {
    igraph_inclist_init(G, &succs, IGRAPH_ALL);
    preds = succs;
  }

  igraph_vector_int_t targets_;
  igraph_vector_int_t sources, targets;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);
  igraph_vector_int_init(&targets_, 0);
  if(!igraph_is_directed(G))
    affected_sources_inc(G, &preds, &targets_, D, v, u, u, NULL, 0, 0);
  else
    affected_targets_inc(G, &succs, &targets_, D, u, v, u, NULL, 0, 0);
  affected_sources_inc(G, &preds, &sources, D, u, v, v, NULL, 0, 0);

  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    // decrease betweenness
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_inc(G, &succs, &targets, D, u, v, s, NULL, 0, 0);

    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    update_deps_unweighted(G, &preds, D, S, B, u, v, s, &targets, -factor, 0);

    // add edge
    igraph_add_edge(G, u, v);
    igraph_integer_t eid = igraph_ecount(G) - 1;
    igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
    igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);

    update_sssp_inc_unweighted(G, &preds, &succs, D, S, u, v, s, 0);

    // increase betweenness
    // factor is 2 for undirected and 1 for directed
    update_deps_unweighted(G, &preds, D, S, B, u, v, s, &targets, factor, 0);

    // cleanup
    igraph_delete_edges(G, igraph_ess_1(eid));
    igraph_vector_int_pop_back(igraph_inclist_get(&succs, u));
    igraph_vector_int_pop_back(igraph_inclist_get(&preds, v));
  }

  // add edge
  igraph_integer_t eid = igraph_ecount(G);
  igraph_add_edge(G, u, v);
  igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
  igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);

  for(int ti = 0; ti < igraph_vector_int_size(&targets_); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets_, ti);
    if(!igraph_is_directed(G))
      update_sssp_inc_unweighted(G, &succs, &preds, D, S, v, u, t, 0);
    else
      update_stsp_inc_unweighted(G, &preds, &succs, D, S, u, v, t, 0);
  }

  // cleanup
  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
  igraph_vector_int_destroy(&targets_);
  igraph_inclist_destroy(&succs);
  if(igraph_is_directed(G))
    igraph_inclist_destroy(&preds);
}

void decremental_update_unweighted(igraph_t* G,
                                   igraph_matrix_t* D,
                                   igraph_matrix_int_t *S,
                                   igraph_vector_t* B,
                                   igraph_integer_t u,
                                   igraph_integer_t v) {
  // move the edge to be deleted to back of list
  igraph_integer_t eid;
  igraph_get_eid(G, &eid, u, v, 0, 1);
  igraph_delete_edges(G, igraph_ess_1(eid));

  igraph_inclist_t succs, preds;
  if(igraph_is_directed(G)) {
    igraph_inclist_init(G, &succs, IGRAPH_OUT);
    igraph_inclist_init(G, &preds, IGRAPH_IN);
  } else {
    igraph_inclist_init(G, &succs, IGRAPH_ALL);
    preds = succs;
  }

  // then push the deleted edge
  eid = igraph_ecount(G);
  igraph_add_edge(G, u, v);
  igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
  igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);

  // finding affected sources
  igraph_vector_int_t targets_;
  igraph_vector_int_t sources, targets;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);
  igraph_vector_int_init(&targets_, 0);
  if(!igraph_is_directed(G))
    affected_sources_dec(G, &preds, &targets_, D, v, u, u, NULL, 0, 0);
  else
    affected_targets_dec(G, &succs, &targets_, D, u, v, u, NULL, 0, 0);
  affected_sources_dec(G, &preds, &sources, D, u, v, v, NULL, 0, 0);

  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_dec(G, &succs, &targets, D, u, v, s, NULL, 0, 0);
    // decrease betweenness
    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    update_deps_unweighted(G, &preds, D, S, B, u, v, s, &targets, -factor, 0);

    // modify
    igraph_delete_edges(G, igraph_ess_1(eid));
    igraph_vector_int_pop_back(igraph_inclist_get(&succs, u));
    igraph_vector_int_pop_back(igraph_inclist_get(&preds, v));

    // update sssp
    update_sssp_dec_unweighted(G, &preds, &succs, D, S, u, v, s, 0);

    // increase betweenness
    // factor is 2 for undirected and 1 for directed
    update_deps_unweighted(G, &preds, D, S, B, u, v, s, &targets, factor, 0);

    // cleanup for next round
    igraph_add_edge(G, u, v);
    igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
    igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);
  }

  // delete an edge
  igraph_delete_edges(G, igraph_ess_1(eid));
  igraph_vector_int_pop_back(igraph_inclist_get(&succs, u));
  igraph_vector_int_pop_back(igraph_inclist_get(&preds, v));
  // update stsp
  for(int ti = 0; ti < igraph_vector_int_size(&targets_); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets_, ti);
    if(!igraph_is_directed(G))
      update_sssp_dec_unweighted(G, &succs, &preds, D, S, v, u, t, 0);
    else
      update_stsp_dec_unweighted(G, &preds, &succs, D, S, u, v, t, 0);
  }
  // cleanup
  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
  igraph_vector_int_destroy(&targets_);
  igraph_inclist_destroy(&succs);
  if(igraph_is_directed(G))
    igraph_inclist_destroy(&preds);
}
