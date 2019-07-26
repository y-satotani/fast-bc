
#include <assert.h>
#include <igraph.h>
#include <math.h>

#include "dybc/aug_dist.h"
#include "dybc/decremental.h"
#include "dybc/betweenness.h"
#include "minigraph.h"

int main(int argc, char* argv[]) {
  igraph_i_set_attribute_table(&igraph_cattribute_table);
  unsigned long int seed;
  if(argc < 2) seed = 0;
  else seed = atoll(argv[1]);
  igraph_rng_seed(igraph_rng_default(), seed);

  // initialise graph
  igraph_t G;
  igraph_integer_t n = 20;
  igraph_integer_t m = 100;
  igraph_erdos_renyi_game(&G, IGRAPH_ERDOS_RENYI_GNM, n, m, 0, 0);

  // find vertex pair to remove
  igraph_integer_t eid = igraph_rng_get_integer
    (igraph_rng_default(), 0, igraph_ecount(&G)-1);
  igraph_integer_t v, w;
  igraph_edge(&G, eid, &v, &w);

  // set weights
  const char* weight = "length";
  for(igraph_integer_t eid = 0; eid < igraph_ecount(&G); eid++) {
    long int l = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
    SETEAN(&G, weight, eid, l);
  }

  // find betweenness centrality
  igraph_matrix_t D;
  igraph_matrix_int_t Sigma;
  igraph_matrix_t Delta;
  igraph_vector_t B;
  igraph_vector_init(&B, igraph_vcount(&G));
  aug_dist(&G, &D, &Sigma, weight);
  pairwise_dependency(&G, &Delta, weight);
  decremental(&G, v, w, &D, &Sigma, &Delta, weight, 0, 0, 0);
  igraph_matrix_colsum(&Delta, &B);
  igraph_vector_scale(&B, 0.5);

  // find true betweenness centrality
  igraph_vector_t weights;
  igraph_vector_init(&weights, igraph_ecount(&G));
  for(igraph_integer_t eid = 0; eid < igraph_ecount(&G); eid++) {
    VECTOR(weights)[eid] = EAN(&G, weight, eid);
  }
  igraph_vector_t Btrue;
  igraph_matrix_t Deltatrue;
  igraph_vector_init(&Btrue, igraph_vcount(&G));
  igraph_betweenness(&G, &Btrue, igraph_vss_all(), 0, &weights, 0);
  pairwise_dependency(&G, &Deltatrue, weight);

  // print result
  for(igraph_integer_t vid = 0; vid < igraph_vcount(&G); vid++) {
    igraph_real_t b = VECTOR(B)[vid];
    igraph_real_t bt = VECTOR(Btrue)[vid];
    printf("%ld\t%d\t%f\t%f\t%f\n", seed, vid, b, bt, fabs(b-bt));
  }

  igraph_vector_destroy(&weights);
  igraph_vector_destroy(&B);
  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&Sigma);
  igraph_matrix_destroy(&Delta);
  igraph_matrix_destroy(&Deltatrue);
  igraph_vector_destroy(&Btrue);
  igraph_destroy(&G);

  return 0;
}

