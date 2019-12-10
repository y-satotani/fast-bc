
#include <stdio.h>
#include <assert.h>
#include <igraph/igraph.h>
#include <igraph/igraph_math.h>
#include <math.h>

#include "dybc/incremental_betweenness.h"
#include "dybc/decremental_betweenness.h"
#include "dybc/static_betweenness.h"
#include "minigraph.h"

int test_inc_mini(void);
int test_dec_mini(void);
int test_inc_random(unsigned long int seed);
int test_dec_random(unsigned long int seed);
int test_inc_components(void);
int test_dec_bridge(void);
int test_inc_repeat(void);
int test_dec_repeat(void);
int test_repeat(void);

int main(int argc, char* argv[]) {
  test_inc_mini();
  test_dec_mini();
  test_inc_random(42);
  test_dec_random(42);
  test_inc_components();
  test_dec_bridge();
  test_inc_repeat();
  test_dec_repeat();
  test_repeat();
  return 0;
}

int _check_quantities(const char* test_name,
                      igraph_t* G,
                      igraph_matrix_t* D,
                      igraph_matrix_int_t* S,
                      igraph_vector_t* B,
                      igraph_vector_t* weights);

void _incremental_update_weighted(igraph_t* G,
                                  igraph_matrix_t* D,
                                  igraph_matrix_int_t *S,
                                  igraph_vector_t* B,
                                  igraph_integer_t u,
                                  igraph_integer_t v,
                                  igraph_vector_t* weights,
                                  igraph_real_t weight);

void _decremental_update_weighted(igraph_t* G,
                                  igraph_matrix_t* D,
                                  igraph_matrix_int_t *S,
                                  igraph_vector_t* B,
                                  igraph_integer_t u,
                                  igraph_integer_t v,
                                  igraph_vector_t* weights,
                                  igraph_real_t weight);

int test_inc_mini() {
  igraph_t G;
  igraph_integer_t u, v;
  igraph_real_t weight;
  igraph_vector_t weights;
  igraph_matrix_t D;
  igraph_matrix_int_t S;
  igraph_vector_t B;

  make_less_graph_and_edge(&G, &u, &v, &weight, &weights);

  // find distance and geodesics and betweenness
  igraph_matrix_init(&D, igraph_vcount(&G), igraph_vcount(&G));
  igraph_matrix_int_init(&S, igraph_vcount(&G), igraph_vcount(&G));
  igraph_vector_init(&B, igraph_vcount(&G));
  betweenness_with_redundant_information(&G, &D, &S, &B, &weights);

  _incremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);

  int res = _check_quantities("test_inc_mini", &G, &D, &S, &B, &weights);

  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&S);
  igraph_vector_destroy(&B);
  igraph_vector_destroy(&weights);
  igraph_destroy(&G);
  return res;
}

int test_dec_mini() {
  igraph_t G;
  igraph_integer_t u, v;
  igraph_real_t weight;
  igraph_vector_t weights;
  igraph_matrix_t D;
  igraph_matrix_int_t S;
  igraph_vector_t B;

  make_more_graph_and_edge(&G, &u, &v, &weight, &weights);

  // find distance and geodesics and betweenness
  igraph_matrix_init(&D, igraph_vcount(&G), igraph_vcount(&G));
  igraph_matrix_int_init(&S, igraph_vcount(&G), igraph_vcount(&G));
  igraph_vector_init(&B, igraph_vcount(&G));
  betweenness_with_redundant_information(&G, &D, &S, &B, &weights);

  _decremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);

  int res = _check_quantities("test_dec_mini", &G, &D, &S, &B, &weights);

  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&S);
  igraph_vector_destroy(&B);
  igraph_vector_destroy(&weights);
  igraph_destroy(&G);
  return res;
}

int test_inc_random(unsigned long int seed) {
  igraph_t G;
  igraph_integer_t eid;
  igraph_integer_t u, v;
  igraph_real_t weight;
  igraph_vector_t weights;
  igraph_matrix_t D;
  igraph_matrix_int_t S;
  igraph_vector_t B;

  // initialize a graph
  igraph_rng_seed(igraph_rng_default(), seed);
  igraph_erdos_renyi_game(&G, IGRAPH_ERDOS_RENYI_GNM, 100, 400, 0, 0);

  // select endpoints to insert
  igraph_t C;
  igraph_complementer(&C, &G, 0);
  eid = igraph_rng_get_integer
    (igraph_rng_default(), 0, igraph_ecount(&C)-1);
  igraph_edge(&C, eid, &u, &v);
  igraph_destroy(&C);

  // set weights
  igraph_vector_init(&weights, igraph_ecount(&G));
  for(eid = 0; eid < igraph_ecount(&G); eid++) {
    weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
    igraph_vector_set(&weights, eid, weight);
  }
  weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);

  // find distance and geodesics and betweenness
  igraph_matrix_init(&D, igraph_vcount(&G), igraph_vcount(&G));
  igraph_matrix_int_init(&S, igraph_vcount(&G), igraph_vcount(&G));
  igraph_vector_init(&B, igraph_vcount(&G));
  betweenness_with_redundant_information(&G, &D, &S, &B, &weights);

  _incremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);

  int res = _check_quantities("test_inc_random", &G, &D, &S, &B, &weights);

  // cleanup
  igraph_destroy(&G);
  igraph_vector_destroy(&weights);
  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&S);
  igraph_vector_destroy(&B);

  return res;
}

