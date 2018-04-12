
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <igraph.h>
#include <bcdec/arg_dist.h>
#include <bcdec/arg_dist_bc.h>

void print_vector(igraph_vector_t* v);
double l2_norm(igraph_vector_t* v1, igraph_vector_t* v2);

int main(void) {

  igraph_vector_t Btrain, Btest;
  igraph_matrix_t D, Sigma;
  igraph_t G;
  clock_t start, end;
  double sec;

  igraph_k_regular_game(&G, 100, 3, 0, 0);

  // calculate train BC
  igraph_vector_init(&Btrain, igraph_vcount(&G));
  start = clock();
  igraph_betweenness(&G, &Btrain, igraph_vss_all(), 0, 0, 1);
  end = clock();
  sec = (double)(end - start) / CLOCKS_PER_SEC;
  printf("train: "); /* print_vector(&Btrain); */ printf("%f\n", sec);

  // calculate test BC
  igraph_vector_init(&Btest, igraph_vcount(&G));
  start = clock();
  arg_dist(&D, &Sigma, &G);
  arg_dist_bc(&Btest, &D, &Sigma);
  end = clock();
  sec = (double)(end - start) / CLOCKS_PER_SEC;
  printf("test : "); /* print_vector(&Btest); */ printf("%f\n", sec);

  printf("L2 norm: %f\n", l2_norm(&Btrain, &Btest));

  igraph_vector_destroy(&Btrain);
  igraph_vector_destroy(&Btest);
  igraph_matrix_destroy(&D);
  igraph_matrix_destroy(&Sigma);
  igraph_destroy(&G);

  return 0;
}

void print_vector(igraph_vector_t* v) {
  int i;
  printf("[");
  for(i = 0; i < igraph_vector_size(v); i++) {
    printf("%f", igraph_vector_e(v, i));
    if(i < igraph_vector_size(v)-1)
      printf(", ");
  }
  printf("]\n");
}

double l2_norm(igraph_vector_t* v1, igraph_vector_t* v2) {
  int i;
  double sub, sum = 0;
  for(i = 0; i < igraph_vector_size(v1); i++) {
    sub = igraph_vector_e(v1, i) - igraph_vector_e(v2, i);
    sum += sub * sub;
  }
  return sqrt(sum);
}
