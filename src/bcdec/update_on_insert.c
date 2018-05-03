
#include "update_on_insert.h"

void update_on_insert(igraph_matrix_t* D, igraph_matrix_t* Sigma,
                      igraph_integer_t u, igraph_integer_t v) {

  long int N = igraph_matrix_nrow(D);
  igraph_integer_t s, t;
  for(s = 0; s < N; s++) {
    if(MATRIX(*D, s, u) == MATRIX(*D, s, v))
      continue;
    igraph_integer_t a, b;
    if(MATRIX(*D, s, u) < MATRIX(*D, s, v)) {
      a = u; b = v;
    } else {
      a = v; b = u;
    }

    for(t = s+1; t < N; t++) {
      if(MATRIX(*D, b, t) == IGRAPH_INFINITY) {
        continue;
      }
      igraph_integer_t dsabt =
        (igraph_integer_t)MATRIX(*D, s, a) +
        (igraph_integer_t)MATRIX(*D, b, t) + 1;
      igraph_integer_t ssabt =
        (igraph_integer_t)MATRIX(*Sigma, s, a) *
        (igraph_integer_t)MATRIX(*Sigma, b, t);
      if(MATRIX(*D, s, t) == IGRAPH_INFINITY || dsabt < MATRIX(*D, s, t)) {
        MATRIX(*D, s, t) = MATRIX(*D, t, s) = dsabt;
        MATRIX(*Sigma, s, t) = MATRIX(*Sigma, t, s) = ssabt;
      } else if(dsabt == MATRIX(*D, s, t)) {
        MATRIX(*Sigma, s, t) = MATRIX(*Sigma, t, s) =
          (igraph_integer_t)MATRIX(*Sigma, s, t) + ssabt;
      }
    }
  }
}
