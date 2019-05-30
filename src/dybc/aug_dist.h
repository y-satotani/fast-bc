
#ifndef _AUG_DIST_H_
#define _AUG_DIST_H_

#include <igraph.h>

// distance is in lower triangular part of augmented distance matrix
#define DIST(m, i ,j)                                                   \
  ((i) > (j) ? MATRIX((m), (i), (j)) : ((i) < (j) ? MATRIX((m), (j), (i)) : 0))
#define SETDIST(m, i, j, d) {                \
  if((i) > (j)) MATRIX((m), (i), (j)) = (d); \
  else MATRIX((m), (j), (i)) = (d);          \
}

// sigma is in upper triangular part of augmented distance matrix
#define SIGMA(m, i ,j)                                                  \
  ((i) < (j) ? MATRIX((m), (i), (j)) : ((i) > (j) ? MATRIX((m), (j), (i)) : 1))
#define SETSIGMA(m, i, j, s) {               \
  if((i) < (j)) MATRIX((m), (i), (j)) = (s); \
  else MATRIX((m), (j), (i)) = (s);          \
}

void aug_dist(igraph_matrix_t* DS, igraph_t* G, const char* weight);

#endif // _AUG_DIST_H_