int test_dec_random(unsigned long int seed) {
  igraph_rng_seed(igraph_rng_default(), seed);
  return 0;
}

int test_inc_components() {
  unsigned long int seed = 42;
  igraph_rng_seed(igraph_rng_default(), seed);
  return 0;
}

int test_dec_bridge() {
  unsigned long int seed = 42;
  igraph_rng_seed(igraph_rng_default(), seed);
  return 0;
}

int test_inc_repeat() {
  unsigned long int seed = 42;
  igraph_rng_seed(igraph_rng_default(), seed);
  return 0;
}

int test_dec_repeat() {
  unsigned long int seed = 42;
  igraph_rng_seed(igraph_rng_default(), seed);
  return 0;
}

int test_repeat() {
  unsigned long int seed = 42;
  igraph_rng_seed(igraph_rng_default(), seed);
  return 0;
}


int _check_quantities(const char* test_name,
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
  igraph_betweenness(G, &B_grand, igraph_vss_all(), 0, weights, 0);
  igraph_vector_scale(&B_grand, 2);

  int dist_err = 0, sigma_err = 0, bet_err = 0;
  // check distance
  for(long int s = 0; s < igraph_vcount(G); s++)
    for(long int t = 0; t < igraph_vcount(G); t++)
      if(cmp(d(s, t), dt(s, t))) {
        if(!dist_err) {
          printf("distance error on %s:\n", test_name);
          dist_err = 1;
        }
        printf("%ld %ld (res:%f true:%f)\n", s, t, d(s, t), dt(s, t));
      }
  // check geodesics
  for(long int s = 0; s < igraph_vcount(G); s++)
    for(long int t = 0; t < igraph_vcount(G); t++)
      if(s(s, t) != st(s, t)) {
        if(!sigma_err) {
          printf("geodesics error on %s:\n", test_name);
          sigma_err = 1;
        }
        printf("%ld %ld (res:%d true:%d)\n", s, t, s(s, t), st(s, t));
      }
  // check betweenness
  for(long int x = 0; x < igraph_vcount(G); x++) {
    igraph_real_t diff1 = igraph_vector_maxdifference(B, &B_true);
    igraph_real_t diff2 = igraph_vector_maxdifference(B, &B_grand);
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

void _incremental_update_weighted(igraph_t* G,
                                  igraph_matrix_t* D,
                                  igraph_matrix_int_t *S,
                                  igraph_vector_t* B,
                                  igraph_integer_t u,
                                  igraph_integer_t v,
                                  igraph_vector_t* weights,
                                  igraph_real_t weight) {
  igraph_inclist_t inclist, succs, preds;
  if(igraph_is_directed(G)) {
    igraph_inclist_init(G, &succs, IGRAPH_OUT);
    igraph_inclist_init(G, &preds, IGRAPH_IN);
  } else {
    igraph_inclist_init(G, &succs, IGRAPH_ALL);
  }
  igraph_inclist_init(G, &inclist, IGRAPH_ALL);

  igraph_vector_int_t sources, targets;
  igraph_vector_ptr_t targets_of_s, sources_of_t;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);
  affected_sources_inc(G, &inclist, &sources, D, u, v, v, weights, weight);
  affected_targets_inc(G, &inclist, &targets, D, u, v, u, weights, weight);
  igraph_vector_ptr_init(&targets_of_s, igraph_vector_int_size(&sources));
  igraph_vector_ptr_init(&sources_of_t, igraph_vector_int_size(&targets));
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    igraph_vector_int_t *ts
      = (igraph_vector_int_t*) malloc(sizeof(igraph_vector_int_t));
    igraph_vector_int_init(ts, 0);
    affected_targets_inc(G, &inclist, ts, D, u, v, s, weights, weight);
    igraph_vector_ptr_set(&targets_of_s, si, ts);
  }
  for(igraph_integer_t ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    igraph_vector_int_t *ss
      = (igraph_vector_int_t*) malloc(sizeof(igraph_vector_int_t));
    igraph_vector_int_init(ss, 0);
    affected_sources_inc(G, &inclist, ss, D, u, v, t, weights, weight);
    igraph_vector_ptr_set(&sources_of_t, ti, ss);
  }

  // decrease betweenness
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    igraph_vector_int_t *ts
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&targets_of_s, si);
    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? -1 : -2;
    update_deps_inc_weighted(G, &inclist, D, S, B, u, v,
                             s, ts, weights, weight, factor);
  }
  /*
  for(igraph_integer_t ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    igraph_vector_int_t *ss
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&sources_of_t, ti);
    update_deps_inc_weighted(G, &inclist, &D, &S, &B, u, v,
                             t, ss, &weights, weight, -1);
  }
  */

  igraph_add_edge(G, u, v);
  igraph_vector_push_back(weights, weight);
  igraph_integer_t eid = igraph_ecount(G) - 1;
  igraph_vector_int_push_back(igraph_inclist_get(&inclist, u), eid);
  igraph_vector_int_push_back(igraph_inclist_get(&inclist, v), eid);

  for(int ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    update_sssp_inc_weighted
      (G, &inclist, &inclist, D, S, v, u, t, weights, weight);
  }
  for(int si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    update_sssp_inc_weighted
      (G, &inclist, &inclist, D, S, u, v, s, weights, weight);
  }

  // increase betweenness
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    igraph_vector_int_t *ts
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&targets_of_s, si);
    // factor is 2 for undirected and 1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    update_deps_inc_weighted(G, &inclist, D, S, B, u, v,
                             s, ts, weights, weight, factor);
  }

  // cleanup
  for(igraph_integer_t ti = 0; ti < igraph_vector_ptr_size(&sources_of_t); ti++) {
    igraph_vector_int_t *ss = igraph_vector_ptr_e(&sources_of_t, ti);
    igraph_vector_int_destroy(ss);
    free(ss);
  }
  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
  igraph_inclist_destroy(&succs);
  if(igraph_is_directed(G))
    igraph_inclist_destroy(&preds);
}

