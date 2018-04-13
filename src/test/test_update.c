
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <igraph.h>
#include <assert.h>
#include "bcdec/arg_dist.h"
#include "bcdec/update_on_insert.h"
#include "bcdec/update_on_delete.h"

double count_diff(igraph_matrix_t* v1, igraph_matrix_t* v2);

int main(int argc, char* argv[]) {
  int n = 50;
  igraph_t G, H;
  igraph_matrix_t DTest, SigmaTest, DTrain, SigmaTrain;
  clock_t start, end, sum;

  igraph_empty(&G, n, IGRAPH_UNDIRECTED);
  igraph_full(&H, n, 0, 0);
  arg_dist(&DTest, &SigmaTest, &G);

  // perform incremental update
  sum = 0;
  printf("performing incremental update...\n");
  while(igraph_ecount(&G) < n*(n-1)/2) {
    // select edge
    igraph_integer_t eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&H)-1);
    igraph_integer_t u, v;
    igraph_edge(&H, eid, &u, &v);
    // increment
    start = clock();
    update_on_insert(&DTest, &SigmaTest, u, v);
    end = clock();
    sum += end - start;
    // testing
    igraph_add_edge(&G, u, v);
    igraph_delete_edges(&H, igraph_ess_1(eid));
    arg_dist(&DTrain, &SigmaTrain, &G);
    assert(count_diff(&DTest, &DTrain) == 0);
    assert(count_diff(&SigmaTest, &SigmaTrain) == 0);
    // cleanup
    igraph_matrix_destroy(&DTrain);
    igraph_matrix_destroy(&SigmaTrain);
  }
  printf("done. %f seconds (total %d edges)\n",
         (double)sum/CLOCKS_PER_SEC, n*(n-1)/2);
  igraph_destroy(&H);

  // perform decremental update
  sum = 0;
  printf("performing decremental update...\n");
  while(igraph_ecount(&G) > 0) {
    // select edge
    igraph_integer_t eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&G)-1);
    igraph_integer_t u, v;
    igraph_edge(&G, eid, &u, &v);
    // decrement
    start = clock();
    update_on_delete(&DTest, &SigmaTest, u, v);
    end = clock();
    sum += end - start;
    // testing
    igraph_delete_edges(&G, igraph_ess_1(eid));
    arg_dist(&DTrain, &SigmaTrain, &G);
    assert(count_diff(&DTest, &DTrain) == 0);
    assert(count_diff(&SigmaTest, &SigmaTrain) == 0);
    // cleanup
    igraph_matrix_destroy(&DTrain);
    igraph_matrix_destroy(&SigmaTrain);
  }
  printf("done. %f seconds (total %d edges)\n",
         (double)sum/CLOCKS_PER_SEC, n*(n-1)/2);

  igraph_destroy(&G);
  igraph_matrix_destroy(&DTest);
  igraph_matrix_destroy(&SigmaTest);
  return 0;
}

double count_diff(igraph_matrix_t* v1, igraph_matrix_t* v2) {
  int i, j, count = 0;
  for(i = 0; i < igraph_matrix_nrow(v1); i++) {
    for(j = 0; j < igraph_matrix_ncol(v1); j++) {
      if(igraph_matrix_e(v1, i, j) != igraph_matrix_e(v2, i, j))
        count++;
    }
  }
  return count;
}
