
#include "custom_igraph_math.h"

#include <math.h>

int igraph_almost_equals(double a, double b, double eps) {
  return igraph_cmp_epsilon(a, b, eps) == 0 ? 1 : 0;
}

int igraph_cmp_epsilon(double a, double b, double eps) {
  double diff;
  double abs_diff;

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
