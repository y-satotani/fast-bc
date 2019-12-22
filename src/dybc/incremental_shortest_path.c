#include "incremental_shortest_path.h"

#include <assert.h>
#include <igraph/igraph.h>
#include <igraph/igraph_math.h>
#include <igraph/igraph_types_internal.h>

void update_sssp_inc_weighted(igraph_t* G,
                              igraph_inclist_t* preds,
                              igraph_inclist_t* succs,
                              igraph_matrix_t* D,
                              igraph_matrix_int_t* S,
                              igraph_integer_t u,
                              igraph_integer_t v,
                              igraph_integer_t source,
                              igraph_vector_t* weights,
                              igraph_real_t weight,
                              igraph_bool_t is_post_mod) {
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
  igraph_vector_t d_old;
  igraph_vector_int_t s_old;
  igraph_2wheap_init(&queue, igraph_vcount(G));
  igraph_vector_init(&d_old, igraph_vcount(G));
  igraph_vector_int_init(&s_old, igraph_vcount(G));

  VECTOR(d_old)[u] = d(source, u);
  VECTOR(s_old)[u] = s(source, u);
  VECTOR(d_old)[v] = d(source, v);
  VECTOR(s_old)[v] = s(source, v);
  if(cmp(d(source, u) + weight, d(source, v)) < 0) {
    d(source, v) = d(source, u) + weight;
    s(source, v) = s(source, u);
    igraph_2wheap_push_with_index(&queue, v, -d(source, v));
  } else if(cmp(d(source, u) + weight, d(source, v)) == 0) {
    s(source, v) += s(source, u);
    igraph_2wheap_push_with_index(&queue, v, -d(source, v));
  }

  while(!igraph_2wheap_empty(&queue)) {
    long int x = igraph_2wheap_max_index(&queue);
    igraph_2wheap_delete_max(&queue);

    igraph_vector_int_t* neis = igraph_inclist_get(succs, x);
    long int nneis = igraph_vector_int_size(neis);
    for(long int ni = 0; ni < nneis; ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      if(x == u && y == v) continue;

      if(VECTOR(d_old)[y] == 0.) {
        VECTOR(d_old)[y] = d(source, y);
        VECTOR(s_old)[y] = s(source, y);
      }
      if(cmp(d(source, x) + l(eid), d(source, y)) < 0) {
        d(source, y) = d(source, x) + l(eid);
        s(source, y) = s(source, x);
      } else if(cmp(d(source, x) + l(eid), d(source, y)) == 0) {
        if(cmp(VECTOR(d_old)[y], d(source, y)) == 0
           && cmp(VECTOR(d_old)[x] + l(eid), VECTOR(d_old)[y]) == 0) {
          s(source, y) += s(source, x) - VECTOR(s_old)[x];
        } else {
          s(source, y) += s(source, x);
        }
      } // end if(d_sx+l_xy == d_sy)
      if(cmp(d(source, x) + l(eid), d(source, y)) == 0) {
        if(!igraph_2wheap_has_elem(&queue, y))
          igraph_2wheap_push_with_index(&queue, y, -d(source, y));
        else
          igraph_2wheap_modify(&queue, y, -d(source, y));
      }
    } // end for all y in neighbors
  } // end while queue is not empty

  igraph_2wheap_destroy(&queue);
  igraph_vector_destroy(&d_old);
  igraph_vector_int_destroy(&s_old);

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
                              igraph_real_t weight,
                              igraph_bool_t is_post_mod) {
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
  igraph_vector_t d_old;
  igraph_vector_int_t s_old;
  igraph_2wheap_init(&queue, igraph_vcount(G));
  igraph_vector_init(&d_old, igraph_vcount(G));
  igraph_vector_int_init(&s_old, igraph_vcount(G));

  VECTOR(d_old)[u] = d(u, target);
  VECTOR(s_old)[u] = s(u, target);
  VECTOR(d_old)[v] = d(v, target);
  VECTOR(s_old)[v] = s(v, target);
  if(cmp(weight + d(v, target), d(u, target)) < 0) {
    d(u, target) = weight + d(v, target);
    s(u, target) = s(v, target);
    igraph_2wheap_push_with_index(&queue, u, -d(u, target));
  } else if(cmp(weight + d(v, target), d(u, target)) == 0) {
    s(u, target) += s(v, target);
    igraph_2wheap_push_with_index(&queue, u, -d(u, target));
  }

  while(!igraph_2wheap_empty(&queue)) {
    long int x = igraph_2wheap_max_index(&queue);
    igraph_2wheap_delete_max(&queue);

    igraph_vector_int_t* neis = igraph_inclist_get(preds, x);
    long int nneis = igraph_vector_int_size(neis);
    for(long int ni = 0; ni < nneis; ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      if(x == v && y == u) continue;

      if(VECTOR(d_old)[y] == 0.) {
        VECTOR(d_old)[y] = d(y, target);
        VECTOR(s_old)[y] = s(y, target);
      }
      if(cmp(l(eid) + d(x, target), d(y, target)) < 0) {
        d(y, target) = l(eid) + d(x, target);
        s(y, target) = s(x, target);
      } else if(cmp(l(eid) + d(x, target), d(y, target)) == 0) {
        if(cmp(VECTOR(d_old)[y], d(y, target)) == 0
           && cmp(l(eid) + VECTOR(d_old)[x], VECTOR(d_old)[y]) == 0) {
          s(y, target) += s(x, target) - VECTOR(s_old)[x];
        } else {
          s(y, target) += s(x, target);
        }
      } // end if(d_sx+l_xy == d_sy)
      if(cmp(l(eid) + d(x, target), d(y, target)) == 0) {
        if(!igraph_2wheap_has_elem(&queue, y))
          igraph_2wheap_push_with_index(&queue, y, -d(y, target));
        else
          igraph_2wheap_modify(&queue, y, -d(y, target));
      }
    } // end for all y in neighbors
  } // end while queue is not empty

  igraph_2wheap_destroy(&queue);
  igraph_vector_destroy(&d_old);
  igraph_vector_int_destroy(&s_old);

#undef EPS
#undef cmp
#undef d
#undef s
}

