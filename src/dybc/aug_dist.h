
#ifndef _AUG_DIST_H_
#define _AUG_DIST_H_

#include <igraph/igraph.h>

void aug_dist(igraph_t*            G,
              igraph_matrix_t*     D,
              igraph_matrix_int_t* Sigma,
              const char*          weight);

#endif // _AUG_DIST_H_
