
#ifndef _MINIGRAPH_H_
#define _MINIGRAPH_H_

void make_test_graph(igraph_t* G, const char* weight) {
  igraph_empty(G, 8, 0);

  igraph_integer_t eid;

  igraph_add_edge(G, 0, 1);
  igraph_get_eid(G, &eid, 0, 1, 0, 1);
  SETEAN(G, weight, eid, 1);

  igraph_add_edge(G, 0, 4);
  igraph_get_eid(G, &eid, 0, 4, 0, 1);
  SETEAN(G, weight, eid, 4);

  igraph_add_edge(G, 1, 2);
  igraph_get_eid(G, &eid, 1, 2, 0, 1);
  SETEAN(G, weight, eid, 1);

  igraph_add_edge(G, 1, 3);
  igraph_get_eid(G, &eid, 1, 3, 0, 1);
  SETEAN(G, weight, eid, 2);

  igraph_add_edge(G, 2, 4);
  igraph_get_eid(G, &eid, 2, 4, 0, 1);
  SETEAN(G, weight, eid, 2);

  igraph_add_edge(G, 3, 4);
  igraph_get_eid(G, &eid, 3, 4, 0, 1);
  SETEAN(G, weight, eid, 1);

  igraph_add_edge(G, 4, 5);
  igraph_get_eid(G, &eid, 4, 5, 0, 1);
  SETEAN(G, weight, eid, 1);

  igraph_add_edge(G, 4, 6);
  igraph_get_eid(G, &eid, 4, 6, 0, 1);
  SETEAN(G, weight, eid, 2);

  igraph_add_edge(G, 5, 7);
  igraph_get_eid(G, &eid, 5, 7, 0, 1);
  SETEAN(G, weight, eid, 3);

  igraph_add_edge(G, 6, 7);
  igraph_get_eid(G, &eid, 6, 7, 0, 1);
  SETEAN(G, weight, eid, 2);
}

#endif
