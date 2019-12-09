#include "incremental_betweenness.h"

#include <assert.h>
#include <igraph/igraph.h>
#include <igraph/igraph_math.h>
#include <igraph/igraph_types_internal.h>

void update_deps_inc_weighted(igraph_t* G,
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
                              igraph_real_t factor) {
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
    igraph_2wheap_push_with_index(&queue, t, d(source, t));
    igraph_vector_bool_set(&is_target, t, 1);
  }

  while(!igraph_2wheap_empty(&queue)) {
    igraph_integer_t x = igraph_2wheap_max_index(&queue);
    igraph_2wheap_delete_max(&queue);
    B(x) += factor * Delta(x);
    if(igraph_is_inf(d(source, x))) continue;

    igraph_vector_int_t* ps = igraph_inclist_get(preds, x);
    for(long int pi = 0; pi < igraph_vector_int_size(ps); pi++) {
      igraph_integer_t eid = VECTOR(*ps)[pi];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);

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

void update_sssp_inc_weighted(igraph_t* G,
                              igraph_inclist_t* preds,
                              igraph_inclist_t* succs,
                              igraph_matrix_t* D,
                              igraph_matrix_int_t* S,
                              igraph_integer_t u,
                              igraph_integer_t v,
                              igraph_integer_t source,
                              igraph_vector_t* weights,
                              igraph_real_t weight) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define s(a, b) (MATRIX(*S, (a), (b)))
#define l(eid) (weights ? VECTOR(*weights)[eid] : 1)

  if(igraph_is_inf(d(source, u))
     || cmp(d(source, v), d(source, u) + weight) < 0) {
    return;
  }

  igraph_2wheap_t queue;
  igraph_2wheap_init(&queue, igraph_vcount(G));
  igraph_2wheap_push_with_index(&queue, v, -(d(source, u) + weight));

  while(!igraph_2wheap_empty(&queue)) {
    long int x = igraph_2wheap_max_index(&queue);
    d(source, x) = -igraph_2wheap_delete_max(&queue);
    s(source, x) = 0;

    igraph_vector_int_t* ps = igraph_inclist_get(preds, x);
    for(long int pi = 0; pi < igraph_vector_int_size(ps); pi++) {
      igraph_integer_t eid = VECTOR(*ps)[pi];
      igraph_integer_t p = IGRAPH_OTHER(G, eid, x);
      if(cmp(d(source, x), d(source, p) + l(eid)) == 0)
        s(source, x) += s(source, p);
    } /* for preds */

    igraph_vector_int_t* ss = igraph_inclist_get(succs, x);
    for(long int si = 0; si < igraph_vector_int_size(ss); si++) {
      igraph_integer_t eid = VECTOR(*ss)[si];
      igraph_integer_t s = IGRAPH_OTHER(G, eid, x);
      igraph_real_t d_s = d(source, x) + l(eid);
      int cmp_result = cmp(d(source, s), d_s);
      if(cmp_result >= 0) {
        if(igraph_2wheap_has_elem(&queue, s))
          igraph_2wheap_modify(&queue, s, -d_s);
        else
          igraph_2wheap_push_with_index(&queue, s, -d_s);
      }
    } /* for succs */
  } /* !igraph_2wheap_empty(&Q) */

  igraph_2wheap_destroy(&queue);
#undef EPS
#undef cmp
#undef d
#undef s
}

void update_stsp_inc_weighted(igraph_t* G,
                              igraph_inclist_t* preds,
                              igraph_inclist_t* succs,
                              igraph_matrix_t* D,
                              igraph_matrix_int_t* S,
                              igraph_integer_t u,
                              igraph_integer_t v,
                              igraph_integer_t target,
                              igraph_vector_t* weights,
                              igraph_real_t weight) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define s(a, b) (MATRIX(*S, (a), (b)))
#define l(eid) (weights ? VECTOR(*weights)[eid] : 1)

  if(igraph_is_inf(d(v, target))
     || cmp(d(u, target), weight + d(v, target)) < 0) {
    return;
  }

  igraph_2wheap_t queue;
  igraph_2wheap_init(&queue, igraph_vcount(G));
  igraph_2wheap_push_with_index(&queue, u, -(weight + d(v, target)));

  while(!igraph_2wheap_empty(&queue)) {
    long int x = igraph_2wheap_max_index(&queue);
    d(x, target) = -igraph_2wheap_delete_max(&queue);
    s(x, target) = 0;

    igraph_vector_int_t* ss = igraph_inclist_get(succs, x);
    for(long int si = 0; si < igraph_vector_int_size(ss); si++) {
      igraph_integer_t eid = VECTOR(*ss)[si];
      igraph_integer_t s = IGRAPH_OTHER(G, eid, x);
      if(cmp(d(x, target), l(eid) + d(s, target)) == 0)
        s(x, target) += s(s, target);
    } /* for succs */

    igraph_vector_int_t* ps = igraph_inclist_get(preds, x);
    for(long int pi = 0; pi < igraph_vector_int_size(ps); pi++) {
      igraph_integer_t eid = VECTOR(*ps)[pi];
      igraph_integer_t p = IGRAPH_OTHER(G, eid, x);
      igraph_real_t d_p = l(eid) + d(x, target);
      int cmp_result = cmp(d(p, target), d_p);
      if(cmp_result >= 0) {
        if(igraph_2wheap_has_elem(&queue, p))
          igraph_2wheap_modify(&queue, p, -d_p);
        else
          igraph_2wheap_push_with_index(&queue, p, -d_p);
      }
    } /* for preds */
  } /* !igraph_2wheap_empty(&Q) */

  igraph_2wheap_destroy(&queue);
