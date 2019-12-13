
#ifndef _GRAPH_INFO_H_
#define _GRAPH_INFO_H_

#include <igraph/igraph.h>

typedef struct graph_info_t {
  igraph_t* G;
  igraph_vector_t* l;
  igraph_matrix_t* D;
  igraph_matrix_int_t* S;
  igraph_vector_t* B;
} graph_info_t;

void graph_info_null(graph_info_t* info, igraph_integer_t n);
void graph_info_from(graph_info_t* info,
                     igraph_t* G,
                     igraph_vector_t* l);

int graph_info_is_directed(graph_info_t* info);
int graph_info_is_weighted(graph_info_t* info);

void graph_info_dump(graph_info_t* info, FILE* ostream);
void graph_info_load(graph_info_t* info, FILE* istream);

#endif // _GRAPH_INFO_H_
