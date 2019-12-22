
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

int test_inc_mini_unweighted() {
  _DYBC_TEST_DECL_;
  make_less_graph_and_edge(&G, &u, &v, &weight, &weights);
  igraph_vector_fill(&weights, 1.);
  weight = 1.;
  _DYBC_TEST_INIT_;
  incremental_update_unweighted(&G, &D, &S, &B, u, v);
  int res = check_quantities("test_inc_mini_unweighted", &G, &D, &S, &B, 0);
  _DYBC_TEST_DEST_;
  return res;
}

int test_dec_mini_unweighted() {
  _DYBC_TEST_DECL_;
  make_more_graph_and_edge(&G, &u, &v, &weight, &weights);
  igraph_vector_fill(&weights, 1.);
  weight = 1.;
  _DYBC_TEST_INIT_;
  decremental_update_unweighted(&G, &D, &S, &B, u, v);
  int res = check_quantities("test_dec_mini_unweighted", &G, &D, &S, &B, 0);
  _DYBC_TEST_DEST_;
  return res;
}

int main(int argc, char* argv[]) {
  test_inc_mini_unweighted();
  test_dec_mini_unweighted();
  test_inc_mini();
  test_dec_mini();
  return 0;
}

