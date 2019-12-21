
#include "dybc_exp_common.h"

#include <stdio.h>
#include <time.h>
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
                                      dybc_update_stats_t* upd_stats) {
  if(!upd_stats) return;

  long int n_sources, n_targets;
  long int n_affected_targets, n_affected_sources;
  igraph_vector_int_t sources, targets;
  n_affected_sources = n_affected_targets = 0;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);

  // count targets
  affected_sources_inc(G, preds, &sources, D, u, v, v, weights, weight);
  n_sources = igraph_vector_int_size(&sources);
  for(long int si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = VECTOR(sources)[si];
    if(!igraph_is_directed(G))
      affected_sources_inc(G, preds, &targets, D, v, u, s, weights, weight);
    else
      affected_targets_inc(G, succs, &targets, D, u, v, s, weights, weight);
    n_affected_targets += igraph_vector_int_size(&targets);
  }

  // count sources
  if(!igraph_is_directed(G))
    affected_sources_inc(G, preds, &targets, D, v, u, u, weights, weight);
  else
    affected_targets_inc(G, succs, &targets, D, u, v, u, weights, weight);
  n_targets = igraph_vector_int_size(&targets);
  for(long int ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = VECTOR(targets)[ti];
    affected_sources_inc(G, preds, &sources, D, u, v, t, weights, weight);
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

void count_affected_vertices_betw(igraph_t* G,
                                  igraph_vector_int_t* aff_deps_before,
                                  igraph_vector_int_t* aff_deps_after,
                                  igraph_vector_int_t* sources,
                                  dybc_update_stats_t* upd_stats) {
  if(!upd_stats) return;

  igraph_integer_t n_sources = igraph_vector_int_size(sources);
  if(n_sources > 0)
    upd_stats->upd_betw
      = (double)(igraph_vector_int_size(aff_deps_before)
                 + igraph_vector_int_size(aff_deps_after))
      / (2 * n_sources);
  else
    upd_stats->upd_betw = 0;

  upd_stats->n_tau_hat = 0;
  igraph_vector_bool_t is_affected;
  igraph_vector_bool_init(&is_affected, igraph_vcount(G));
  for(long int i = 0; i < igraph_vector_int_size(aff_deps_before); i++)
    VECTOR(is_affected)[VECTOR(*aff_deps_before)[i]] = 1;
  for(long int i = 0; i < igraph_vector_int_size(aff_deps_after); i++)
    VECTOR(is_affected)[VECTOR(*aff_deps_after)[i]] = 1;
  for(long int i = 0; i < igraph_vector_bool_size(&is_affected); i++)
    upd_stats->n_tau_hat += VECTOR(is_affected)[i];
  igraph_vector_bool_destroy(&is_affected);
}

void incremental_update(igraph_t* G,
                        igraph_matrix_t* D,
                        igraph_matrix_int_t *S,
                        igraph_vector_t* B,
                        igraph_integer_t u,
                        igraph_integer_t v,
                        igraph_vector_t* weights,
                        igraph_real_t weight,
                        dybc_update_stats_t* upd_stats) {
  // initialize the edgelists of the graph
  clock_t start, end;
  igraph_inclist_t succs, preds;
  start = clock();
  if(igraph_is_directed(G)) {
    igraph_inclist_init(G, &succs, IGRAPH_OUT);
    igraph_inclist_init(G, &preds, IGRAPH_IN);
  } else {
    igraph_inclist_init(G, &succs, IGRAPH_ALL);
    preds = succs;
  }
  end = clock();
  if(upd_stats) {
    upd_stats->time_full = (double)(end - start) / CLOCKS_PER_SEC;
    upd_stats->time_path = 0.0;
    upd_stats->time_betw = 0.0;
  }

  count_affected_vertices_path_inc
    (G, &preds, &succs, D, S, B, u, v, weights, weight, upd_stats);

  // initialize vectors store affected vertices
  igraph_vector_int_t targets_;
  igraph_vector_int_t sources, targets;
  igraph_vector_int_t aff_deps_before, aff_deps_after;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);
  igraph_vector_int_init(&targets_, 0);
  igraph_vector_int_init(&aff_deps_before, 0);
  igraph_vector_int_init(&aff_deps_after, 0);

  // find affected sources/targets
  start = clock();
  if(!igraph_is_directed(G))
    affected_sources_inc(G, &preds, &targets_, D, v, u, u, weights, weight);
  else
    affected_targets_inc(G, &succs, &targets_, D, u, v, u, weights, weight);
  affected_sources_inc(G, &preds, &sources, D, u, v, v, weights, weight);
  end = clock();
  if(upd_stats)
    upd_stats->time_full += (double)(end - start) / CLOCKS_PER_SEC;

  // for each vertex in affected sources
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    // decrease betweenness
    start = clock();
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_inc(G, &succs, &targets, D, u, v, s, weights, weight);
    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    if(weights)
      update_deps_weighted_statistics(G, &preds, D, S, B, u, v,
         s, &targets, weights, weight, -factor, &aff_deps_before);
    else
      update_deps_unweighted_statistics(G, &preds, D, S, B, u, v,
         s, &targets, -factor, &aff_deps_before);
    end = clock();
    if(upd_stats) {
      double time_betw = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_betw;
      upd_stats->time_betw += time_betw;
    }

    // add edge
    igraph_add_edge(G, u, v);
    if(weights)
      igraph_vector_push_back(weights, weight);
    igraph_integer_t eid = igraph_ecount(G) - 1;
    igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
    igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);

    // update sssp
    start = clock();
    if(weights)
      update_sssp_inc_weighted
        (G, &preds, &succs, D, S, u, v, s, weights, weight);
    else
      update_sssp_inc_unweighted
        (G, &preds, &succs, D, S, u, v, s);
    end = clock();
    if(upd_stats) {
      double time_path = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_path;
      upd_stats->time_path += time_path;
    }

    // increase betweenness
    // factor is 2 for undirected and 1 for directed
    start = clock();
    if(weights)
      update_deps_weighted_statistics(G, &preds, D, S, B, u, v,
         s, &targets, weights, weight, factor, &aff_deps_after);
    else
      update_deps_unweighted_statistics(G, &preds, D, S, B, u, v,
         s, &targets, factor, &aff_deps_after);
    end = clock();
    if(upd_stats) {
      double time_betw = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_betw;
      upd_stats->time_betw += time_betw;
    }

    // cleanup
    igraph_delete_edges(G, igraph_ess_1(eid));
    if(weights)
      igraph_vector_remove(weights, eid);
    igraph_vector_int_pop_back(igraph_inclist_get(&succs, u));
    igraph_vector_int_pop_back(igraph_inclist_get(&preds, v));
  } // for s in sources

  // add edge
  igraph_integer_t eid = igraph_ecount(G);
  igraph_add_edge(G, u, v);
  if(weights)
    igraph_vector_push_back(weights, weight);
  igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
  igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);

  // update single target shortest path for each target
  start = clock();
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
  end = clock();
  if(upd_stats) {
    double time_path = (double)(end - start) / CLOCKS_PER_SEC;
    upd_stats->time_full += time_path;
    upd_stats->time_path += time_path;
  }

  count_affected_vertices_betw
    (G, &aff_deps_before, &aff_deps_after, &sources, upd_stats);

  // cleanup
  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
  igraph_vector_int_destroy(&targets_);
  igraph_vector_int_destroy(&aff_deps_before);
  igraph_vector_int_destroy(&aff_deps_after);
  igraph_inclist_destroy(&succs);
  if(igraph_is_directed(G))
    igraph_inclist_destroy(&preds);
}


