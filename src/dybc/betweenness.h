
#ifndef _BETWEENNESS_H_
#define _BETWEENNESS_H_

#include <igraph/igraph.h>
#include <igraph/igraph_types_internal.h>

void pairwise_dependency(igraph_t*        G,
                         igraph_matrix_t* Delta,
                         const char*      weight);

#endif // _BETWEENNESS_H_
