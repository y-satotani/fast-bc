
#ifndef _DYNAMIC_BETWEENNESS_H_
#define _DYNAMIC_BETWEENNESS_H_

#include <igraph/igraph.h>
#include "graph_info.h"
#include "update_info.h"

void insert_edge(graph_info_t* graph, update_info_t* update);
void delete_edge(graph_info_t* graph, update_info_t* update);
void update_weight(graph_info_t* graph, update_info_t* update);
void decrease_weight(graph_info_t* graph, update_info_t* update);
void increase_weight(graph_info_t* graph, update_info_t* update);

#endif // _DYNAMIC_BETWEENNESS_H_
