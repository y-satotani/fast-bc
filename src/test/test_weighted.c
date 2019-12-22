
#include <stdio.h>
#include <assert.h>
#include <igraph/igraph.h>

#include "dybc_test_common.h"

int test_weighted_random(igraph_bool_t directed,
                         igraph_bool_t incremental,
                         unsigned long int seed) {
  _DYBC_TEST_DECL_;
  // initialize a graph
  igraph_rng_seed(igraph_rng_default(), seed);
  igraph_erdos_renyi_game
    (&G, IGRAPH_ERDOS_RENYI_GNM, 100, 400, directed, 0);
  // set weight
  igraph_vector_init(&weights, igraph_ecount(&G));
  for(igraph_integer_t eid = 0; eid < igraph_ecount(&G); eid++) {
    weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
    igraph_vector_set(&weights, eid, weight);
  }
  if(incremental)
    weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
  else
    weight = IGRAPH_INFINITY;
  // select endpoints
  if(incremental) {
    igraph_t C;
    igraph_complementer(&C, &G, 0);
    igraph_integer_t eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&C)-1);
    igraph_edge(&C, eid, &u, &v);
    igraph_destroy(&C);
  } else {
    igraph_integer_t eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&G)-1);
    igraph_edge(&G, eid, &u, &v);
  }
  _DYBC_TEST_INIT_;

  if(incremental) {
    incremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);
  } else {
    decremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);
  }

  char test_name[1024];
  sprintf(test_name,
          "test_weighted (%s %s %lu)",
          directed ? "directed" : "undirected",
          incremental ? "incremental" : "decremental",
          seed);
  int res = check_quantities(test_name, &G, &D, &S, &B, &weights);

  _DYBC_TEST_DEST_;
  return res;
}

int main(int argc, char* argv[]) {
  for(unsigned int seed = 0; seed < 100; seed++)
    test_weighted_random(0, 1, seed);
  for(unsigned int seed = 0; seed < 100; seed++)
    test_weighted_random(0, 0, seed);
  for(unsigned int seed = 0; seed < 100; seed++)
    test_weighted_random(1, 1, seed);
  for(unsigned int seed = 0; seed < 100; seed++)
    test_weighted_random(1, 0, seed);
  return 0;
}
