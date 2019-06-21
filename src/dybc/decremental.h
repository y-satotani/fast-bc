
#ifndef _DECREMENTAL_H_
#define _DECREMENTAL_H_

#include <igraph.h>
#include "aug_dist.h"

void decremental(igraph_t*            G,
                 igraph_integer_t     v,
                 igraph_integer_t     w,
                 igraph_matrix_t*     D,
                 igraph_matrix_int_t* Sigma,
                 igraph_matrix_t*     Delta,
                 const char*          weight);

void decremental_part(igraph_t*            G,
                      igraph_inclist_t*    inclist,
                      igraph_integer_t     v,
                      igraph_integer_t     w,
                      igraph_integer_t     z,
                      igraph_real_t        c,
                      igraph_matrix_t*     D,
                      igraph_matrix_int_t* Sigma,
                      igraph_matrix_t*     Delta,
                      const char*          weight);

#endif // _DECREMENTAL_H_