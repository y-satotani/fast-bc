
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
    igraph_integer_t dsa, ssa;
    dsa = MATRIX(*D, s, a);
    ssa = MATRIX(*Sigma, s, a);

    for(t = s+1; t < N; t++) {
      if(MATRIX(*D, b, t) == IGRAPH_INFINITY) {
        continue;
      }
      igraph_integer_t dbt, dst, sbt, sst;
      dbt = MATRIX(*D, b, t);
      dst = MATRIX(*D, s, t);
      sbt = MATRIX(*Sigma, b, t);
      sst = MATRIX(*Sigma, s, t);

      if(MATRIX(*D, s, t) == IGRAPH_INFINITY ||
         dsa + dbt + 1 < dst) {
        dst = dsa + dbt + 1;
        sst = ssa * sbt;
      } else if(dsa + dbt + 1 == dst) {
        sst = sst + ssa * sbt;
      } else {
      }

      MATRIX(*D, s, t) = MATRIX(*D, t, s) = dst;
      MATRIX(*Sigma, s, t) = MATRIX(*Sigma, t, s) = sst;
    }
  }
}
