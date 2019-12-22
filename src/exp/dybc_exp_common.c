
#include "dybc_exp_common.h"

#include <stdio.h>
#include <time.h>
#include <igraph/igraph.h>
#include <igraph/igraph_math.h>

#include "dybc/static_betweenness.h"
#include "dybc/dynamic_betweenness.h"
#include "dybc/incremental_shortest_path.h"
#include "dybc/incremental_betweenness.h"
#include "dybc/decremental_shortest_path.h"
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
    (G, &preds, &succs, D, S, B, u, v, weights, weight, upd_stats, 0);

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
    affected_sources_inc
      (G, &preds, &targets_, D, v, u, u, weights, weight, 0);
  else
    affected_targets_inc
      (G, &succs, &targets_, D, u, v, u, weights, weight, 0);
  affected_sources_inc
    (G, &preds, &sources, D, u, v, v, weights, weight, 0);
  end = clock();
  if(upd_stats)
    upd_stats->time_full += (double)(end - start) / CLOCKS_PER_SEC;

  // for each vertex in affected sources
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    // decrease betweenness
    start = clock();
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_inc
      (G, &succs, &targets, D, u, v, s, weights, weight, 0);
    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    if(weights)
      update_betw_inc_weighted_statistics
        (G, &preds, D, S, B, u, v,
         s, &targets, weights, weight, -factor, 0, &aff_deps_before);
    else
      update_betw_inc_unweighted_statistics
        (G, &preds, D, S, B, u, v,
         s, &targets, -factor, 0, &aff_deps_before);
    end = clock();
    if(upd_stats) {
      double time_betw = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_betw;
      upd_stats->time_betw += time_betw;
    }

    // update sssp
    start = clock();
    if(weights)
      update_sssp_inc_weighted
        (G, &preds, &succs, D, S, u, v, s, weights, weight, 0);
    else
      update_sssp_inc_unweighted
        (G, &preds, &succs, D, S, u, v, s, 0);
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
      update_betw_inc_weighted_statistics
        (G, &preds, D, S, B, u, v,
         s, &targets, weights, weight, factor, 1, &aff_deps_after);
    else
      update_betw_inc_unweighted_statistics
        (G, &preds, D, S, B, u, v,
         s, &targets, factor, 1, &aff_deps_after);
    end = clock();
    if(upd_stats) {
      double time_betw = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_betw;
      upd_stats->time_betw += time_betw;
    }

  } // for s in sources

  // update single target shortest path for each target
  start = clock();
  for(int ti = 0; ti < igraph_vector_int_size(&targets_); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets_, ti);
    if(!igraph_is_directed(G) && weights)
      update_sssp_inc_weighted
        (G, &succs, &preds, D, S, v, u, t, weights, weight, 0);
    else if(igraph_is_directed(G) && weights)
      update_stsp_inc_weighted
        (G, &preds, &succs, D, S, u, v, t, weights, IGRAPH_INFINITY, 0);
    else if(!igraph_is_directed(G) && !weights)
      update_sssp_inc_unweighted
        (G, &succs, &preds, D, S, v, u, t, 0);
    else if(igraph_is_directed(G) && !weights)
      update_stsp_inc_unweighted
        (G, &preds, &succs, D, S, u, v, t, 0);
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

void decremental_update(igraph_t* G,
                        igraph_matrix_t* D,
                        igraph_matrix_int_t *S,
                        igraph_vector_t* B,
                        igraph_integer_t u,
                        igraph_integer_t v,
                        igraph_vector_t* weights,
                        igraph_real_t weight,
                        dybc_update_stats_t* upd_stats) {

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

  count_affected_vertices_path_dec
    (G, &preds, &succs, D, S, B, u, v, weights, weight, upd_stats, 0);

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
    affected_sources_dec(G, &preds, &targets_, D, v, u, u, weights, weight, 0);
  else
    affected_targets_dec(G, &succs, &targets_, D, u, v, u, weights, weight, 0);
  affected_sources_dec(G, &preds, &sources, D, u, v, v, weights, weight, 0);
  end = clock();
  if(upd_stats)
    upd_stats->time_full += (double)(end - start) / CLOCKS_PER_SEC;

  // for each vertex in affected sources
  for(igraph_integer_t si = 0; si < igraph_vector_int_size(&sources); si++) {
    // decrease betweenness
    start = clock();
    igraph_integer_t s = igraph_vector_int_e(&sources, si);
    affected_targets_dec(G, &succs, &targets, D, u, v, s, weights, weight, 0);
    // factor is -2 for undirected and -1 for directed
    igraph_real_t factor = igraph_is_directed(G) ? 1 : 2;
    if(weights)
      update_betw_dec_weighted_statistics
        (G, &preds, D, S, B, u, v,
         s, &targets, weights, weight, -factor, 0, &aff_deps_before);
    else
      update_betw_dec_unweighted_statistics
        (G, &preds, D, S, B, u, v,
         s, &targets, -factor, 0, &aff_deps_before);
    end = clock();
    if(upd_stats) {
      double time_betw = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_betw;
      upd_stats->time_betw += time_betw;
    }

    // update sssp
    start = clock();
    if(weights)
      update_sssp_dec_weighted
        (G, &preds, &succs, D, S, u, v, s, weights, weight, 0);
    else
      update_sssp_dec_unweighted
        (G, &preds, &succs, D, S, u, v, s, 0);
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
      update_betw_dec_weighted_statistics
        (G, &preds, D, S, B, u, v,
         s, &targets, weights, weight, factor, 1, &aff_deps_after);
    else
      update_betw_dec_unweighted_statistics
        (G, &preds, D, S, B, u, v,
         s, &targets, factor, 1, &aff_deps_after);
    end = clock();
    if(upd_stats) {
      double time_betw = (double)(end - start) / CLOCKS_PER_SEC;
      upd_stats->time_full += time_betw;
      upd_stats->time_betw += time_betw;
    }

  } // for s in affected sources

  // update stsp for each target
  start = clock();
  for(int ti = 0; ti < igraph_vector_int_size(&targets_); ti++) {
    igraph_integer_t t = igraph_vector_int_e(&targets_, ti);
    if(!igraph_is_directed(G) && weights)
      update_sssp_dec_weighted
        (G, &succs, &preds, D, S, v, u, t, weights, weight, 0);
    else if(igraph_is_directed(G) && weights)
      update_stsp_dec_weighted
        (G, &preds, &succs, D, S, u, v, t, weights, IGRAPH_INFINITY, 0);
    else if(!igraph_is_directed(G) && !weights)
      update_sssp_dec_unweighted
        (G, &succs, &preds, D, S, v, u, t, 0);
    else if(igraph_is_directed(G) && !weights)
      update_stsp_dec_unweighted
        (G, &preds, &succs, D, S, u, v, t, 0);
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