void count_affected_vertices_path_dec(igraph_t* G,
                                      igraph_inclist_t* preds,
                                      igraph_inclist_t* succs,
                                      igraph_matrix_t* D,
                                      igraph_matrix_int_t* S,
                                      igraph_vector_t* B,
                                      igraph_integer_t u,
                                      igraph_integer_t v,
                                      igraph_vector_t* weights,
                                      igraph_real_t weight,
                                      dybc_update_stats_t* upd_stats) {
  if(!upd_stats) return;

  long int n_sources, n_targets;
  long int n_affected_targets, n_affected_sources;
  igraph_vector_int_t sources, targets;
  n_affected_sources = n_affected_targets = 0;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);

  // count targets
  affected_sources_dec(G, preds, &sources, D, u, v, v, weights, weight);
  n_sources = igraph_vector_int_size(&sources);
  for(long int si = 0; si < igraph_vector_int_size(&sources); si++) {
    igraph_integer_t s = VECTOR(sources)[si];
    if(!igraph_is_directed(G))
      affected_sources_dec(G, preds, &targets, D, v, u, s, weights, weight);
    else
      affected_targets_dec(G, succs, &targets, D, u, v, s, weights, weight);
    n_affected_targets += igraph_vector_int_size(&targets);
  }

  // count sources
  if(!igraph_is_directed(G))
    affected_sources_dec(G, preds, &targets, D, v, u, u, weights, weight);
  else
    affected_targets_dec(G, succs, &targets, D, u, v, u, weights, weight);
  n_targets = igraph_vector_int_size(&targets);
  for(long int ti = 0; ti < igraph_vector_int_size(&targets); ti++) {
    igraph_integer_t t = VECTOR(targets)[ti];
    affected_sources_dec(G, preds, &sources, D, u, v, t, weights, weight);
    n_affected_sources += igraph_vector_int_size(&sources);
  }

  // calculate the mean of the number of affected vertices
  if(n_sources + n_targets > 0)
    upd_stats->upd_path
      = (double)(n_affected_sources + n_affected_targets)
      / (n_targets + n_sources);
  else
    upd_stats->upd_path = 0;
  upd_stats->n_aff_src = n_sources;
  upd_stats->n_aff_tgt = n_targets;

  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
}

