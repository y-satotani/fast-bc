
#include <assert.h>
#include <igraph.h>
#include "dybc/aug_dist.h"

int main(int argc, char* argv[]) {
  igraph_matrix_t matrix;
  igraph_matrix_init(&matrix, 3, 3);

  // initialize matrix in:
  // 0 1 2
  // 3 4 5
  // 6 7 8
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      MATRIX(matrix, i, j) = j+i*3;
    }
  }

  // test distance matrix
  // 0 3 6
  // 3 0 7
  // 6 7 0
  assert(DIST(matrix, 0, 0) == 0);
  assert(DIST(matrix, 0, 1) == 3);
  assert(DIST(matrix, 0, 2) == 6);
  assert(DIST(matrix, 1, 0) == 3);
  assert(DIST(matrix, 1, 1) == 0);
  assert(DIST(matrix, 1, 2) == 7);
  assert(DIST(matrix, 2, 0) == 6);
  assert(DIST(matrix, 2, 1) == 7);
  assert(DIST(matrix, 2, 2) == 0);
  // SETDIST test
  SETDIST(matrix, 0, 1, 100);
  SETDIST(matrix, 0, 0, 100);
  assert(DIST(matrix, 0, 1) == 100);
  assert(DIST(matrix, 1, 0) == 100);
  assert(DIST(matrix, 0, 0) == 0);

  // test sigma matrix
  // 1 1 2
  // 1 1 5
  // 2 5 1
  assert(SIGMA(matrix, 0, 0) == 1);
  assert(SIGMA(matrix, 0, 1) == 1);
  assert(SIGMA(matrix, 0, 2) == 2);
  assert(SIGMA(matrix, 1, 0) == 1);
  assert(SIGMA(matrix, 1, 1) == 1);
  assert(SIGMA(matrix, 1, 2) == 5);
  assert(SIGMA(matrix, 2, 0) == 2);
  assert(SIGMA(matrix, 2, 1) == 5);
  assert(SIGMA(matrix, 2, 2) == 1);
  // SETSIGMA test
  SETSIGMA(matrix, 0, 1, 100);
  SETSIGMA(matrix, 0, 0, 100);
  assert(SIGMA(matrix, 0, 1) == 100);
  assert(SIGMA(matrix, 1, 0) == 100);
  assert(SIGMA(matrix, 0, 0) == 1);

  igraph_matrix_destroy(&matrix);
}
