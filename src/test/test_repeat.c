
#include <stdio.h>
#include <assert.h>
#include <igraph/igraph.h>

#include "dybc_test_common.h"

int test_repeat(unsigned long int seed, int steps, igraph_bool_t directed);

int main(int argc, char* argv[]) {
  for(unsigned int seed = 0; seed < 10; seed++)
    test_repeat(seed, 100, 0);
  for(unsigned int seed = 0; seed < 10; seed++)
    test_repeat(seed, 100, 1);
  return 0;
}

int test_repeat(unsigned long int seed, int steps, igraph_bool_t directed) {
  _DYBC_TEST_DECL_;
  // initialize a graph
  igraph_rng_seed(igraph_rng_default(), seed);
  igraph_erdos_renyi_game(&G, IGRAPH_ERDOS_RENYI_GNM, 20, 50, directed, 0);
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
    incremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);

    sprintf(test_name, "test_repeat (%lu) at inc step %d", seed, step);
    res |= check_quantities(test_name, &G, &D, &S, &B, &weights);

    // select endpoints to delete
    eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&G)-1);
    igraph_edge(&G, eid, &u, &v);
    weight = igraph_vector_e(&weights, eid);
    decremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);

    sprintf(test_name, "test_repeat (%lu) at dec step %d", seed, step);
    int res = check_quantities(test_name, &G, &D, &S, &B, &weights);
  }

  _DYBC_TEST_DEST_;
  return res;
}
