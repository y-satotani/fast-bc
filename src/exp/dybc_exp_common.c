
#include "dybc_exp_common.h"

#include <stdio.h>
#include <igraph/igraph.h>
#include <igraph/igraph_math.h>

#include "dybc/static_betweenness.h"
#include "dybc/dynamic_betweenness.h"
#include "dybc/incremental_betweenness.h"
#include "dybc/decremental_betweenness.h"

int choose_random_edge_for(igraph_t* G,
                           dybc_update_query_t query,
                           igraph_integer_t *u,
                           igraph_integer_t *v,
                           unsigned long int seed) {
  igraph_rng_seed(igraph_rng_default(), seed);
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