void _decremental_update_weighted(igraph_t* G,
                                  igraph_matrix_t* D,
                                  igraph_matrix_int_t *S,
                                  igraph_vector_t* B,
                                  igraph_integer_t u,
                                  igraph_integer_t v,
                                  igraph_vector_t* weights,
                                  igraph_real_t weight) {
  igraph_inclist_t inclist, succs, preds;
  if(igraph_is_directed(G)) {
    igraph_inclist_init(G, &succs, IGRAPH_OUT);
    igraph_inclist_init(G, &preds, IGRAPH_IN);
  } else {
    igraph_inclist_init(G, &succs, IGRAPH_ALL);
  }
  igraph_inclist_init(G, &inclist, IGRAPH_ALL);

  igraph_vector_int_t sources, targets;
  igraph_vector_ptr_t targets_of_s, sources_of_t;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);
  affected_sources_dec(G, &inclist, &sources, D, u, v, v, weights, weight);
  affected_targets_dec(G, &inclist, &targets, D, u, v, u, weights, weight);
  igraph_vector_ptr_init(&targets_of_s, igraph_vector_int_size(&sources));
  igraph_vector_ptr_init(&sources_of_t, igraph_vector_int_size(&targets));
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    igraph_vector_int_t *ts
      = (igraph_vector_int_t*) malloc(sizeof(igraph_vector_int_t));
    igraph_vector_int_init(ts, 0);
    affected_targets_dec(G, &inclist, ts, D, u, v, s, weights, weight);
    igraph_vector_ptr_set(&targets_of_s, si, ts);
  }
  for(igraph_integer_t ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    igraph_vector_int_t *ss
      = (igraph_vector_int_t*) malloc(sizeof(igraph_vector_int_t));
    igraph_vector_int_init(ss, 0);
    affected_sources_dec(G, &inclist, ss, D, u, v, t, weights, weight);
    igraph_vector_ptr_set(&sources_of_t, ti, ss);
  }

  // decrease betweenness
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    igraph_vector_int_t *ts
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&targets_of_s, si);
    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? -1 : -2;
    update_deps_dec_weighted(G, &inclist, D, S, B, u, v,
                             s, ts, weights, weight, factor);
  }
  /*
  for(igraph_integer_t ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    igraph_vector_int_t *ss
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&sources_of_t, ti);
    update_deps_dec_weighted(G, &inclist, &D, &S, &B, u, v,
                             t, ss, weights, weight, -1);
  }
  */

  // modify
  igraph_integer_t eid;
  igraph_get_eid(G, &eid, u, v, 0, 1);
  igraph_delete_edges(G, igraph_ess_1(eid));
  igraph_vector_remove(weights, eid);
  for(long int vid = 0; vid < igraph_vcount(G); vid++) {
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
      (G, &inclist, &inclist, D, S, v, u, t, weights, weight);
  }
  for(int si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    update_sssp_dec_weighted
      (G, &inclist, &inclist, D, S, u, v, s, weights, weight);
  }

  // increase betweenness
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    igraph_vector_int_t *ts
      = (igraph_vector_int_t*) igraph_vector_ptr_e(&targets_of_s, si);
    // factor is 2 for undirected and 1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    update_deps_dec_weighted(G, &inclist, D, S, B, u, v,
                             s, ts, weights, weight, factor);
  }

  // cleanup
  for(igraph_integer_t ti = 0; ti < igraph_vector_ptr_size(&sources_of_t); ti++) {
    igraph_vector_int_t *ss = igraph_vector_ptr_e(&sources_of_t, ti);
    igraph_vector_int_destroy(ss);
    free(ss);
  }
  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
  igraph_inclist_destroy(&succs);
  if(igraph_is_directed(G))
    igraph_inclist_destroy(&preds);
}
