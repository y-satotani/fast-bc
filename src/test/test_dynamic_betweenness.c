
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
int test_inc_components(unsigned long int seed);
int test_dec_bridge(unsigned long int seed);
int test_repeat(unsigned long int seed, int steps);

int main(int argc, char* argv[]) {
  test_inc_mini();
  test_dec_mini();
  // confirmed no error for -10k
  //for(unsigned int seed = 0; seed < 100; seed++)
  //  test_inc_random(seed);
  // confirmed no error for -10k
  //for(unsigned int seed = 0; seed < 100; seed++)
  //  test_dec_random(seed);
  // confirmed no error for -10k
  //for(unsigned int seed = 0; seed < 100; seed++)
  //  test_inc_components(seed);
  // confirmed no error for -10k
  //for(unsigned int seed = 0; seed < 10000; seed++)
  //  test_dec_bridge(seed);
  // confirmed no error for -100 seeds and -1000 steps
  for(unsigned int seed = 0; seed < 100; seed++)
    test_repeat(seed, 1000);
  return 0;
}

int _check_quantities(const char* test_name,
                      igraph_t* G,
                      igraph_matrix_t* D,
                      igraph_matrix_int_t* S,
                      igraph_vector_t* B,
                      igraph_vector_t* weights,
                      igraph_bool_t force_print);

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

#define _DYBC_TEST_DECL_                        \
  igraph_t G;                                   \
  igraph_integer_t u, v;                        \
  igraph_real_t weight;                         \
  igraph_vector_t weights;                      \
  igraph_matrix_t D;                            \
  igraph_matrix_int_t S;                        \
  igraph_vector_t B;

// find distance and geodesics and betweenness
#define _DYBC_TEST_INIT_                        \
  igraph_matrix_init                            \
  (&D, igraph_vcount(&G), igraph_vcount(&G));   \
  igraph_matrix_int_init                        \
  (&S, igraph_vcount(&G), igraph_vcount(&G));   \
  igraph_vector_init                            \
  (&B, igraph_vcount(&G));                      \
  betweenness_with_redundant_information        \
  (&G, &D, &S, &B, &weights);

#define _DYBC_TEST_DEST_                        \
  igraph_matrix_destroy(&D);                    \
  igraph_matrix_int_destroy(&S);                \
  igraph_vector_destroy(&B);                    \
  igraph_vector_destroy(&weights);              \
  igraph_destroy(&G);

int test_inc_mini() {
  _DYBC_TEST_DECL_;
  make_less_graph_and_edge(&G, &u, &v, &weight, &weights);
  _DYBC_TEST_INIT_;
  _incremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);
  int res = _check_quantities("test_inc_mini", &G, &D, &S, &B, &weights, 0);
  _DYBC_TEST_DEST_;
  return res;
}

int test_dec_mini() {
  _DYBC_TEST_DECL_;
  make_more_graph_and_edge(&G, &u, &v, &weight, &weights);
  _DYBC_TEST_INIT_;
  _decremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);
  int res = _check_quantities("test_dec_mini", &G, &D, &S, &B, &weights, 0);
  _DYBC_TEST_DEST_;
  return res;
}

int test_inc_random(unsigned long int seed) {
  _DYBC_TEST_DECL_;
  // initialize a graph
  igraph_rng_seed(igraph_rng_default(), seed);
  igraph_erdos_renyi_game(&G, IGRAPH_ERDOS_RENYI_GNM, 100, 400, 0, 0);
  // select endpoints to insert
  igraph_t C;
  igraph_complementer(&C, &G, 0);
  igraph_integer_t eid = igraph_rng_get_integer
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
  _DYBC_TEST_INIT_;

  _incremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);
  char test_name[1024];
  sprintf(test_name, "test_inc_random (%lu)", seed);
  int res = _check_quantities(test_name, &G, &D, &S, &B, &weights, 0);

  _DYBC_TEST_DEST_;
  return res;
}

