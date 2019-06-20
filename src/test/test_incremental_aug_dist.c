
#include <assert.h>
#include <igraph.h>
#include <math.h>

#include "dybc/aug_dist.h"
#include "dybc/incremental.h"
#include "dybc/betweenness.h"
#include "minigraph.h"

int main(int argc, char* argv[]) {
  igraph_i_set_attribute_table(&igraph_cattribute_table);

  // initialise graph
  igraph_t G;
  igraph_integer_t n = 20;
  igraph_integer_t m = 100;
  igraph_erdos_renyi_game(&G, IGRAPH_ERDOS_RENYI_GNM, n, m, 0, 0);

  // find vertex pair to add as an edge
  igraph_vector_long_t vertex_pair;
  igraph_vector_long_init(&vertex_pair, 0);
  for(igraph_integer_t u = 0; u < igraph_vcount(&G); u++)
    for(igraph_integer_t v = 0; v < igraph_vcount(&G); v++) {
      igraph_integer_t eid;
      igraph_get_eid(&G, &eid, u, v, 0, 0);
      if(eid < 0) {
        igraph_vector_long_push_back(&vertex_pair, u);
        igraph_vector_long_push_back(&vertex_pair, v);
      }
    }
  long int vpi = igraph_rng_get_integer
    (igraph_rng_default(), 0, igraph_vector_long_size(&vertex_pair) / 2 - 1);
  igraph_integer_t v = VECTOR(vertex_pair)[2 * vpi];
  igraph_integer_t w = VECTOR(vertex_pair)[2 * vpi + 1];

  // set weights
  const char* weight = "length";
  unsigned long int seed;
  if(argc < 2) seed = 0;
  else seed = atoll(argv[1]);
  igraph_rng_seed(igraph_rng_default(), seed);
  igraph_real_t c = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
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
  incremental(&G, v, w, c, &D, &Sigma, &Delta, weight);
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
  /*
  for(igraph_integer_t uid = 0; uid < igraph_vcount(&G); uid++)
    for(igraph_integer_t vid = 0; vid < igraph_vcount(&G); vid++)
      printf("%d %d %f %f\n", uid, vid, MATRIX(Delta, uid, vid), MATRIX(Deltatrue, uid, vid));
  */

  igraph_vector_long_destroy(&vertex_pair);
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