void update_sssp_inc_unweighted(igraph_t* G,
                                igraph_inclist_t* preds,
                                igraph_inclist_t* succs,
                                igraph_matrix_t* D,
                                igraph_matrix_int_t* S,
                                igraph_integer_t u,
                                igraph_integer_t v,
                                igraph_integer_t source,
                                igraph_bool_t is_post_mod) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define s(a, b) (MATRIX(*S, (a), (b)))

  if(igraph_is_inf(d(source, u))
     || cmp(d(source, v), d(source, u) + 1.0) < 0) {
    return;
  }

  igraph_dqueue_t queue;
  igraph_vector_t d_old;
  igraph_vector_int_t s_old;
  igraph_vector_bool_t pushed;
  igraph_dqueue_init(&queue, igraph_vcount(G));
  igraph_vector_init(&d_old, igraph_vcount(G));
  igraph_vector_int_init(&s_old, igraph_vcount(G));
  igraph_vector_bool_init(&pushed, igraph_vcount(G));
  VECTOR(d_old)[u] = d(source, u);
  VECTOR(s_old)[u] = s(source, u);
  // not visited
  VECTOR(d_old)[v] = d(source, v);
  VECTOR(s_old)[v] = s(source, v);
  if(cmp(d(source, u) + 1.0, d(source, v)) < 0) {
    d(source, v) = d(source, u) + 1.0;
    s(source, v) = s(source, u);
    igraph_dqueue_push(&queue, v);
    VECTOR(pushed)[v] = 1;
  } else if(cmp(d(source, u) + 1.0, d(source, v)) == 0) {
    s(source, v) += s(source, u);
    igraph_dqueue_push(&queue, v);
    VECTOR(pushed)[v] = 1;
  }

  while(!igraph_dqueue_empty(&queue)) {
    long int x = igraph_dqueue_pop(&queue);

    igraph_vector_int_t* neis = igraph_inclist_get(succs, x);
    long int nneis = igraph_vector_int_size(neis);
    for(long int ni = 0; ni < nneis; ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);

      if(VECTOR(d_old)[y] == 0.) {
        VECTOR(d_old)[y] = d(source, y);
        VECTOR(s_old)[y] = s(source, y);
      }
      if(cmp(d(source, x) + 1.0, d(source, y)) < 0) {
        d(source, y) = d(source, x) + 1.0;
        s(source, y) = s(source, x);
        if(!VECTOR(pushed)[y]) {
          igraph_dqueue_push(&queue, y);
          VECTOR(pushed)[y] = 1;
        }
      } else if(cmp(d(source, x) + 1.0, d(source, y)) == 0) {
        if(cmp(VECTOR(d_old)[y], d(source, y)) == 0
           && cmp(VECTOR(d_old)[x] + 1.0, VECTOR(d_old)[y]) == 0) {
          s(source, y) += s(source, x) - VECTOR(s_old)[x];
        } else {
          s(source, y) += s(source, x);
        }
        if(!VECTOR(pushed)[y]) {
          igraph_dqueue_push(&queue, y);
          VECTOR(pushed)[y] = 1;
        }
      } // end if(d_sx+l_xy == d_sy)
    } // end for all y in neighbors
  } // end while queue is not empty

  igraph_dqueue_destroy(&queue);
  igraph_vector_destroy(&d_old);
  igraph_vector_int_destroy(&s_old);
  igraph_vector_bool_destroy(&pushed);

