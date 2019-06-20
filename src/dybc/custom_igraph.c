
#include "custom_igraph.h"

#include <math.h>

int igraph_almost_equals(double a, double b) {
  return igraph_cmp_epsilon(a, b) == 0 ? 1 : 0;
}

int igraph_cmp_epsilon(double a, double b) {
  double diff;
  double abs_diff;
  const double eps = IGRAPH_SHORTEST_PATH_EPSILON;

  if (a == b) {
    /* shortcut, handles infinities */
    return 0;
  }

  diff = a-b;
  abs_diff = fabs(diff);

  if (a == 0 || b == 0 || diff < __DBL_MIN__) {
    /* a or b is zero or both are extremely close to it; relative
     * error is less meaningful here so just compare it with
     * epsilon */
    return abs_diff < (eps * __DBL_MIN__) ? 0 : (diff < 0 ? -1 : 1);
  } else {
    /* use relative error */
    return (abs_diff / (fabs(a) + fabs(b)) < eps) ? 0 : (diff < 0 ? -1 : 1);
  }
}

void igraph_2wheap_update(igraph_2wheap_t* queue,
                          long int         idx,
                          igraph_real_t    val) {
  if(!igraph_2wheap_has_elem(queue, idx)) {
    igraph_2wheap_push_with_index(queue, idx, val);
  } else if(igraph_cmp_epsilon(igraph_2wheap_get(queue, idx), val) < 0) {
    igraph_2wheap_modify(queue, idx, val);
  }
}
