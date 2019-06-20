
#include <assert.h>
#include <igraph.h>
#include <stdio.h>

#include "dybc/aug_dist.h"
#include "minigraph.h"

int main(int argc, char* argv[]) {
  igraph_i_set_attribute_table(&igraph_cattribute_table);

  igraph_t G;
  make_test_graph(&G, "length");

  igraph_matrix_t D;
  igraph_matrix_int_t Sigma;
  aug_dist(&G, &D, &Sigma, "length");
  assert(check_aug_dist_of_minigraph(&D, &Sigma));

  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&Sigma);
  igraph_destroy(&G);

  return 0;
}