#undef EPS
#undef cmp
#undef d
#undef s
}

void update_stsp_inc_unweighted(igraph_t* G,
                                igraph_inclist_t* preds,
                                igraph_inclist_t* succs,
                                igraph_matrix_t* D,
                                igraph_matrix_int_t* S,
                                igraph_integer_t u,
                                igraph_integer_t v,
                                igraph_integer_t target,
                                igraph_bool_t is_post_mod) {
#define EPS IGRAPH_SHORTEST_PATH_EPSILON
#define cmp(a, b) (igraph_cmp_epsilon((a), (b), EPS))
#define d(a, b) (MATRIX(*D, (a), (b)))
#define s(a, b) (MATRIX(*S, (a), (b)))

  if(igraph_is_inf(d(v, target))
     || cmp(d(v, target), 1.0 + d(u, target)) < 0) {
    return;
  }

  igraph_dqueue_t queue;
  igraph_vector_t d_old;
  igraph_vector_int_t s_old;
  igraph_vector_bool_t pushed;
  igraph_dqueue_init(&queue, igraph_vcount(G));
  igraph_vector_init(&d_old, igraph_vcount(G));
  igraph_vector_int_init(&s_old, igraph_vcount(G));
  igraph_vector_bool_init(&pushed, igraph_vcount(G));
  VECTOR(d_old)[u] = d(u, target);
  VECTOR(s_old)[u] = s(u, target);
  // not visited
  VECTOR(d_old)[v] = d(v, target);
  VECTOR(s_old)[v] = s(v, target);
  if(cmp(1.0 + d(u, target), d(v, target)) < 0) {
    d(v, target) = 1.0 + d(u, target);
    s(v, target) = s(u, target);
    igraph_dqueue_push(&queue, v);
    VECTOR(pushed)[v] = 1;
  } else if(cmp(1.0 + d(u, target), d(v, target)) == 0) {
    s(v, target) += s(u, target);
    igraph_dqueue_push(&queue, v);
    VECTOR(pushed)[v] = 1;
  }

  while(!igraph_dqueue_empty(&queue)) {
    long int x = igraph_dqueue_pop(&queue);

    igraph_vector_int_t* neis = igraph_inclist_get(succs, x);
    long int nneis = igraph_vector_int_size(neis);
    for(long int ni = 0; ni < nneis; ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);

      if(VECTOR(d_old)[y] == 0.) {
        VECTOR(d_old)[y] = d(y, target);
        VECTOR(s_old)[y] = s(y, target);
      }
      if(cmp(1.0 + d(x, target), d(y, target)) < 0) {
        d(y, target) = 1.0 + d(x, target);
        s(y, target) = s(x, target);
        if(!VECTOR(pushed)[y]) {
          igraph_dqueue_push(&queue, y);
          VECTOR(pushed)[y] = 1;
        }
      } else if(cmp(1.0 + d(x, target), d(y, target)) == 0) {
        if(cmp(VECTOR(d_old)[y], d(y, target)) == 0
           && cmp(1.0 + VECTOR(d_old)[x], VECTOR(d_old)[y]) == 0) {
          s(y, target) += s(x, target) - VECTOR(s_old)[x];
        } else {
          s(y, target) += s(x, target);
        }
        if(!VECTOR(pushed)[y]) {
          igraph_dqueue_push(&queue, y);
          VECTOR(pushed)[y] = 1;
        }
      } // end if(d_sx+l_xy == d_sy)
    } // end for all y in neighbors
  } // end while queue is not empty

  igraph_dqueue_destroy(&queue);
  igraph_vector_destroy(&d_old);
  igraph_vector_int_destroy(&s_old);
  igraph_vector_bool_destroy(&pushed);

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
                          igraph_real_t weight,
                          igraph_bool_t is_post_mod) {
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
  igraph_dqueue_int_t queue;
  igraph_vector_bool_init(&visited, igraph_vcount(G));
  igraph_dqueue_int_init(&queue, 0);
  igraph_vector_bool_set(&visited, v, 1);
  igraph_dqueue_int_push(&queue, v);

  igraph_bool_t use_post_mod = 0;
  if(is_post_mod) {
    igraph_integer_t eid;
    igraph_get_eid(G, &eid, u, v, 1, 0);
    if(eid < 0) use_post_mod = 1;                        // an edge is added
    else if(weight && weight < l(eid)) use_post_mod = 1; // weight decreases
  }
  if(use_post_mod) {
    igraph_vector_bool_set(&visited, v, 1);
    igraph_dqueue_int_push(&queue, v);
  }

  while(!igraph_dqueue_int_empty(&queue)) {
    igraph_integer_t x = igraph_dqueue_int_pop(&queue);
    igraph_vector_int_push_back(out, x);
    igraph_vector_int_t* neis = igraph_inclist_get(inclist, x);
    igraph_integer_t ni;
    for(ni = 0; ni < igraph_vector_int_size(neis); ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      if(use_post_mod && x == u && y == v) continue; // this edge is traversed
      igraph_real_t d_sy = d(source, y);
      igraph_real_t d_sy_p = d(source, u) + weight + d(v, x) + l(eid);
      if(cmp(d_sy, d_sy_p) >= 0 && !igraph_vector_bool_e(&visited, y)) {
        igraph_vector_bool_set(&visited, y, 1);
        igraph_dqueue_int_push(&queue, y);
      }
    }
  }

  igraph_vector_bool_destroy(&visited);
  igraph_dqueue_int_destroy(&queue);

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
                          igraph_real_t weight,
                          igraph_bool_t is_post_mod) {
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
  igraph_dqueue_int_t queue;
  igraph_vector_bool_init(&visited, igraph_vcount(G));
  igraph_dqueue_int_init(&queue, 0);
  igraph_vector_bool_set(&visited, u, 1);
  igraph_dqueue_int_push(&queue, u);

  igraph_bool_t use_post_mod = 0;
  if(is_post_mod) {
    igraph_integer_t eid;
    igraph_get_eid(G, &eid, u, v, 1, 0);
    if(eid < 0) use_post_mod = 1;                        // an edge is added
    else if(weight && weight < l(eid)) use_post_mod = 1; // weight decreases
  }
  if(use_post_mod) {
    igraph_vector_bool_set(&visited, v, 1);
    igraph_dqueue_int_push(&queue, v);
  }

  while(!igraph_dqueue_int_empty(&queue)) {
    igraph_integer_t x = igraph_dqueue_int_pop(&queue);
    igraph_vector_int_push_back(out, x);
    igraph_vector_int_t* neis = igraph_inclist_get(inclist, x);
    igraph_integer_t ni;
    for(ni = 0; ni < igraph_vector_int_size(neis); ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      if(use_post_mod && x == u && y == v) continue; // this edge is traversed
      igraph_real_t d_sy = d(y, target);
      igraph_real_t d_sy_p = l(eid) + d(x, u) + weight + d(v, target);
      if(cmp(d_sy, d_sy_p) >= 0 && !igraph_vector_bool_e(&visited, y)) {
        igraph_vector_bool_set(&visited, y, 1);
        igraph_dqueue_int_push(&queue, y);
      }
    }
  }

  igraph_vector_bool_destroy(&visited);
  igraph_dqueue_int_destroy(&queue);

#undef EPS
#undef cmp
#undef d
}