#undef EPS
#undef cmp
#undef d
#undef s
}

void affected_targets_inc(igraph_t* G,
                          igraph_inclist_t* inclist,
                          igraph_vector_int_t* out,
                          igraph_matrix_t* D,
                          igraph_integer_t u,
                          igraph_integer_t v,
                          igraph_integer_t source,
                          igraph_vector_t* weights,
                          igraph_real_t weight) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define l(eid) (weights ? VECTOR(*weights)[eid] : 1)
  if(!weights) weight = 1;
  assert(weight > 0);
  assert(out);

  igraph_vector_int_clear(out);
  if(igraph_is_inf(d(source, u))
     || cmp(d(source, v), d(source, u) + weight) < 0) {
    return;
  }
  igraph_vector_bool_t visited;
  igraph_stack_int_t stack;
  igraph_vector_bool_init(&visited, igraph_vcount(G));
  igraph_stack_int_init(&stack, 0);
  igraph_vector_bool_set(&visited, v, 1);
  igraph_stack_int_push(&stack, v);

  while(!igraph_stack_int_empty(&stack)) {
    igraph_integer_t x = igraph_stack_int_pop(&stack);
    igraph_vector_int_push_back(out, x);
    igraph_vector_int_t* neis = igraph_inclist_get(inclist, x);
    igraph_integer_t ni;
    for(ni = 0; ni < igraph_vector_int_size(neis); ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      igraph_real_t d_sy = d(source, y);
      igraph_real_t d_sy_p = d(source, u) + weight + d(v, x) + l(eid);
      if(cmp(d_sy, d_sy_p) >= 0 && !igraph_vector_bool_e(&visited, y)) {
        igraph_vector_bool_set(&visited, y, 1);
        igraph_stack_int_push(&stack, y);
      }
    }
  }

  igraph_vector_bool_destroy(&visited);
  igraph_stack_int_destroy(&stack);

#undef EPS
#undef cmp
#undef d
}

void affected_sources_inc(igraph_t* G,
                          igraph_inclist_t* inclist,
                          igraph_vector_int_t* out,
                          igraph_matrix_t* D,
                          igraph_integer_t u,
                          igraph_integer_t v,
                          igraph_integer_t target,
                          igraph_vector_t* weights,
                          igraph_real_t weight) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define l(eid) (weights ? VECTOR(*weights)[eid] : 1)
  if(!weights) weight = 1;
  assert(weight > 0);
  assert(out);

  igraph_vector_int_clear(out);
  if(igraph_is_inf(d(v, target))
     || cmp(d(u, target), weight + d(v, target)) < 0) {
    return;
  }
  igraph_vector_bool_t visited;
  igraph_stack_int_t stack;
  igraph_vector_bool_init(&visited, igraph_vcount(G));
  igraph_stack_int_init(&stack, 0);
  igraph_vector_bool_set(&visited, u, 1);
  igraph_stack_int_push(&stack, u);

  while(!igraph_stack_int_empty(&stack)) {
    igraph_integer_t x = igraph_stack_int_pop(&stack);
    igraph_vector_int_push_back(out, x);
    igraph_vector_int_t* neis = igraph_inclist_get(inclist, x);
    igraph_integer_t ni;
    for(ni = 0; ni < igraph_vector_int_size(neis); ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      igraph_real_t d_sy = d(y, target);
      igraph_real_t d_sy_p = l(eid) + d(x, u) + weight + d(v, target);
      if(cmp(d_sy, d_sy_p) >= 0 && !igraph_vector_bool_e(&visited, y)) {
        igraph_vector_bool_set(&visited, y, 1);
        igraph_stack_int_push(&stack, y);
      }
    }
  }

  igraph_vector_bool_destroy(&visited);
  igraph_stack_int_destroy(&stack);

#undef EPS
#undef cmp
#undef d
}
