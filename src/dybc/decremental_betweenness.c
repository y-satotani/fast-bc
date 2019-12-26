#include "decremental_betweenness.h"

#include <igraph/igraph.h>
#include <igraph/igraph_math.h>
#include <igraph/igraph_types_internal.h>

void update_betw_dec_weighted(igraph_t* G,
                          igraph_inclist_t* preds,
                          igraph_matrix_t* D,
                          igraph_matrix_int_t* S,
                          igraph_vector_t* B,
                          igraph_integer_t u,
                          igraph_integer_t v,
                          igraph_integer_t source,
                          igraph_vector_int_t* targets,
                          igraph_vector_t* weights,
                          igraph_real_t weight,
                          igraph_real_t factor,
                          igraph_bool_t is_post_update) {
  update_betw_dec_weighted_statistics
    (G, preds, D, S, B, u, v, source, targets, weights, weight, factor, is_post_update, 0);
}

void update_betw_dec_unweighted(igraph_t* G,
                            igraph_inclist_t* preds,
                            igraph_matrix_t* D,
                            igraph_matrix_int_t* S,
                            igraph_vector_t* B,
                            igraph_integer_t u,
                            igraph_integer_t v,
                            igraph_integer_t source,
                            igraph_vector_int_t* targets,
                            igraph_real_t factor,
                            igraph_bool_t is_post_update) {
  update_betw_dec_unweighted_statistics
    (G, preds, D, S, B, u, v, source, targets, factor, is_post_update, 0);
}

void update_betw_dec_weighted_statistics(igraph_t* G,
                                     igraph_inclist_t* preds,
                                     igraph_matrix_t* D,
                                     igraph_matrix_int_t* S,
                                     igraph_vector_t* B,
                                     igraph_integer_t u,
                                     igraph_integer_t v,
                                     igraph_integer_t source,
                                     igraph_vector_int_t* targets,
                                     igraph_vector_t* weights,
                                     igraph_real_t weight,
                                     igraph_real_t factor,
                                     igraph_bool_t is_post_update,
                                     igraph_vector_int_t* traversed_vertices) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define s(a, b) (MATRIX(*S, (a), (b)))
#define l(eid) (weights ? VECTOR(*weights)[eid] : 1)
#define B(a) (VECTOR(*B)[(a)])
#define Delta(a) (VECTOR(Delta)[(a)])
  igraph_vector_t Delta;
  igraph_2wheap_t queue;
  igraph_vector_bool_t is_target;

  igraph_vector_init(&Delta, igraph_vcount(G));
  igraph_2wheap_init(&queue, igraph_vcount(G));
  igraph_vector_bool_init(&is_target, igraph_vcount(G));

  for(long int ti = 0; ti < igraph_vector_int_size(targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(targets, ti);
    if(igraph_is_inf(d(source, t))) continue;
    igraph_2wheap_push_with_index(&queue, t, d(source, t));
    igraph_vector_bool_set(&is_target, t, 1);
  }

  while(!igraph_2wheap_empty(&queue)) {
    igraph_integer_t x = igraph_2wheap_max_index(&queue);
    igraph_2wheap_delete_max(&queue);
    B(x) += factor * Delta(x);
    if(traversed_vertices)
      igraph_vector_int_push_back(traversed_vertices, x);
    if(igraph_is_inf(d(source, x))) continue;

    igraph_vector_int_t* ps = igraph_inclist_get(preds, x);
    for(long int pi = 0; pi < igraph_vector_int_size(ps); pi++) {
      igraph_integer_t eid = VECTOR(*ps)[pi];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      if(is_post_update && x == v && y == u) continue;

      if(y == source || cmp(d(source, x), d(source, y) + l(eid)) < 0)
        continue;
      if(VECTOR(is_target)[x])
        Delta(y) += (1. + Delta(x)) * s(source, y) / s(source, x);
      else
        Delta(y) += Delta(x) * s(source, y) / s(source, x);
      if(!igraph_2wheap_has_elem(&queue, y))
        igraph_2wheap_push_with_index(&queue, y, d(source, y));
    }

  }

  igraph_vector_destroy(&Delta);
  igraph_2wheap_destroy(&queue);
  igraph_vector_bool_destroy(&is_target);
#undef EPS
#undef cmp
#undef d
#undef s
#undef l
#undef B
#undef Delta
}

void update_betw_dec_unweighted_statistics(igraph_t* G,
                                       igraph_inclist_t* preds,
                                       igraph_matrix_t* D,
                                       igraph_matrix_int_t* S,
                                       igraph_vector_t* B,
                                       igraph_integer_t u,
                                       igraph_integer_t v,
                                       igraph_integer_t source,
                                       igraph_vector_int_t* targets,
                                       igraph_real_t factor,
                                       igraph_bool_t is_post_update,
                                       igraph_vector_int_t* traversed_vertices) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define s(a, b) (MATRIX(*S, (a), (b)))
#define B(a) (VECTOR(*B)[(a)])
#define Delta(a) (VECTOR(Delta)[(a)])
  igraph_vector_t Delta;
  igraph_buckets_t queue;
  igraph_vector_bool_t visited;
  igraph_vector_bool_t is_target;

  igraph_vector_init(&Delta, igraph_vcount(G));
  igraph_buckets_init(&queue, igraph_vcount(G), igraph_vcount(G));
  igraph_vector_bool_init(&visited, igraph_vcount(G));
  igraph_vector_bool_init(&is_target, igraph_vcount(G));

  // assumes that `target` is sorted in accending order of distance
  for(long int ti = 0; ti < igraph_vector_int_size(targets); ti++) {
    igraph_integer_t t = igraph_vector_int_e(targets, ti);
    if(igraph_is_inf(d(source, t))) continue;
    igraph_buckets_add(&queue, (long int)d(source, t), t);
    igraph_vector_bool_set(&visited, t, 1);
    igraph_vector_bool_set(&is_target, t, 1);
  }

  while(!igraph_buckets_empty(&queue)) {
    igraph_integer_t x = igraph_buckets_popmax(&queue);
    B(x) += factor * Delta(x);
    if(traversed_vertices)
      igraph_vector_int_push_back(traversed_vertices, x);
    if(igraph_is_inf(d(source, x))) continue;

    igraph_vector_int_t* ps = igraph_inclist_get(preds, x);
    for(long int pi = 0; pi < igraph_vector_int_size(ps); pi++) {
      igraph_integer_t eid = VECTOR(*ps)[pi];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      if(is_post_update && x == v && y == u) continue;

      if(y == source || cmp(d(source, x), d(source, y) + 1.0) < 0)
        continue;
      if(VECTOR(is_target)[x])
        Delta(y) += (1. + Delta(x)) * s(source, y) / s(source, x);
      else
        Delta(y) += Delta(x) * s(source, y) / s(source, x);
      if(!igraph_vector_bool_e(&visited, y)) {
        igraph_buckets_add(&queue, (long int)d(source, y), y);
        igraph_vector_bool_set(&visited, y, 1);
      }
    }
  }

  igraph_vector_destroy(&Delta);
  igraph_buckets_destroy(&queue);
  igraph_vector_bool_destroy(&visited);
  igraph_vector_bool_destroy(&is_target);
#undef EPS
#undef cmp
#undef d
#undef s
#undef B
#undef Delta
}