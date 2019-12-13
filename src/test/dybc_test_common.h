
#ifndef _DYBC_TEST_COMMON_
#define _DYBC_TEST_COMMON_

#include <igraph/igraph.h>
#include <dybc/static_betweenness.h>

int check_quantities(const char* test_name,
                     igraph_t* G,
                     igraph_matrix_t* D,
                     igraph_matrix_int_t* S,
                     igraph_vector_t* B,
                     igraph_vector_t* weights);

void incremental_update_weighted(igraph_t* G,
                                 igraph_matrix_t* D,
                                 igraph_matrix_int_t *S,
                                 igraph_vector_t* B,
                                 igraph_integer_t u,
                                 igraph_integer_t v,
                                 igraph_vector_t* weights,
                                 igraph_real_t weight);

void decremental_update_weighted(igraph_t* G,
                                 igraph_matrix_t* D,
                                 igraph_matrix_int_t *S,
                                 igraph_vector_t* B,
                                 igraph_integer_t u,
                                 igraph_integer_t v,
                                 igraph_vector_t* weights,
                                 igraph_real_t weight);

void incremental_update_unweighted(igraph_t* G,
                                   igraph_matrix_t* D,
                                   igraph_matrix_int_t *S,
                                   igraph_vector_t* B,
                                   igraph_integer_t u,
                                   igraph_integer_t v);

void decremental_update_unweighted(igraph_t* G,
                                   igraph_matrix_t* D,
                                   igraph_matrix_int_t *S,
                                   igraph_vector_t* B,
                                   igraph_integer_t u,
                                   igraph_integer_t v);

#define _DYBC_TEST_DECL_                        \
  igraph_t G;                                   \
  igraph_integer_t u, v;                        \
  igraph_real_t weight;                         \
  igraph_vector_t weights;                      \
  igraph_matrix_t D;                            \
  igraph_matrix_int_t S;                        \
  igraph_vector_t B;

// find distance and geodesics and betweenness
#define _DYBC_TEST_INIT_                        \
  igraph_matrix_init                            \
  (&D, igraph_vcount(&G), igraph_vcount(&G));   \
  igraph_matrix_int_init                        \
  (&S, igraph_vcount(&G), igraph_vcount(&G));   \
  igraph_vector_init                            \
  (&B, igraph_vcount(&G));                      \
  betweenness_with_redundant_information        \
  (&G, &D, &S, &B, &weights);

#define _DYBC_TEST_DEST_                        \
  igraph_matrix_destroy(&D);                    \
  igraph_matrix_int_destroy(&S);                \
  igraph_vector_destroy(&B);                    \
  igraph_vector_destroy(&weights);              \
  igraph_destroy(&G);

#endif // _DYBC_TEST_COMMON_
