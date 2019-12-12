
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <igraph/igraph.h>

#include "dybc/static_betweenness.h"
#include "minigraph.h"

int main(int argc, char* argv[]) {
  igraph_t G;
  igraph_vector_t weight;
  make_test_graph(&G, &weight);

  igraph_matrix_t D;
  igraph_matrix_int_t S;
  igraph_vector_t B;
  igraph_matrix_init(&D, igraph_vcount(&G), igraph_vcount(&G));
  igraph_matrix_int_init(&S, igraph_vcount(&G), igraph_vcount(&G));
  igraph_vector_init(&B, igraph_vcount(&G));

  betweenness_with_redundant_information(&G, &D, &S, &B, &weight);
  igraph_vector_scale(&B, 0.5);
  assert(check_aug_dist_of_minigraph(&D, &S));
  assert(check_betweenness_of_minigraph(&B));

  igraph_vector_destroy(&B);
  igraph_matrix_int_destroy(&S);
  igraph_matrix_destroy(&D);
  igraph_vector_destroy(&weight);
  igraph_destroy(&G);

  return 0;
}

