
#include "dybc_exp_common.h"

#include <stdio.h>
#include <igraph/igraph.h>
#include <igraph/igraph_math.h>

#include "dybc/static_betweenness.h"
#include "dybc/dynamic_betweenness.h"
#include "dybc/incremental_betweenness.h"
#include "dybc/decremental_betweenness.h"

int dybc_read_edgelist(igraph_t* G,
                       igraph_vector_t* weights,
                       igraph_bool_t is_directed,
                       FILE* istream) {
  igraph_integer_t u, v;
  igraph_real_t w;
  int n_fields;

  igraph_empty(G, 0, is_directed);
  igraph_vector_init(weights, 0);
  while((n_fields = fscanf(istream, "%d %d %lf\n", &u, &v, &w)) != EOF) {
    igraph_integer_t max_v = u > v ? u : v;
    if(igraph_vcount(G) <= max_v)
      igraph_add_vertices(G, max_v-igraph_vcount(G)+1, 0);
    igraph_add_edge(G, u, v);
    if(n_fields == 3)
      igraph_vector_push_back(weights, w);
  }
  if(igraph_ecount(G) != igraph_vector_size(weights))
    igraph_vector_clear(weights);
  return 1;
}

int dybc_dump_cache(igraph_matrix_t* D,
                    igraph_matrix_int_t* S,
                    igraph_vector_t* B,
                    FILE* ostream) {
  igraph_integer_t N = igraph_vector_size(B);
  igraph_integer_t *N_ =
    (igraph_integer_t*) malloc(sizeof(igraph_integer_t));
  N_[0] = N;
  igraph_real_t* D_ =
    (igraph_real_t*) malloc(sizeof(igraph_real_t)*N*N);
  igraph_matrix_copy_to(D, D_);
  igraph_integer_t* S_ =
    (igraph_integer_t*) malloc(sizeof(igraph_integer_t)*N*N);
  igraph_matrix_int_copy_to(S, S_);
  igraph_real_t* B_ =
    (igraph_real_t*) malloc(sizeof(igraph_real_t)*N);
  igraph_vector_copy_to(B, B_);

  fwrite(N_, sizeof(igraph_integer_t), 1, ostream);
  fwrite(D_, sizeof(igraph_real_t), N*N, ostream);
  fwrite(S_, sizeof(igraph_integer_t), N*N, ostream);
  fwrite(B_, sizeof(igraph_real_t), N, ostream);

  free(N_);
  free(D_);
  free(S_);
  free(B_);
  return 1;
}

int dybc_load_cache(igraph_matrix_t* D,
                    igraph_matrix_int_t* S,
                    igraph_vector_t* B,
                    FILE* istream) {
  igraph_integer_t* N_ =
    (igraph_integer_t*) malloc(sizeof(igraph_integer_t));
  fread(N_, sizeof(igraph_integer_t), 1, istream);
  igraph_integer_t N = N_[0];

  igraph_matrix_resize(D, N, N);
  igraph_matrix_int_resize(S, N, N);
  igraph_vector_resize(B, N);

  igraph_real_t* D_ =
    (igraph_real_t*) malloc(sizeof(igraph_real_t)*N*N);
  fread(D_, sizeof(igraph_real_t), N*N, istream);
  for(long i = 0; i < N*N; i++) VECTOR(D->data)[i] = D_[i];
  igraph_integer_t* S_ =
    (igraph_integer_t*) malloc(sizeof(igraph_integer_t)*N*N);
  fread(S_, sizeof(igraph_integer_t), N*N, istream);
  for(long i = 0; i < N*N; i++) VECTOR(S->data)[i] = S_[i];
  igraph_real_t* B_
    = (igraph_real_t*) malloc(sizeof(igraph_real_t)*N);
  fread(B_, sizeof(igraph_real_t), N, istream);
  for(long i = 0; i < N; i++) VECTOR(*B)[i] = B_[i];

  free(N_);
  free(D_);
  free(S_);
  free(B_);
  return 1;
}

int choose_random_edge_for(igraph_t* G,
                           dybc_query_t query,
                           igraph_integer_t *u,
                           igraph_integer_t *v,
                           unsigned long int seed) {
  if(query == QUERY_INSERT) {
    igraph_integer_t eid;
    igraph_t C;
    igraph_complementer(&C, G, 0);
    eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&C)-1);
    igraph_edge(&C, eid, u, v);
    igraph_destroy(&C);
    return 1;
  } else if(query == QUERY_DELETE) {
    igraph_integer_t eid;
    eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(G)-1);
    igraph_edge(G, eid, u, v);
    return 1;
  } else {
    *u = *v = -1;
    return 0;
  }
}

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

