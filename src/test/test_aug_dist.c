
#include <assert.h>
#include <igraph.h>

#include "dybc/aug_dist.h"
#include "minigraph.h"

int check_aug_dist_of_minigraph(igraph_matrix_t* mat);

int main(int argc, char* argv[]) {
  igraph_i_set_attribute_table(&igraph_cattribute_table);

  igraph_t G;
  make_test_graph(&G, "length");

  igraph_matrix_t mat;
  aug_dist(&mat, &G, "length");
  assert(check_aug_dist_of_minigraph(&mat));

  igraph_matrix_destroy(&mat);
  igraph_destroy(&G);

  return 0;
}

int check_aug_dist_of_minigraph(igraph_matrix_t* mat) {
  assert(DIST(*mat, 0, 0) == 0);
  assert(DIST(*mat, 0, 1) == 1);
  assert(DIST(*mat, 0, 2) == 2);
  assert(DIST(*mat, 0, 3) == 3);
  assert(DIST(*mat, 0, 4) == 4);
  assert(DIST(*mat, 0, 5) == 5);
  assert(DIST(*mat, 0, 6) == 6);
  assert(DIST(*mat, 0, 7) == 8);
  assert(DIST(*mat, 1, 0) == 1);
  assert(DIST(*mat, 1, 1) == 0);
  assert(DIST(*mat, 1, 2) == 1);
  assert(DIST(*mat, 1, 3) == 2);
  assert(DIST(*mat, 1, 4) == 3);
  assert(DIST(*mat, 1, 5) == 4);
  assert(DIST(*mat, 1, 6) == 5);
  assert(DIST(*mat, 1, 7) == 7);
  assert(DIST(*mat, 2, 0) == 2);
  assert(DIST(*mat, 2, 1) == 1);
  assert(DIST(*mat, 2, 2) == 0);
  assert(DIST(*mat, 2, 3) == 3);
  assert(DIST(*mat, 2, 4) == 2);
  assert(DIST(*mat, 2, 5) == 3);
  assert(DIST(*mat, 2, 6) == 4);
  assert(DIST(*mat, 2, 7) == 6);
  assert(DIST(*mat, 3, 0) == 3);
  assert(DIST(*mat, 3, 1) == 2);
  assert(DIST(*mat, 3, 2) == 3);
  assert(DIST(*mat, 3, 3) == 0);
  assert(DIST(*mat, 3, 4) == 1);
  assert(DIST(*mat, 3, 5) == 2);
  assert(DIST(*mat, 3, 6) == 3);
  assert(DIST(*mat, 3, 7) == 5);
  assert(DIST(*mat, 4, 0) == 4);
  assert(DIST(*mat, 4, 1) == 3);
  assert(DIST(*mat, 4, 2) == 2);
  assert(DIST(*mat, 4, 3) == 1);
  assert(DIST(*mat, 4, 4) == 0);
  assert(DIST(*mat, 4, 5) == 1);
  assert(DIST(*mat, 4, 6) == 2);
  assert(DIST(*mat, 4, 7) == 4);
  assert(DIST(*mat, 5, 0) == 5);
  assert(DIST(*mat, 5, 1) == 4);
  assert(DIST(*mat, 5, 2) == 3);
  assert(DIST(*mat, 5, 3) == 2);
  assert(DIST(*mat, 5, 4) == 1);
  assert(DIST(*mat, 5, 5) == 0);
  assert(DIST(*mat, 5, 6) == 3);
  assert(DIST(*mat, 5, 7) == 3);
  assert(DIST(*mat, 6, 0) == 6);
  assert(DIST(*mat, 6, 1) == 5);
  assert(DIST(*mat, 6, 2) == 4);
  assert(DIST(*mat, 6, 3) == 3);
  assert(DIST(*mat, 6, 4) == 2);
  assert(DIST(*mat, 6, 5) == 3);
  assert(DIST(*mat, 6, 6) == 0);
  assert(DIST(*mat, 6, 7) == 2);
  assert(DIST(*mat, 7, 0) == 8);
  assert(DIST(*mat, 7, 1) == 7);
  assert(DIST(*mat, 7, 2) == 6);
  assert(DIST(*mat, 7, 3) == 5);
  assert(DIST(*mat, 7, 4) == 4);
  assert(DIST(*mat, 7, 5) == 3);
  assert(DIST(*mat, 7, 6) == 2);
  assert(DIST(*mat, 7, 7) == 0);

  assert(SIGMA(*mat, 0, 0) == 1);
  assert(SIGMA(*mat, 0, 1) == 1);
  assert(SIGMA(*mat, 0, 2) == 1);
  assert(SIGMA(*mat, 0, 3) == 1);
  assert(SIGMA(*mat, 0, 4) == 3);
  assert(SIGMA(*mat, 0, 5) == 3);
  assert(SIGMA(*mat, 0, 6) == 3);
  assert(SIGMA(*mat, 0, 7) == 6);
  assert(SIGMA(*mat, 1, 0) == 1);
  assert(SIGMA(*mat, 1, 1) == 1);
  assert(SIGMA(*mat, 1, 2) == 1);
  assert(SIGMA(*mat, 1, 3) == 1);
  assert(SIGMA(*mat, 1, 4) == 2);
  assert(SIGMA(*mat, 1, 5) == 2);
  assert(SIGMA(*mat, 1, 6) == 2);
  assert(SIGMA(*mat, 1, 7) == 4);
  assert(SIGMA(*mat, 2, 0) == 1);
  assert(SIGMA(*mat, 2, 1) == 1);
  assert(SIGMA(*mat, 2, 2) == 1);
  assert(SIGMA(*mat, 2, 3) == 2);
  assert(SIGMA(*mat, 2, 4) == 1);
  assert(SIGMA(*mat, 2, 5) == 1);
  assert(SIGMA(*mat, 2, 6) == 1);
  assert(SIGMA(*mat, 2, 7) == 2);
  assert(SIGMA(*mat, 3, 0) == 1);
  assert(SIGMA(*mat, 3, 1) == 1);
  assert(SIGMA(*mat, 3, 2) == 2);
  assert(SIGMA(*mat, 3, 3) == 1);
  assert(SIGMA(*mat, 3, 4) == 1);
  assert(SIGMA(*mat, 3, 5) == 1);
  assert(SIGMA(*mat, 3, 6) == 1);
  assert(SIGMA(*mat, 3, 7) == 2);
  assert(SIGMA(*mat, 4, 0) == 3);
  assert(SIGMA(*mat, 4, 1) == 2);
  assert(SIGMA(*mat, 4, 2) == 1);
  assert(SIGMA(*mat, 4, 3) == 1);
  assert(SIGMA(*mat, 4, 4) == 1);
  assert(SIGMA(*mat, 4, 5) == 1);
  assert(SIGMA(*mat, 4, 6) == 1);
  assert(SIGMA(*mat, 4, 7) == 2);
  assert(SIGMA(*mat, 5, 0) == 3);
  assert(SIGMA(*mat, 5, 1) == 2);
  assert(SIGMA(*mat, 5, 2) == 1);
  assert(SIGMA(*mat, 5, 3) == 1);
  assert(SIGMA(*mat, 5, 4) == 1);
  assert(SIGMA(*mat, 5, 5) == 1);
  assert(SIGMA(*mat, 5, 6) == 1);
  assert(SIGMA(*mat, 5, 7) == 1);
  assert(SIGMA(*mat, 6, 0) == 3);
  assert(SIGMA(*mat, 6, 1) == 2);
  assert(SIGMA(*mat, 6, 2) == 1);
  assert(SIGMA(*mat, 6, 3) == 1);
  assert(SIGMA(*mat, 6, 4) == 1);
  assert(SIGMA(*mat, 6, 5) == 1);
  assert(SIGMA(*mat, 6, 6) == 1);
  assert(SIGMA(*mat, 6, 7) == 1);
  assert(SIGMA(*mat, 7, 0) == 6);
  assert(SIGMA(*mat, 7, 1) == 4);
  assert(SIGMA(*mat, 7, 2) == 2);
  assert(SIGMA(*mat, 7, 3) == 2);
  assert(SIGMA(*mat, 7, 4) == 2);
  assert(SIGMA(*mat, 7, 5) == 1);
  assert(SIGMA(*mat, 7, 6) == 1);
  assert(SIGMA(*mat, 7, 7) == 1);

  return 1;
}