int test_dec_random(unsigned long int seed) {
  _DYBC_TEST_DECL_;
  // initialize a graph
  igraph_rng_seed(igraph_rng_default(), seed);
  igraph_erdos_renyi_game(&G, IGRAPH_ERDOS_RENYI_GNM, 100, 400, 0, 0);
  // set weights
  igraph_vector_init(&weights, igraph_ecount(&G));
  for(igraph_integer_t eid = 0; eid < igraph_ecount(&G); eid++) {
    weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
    igraph_vector_set(&weights, eid, weight);
  }
  // select endpoints to delete
  igraph_integer_t eid = igraph_rng_get_integer
  (igraph_rng_default(), 0, igraph_ecount(&G)-1);
  igraph_edge(&G, eid, &u, &v);
  weight = igraph_vector_e(&weights, eid);
  _DYBC_TEST_INIT_;

  _decremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);
  char test_name[1024];
  sprintf(test_name, "test_dec_random (%lu)", seed);
  int res = _check_quantities(test_name, &G, &D, &S, &B, &weights, 0);

  _DYBC_TEST_DEST_;
  return res;
}

int test_inc_components(unsigned long int seed) {
  _DYBC_TEST_DECL_;
  // initialize a graph
  igraph_rng_seed(igraph_rng_default(), seed);
  igraph_integer_t n1 = 20, n2 = 20, n3 = 20;
  igraph_integer_t m1 = 40, m2 = 50, m3 = 60;
  igraph_t G1, G2, G3;
  igraph_erdos_renyi_game(&G1, IGRAPH_ERDOS_RENYI_GNM, n1, m1, 0, 0);
  igraph_erdos_renyi_game(&G2, IGRAPH_ERDOS_RENYI_GNM, n2, m2, 0, 0);
  igraph_erdos_renyi_game(&G3, IGRAPH_ERDOS_RENYI_GNM, n3, m3, 0, 0);
  igraph_empty(&G, n1+n2+n3, 0);
  for(igraph_integer_t eid = 0; eid < m1; eid++) {
    igraph_edge(&G1, eid, &u, &v);
    igraph_add_edge(&G, u, v);
  }
  for(igraph_integer_t eid = 0; eid < m2; eid++) {
    igraph_edge(&G2, eid, &u, &v);
    igraph_add_edge(&G, u + n1, v + n1);
  }
  for(igraph_integer_t eid = 0; eid < m3; eid++) {
    igraph_edge(&G3, eid, &u, &v);
    igraph_add_edge(&G, u + n1 + n2, v + n1 + n2);
  }

  // select endpoints to insert
  u = igraph_rng_get_integer
    (igraph_rng_default(), 0, n1-1);
  v = igraph_rng_get_integer
    (igraph_rng_default(), n1, n1+n2-1);

  // set weights
  igraph_vector_init(&weights, igraph_ecount(&G));
  for(igraph_integer_t eid = 0; eid < igraph_ecount(&G); eid++) {
    weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
    igraph_vector_set(&weights, eid, weight);
  }
  weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
  _DYBC_TEST_INIT_;

  _incremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);
  char test_name[1024];
  sprintf(test_name, "test_inc_components (%lu)", seed);
  int res = _check_quantities(test_name, &G, &D, &S, &B, &weights, 0);

  igraph_destroy(&G1);
  igraph_destroy(&G2);
  igraph_destroy(&G3);
  _DYBC_TEST_DEST_;
  return res;
}

