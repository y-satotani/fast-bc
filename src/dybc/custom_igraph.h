#ifndef _CUSTOM_IGRAPH_H_
#define _CUSTOM_IGRAPH_H_

#include <igraph/igraph_types_internal.h>

// took from igraph_math.h of igraph/igraph master

#define IGRAPH_SHORTEST_PATH_EPSILON 1e-10
int igraph_almost_equals(double a, double b);
int igraph_cmp_epsilon(double a, double b);
void igraph_2wheap_update(igraph_2wheap_t* queue,
                          long int         idx,
                          igraph_real_t    val);


#endif // _CUSTOM_IGRAPH_H_
