
#include <stdio.h>
#include <assert.h>
#include <igraph/igraph.h>

#include "dybc_test_common.h"

int test_components(igraph_bool_t directed,
                    igraph_bool_t incremental,
                    unsigned long int seed) {
  _DYBC_TEST_DECL_;
  // initialize a graph
  igraph_rng_seed(igraph_rng_default(), seed);
  igraph_integer_t n1 = 20, n2 = 20, n3 = 20;
  igraph_integer_t m1 = 40, m2 = 50, m3 = 60;
  igraph_t G1, G2, G3;
  igraph_erdos_renyi_game
    (&G1, IGRAPH_ERDOS_RENYI_GNM, n1, m1, directed, 0);
  igraph_erdos_renyi_game
    (&G2, IGRAPH_ERDOS_RENYI_GNM, n2, m2, directed, 0);
  igraph_erdos_renyi_game
    (&G3, IGRAPH_ERDOS_RENYI_GNM, n3, m3, directed, 0);
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
  igraph_destroy(&G1);
  igraph_destroy(&G2);
  igraph_destroy(&G3);

  // select endpoints
  u = igraph_rng_get_integer(igraph_rng_default(), 0, n1-1);
  v = igraph_rng_get_integer(igraph_rng_default(), n1, n1+n2-1);
  if(!incremental) igraph_add_edge(&G, u, v);

  // set weights
  igraph_vector_init(&weights, igraph_ecount(&G));
  for(igraph_integer_t eid = 0; eid < igraph_ecount(&G); eid++) {
    weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
    igraph_vector_set(&weights, eid, weight);
  }
  weight = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
  _DYBC_TEST_INIT_;

  if(incremental)
    incremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);
  else
    decremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);
  char test_name[1024];
  sprintf(test_name,
          "test_components (%s %s %lu)",
          directed ? "directed" : "undirected",
          incremental ? "incremental" : "decremental",
          seed);
  int res = check_quantities(test_name, &G, &D, &S, &B, &weights);

  _DYBC_TEST_DEST_;
  return res;
}

int main(int argc, char* argv[]) {
  for(unsigned int seed = 0; seed < 100; seed++)
    test_components(0, 1, seed);
  for(unsigned int seed = 0; seed < 100; seed++)
    test_components(0, 0, seed);
  for(unsigned int seed = 0; seed < 100; seed++)
    test_components(1, 1, seed);
  for(unsigned int seed = 0; seed < 100; seed++)
    test_components(1, 0, seed);
  return 0;
}