int test_dec_bridge(unsigned long int seed) {
  _DYBC_TEST_DECL_;
  // initialize a graph
  igraph_rng_seed(igraph_rng_default(), seed);
  igraph_integer_t n1 = 20, n2 = 20, n3 = 20;
  igraph_integer_t m1 = 40, m2 = 50, m3 = 60;
  igraph_t G1, G2, G3;
  igraph_erdos_renyi_game(&G1, IGRAPH_ERDOS_RENYI_GNM, n1, m1, 0, 0);
  igraph_erdos_renyi_game(&G2, IGRAPH_ERDOS_RENYI_GNM, n2, m2, 0, 0);
  igraph_erdos_renyi_game(&G3, IGRAPH_ERDOS_RENYI_GNM, n3, m3, 0, 0);
  igraph_empty(&G, n1+n2+n3, 0);
  for(igraph_integer_t eid = 0; eid < m1; eid++) {
    igraph_edge(&G1, eid, &u, &v);
    igraph_add_edge(&G, u, v);
  }
  for(igraph_integer_t eid = 0; eid < m2; eid++) {
    igraph_edge(&G2, eid, &u, &v);
    igraph_add_edge(&G, u + n1, v + n1);
  }
  for(igraph_integer_t eid = 0; eid < m3; eid++) {
    igraph_edge(&G3, eid, &u, &v);
    igraph_add_edge(&G, u + n1 + n2, v + n1 + n2);
  }

  // select endpoints to delete
  u = igraph_rng_get_integer
    (igraph_rng_default(), 0, n1-1);
  v = igraph_rng_get_integer
    (igraph_rng_default(), n1, n1+n2-1);
  igraph_add_edge(&G, u, v);

  // set weights
  igraph_vector_init(&weights, igraph_ecount(&G));
  for(igraph_integer_t eid = 0; eid < igraph_ecount(&G); eid++) {
    weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
    igraph_vector_set(&weights, eid, weight);
  }
  weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
  _DYBC_TEST_INIT_;

  _decremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);
  char test_name[1024];
  sprintf(test_name, "test_inc_components (%lu)", seed);
  int res = _check_quantities(test_name, &G, &D, &S, &B, &weights, 0);

  igraph_destroy(&G1);
  igraph_destroy(&G2);
  igraph_destroy(&G3);
  _DYBC_TEST_DEST_;
  return res;
}

int test_repeat(unsigned long int seed, int steps) {
  _DYBC_TEST_DECL_;
  // initialize a graph
  igraph_rng_seed(igraph_rng_default(), seed);
  igraph_erdos_renyi_game(&G, IGRAPH_ERDOS_RENYI_GNM, 20, 50, 0, 0);
  // set weights
  igraph_vector_init(&weights, igraph_ecount(&G));
  for(igraph_integer_t eid = 0; eid < igraph_ecount(&G); eid++) {
    weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
    igraph_vector_set(&weights, eid, weight);
  }
  _DYBC_TEST_INIT_;

  int res = 0;

  for(int step = 0; step < steps; step++) {
    igraph_integer_t eid;
    char test_name[1024];
    // select endpoints to insert
    igraph_t C;
    igraph_complementer(&C, &G, 0);
    eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&C)-1);
    igraph_edge(&C, eid, &u, &v);
    igraph_destroy(&C);
    weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
    _incremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);

    sprintf(test_name, "test_repeat (%lu) at inc step %d", seed, step);
    res |= _check_quantities(test_name, &G, &D, &S, &B, &weights, 0);

    // select endpoints to delete
    eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&G)-1);
    igraph_edge(&G, eid, &u, &v);
    weight = igraph_vector_e(&weights, eid);
    _decremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);

    sprintf(test_name, "test_repeat (%lu) at dec step %d", seed, step);
    int res = _check_quantities(test_name, &G, &D, &S, &B, &weights, 0);
  }

  _DYBC_TEST_DEST_;
  return res;
}