void count_affected_vertices_path_inc(igraph_t* G,
                                      igraph_inclist_t* preds,
                                      igraph_inclist_t* succs,
                                      igraph_matrix_t* D,
                                      igraph_matrix_int_t* S,
                                      igraph_vector_t* B,
                                      igraph_integer_t u,
                                      igraph_integer_t v,
                                      igraph_vector_t* weights,
                                      igraph_real_t weight,
                                      dybc_update_stats_t* upd_stats,
                                      igraph_bool_t is_post_mod) {
  if(!upd_stats) return;

  long int n_sources, n_targets;
  long int n_affected_targets, n_affected_sources;
  igraph_vector_int_t sources, targets;
  n_affected_sources = n_affected_targets = 0;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);

  // count targets
  affected_sources_inc
    (G, preds, &sources, D, u, v, v, weights, weight, is_post_mod);
  n_sources = igraph_vector_int_size(&sources);
  for(long int si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = VECTOR(sources)[si];
    if(!igraph_is_directed(G))
      affected_sources_inc
        (G, preds, &targets, D, v, u, s, weights, weight, is_post_mod);
    else
      affected_targets_inc
        (G, succs, &targets, D, u, v, s, weights, weight, is_post_mod);
    n_affected_targets += igraph_vector_int_size(&targets);
  }

  // count sources
  if(!igraph_is_directed(G))
    affected_sources_inc
      (G, preds, &targets, D, v, u, u, weights, weight, is_post_mod);
  else
    affected_targets_inc
      (G, succs, &targets, D, u, v, u, weights, weight, is_post_mod);
  n_targets = igraph_vector_int_size(&targets);
  for(long int ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = VECTOR(targets)[ti];
    affected_sources_inc
      (G, preds, &sources, D, u, v, t, weights, weight, is_post_mod);
    n_affected_sources += igraph_vector_int_size(&sources);
  }

  // calculate the average of the number of affected vertices
  if(n_sources + n_targets > 0)
    upd_stats->upd_path
      = (double)(n_affected_sources + n_affected_targets)
      / (n_targets + n_sources);
  else
    upd_stats->upd_path = 0.0;
  upd_stats->n_aff_src = n_sources;
  upd_stats->n_aff_tgt = n_targets;

  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
}
