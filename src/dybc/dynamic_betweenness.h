
#ifndef _DYNAMIC_BETWEENNESS_H_
#define _DYNAMIC_BETWEENNESS_H_

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

typedef struct update_info_t {
  igraph_integer_t u;
  igraph_integer_t v;
  igraph_integer_t eid;
  igraph_real_t weight_before;
  igraph_real_t weight_after;
} update_info_t;

void insert_edge(graph_info_t* graph, update_info_t* update);
void delete_edge(graph_info_t* graph, update_info_t* update);
void update_weight(graph_info_t* graph, update_info_t* update);
void decrease_weight(graph_info_t* graph, update_info_t* update);
void increase_weight(graph_info_t* graph, update_info_t* update);

#endif // _DYNAMIC_BETWEENNESS_H_