int _check_quantities(const char* test_name,
                      igraph_t* G,
                      igraph_matrix_t* D,
                      igraph_matrix_int_t* S,
                      igraph_vector_t* B,
                      igraph_vector_t* weights,
                      igraph_bool_t force_print) {
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
  for(long int s = 0; s < igraph_vcount(G); s++) {
    for(long int t = 0; t < igraph_vcount(G); t++) {
      if(cmp(d(s, t), dt(s, t))) {
        if(!dist_err) {
          printf("distance error on %s:\n", test_name);
          dist_err = 1;
        }
        printf("%ld %ld (res:%f true:%f)\n", s, t, d(s, t), dt(s, t));
      } else if(force_print) {
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
      } else if(force_print) {
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
    } else if(force_print) {
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
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);
  affected_sources_inc(G, &inclist, &sources, D, u, v, v, weights, weight);

  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    // decrease betweenness
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_inc(G, &inclist, &targets, D, u, v, s, weights, weight);
    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    update_deps_inc_weighted(G, &inclist, D, S, B, u, v,
                             s, &targets, weights, weight, -factor);

    // add edge
    igraph_add_edge(G, u, v);
    igraph_vector_push_back(weights, weight);
    igraph_integer_t eid = igraph_ecount(G) - 1;
    igraph_vector_int_push_back(igraph_inclist_get(&inclist, u), eid);
    igraph_vector_int_push_back(igraph_inclist_get(&inclist, v), eid);

    update_sssp_inc_weighted
      (G, &inclist, &inclist, D, S, u, v, s, weights, weight);

    // increase betweenness
    // factor is 2 for undirected and 1 for directed
    update_deps_inc_weighted(G, &inclist, D, S, B, u, v,
                             s, &targets, weights, weight, factor);

    // cleanup
    igraph_delete_edges(G, igraph_ess_1(eid));
    igraph_vector_remove(weights, eid);
    igraph_vector_int_pop_back(igraph_inclist_get(&inclist, u));
    igraph_vector_int_pop_back(igraph_inclist_get(&inclist, v));
  }

  affected_sources_inc(G, &inclist, &targets, D, v, u, u, weights, weight);

  // add edge
  igraph_integer_t eid = igraph_ecount(G);
  igraph_add_edge(G, u, v);
  igraph_vector_push_back(weights, weight);
  igraph_vector_int_push_back(igraph_inclist_get(&inclist, u), eid);
  igraph_vector_int_push_back(igraph_inclist_get(&inclist, v), eid);

  for(int ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    update_sssp_inc_weighted
      (G, &inclist, &inclist, D, S, v, u, t, weights, weight);
  }

  // cleanup
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
  // move the edge to be deleted to back of list
  igraph_integer_t eid;
  igraph_get_eid(G, &eid, u, v, 0, 1);
  igraph_delete_edges(G, igraph_ess_1(eid));
  weight = igraph_vector_e(weights, eid);
  igraph_vector_remove(weights, eid);

  igraph_inclist_t inclist, succs, preds;
  if(igraph_is_directed(G)) {
    igraph_inclist_init(G, &succs, IGRAPH_OUT);
    igraph_inclist_init(G, &preds, IGRAPH_IN);
  } else {
    igraph_inclist_init(G, &succs, IGRAPH_ALL);
  }
  igraph_inclist_init(G, &inclist, IGRAPH_ALL);

  // then push the deleted edge
  eid = igraph_ecount(G);
  igraph_add_edge(G, u, v);
  igraph_vector_push_back(weights, weight);
  igraph_vector_int_push_back(igraph_inclist_get(&inclist, u), eid);
  igraph_vector_int_push_back(igraph_inclist_get(&inclist, v), eid);

  // finding affected sources
  igraph_vector_int_t sources, targets;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);
  affected_sources_dec(G, &inclist, &sources, D, u, v, v, weights, weight);

  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_dec(G, &inclist, &targets, D, u, v, s, weights, weight);
    // decrease betweenness
    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    update_deps_dec_weighted(G, &inclist, D, S, B, u, v,
                             s, &targets, weights, weight, -factor);

    // modify
    igraph_delete_edges(G, igraph_ess_1(eid));
    igraph_vector_pop_back(weights);
    igraph_vector_int_pop_back(igraph_inclist_get(&inclist, u));
    igraph_vector_int_pop_back(igraph_inclist_get(&inclist, v));

    // update sssp
    update_sssp_dec_weighted
      (G, &inclist, &inclist, D, S, u, v, s, weights, weight);

    // increase betweenness
    // factor is 2 for undirected and 1 for directed
    update_deps_dec_weighted(G, &inclist, D, S, B, u, v,
                             s, &targets, weights, weight, factor);

    // cleanup for next round
    igraph_add_edge(G, u, v);
    igraph_vector_push_back(weights, weight);
    igraph_vector_int_push_back(igraph_inclist_get(&inclist, u), eid);
    igraph_vector_int_push_back(igraph_inclist_get(&inclist, v), eid);
  }

  affected_sources_dec(G, &inclist, &targets, D, v, u, u, weights, weight);
  // delete an edge
  igraph_delete_edges(G, igraph_ess_1(eid));
  igraph_vector_pop_back(weights);
  igraph_vector_int_pop_back(igraph_inclist_get(&inclist, u));
  igraph_vector_int_pop_back(igraph_inclist_get(&inclist, v));
  // update stsp
  for(int ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets, ti);
    update_sssp_dec_weighted
      (G, &inclist, &inclist, D, S, v, u, t, weights, weight);
  }
  // cleanup
  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
  igraph_inclist_destroy(&succs);
  if(igraph_is_directed(G))
    igraph_inclist_destroy(&preds);
}