void incremental_update(igraph_t* G,
                        igraph_matrix_t* D,
                        igraph_matrix_int_t *S,
                        igraph_vector_t* B,
                        igraph_integer_t u,
                        igraph_integer_t v,
                        igraph_vector_t* weights,
                        igraph_real_t weight,
                        igraph_integer_t* upd_stats) {
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
    affected_sources_inc(G, &preds, &targets_, D, v, u, u, weights, weight);
  else
    affected_targets_inc(G, &succs, &targets_, D, u, v, u, weights, weight);
  affected_sources_inc(G, &preds, &sources, D, u, v, v, weights, weight);

  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    // decrease betweenness
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_inc(G, &succs, &targets, D, u, v, s, weights, weight);

    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    if(weights)
      update_deps_weighted(G, &preds, D, S, B, u, v,
                           s, &targets, weights, weight, -factor);
    else
      update_deps_unweighted(G, &preds, D, S, B, u, v,
                             s, &targets, -factor);

    // add edge
    igraph_add_edge(G, u, v);
    if(weights)
      igraph_vector_push_back(weights, weight);
    igraph_integer_t eid = igraph_ecount(G) - 1;
    igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
    igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);

    if(weights)
      update_sssp_inc_weighted
        (G, &preds, &succs, D, S, u, v, s, weights, weight);
    else
      update_sssp_inc_unweighted
        (G, &preds, &succs, D, S, u, v, s);

    // increase betweenness
    // factor is 2 for undirected and 1 for directed
    if(weights)
      update_deps_weighted(G, &preds, D, S, B, u, v,
                           s, &targets, weights, weight, factor);
    else
      update_deps_unweighted(G, &preds, D, S, B, u, v,
                             s, &targets, factor);

    // cleanup
    igraph_delete_edges(G, igraph_ess_1(eid));
    if(weights)
      igraph_vector_remove(weights, eid);
    igraph_vector_int_pop_back(igraph_inclist_get(&succs, u));
    igraph_vector_int_pop_back(igraph_inclist_get(&preds, v));
  }

  // add edge
  igraph_integer_t eid = igraph_ecount(G);
  igraph_add_edge(G, u, v);
  if(weights)
    igraph_vector_push_back(weights, weight);
  igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
  igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);

  for(int ti = 0; ti < igraph_vector_int_size(&targets_); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets_, ti);
    if(!igraph_is_directed(G) && weights)
      update_sssp_inc_weighted
        (G, &succs, &preds, D, S, v, u, t, weights, weight);
    else if(igraph_is_directed(G) && weights)
      update_stsp_inc_weighted
        (G, &preds, &succs, D, S, u, v, t, weights, IGRAPH_INFINITY);
    else if(!igraph_is_directed(G) && !weights)
      update_sssp_inc_unweighted
        (G, &succs, &preds, D, S, v, u, t);
    else if(igraph_is_directed(G) && !weights)
      update_stsp_inc_unweighted
        (G, &preds, &succs, D, S, u, v, t);
  }

  // cleanup
  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
  igraph_vector_int_destroy(&targets_);
  igraph_inclist_destroy(&succs);
  if(igraph_is_directed(G))
    igraph_inclist_destroy(&preds);
}

void decremental_update(igraph_t* G,
                        igraph_matrix_t* D,
                        igraph_matrix_int_t *S,
                        igraph_vector_t* B,
                        igraph_integer_t u,
                        igraph_integer_t v,
                        igraph_vector_t* weights,
                        igraph_real_t weight,
                        igraph_integer_t* upd_stats) {
  // move the edge to be deleted to back of list
  igraph_integer_t eid;
  igraph_get_eid(G, &eid, u, v, 0, 1);
  igraph_delete_edges(G, igraph_ess_1(eid));
  if(weights) {
    weight = igraph_vector_e(weights, eid);
    igraph_vector_remove(weights, eid);
  }
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
  if(weights)
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
    affected_sources_dec(G, &preds, &targets_, D, v, u, u, weights, weight);
  else
    affected_targets_dec(G, &succs, &targets_, D, u, v, u, weights, weight);
  affected_sources_dec(G, &preds, &sources, D, u, v, v, weights, weight);

  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_dec(G, &succs, &targets, D, u, v, s, weights, weight);
    // decrease betweenness
    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    if(weights)
      update_deps_weighted(G, &preds, D, S, B, u, v,
                           s, &targets, weights, weight, -factor);
    else
      update_deps_unweighted(G, &preds, D, S, B, u, v,
                             s, &targets, -factor);

    // modify
    igraph_delete_edges(G, igraph_ess_1(eid));
    if(weights)
      igraph_vector_pop_back(weights);
    igraph_vector_int_pop_back(igraph_inclist_get(&succs, u));
    igraph_vector_int_pop_back(igraph_inclist_get(&preds, v));

    // update sssp
    if(weights)
      update_sssp_dec_weighted
        (G, &preds, &succs, D, S, u, v, s, weights, weight);
    else
      update_sssp_dec_unweighted
        (G, &preds, &succs, D, S, u, v, s);

    // increase betweenness
    // factor is 2 for undirected and 1 for directed
    if(weights)
      update_deps_weighted(G, &preds, D, S, B, u, v,
                           s, &targets, weights, weight, factor);
    else
      update_deps_unweighted(G, &preds, D, S, B, u, v,
                             s, &targets, factor);

    // cleanup for next round
    igraph_add_edge(G, u, v);
    if(weights)
      igraph_vector_push_back(weights, weight);
    igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
    igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);
  }

  // delete an edge
  igraph_delete_edges(G, igraph_ess_1(eid));
  if(weights)
    igraph_vector_pop_back(weights);
  igraph_vector_int_pop_back(igraph_inclist_get(&succs, u));
  igraph_vector_int_pop_back(igraph_inclist_get(&preds, v));
  // update stsp
  for(int ti = 0; ti < igraph_vector_int_size(&targets_); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets_, ti);
    if(!igraph_is_directed(G) && weights)
      update_sssp_dec_weighted
        (G, &succs, &preds, D, S, v, u, t, weights, weight);
    else if(igraph_is_directed(G) && weights)
      update_stsp_dec_weighted
        (G, &preds, &succs, D, S, u, v, t, weights, IGRAPH_INFINITY);
    else if(!igraph_is_directed(G) && !weights)
      update_sssp_dec_unweighted
        (G, &succs, &preds, D, S, v, u, t);
    else if(igraph_is_directed(G) && !weights)
      update_stsp_dec_unweighted
        (G, &preds, &succs, D, S, u, v, t);
  }
  // cleanup
  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
  igraph_vector_int_destroy(&targets_);
  igraph_inclist_destroy(&succs);
  if(igraph_is_directed(G))
    igraph_inclist_destroy(&preds);
}