void decremental_update(igraph_t* G,
                        igraph_matrix_t* D,
                        igraph_matrix_int_t *S,
                        igraph_vector_t* B,
                        igraph_integer_t u,
                        igraph_integer_t v,
                        igraph_vector_t* weights,
                        igraph_real_t weight,
                        dybc_update_stats_t* upd_stats) {

  // move the edge to be deleted to back of list
  igraph_integer_t eid;
  igraph_get_eid(G, &eid, u, v, 0, 1);
  igraph_delete_edges(G, igraph_ess_1(eid));
  if(weights) {
    weight = igraph_vector_e(weights, eid);
    igraph_vector_remove(weights, eid);
  }
  clock_t start, end;
  igraph_inclist_t succs, preds;
  start = clock();
  if(igraph_is_directed(G)) {
    igraph_inclist_init(G, &succs, IGRAPH_OUT);
    igraph_inclist_init(G, &preds, IGRAPH_IN);
  } else {
    igraph_inclist_init(G, &succs, IGRAPH_ALL);
    preds = succs;
  }
  end = clock();
  if(upd_stats) {
    upd_stats->time_full = (double)(end - start) / CLOCKS_PER_SEC;
    upd_stats->time_path = 0.0;
    upd_stats->time_betw = 0.0;
  }

  // then push the deleted edge
  eid = igraph_ecount(G);
  igraph_add_edge(G, u, v);
  if(weights)
    igraph_vector_push_back(weights, weight);
  igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
  igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);

  count_affected_vertices_path_dec
    (G, &preds, &succs, D, S, B, u, v, weights, weight, upd_stats);

  // initialize vectors for storing affected vertices
  igraph_vector_int_t targets_;
  igraph_vector_int_t sources, targets;
  igraph_vector_int_t aff_deps_before, aff_deps_after;
  igraph_vector_int_init(&sources, 0);
  igraph_vector_int_init(&targets, 0);
  igraph_vector_int_init(&targets_, 0);
  igraph_vector_int_init(&aff_deps_before, 0);
  igraph_vector_int_init(&aff_deps_after, 0);

  // find affected source/targets
  start = clock();
  if(!igraph_is_directed(G))
    affected_sources_dec(G, &preds, &targets_, D, v, u, u, weights, weight);
  else
    affected_targets_dec(G, &succs, &targets_, D, u, v, u, weights, weight);
  affected_sources_dec(G, &preds, &sources, D, u, v, v, weights, weight);
  end = clock();
  if(upd_stats)
    upd_stats->time_full += (double)(end - start) / CLOCKS_PER_SEC;

  // for each vertex in affected sources
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    // decrease betweenness
    start = clock();
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_dec(G, &succs, &targets, D, u, v, s, weights, weight);
    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    if(weights)
      update_deps_weighted_statistics(G, &preds, D, S, B, u, v,
         s, &targets, weights, weight, -factor, &aff_deps_before);
    else
      update_deps_unweighted_statistics(G, &preds, D, S, B, u, v,
         s, &targets, -factor, &aff_deps_before);
    end = clock();
    if(upd_stats) {
      double time_betw = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_betw;
      upd_stats->time_betw += time_betw;
    }

    // remove the edge
    igraph_delete_edges(G, igraph_ess_1(eid));
    if(weights)
      igraph_vector_pop_back(weights);
    igraph_vector_int_pop_back(igraph_inclist_get(&succs, u));
    igraph_vector_int_pop_back(igraph_inclist_get(&preds, v));

    // update sssp
    start = clock();
    if(weights)
      update_sssp_dec_weighted
        (G, &preds, &succs, D, S, u, v, s, weights, weight);
    else
      update_sssp_dec_unweighted
        (G, &preds, &succs, D, S, u, v, s);
    end = clock();
    if(upd_stats) {
      double time_path = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_path;
      upd_stats->time_path += time_path;
    }

    // increase betweenness
    // factor is 2 for undirected and 1 for directed
    start = clock();
    if(weights)
      update_deps_weighted_statistics(G, &preds, D, S, B, u, v,
         s, &targets, weights, weight, factor, &aff_deps_after);
    else
      update_deps_unweighted_statistics(G, &preds, D, S, B, u, v,
         s, &targets, factor, &aff_deps_after);
    end = clock();
    if(upd_stats) {
      double time_betw = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_betw;
      upd_stats->time_betw += time_betw;
    }

    // cleanup for next round
    igraph_add_edge(G, u, v);
    if(weights)
      igraph_vector_push_back(weights, weight);
    igraph_vector_int_push_back(igraph_inclist_get(&succs, u), eid);
    igraph_vector_int_push_back(igraph_inclist_get(&preds, v), eid);
  } // for s in affected sources

  // delete the edge
  igraph_delete_edges(G, igraph_ess_1(eid));
  if(weights)
    igraph_vector_pop_back(weights);
  igraph_vector_int_pop_back(igraph_inclist_get(&succs, u));
  igraph_vector_int_pop_back(igraph_inclist_get(&preds, v));

  // update stsp for each target
  start = clock();
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
  end = clock();
  if(upd_stats) {
    double time_path = (double)(end - start) / CLOCKS_PER_SEC;
    upd_stats->time_full += time_path;
    upd_stats->time_path += time_path;
  }

  count_affected_vertices_betw
    (G, &aff_deps_before, &aff_deps_after, &sources, upd_stats);

  // cleanup
  igraph_vector_int_destroy(&sources);
  igraph_vector_int_destroy(&targets);
  igraph_vector_int_destroy(&targets_);
  igraph_vector_int_destroy(&aff_deps_before);
  igraph_vector_int_destroy(&aff_deps_after);
  igraph_inclist_destroy(&succs);
  if(igraph_is_directed(G))
    igraph_inclist_destroy(&preds);
}
