
#include "dybc_batch_update.h"

#include <assert.h>
#include <igraph/igraph.h>

#include "dybc_update_query.h"
#include "dybc_exp_common.h"

void batch_update(igraph_t* G,
                  igraph_matrix_t* D,
                  igraph_matrix_int_t *S,
                  igraph_vector_t* B,
                  igraph_vector_int_t* endpoints,
                  igraph_vector_t* weights,
                  igraph_vector_t* weights_,
                  igraph_vector_int_t* queries) {

  assert
    (igraph_vector_int_size(endpoints) == igraph_vector_int_size(queries) * 2
     && "the size of endpoints must be two times of the size of queries\n");

  for(igraph_integer_t i = 0; i < igraph_vector_int_size(queries); i++) {
    igraph_integer_t u = VECTOR(*endpoints)[2*i];
    igraph_integer_t v = VECTOR(*endpoints)[2*i+1];
    igraph_real_t weight;
    if(weights && weights_) weight = VECTOR(*weights_)[i];

    if(VECTOR(*queries)[i] == QUERY_INSERT)
      incremental_update(G, D, S, B, u, v, weights, weight, 0);
    else if(VECTOR(*queries)[i] == QUERY_DELETE)
      decremental_update(G, D, S, B, u, v, weights, weight, 0);
    else assert(0 && "invalid query\n");
  }
}
