
#include "dynamic_betweenness.h"

void graph_info_null(graph_info_t* info, igraph_integer_t n) {
}

void graph_info_from(graph_info_t* info,
                     igraph_t* G,
                     igraph_vector_t* l) {
}

int graph_info_is_directed(graph_info_t* info) {
  return 0;
}

int graph_info_is_weighted(graph_info_t* info) {
  return 0;
}

void graph_info_dump(graph_info_t* info, FILE* ostream) {
}

void graph_info_load(graph_info_t* info, FILE* istream) {
}

void insert_edge(graph_info_t* graph, update_info_t* update) {
}

void delete_edge(graph_info_t* graph, update_info_t* update) {
}

void update_weight(graph_info_t* graph, update_info_t* update) {
}

void decrease_weight(graph_info_t* graph, update_info_t* update) {
}

void increase_weight(graph_info_t* graph, update_info_t* update) {
}
