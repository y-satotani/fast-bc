
#ifndef _INCREMENTAL_H_
#define _INCREMENTAL_H_

#include <igraph.h>
#include "aug_dist.h"

void incremental(igraph_t*            G,
                 igraph_integer_t     v,
                 igraph_integer_t     w,
                 igraph_real_t        c,
                 igraph_matrix_t*     D,
                 igraph_matrix_int_t* Sigma,
                 igraph_matrix_t*     Delta,
                 const char*          weight,
                 igraph_integer_t*    n_update_path_pairs,
                 igraph_integer_t*    n_update_dep_verts);

void incremental_part(igraph_t*            G,
                      igraph_inclist_t*    inclist,
                      igraph_integer_t     v,
                      igraph_integer_t     w,
                      igraph_integer_t     z,
                      igraph_real_t        c,
                      igraph_matrix_t*     D,
                      igraph_matrix_int_t* Sigma,
                      igraph_matrix_t*     Delta,
                      const char*          weight,
                      igraph_integer_t*     n_update_path_pairs,
                      igraph_vector_bool_t* update_dep_verts);

#endif // _INCREMENTAL_H_
