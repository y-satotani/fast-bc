#ifndef _CUSTOM_IGRAPH_MATH_H_
#define _CUSTOM_IGRAPH_MATH_H_

// took from igraph_math.h of igraph/igraph master

#define IGRAPH_SHORTEST_PATH_EPSILON 1e-10
int igraph_almost_equals(double a, double b, double eps);
int igraph_cmp_epsilon(double a, double b, double eps);

#endif // _CUSTOM_IGRAPH_MATH_H_
