
#include <math.h>
#include <assert.h>
#include <igraph.h>

#include "dybc/aug_dist.h"
#include "dybc/betweenness.h"
#include "minigraph.h"

int check_pairwise_dependency_of_minigraph(igraph_matrix_t*);

int main(int argc, char* argv[]) {
  igraph_i_set_attribute_table(&igraph_cattribute_table);

  igraph_t G;
  make_test_graph(&G, "length");

  igraph_matrix_t Delta;
  pairwise_dependency(&Delta, &G, "length");

  assert(check_pairwise_dependency_of_minigraph(&Delta));

  igraph_matrix_destroy(&Delta);
  igraph_destroy(&G);

  return 0;
}

int check_pairwise_dependency_of_minigraph(igraph_matrix_t* mat) {

  assert(abs(MATRIX(*mat, 0, 0) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 0, 1) - 4.666666666666666) < 1e-9);
  assert(abs(MATRIX(*mat, 0, 2) - 1.3333333333333333) < 1e-9);
  assert(abs(MATRIX(*mat, 0, 3) - 1.3333333333333333) < 1e-9);
  assert(abs(MATRIX(*mat, 0, 4) - 3.0) < 1e-9);
  assert(abs(MATRIX(*mat, 0, 5) - 0.5) < 1e-9);
  assert(abs(MATRIX(*mat, 0, 6) - 0.5) < 1e-9);
  assert(abs(MATRIX(*mat, 0, 7) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 1, 0) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 1, 1) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 1, 2) - 2.0) < 1e-9);
  assert(abs(MATRIX(*mat, 1, 3) - 2.0) < 1e-9);
  assert(abs(MATRIX(*mat, 1, 4) - 3.0) < 1e-9);
  assert(abs(MATRIX(*mat, 1, 5) - 0.5) < 1e-9);
  assert(abs(MATRIX(*mat, 1, 6) - 0.5) < 1e-9);
  assert(abs(MATRIX(*mat, 1, 7) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 2, 0) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 2, 1) - 1.5) < 1e-9);
  assert(abs(MATRIX(*mat, 2, 2) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 2, 3) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 2, 4) - 3.5) < 1e-9);
  assert(abs(MATRIX(*mat, 2, 5) - 0.5) < 1e-9);
  assert(abs(MATRIX(*mat, 2, 6) - 0.5) < 1e-9);
  assert(abs(MATRIX(*mat, 2, 7) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 3, 0) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 3, 1) - 1.5) < 1e-9);
  assert(abs(MATRIX(*mat, 3, 2) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 3, 3) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 3, 4) - 3.5) < 1e-9);
  assert(abs(MATRIX(*mat, 3, 5) - 0.5) < 1e-9);
  assert(abs(MATRIX(*mat, 3, 6) - 0.5) < 1e-9);
  assert(abs(MATRIX(*mat, 3, 7) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 4, 0) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 4, 1) - 0.6666666666666666) < 1e-9);
  assert(abs(MATRIX(*mat, 4, 2) - 0.8333333333333333) < 1e-9);
  assert(abs(MATRIX(*mat, 4, 3) - 0.8333333333333333) < 1e-9);
  assert(abs(MATRIX(*mat, 4, 4) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 4, 5) - 0.5) < 1e-9);
  assert(abs(MATRIX(*mat, 4, 6) - 0.5) < 1e-9);
  assert(abs(MATRIX(*mat, 4, 7) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 5, 0) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 5, 1) - 0.6666666666666666) < 1e-9);
  assert(abs(MATRIX(*mat, 5, 2) - 0.8333333333333333) < 1e-9);
  assert(abs(MATRIX(*mat, 5, 3) - 0.8333333333333333) < 1e-9);
  assert(abs(MATRIX(*mat, 5, 4) - 5.0) < 1e-9);
  assert(abs(MATRIX(*mat, 5, 5) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 5, 6) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 5, 7) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 6, 0) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 6, 1) - 0.6666666666666666) < 1e-9);
  assert(abs(MATRIX(*mat, 6, 2) - 0.8333333333333333) < 1e-9);
  assert(abs(MATRIX(*mat, 6, 3) - 0.8333333333333333) < 1e-9);
  assert(abs(MATRIX(*mat, 6, 4) - 5.0) < 1e-9);
  assert(abs(MATRIX(*mat, 6, 5) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 6, 6) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 6, 7) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 7, 0) - 0) < 1e-9);
  assert(abs(MATRIX(*mat, 7, 1) - 0.6666666666666666) < 1e-9);
  assert(abs(MATRIX(*mat, 7, 2) - 0.8333333333333333) < 1e-9);
  assert(abs(MATRIX(*mat, 7, 3) - 0.8333333333333333) < 1e-9);
  assert(abs(MATRIX(*mat, 7, 4) - 4.0) < 1e-9);
  assert(abs(MATRIX(*mat, 7, 5) - 2.5) < 1e-9);
  assert(abs(MATRIX(*mat, 7, 6) - 2.5) < 1e-9);
  assert(abs(MATRIX(*mat, 7, 7) - 0) < 1e-9);

  return 1;
}
