#include "decremental_betweenness.h"

#include <assert.h>
#include <igraph/igraph.h>
#include <igraph/igraph_math.h>
#include <igraph/igraph_types_internal.h>

void update_sssp_dec_weighted(igraph_t* G,
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

  igraph_vector_int_t targets, is_affected;
  igraph_vector_int_init(&targets, 0);
  igraph_vector_int_init(&is_affected, igraph_vcount(G));
  affected_targets_dec(G, succs, &targets, D, u, v, source, weights, weight);
  for(long int ti = 0; ti < igraph_vector_int_size(&targets); ti++)
    VECTOR(is_affected)[igraph_vector_int_e(&targets, ti)] = 1;

  igraph_2wheap_t queue;
  igraph_2wheap_init(&queue, igraph_vcount(G));
  for(long int xi = 0; xi < igraph_vector_int_size(&targets); xi++) {
    igraph_integer_t x = igraph_vector_int_e(&targets, xi);
    igraph_vector_int_t* ys = igraph_inclist_get(preds, x);
    igraph_real_t d_hat = IGRAPH_INFINITY;
    for(long int yi = 0; yi < igraph_vector_int_size(ys); yi++) {
      igraph_integer_t eid = igraph_vector_int_e(ys, yi);
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      if(!VECTOR(is_affected)[y] && cmp(d(source, y) + l(eid), d_hat) < 0)
        d_hat = d(source, y) + l(eid);
    }
    if(igraph_is_inf(d_hat)) {
      d(source, x) = IGRAPH_INFINITY;
      s(source, x) = 0;
    } else {
      igraph_2wheap_push_with_index(&queue, x, -d_hat);
    }
  }

  while(!igraph_2wheap_empty(&queue)) {
    long int x = igraph_2wheap_max_index(&queue);
    d(source, x) = -igraph_2wheap_delete_max(&queue);
    s(source, x) = 0;
    VECTOR(is_affected)[x] = 0;

    igraph_vector_int_t* ps = igraph_inclist_get(preds, x);
    for(long int pi = 0; pi < igraph_vector_int_size(ps); pi++) {
      igraph_integer_t eid = VECTOR(*ps)[pi];
      igraph_integer_t p = IGRAPH_OTHER(G, eid, x);
      if(!VECTOR(is_affected)[p]
         && cmp(d(source, x), d(source, p) + l(eid)) == 0)
        s(source, x) += s(source, p);
    } /* for preds */

    igraph_vector_int_t* ss = igraph_inclist_get(succs, x);
    for(long int si = 0; si < igraph_vector_int_size(ss); si++) {
      igraph_integer_t eid = VECTOR(*ss)[si];
      igraph_integer_t s = IGRAPH_OTHER(G, eid, x);
      igraph_real_t d_s = d(source, x) + l(eid);
      if(VECTOR(is_affected)[s]) {
        if(!igraph_2wheap_has_elem(&queue, s))
          igraph_2wheap_push_with_index(&queue, s, -d_s);
        else if(cmp(d_s, -igraph_2wheap_get(&queue, s)) < 0)
          igraph_2wheap_modify(&queue, s, -d_s);
      }
    } /* for succs */
  } /* !igraph_2wheap_empty(&Q) */

  igraph_vector_int_destroy(&targets);
  igraph_vector_int_destroy(&is_affected);
  igraph_2wheap_destroy(&queue);
#undef EPS
#undef cmp
#undef d
#undef s
#undef l
}

void update_stsp_dec_weighted(igraph_t* G,
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

  igraph_vector_int_t sources, is_affected;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&is_affected, igraph_vcount(G));
  affected_sources_dec(G, preds, &sources, D, u, v, target, weights, weight);
  for(long int si = 0; si < igraph_vector_int_size(&sources); si++)
    VECTOR(is_affected)[igraph_vector_int_e(&sources, si)] = 1;

  igraph_2wheap_t queue;
  igraph_2wheap_init(&queue, igraph_vcount(G));
  for(long int xi = 0; xi < igraph_vector_int_size(&sources); xi++) {
    igraph_integer_t x = igraph_vector_int_e(&sources, xi);
    igraph_vector_int_t* ys = igraph_inclist_get(succs, x);
    igraph_real_t d_hat = IGRAPH_INFINITY;
    for(long int yi = 0; yi < igraph_vector_int_size(ys); yi++) {
      igraph_integer_t eid = igraph_vector_int_e(ys, yi);
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      if(!VECTOR(is_affected)[y] && cmp(l(eid) + d(y, target), d_hat) < 0)
        d_hat = l(eid) + d(y, target);
    }
    if(igraph_is_inf(d_hat)) {
      d(x, target) = IGRAPH_INFINITY;
      s(x, target) = 0;
    } else
      igraph_2wheap_push_with_index(&queue, x, -d_hat);
  }

  while(!igraph_2wheap_empty(&queue)) {
    long int x = igraph_2wheap_max_index(&queue);
    d(x, target) = -igraph_2wheap_delete_max(&queue);
    s(x, target) = 0;
    VECTOR(is_affected)[x] = 0;

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
      if(VECTOR(is_affected)[p]) {
        if(!igraph_2wheap_has_elem(&queue, p))
          igraph_2wheap_push_with_index(&queue, p, -d_p);
        else if(cmp(d_p, -igraph_2wheap_get(&queue, p)) < 0)
          igraph_2wheap_modify(&queue, p, -d_p);
      }
    } /* for preds */
  } /* !igraph_2wheap_empty(&Q) */

  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&is_affected);
  igraph_2wheap_destroy(&queue);
#undef EPS
#undef cmp
#undef d
#undef s
#undef l
}

void affected_targets_dec(igraph_t* G,
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
    for(igraph_integer_t ni = 0; ni < igraph_vector_int_size(neis); ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      igraph_real_t d_sy = d(source, y);
      igraph_real_t d_sy_p = d(source, x) + l(eid);
      if(cmp(d_sy, d_sy_p) == 0 && !igraph_vector_bool_e(&visited, y)) {
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
#undef l
}

void affected_sources_dec(igraph_t* G,
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
    for(igraph_integer_t ni = 0; ni < igraph_vector_int_size(neis); ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      igraph_real_t d_sy = d(y, target);
      igraph_real_t d_sy_p = l(eid) + d(x, target);
      if(cmp(d_sy, d_sy_p) == 0 && !igraph_vector_bool_e(&visited, y)) {
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
#undef l
}
