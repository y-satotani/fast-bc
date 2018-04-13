
#ifndef _UPDATE_ON_DELETE_
#define _UPDATE_ON_DELETE_

#include <igraph.h>

void update_on_delete(igraph_matrix_t* D, igraph_matrix_t* Sigma,
                      igraph_integer_t u, igraph_integer_t v);

#endif // _UPDATE_ON_DELETE_
