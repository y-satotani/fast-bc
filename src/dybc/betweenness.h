
#ifndef _BETWEENNESS_H_
#define _BETWEENNESS_H_

#include <igraph.h>

void pairwise_dependency(igraph_matrix_t* Delta, igraph_t* G, const char* weight);

#endif // _BETWEENNESS_H_
