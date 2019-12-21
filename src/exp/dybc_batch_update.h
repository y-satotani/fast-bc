
#ifndef _DYBC_BATCH_UPDATE_H_
#define _DYBC_BATCH_UPDATE_H_

#include <igraph/igraph.h>
#include "dybc/dybc_update_query.h"

void batch_update(igraph_t* G,
                  igraph_matrix_t* D,
                  igraph_matrix_int_t *S,
                  igraph_vector_t* B,
                  igraph_vector_int_t* endpoints,
                  igraph_vector_t* weights,
                  igraph_vector_t* weights_,
                  igraph_vector_int_t* queries);

#endif // _DYBC_BATCH_UPDATE_H_
