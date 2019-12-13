
#include <stdio.h>
#include <assert.h>
#include <igraph/igraph.h>

#include "dybc_test_common.h"
#include "minigraph.h"

int test_inc_mini() {
  _DYBC_TEST_DECL_;
  make_less_graph_and_edge(&G, &u, &v, &weight, &weights);
  _DYBC_TEST_INIT_;
  incremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);
  int res = check_quantities("test_inc_mini", &G, &D, &S, &B, &weights);
  _DYBC_TEST_DEST_;
  return res;
}

int test_dec_mini() {
  _DYBC_TEST_DECL_;
  make_more_graph_and_edge(&G, &u, &v, &weight, &weights);
  _DYBC_TEST_INIT_;
  decremental_update_weighted(&G, &D, &S, &B, u, v, &weights, weight);
  int res = check_quantities("test_dec_mini", &G, &D, &S, &B, &weights);
  _DYBC_TEST_DEST_;
  return res;
}

int main(int argc, char* argv[]) {
  test_inc_mini();
  test_dec_mini();
  return 0;
}

