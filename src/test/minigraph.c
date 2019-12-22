
#include "minigraph.h"

void make_test_graph(igraph_t* G, igraph_vector_t* weight) {
  igraph_empty(G, 8, 0);
  igraph_vector_init(weight, 10);

  igraph_integer_t eid;

  igraph_add_edge(G, 0, 1);
  igraph_get_eid(G, &eid, 0, 1, 0, 1);
  igraph_vector_set(weight, eid, 1);

  igraph_add_edge(G, 0, 4);
  igraph_get_eid(G, &eid, 0, 4, 0, 1);
  igraph_vector_set(weight, eid, 4);

  igraph_add_edge(G, 1, 2);
  igraph_get_eid(G, &eid, 1, 2, 0, 1);
  igraph_vector_set(weight, eid, 1);

  igraph_add_edge(G, 1, 3);
  igraph_get_eid(G, &eid, 1, 3, 0, 1);
  igraph_vector_set(weight, eid, 2);

  igraph_add_edge(G, 2, 4);
  igraph_get_eid(G, &eid, 2, 4, 0, 1);
  igraph_vector_set(weight, eid, 2);

  igraph_add_edge(G, 3, 4);
  igraph_get_eid(G, &eid, 3, 4, 0, 1);
  igraph_vector_set(weight, eid, 1);

  igraph_add_edge(G, 4, 5);
  igraph_get_eid(G, &eid, 4, 5, 0, 1);
  igraph_vector_set(weight, eid, 1);

  igraph_add_edge(G, 4, 6);
  igraph_get_eid(G, &eid, 4, 6, 0, 1);
  igraph_vector_set(weight, eid, 2);

  igraph_add_edge(G, 5, 7);
  igraph_get_eid(G, &eid, 5, 7, 0, 1);
  igraph_vector_set(weight, eid, 3);

  igraph_add_edge(G, 6, 7);
  igraph_get_eid(G, &eid, 6, 7, 0, 1);
  igraph_vector_set(weight, eid, 2);
}

void make_less_graph_and_edge(igraph_t* G,
                              igraph_integer_t* v,
                              igraph_integer_t* w,
                              igraph_real_t* c,
                              igraph_vector_t* weight) {
  igraph_empty(G, 8, 0);
  igraph_vector_init(weight, 0);

  igraph_integer_t eid;

  igraph_add_edge(G, 0, 3);
  igraph_get_eid(G, &eid, 0, 3, 0, 1);
  igraph_vector_push_back(weight, 3);
  //SETEAN/g(G, weight, eid, 3);

  igraph_add_edge(G, 1, 2);
  igraph_get_eid(G, &eid, 1, 2, 0, 1);
  igraph_vector_push_back(weight, 1);
  //SETEAN/g(G, weight, eid, 1);

  igraph_add_edge(G, 1, 6);
  igraph_get_eid(G, &eid, 1, 6, 0, 1);
  igraph_vector_push_back(weight, 2);
  //SETEAN/g(G, weight, eid, 2);

  igraph_add_edge(G, 2, 3);
  igraph_get_eid(G, &eid, 2, 3, 0, 1);
  igraph_vector_push_back(weight, 3);
  //SETEAN/g(G, weight, eid, 3);

  igraph_add_edge(G, 2, 5);
  igraph_get_eid(G, &eid, 2, 5, 0, 1);
  igraph_vector_push_back(weight, 2);
  //SETEAN/g(G, weight, eid, 2);

  igraph_add_edge(G, 4, 5);
  igraph_get_eid(G, &eid, 4, 5, 0, 1);
  igraph_vector_push_back(weight, 1);
  //SETEAN/g(G, weight, eid, 1);

  igraph_add_edge(G, 5, 6);
  igraph_get_eid(G, &eid, 5, 6, 0, 1);
  igraph_vector_push_back(weight, 1);
  //SETEAN/g(G, weight, eid, 1);

  igraph_add_edge(G, 6, 7);
  igraph_get_eid(G, &eid, 6, 7, 0, 1);
  igraph_vector_push_back(weight, 3);
  //SETEAN/g(G, weight, eid, 3);

  *v = 4;
  *w = 7;
  *c = 2.;
}

void make_more_graph_and_edge(igraph_t* G,
                              igraph_integer_t* v,
                              igraph_integer_t* w,
                              igraph_real_t* c,
                              igraph_vector_t* weight) {
  igraph_empty(G, 8, 0);
  igraph_vector_init(weight, 0);

  igraph_integer_t eid;

  igraph_add_edge(G, 0, 3);
  igraph_get_eid(G, &eid, 0, 3, 0, 1);
  igraph_vector_push_back(weight, 3);
  //SETEAN(G, weight, eid, 3);

  igraph_add_edge(G, 1, 2);
  igraph_get_eid(G, &eid, 1, 2, 0, 1);
  igraph_vector_push_back(weight, 1);
  //SETEAN(G, weight, eid, 1);

  igraph_add_edge(G, 1, 6);
  igraph_get_eid(G, &eid, 1, 6, 0, 1);
  igraph_vector_push_back(weight, 2);
  //SETEAN(G, weight, eid, 2);

  igraph_add_edge(G, 2, 3);
  igraph_get_eid(G, &eid, 2, 3, 0, 1);
  igraph_vector_push_back(weight, 3);
  //SETEAN(G, weight, eid, 3);

  igraph_add_edge(G, 2, 5);
  igraph_get_eid(G, &eid, 2, 5, 0, 1);
  igraph_vector_push_back(weight, 2);
  //SETEAN(G, weight, eid, 2);

  igraph_add_edge(G, 4, 5);
  igraph_get_eid(G, &eid, 4, 5, 0, 1);
  igraph_vector_push_back(weight, 1);
  //SETEAN(G, weight, eid, 1);

  igraph_add_edge(G, 5, 6);
  igraph_get_eid(G, &eid, 5, 6, 0, 1);
  igraph_vector_push_back(weight, 1);
  //SETEAN(G, weight, eid, 1);

  igraph_add_edge(G, 6, 7);
  igraph_get_eid(G, &eid, 6, 7, 0, 1);
  igraph_vector_push_back(weight, 3);
  //SETEAN(G, weight, eid, 3);

  igraph_add_edge(G, 4, 7);
  igraph_get_eid(G, &eid, 4, 7, 0, 1);
  igraph_vector_push_back(weight, 2);
  //SETEAN(G, weight, eid, 2);

  *v = 4;
  *w = 7;
  *c = IGRAPH_INFINITY;
}

int check_aug_dist_of_minigraph(igraph_matrix_t* D,
                                igraph_matrix_int_t* Sigma) {
  assert(MATRIX(*D, 0, 0) == 0);
  assert(MATRIX(*D, 0, 1) == 1);
  assert(MATRIX(*D, 0, 2) == 2);
  assert(MATRIX(*D, 0, 3) == 3);
  assert(MATRIX(*D, 0, 4) == 4);
  assert(MATRIX(*D, 0, 5) == 5);
  assert(MATRIX(*D, 0, 6) == 6);
  assert(MATRIX(*D, 0, 7) == 8);
  assert(MATRIX(*D, 1, 0) == 1);
  assert(MATRIX(*D, 1, 1) == 0);
  assert(MATRIX(*D, 1, 2) == 1);
  assert(MATRIX(*D, 1, 3) == 2);
  assert(MATRIX(*D, 1, 4) == 3);
  assert(MATRIX(*D, 1, 5) == 4);
  assert(MATRIX(*D, 1, 6) == 5);
  assert(MATRIX(*D, 1, 7) == 7);
  assert(MATRIX(*D, 2, 0) == 2);
  assert(MATRIX(*D, 2, 1) == 1);
  assert(MATRIX(*D, 2, 2) == 0);
  assert(MATRIX(*D, 2, 3) == 3);
  assert(MATRIX(*D, 2, 4) == 2);
  assert(MATRIX(*D, 2, 5) == 3);
  assert(MATRIX(*D, 2, 6) == 4);
  assert(MATRIX(*D, 2, 7) == 6);
  assert(MATRIX(*D, 3, 0) == 3);
  assert(MATRIX(*D, 3, 1) == 2);
  assert(MATRIX(*D, 3, 2) == 3);
  assert(MATRIX(*D, 3, 3) == 0);
  assert(MATRIX(*D, 3, 4) == 1);
  assert(MATRIX(*D, 3, 5) == 2);
  assert(MATRIX(*D, 3, 6) == 3);
  assert(MATRIX(*D, 3, 7) == 5);
  assert(MATRIX(*D, 4, 0) == 4);
  assert(MATRIX(*D, 4, 1) == 3);
  assert(MATRIX(*D, 4, 2) == 2);
  assert(MATRIX(*D, 4, 3) == 1);
  assert(MATRIX(*D, 4, 4) == 0);
  assert(MATRIX(*D, 4, 5) == 1);
  assert(MATRIX(*D, 4, 6) == 2);
  assert(MATRIX(*D, 4, 7) == 4);
  assert(MATRIX(*D, 5, 0) == 5);
  assert(MATRIX(*D, 5, 1) == 4);
  assert(MATRIX(*D, 5, 2) == 3);
  assert(MATRIX(*D, 5, 3) == 2);
  assert(MATRIX(*D, 5, 4) == 1);
  assert(MATRIX(*D, 5, 5) == 0);
  assert(MATRIX(*D, 5, 6) == 3);
  assert(MATRIX(*D, 5, 7) == 3);
  assert(MATRIX(*D, 6, 0) == 6);
  assert(MATRIX(*D, 6, 1) == 5);
  assert(MATRIX(*D, 6, 2) == 4);
  assert(MATRIX(*D, 6, 3) == 3);
  assert(MATRIX(*D, 6, 4) == 2);
  assert(MATRIX(*D, 6, 5) == 3);
  assert(MATRIX(*D, 6, 6) == 0);
  assert(MATRIX(*D, 6, 7) == 2);
  assert(MATRIX(*D, 7, 0) == 8);
  assert(MATRIX(*D, 7, 1) == 7);
  assert(MATRIX(*D, 7, 2) == 6);
  assert(MATRIX(*D, 7, 3) == 5);
  assert(MATRIX(*D, 7, 4) == 4);
  assert(MATRIX(*D, 7, 5) == 3);
  assert(MATRIX(*D, 7, 6) == 2);
  assert(MATRIX(*D, 7, 7) == 0);

  assert(MATRIX(*Sigma, 0, 0) == 1);
  assert(MATRIX(*Sigma, 0, 1) == 1);
  assert(MATRIX(*Sigma, 0, 2) == 1);
  assert(MATRIX(*Sigma, 0, 3) == 1);
  assert(MATRIX(*Sigma, 0, 4) == 3);
  assert(MATRIX(*Sigma, 0, 5) == 3);
  assert(MATRIX(*Sigma, 0, 6) == 3);
  assert(MATRIX(*Sigma, 0, 7) == 6);
  assert(MATRIX(*Sigma, 1, 0) == 1);
  assert(MATRIX(*Sigma, 1, 1) == 1);
  assert(MATRIX(*Sigma, 1, 2) == 1);
  assert(MATRIX(*Sigma, 1, 3) == 1);
  assert(MATRIX(*Sigma, 1, 4) == 2);
  assert(MATRIX(*Sigma, 1, 5) == 2);
  assert(MATRIX(*Sigma, 1, 6) == 2);
  assert(MATRIX(*Sigma, 1, 7) == 4);
  assert(MATRIX(*Sigma, 2, 0) == 1);
  assert(MATRIX(*Sigma, 2, 1) == 1);
  assert(MATRIX(*Sigma, 2, 2) == 1);
  assert(MATRIX(*Sigma, 2, 3) == 2);
  assert(MATRIX(*Sigma, 2, 4) == 1);
  assert(MATRIX(*Sigma, 2, 5) == 1);
  assert(MATRIX(*Sigma, 2, 6) == 1);
  assert(MATRIX(*Sigma, 2, 7) == 2);
  assert(MATRIX(*Sigma, 3, 0) == 1);
  assert(MATRIX(*Sigma, 3, 1) == 1);
  assert(MATRIX(*Sigma, 3, 2) == 2);
  assert(MATRIX(*Sigma, 3, 3) == 1);
  assert(MATRIX(*Sigma, 3, 4) == 1);
  assert(MATRIX(*Sigma, 3, 5) == 1);
  assert(MATRIX(*Sigma, 3, 6) == 1);
  assert(MATRIX(*Sigma, 3, 7) == 2);
  assert(MATRIX(*Sigma, 4, 0) == 3);
  assert(MATRIX(*Sigma, 4, 1) == 2);
  assert(MATRIX(*Sigma, 4, 2) == 1);
  assert(MATRIX(*Sigma, 4, 3) == 1);
  assert(MATRIX(*Sigma, 4, 4) == 1);
  assert(MATRIX(*Sigma, 4, 5) == 1);
  assert(MATRIX(*Sigma, 4, 6) == 1);
  assert(MATRIX(*Sigma, 4, 7) == 2);
  assert(MATRIX(*Sigma, 5, 0) == 3);
  assert(MATRIX(*Sigma, 5, 1) == 2);
  assert(MATRIX(*Sigma, 5, 2) == 1);
  assert(MATRIX(*Sigma, 5, 3) == 1);
  assert(MATRIX(*Sigma, 5, 4) == 1);
  assert(MATRIX(*Sigma, 5, 5) == 1);
  assert(MATRIX(*Sigma, 5, 6) == 1);
  assert(MATRIX(*Sigma, 5, 7) == 1);
  assert(MATRIX(*Sigma, 6, 0) == 3);
  assert(MATRIX(*Sigma, 6, 1) == 2);
  assert(MATRIX(*Sigma, 6, 2) == 1);
  assert(MATRIX(*Sigma, 6, 3) == 1);
  assert(MATRIX(*Sigma, 6, 4) == 1);
  assert(MATRIX(*Sigma, 6, 5) == 1);
  assert(MATRIX(*Sigma, 6, 6) == 1);
  assert(MATRIX(*Sigma, 6, 7) == 1);
  assert(MATRIX(*Sigma, 7, 0) == 6);
  assert(MATRIX(*Sigma, 7, 1) == 4);
  assert(MATRIX(*Sigma, 7, 2) == 2);
  assert(MATRIX(*Sigma, 7, 3) == 2);
  assert(MATRIX(*Sigma, 7, 4) == 2);
  assert(MATRIX(*Sigma, 7, 5) == 1);
  assert(MATRIX(*Sigma, 7, 6) == 1);
  assert(MATRIX(*Sigma, 7, 7) == 1);

  return 1;
}

int check_betweenness_of_minigraph(igraph_vector_t* vec) {
  assert(fabs(VECTOR(*vec)[0] - 0.000000) < 1e-3);
  assert(fabs(VECTOR(*vec)[1] - 5.166667) < 1e-3);
  assert(fabs(VECTOR(*vec)[2] - 3.333333) < 1e-3);
  assert(fabs(VECTOR(*vec)[3] - 3.333333) < 1e-3);
  assert(fabs(VECTOR(*vec)[4] - 13.500000) < 1e-3);
  assert(fabs(VECTOR(*vec)[5] - 2.500000) < 1e-3);
  assert(fabs(VECTOR(*vec)[6] - 2.500000) < 1e-3);
  assert(fabs(VECTOR(*vec)[7] - 0.000000) < 1e-3);

  return 1;
}

int check_aug_dist_of_less(igraph_matrix_t* D,
                           igraph_matrix_int_t* Sigma) {
  assert(MATRIX(*D, 0, 0) == 0);
  assert(MATRIX(*D, 0, 1) == 7);
  assert(MATRIX(*D, 0, 2) == 6);
  assert(MATRIX(*D, 0, 3) == 3);
  assert(MATRIX(*D, 0, 4) == 9);
  assert(MATRIX(*D, 0, 5) == 8);
  assert(MATRIX(*D, 0, 6) == 9);
  assert(MATRIX(*D, 0, 7) == 12);
  assert(MATRIX(*D, 1, 0) == 7);
  assert(MATRIX(*D, 1, 1) == 0);
  assert(MATRIX(*D, 1, 2) == 1);
  assert(MATRIX(*D, 1, 3) == 4);
  assert(MATRIX(*D, 1, 4) == 4);
  assert(MATRIX(*D, 1, 5) == 3);
  assert(MATRIX(*D, 1, 6) == 2);
  assert(MATRIX(*D, 1, 7) == 5);
  assert(MATRIX(*D, 2, 0) == 6);
  assert(MATRIX(*D, 2, 1) == 1);
  assert(MATRIX(*D, 2, 2) == 0);
  assert(MATRIX(*D, 2, 3) == 3);
  assert(MATRIX(*D, 2, 4) == 3);
  assert(MATRIX(*D, 2, 5) == 2);
  assert(MATRIX(*D, 2, 6) == 3);
  assert(MATRIX(*D, 2, 7) == 6);
  assert(MATRIX(*D, 3, 0) == 3);
  assert(MATRIX(*D, 3, 1) == 4);
  assert(MATRIX(*D, 3, 2) == 3);
  assert(MATRIX(*D, 3, 3) == 0);
  assert(MATRIX(*D, 3, 4) == 6);
  assert(MATRIX(*D, 3, 5) == 5);
  assert(MATRIX(*D, 3, 6) == 6);
  assert(MATRIX(*D, 3, 7) == 9);
  assert(MATRIX(*D, 4, 0) == 9);
  assert(MATRIX(*D, 4, 1) == 4);
  assert(MATRIX(*D, 4, 2) == 3);
  assert(MATRIX(*D, 4, 3) == 6);
  assert(MATRIX(*D, 4, 4) == 0);
  assert(MATRIX(*D, 4, 5) == 1);
  assert(MATRIX(*D, 4, 6) == 2);
  assert(MATRIX(*D, 4, 7) == 5);
  assert(MATRIX(*D, 5, 0) == 8);
  assert(MATRIX(*D, 5, 1) == 3);
  assert(MATRIX(*D, 5, 2) == 2);
  assert(MATRIX(*D, 5, 3) == 5);
  assert(MATRIX(*D, 5, 4) == 1);
  assert(MATRIX(*D, 5, 5) == 0);
  assert(MATRIX(*D, 5, 6) == 1);
  assert(MATRIX(*D, 5, 7) == 4);
  assert(MATRIX(*D, 6, 0) == 9);
  assert(MATRIX(*D, 6, 1) == 2);
  assert(MATRIX(*D, 6, 2) == 3);
  assert(MATRIX(*D, 6, 3) == 6);
  assert(MATRIX(*D, 6, 4) == 2);
  assert(MATRIX(*D, 6, 5) == 1);
  assert(MATRIX(*D, 6, 6) == 0);
  assert(MATRIX(*D, 6, 7) == 3);
  assert(MATRIX(*D, 7, 0) == 12);
  assert(MATRIX(*D, 7, 1) == 5);
  assert(MATRIX(*D, 7, 2) == 6);
  assert(MATRIX(*D, 7, 3) == 9);
  assert(MATRIX(*D, 7, 4) == 5);
  assert(MATRIX(*D, 7, 5) == 4);
  assert(MATRIX(*D, 7, 6) == 3);
  assert(MATRIX(*D, 7, 7) == 0);

  assert(MATRIX(*Sigma, 0, 0) == 1);
  assert(MATRIX(*Sigma, 0, 1) == 1);
  assert(MATRIX(*Sigma, 0, 2) == 1);
  assert(MATRIX(*Sigma, 0, 3) == 1);
  assert(MATRIX(*Sigma, 0, 4) == 1);
  assert(MATRIX(*Sigma, 0, 5) == 1);
  assert(MATRIX(*Sigma, 0, 6) == 2);
  assert(MATRIX(*Sigma, 0, 7) == 2);
  assert(MATRIX(*Sigma, 1, 0) == 1);
  assert(MATRIX(*Sigma, 1, 1) == 1);
  assert(MATRIX(*Sigma, 1, 2) == 1);
  assert(MATRIX(*Sigma, 1, 3) == 1);
  assert(MATRIX(*Sigma, 1, 4) == 2);
  assert(MATRIX(*Sigma, 1, 5) == 2);
  assert(MATRIX(*Sigma, 1, 6) == 1);
  assert(MATRIX(*Sigma, 1, 7) == 1);
  assert(MATRIX(*Sigma, 2, 0) == 1);
  assert(MATRIX(*Sigma, 2, 1) == 1);
  assert(MATRIX(*Sigma, 2, 2) == 1);
  assert(MATRIX(*Sigma, 2, 3) == 1);
  assert(MATRIX(*Sigma, 2, 4) == 1);
  assert(MATRIX(*Sigma, 2, 5) == 1);
  assert(MATRIX(*Sigma, 2, 6) == 2);
  assert(MATRIX(*Sigma, 2, 7) == 2);
  assert(MATRIX(*Sigma, 3, 0) == 1);
  assert(MATRIX(*Sigma, 3, 1) == 1);
  assert(MATRIX(*Sigma, 3, 2) == 1);
  assert(MATRIX(*Sigma, 3, 3) == 1);
  assert(MATRIX(*Sigma, 3, 4) == 1);
  assert(MATRIX(*Sigma, 3, 5) == 1);
  assert(MATRIX(*Sigma, 3, 6) == 2);
  assert(MATRIX(*Sigma, 3, 7) == 2);
  assert(MATRIX(*Sigma, 4, 0) == 1);
  assert(MATRIX(*Sigma, 4, 1) == 2);
  assert(MATRIX(*Sigma, 4, 2) == 1);
  assert(MATRIX(*Sigma, 4, 3) == 1);
  assert(MATRIX(*Sigma, 4, 4) == 1);
  assert(MATRIX(*Sigma, 4, 5) == 1);
  assert(MATRIX(*Sigma, 4, 6) == 1);
  assert(MATRIX(*Sigma, 4, 7) == 1);
  assert(MATRIX(*Sigma, 5, 0) == 1);
  assert(MATRIX(*Sigma, 5, 1) == 2);
  assert(MATRIX(*Sigma, 5, 2) == 1);
  assert(MATRIX(*Sigma, 5, 3) == 1);
  assert(MATRIX(*Sigma, 5, 4) == 1);
  assert(MATRIX(*Sigma, 5, 5) == 1);
  assert(MATRIX(*Sigma, 5, 6) == 1);
  assert(MATRIX(*Sigma, 5, 7) == 1);
  assert(MATRIX(*Sigma, 6, 0) == 2);
  assert(MATRIX(*Sigma, 6, 1) == 1);
  assert(MATRIX(*Sigma, 6, 2) == 2);
  assert(MATRIX(*Sigma, 6, 3) == 2);
  assert(MATRIX(*Sigma, 6, 4) == 1);
  assert(MATRIX(*Sigma, 6, 5) == 1);
  assert(MATRIX(*Sigma, 6, 6) == 1);
  assert(MATRIX(*Sigma, 6, 7) == 1);
  assert(MATRIX(*Sigma, 7, 0) == 2);
  assert(MATRIX(*Sigma, 7, 1) == 1);
  assert(MATRIX(*Sigma, 7, 2) == 2);
  assert(MATRIX(*Sigma, 7, 3) == 2);
  assert(MATRIX(*Sigma, 7, 4) == 1);
  assert(MATRIX(*Sigma, 7, 5) == 1);
  assert(MATRIX(*Sigma, 7, 6) == 1);
  assert(MATRIX(*Sigma, 7, 7) == 1);

  return 1;
}

int check_aug_dist_of_more(igraph_matrix_t* D,
                           igraph_matrix_int_t* Sigma) {
  assert(MATRIX(*D, 0, 0) == 0);
  assert(MATRIX(*D, 0, 1) == 7);
  assert(MATRIX(*D, 0, 2) == 6);
  assert(MATRIX(*D, 0, 3) == 3);
  assert(MATRIX(*D, 0, 4) == 9);
  assert(MATRIX(*D, 0, 5) == 8);
  assert(MATRIX(*D, 0, 6) == 9);
  assert(MATRIX(*D, 0, 7) == 11);
  assert(MATRIX(*D, 1, 0) == 7);
  assert(MATRIX(*D, 1, 1) == 0);
  assert(MATRIX(*D, 1, 2) == 1);
  assert(MATRIX(*D, 1, 3) == 4);
  assert(MATRIX(*D, 1, 4) == 4);
  assert(MATRIX(*D, 1, 5) == 3);
  assert(MATRIX(*D, 1, 6) == 2);
  assert(MATRIX(*D, 1, 7) == 5);
  assert(MATRIX(*D, 2, 0) == 6);
  assert(MATRIX(*D, 2, 1) == 1);
  assert(MATRIX(*D, 2, 2) == 0);
  assert(MATRIX(*D, 2, 3) == 3);
  assert(MATRIX(*D, 2, 4) == 3);
  assert(MATRIX(*D, 2, 5) == 2);
  assert(MATRIX(*D, 2, 6) == 3);
  assert(MATRIX(*D, 2, 7) == 5);
  assert(MATRIX(*D, 3, 0) == 3);
  assert(MATRIX(*D, 3, 1) == 4);
  assert(MATRIX(*D, 3, 2) == 3);
  assert(MATRIX(*D, 3, 3) == 0);
  assert(MATRIX(*D, 3, 4) == 6);
  assert(MATRIX(*D, 3, 5) == 5);
  assert(MATRIX(*D, 3, 6) == 6);
  assert(MATRIX(*D, 3, 7) == 8);
  assert(MATRIX(*D, 4, 0) == 9);
  assert(MATRIX(*D, 4, 1) == 4);
  assert(MATRIX(*D, 4, 2) == 3);
  assert(MATRIX(*D, 4, 3) == 6);
  assert(MATRIX(*D, 4, 4) == 0);
  assert(MATRIX(*D, 4, 5) == 1);
  assert(MATRIX(*D, 4, 6) == 2);
  assert(MATRIX(*D, 4, 7) == 2);
  assert(MATRIX(*D, 5, 0) == 8);
  assert(MATRIX(*D, 5, 1) == 3);
  assert(MATRIX(*D, 5, 2) == 2);
  assert(MATRIX(*D, 5, 3) == 5);
  assert(MATRIX(*D, 5, 4) == 1);
  assert(MATRIX(*D, 5, 5) == 0);
  assert(MATRIX(*D, 5, 6) == 1);
  assert(MATRIX(*D, 5, 7) == 3);
  assert(MATRIX(*D, 6, 0) == 9);
  assert(MATRIX(*D, 6, 1) == 2);
  assert(MATRIX(*D, 6, 2) == 3);
  assert(MATRIX(*D, 6, 3) == 6);
  assert(MATRIX(*D, 6, 4) == 2);
  assert(MATRIX(*D, 6, 5) == 1);
  assert(MATRIX(*D, 6, 6) == 0);
  assert(MATRIX(*D, 6, 7) == 3);
  assert(MATRIX(*D, 7, 0) == 11);
  assert(MATRIX(*D, 7, 1) == 5);
  assert(MATRIX(*D, 7, 2) == 5);
  assert(MATRIX(*D, 7, 3) == 8);
  assert(MATRIX(*D, 7, 4) == 2);
  assert(MATRIX(*D, 7, 5) == 3);
  assert(MATRIX(*D, 7, 6) == 3);
  assert(MATRIX(*D, 7, 7) == 0);

  assert(MATRIX(*Sigma, 0, 0) == 1);
  assert(MATRIX(*Sigma, 0, 1) == 1);
  assert(MATRIX(*Sigma, 0, 2) == 1);
  assert(MATRIX(*Sigma, 0, 3) == 1);
  assert(MATRIX(*Sigma, 0, 4) == 1);
  assert(MATRIX(*Sigma, 0, 5) == 1);
  assert(MATRIX(*Sigma, 0, 6) == 2);
  assert(MATRIX(*Sigma, 0, 7) == 1);
  assert(MATRIX(*Sigma, 1, 0) == 1);
  assert(MATRIX(*Sigma, 1, 1) == 1);
  assert(MATRIX(*Sigma, 1, 2) == 1);
  assert(MATRIX(*Sigma, 1, 3) == 1);
  assert(MATRIX(*Sigma, 1, 4) == 2);
  assert(MATRIX(*Sigma, 1, 5) == 2);
  assert(MATRIX(*Sigma, 1, 6) == 1);
  assert(MATRIX(*Sigma, 1, 7) == 1);
  assert(MATRIX(*Sigma, 2, 0) == 1);
  assert(MATRIX(*Sigma, 2, 1) == 1);
  assert(MATRIX(*Sigma, 2, 2) == 1);
  assert(MATRIX(*Sigma, 2, 3) == 1);
  assert(MATRIX(*Sigma, 2, 4) == 1);
  assert(MATRIX(*Sigma, 2, 5) == 1);
  assert(MATRIX(*Sigma, 2, 6) == 2);
  assert(MATRIX(*Sigma, 2, 7) == 1);
  assert(MATRIX(*Sigma, 3, 0) == 1);
  assert(MATRIX(*Sigma, 3, 1) == 1);
  assert(MATRIX(*Sigma, 3, 2) == 1);
  assert(MATRIX(*Sigma, 3, 3) == 1);
  assert(MATRIX(*Sigma, 3, 4) == 1);
  assert(MATRIX(*Sigma, 3, 5) == 1);
  assert(MATRIX(*Sigma, 3, 6) == 2);
  assert(MATRIX(*Sigma, 3, 7) == 1);
  assert(MATRIX(*Sigma, 4, 0) == 1);
  assert(MATRIX(*Sigma, 4, 1) == 2);
  assert(MATRIX(*Sigma, 4, 2) == 1);
  assert(MATRIX(*Sigma, 4, 3) == 1);
  assert(MATRIX(*Sigma, 4, 4) == 1);
  assert(MATRIX(*Sigma, 4, 5) == 1);
  assert(MATRIX(*Sigma, 4, 6) == 1);
  assert(MATRIX(*Sigma, 4, 7) == 1);
  assert(MATRIX(*Sigma, 5, 0) == 1);
  assert(MATRIX(*Sigma, 5, 1) == 2);
  assert(MATRIX(*Sigma, 5, 2) == 1);
  assert(MATRIX(*Sigma, 5, 3) == 1);
  assert(MATRIX(*Sigma, 5, 4) == 1);
  assert(MATRIX(*Sigma, 5, 5) == 1);
  assert(MATRIX(*Sigma, 5, 6) == 1);
  assert(MATRIX(*Sigma, 5, 7) == 1);
  assert(MATRIX(*Sigma, 6, 0) == 2);
  assert(MATRIX(*Sigma, 6, 1) == 1);
  assert(MATRIX(*Sigma, 6, 2) == 2);
  assert(MATRIX(*Sigma, 6, 3) == 2);
  assert(MATRIX(*Sigma, 6, 4) == 1);
  assert(MATRIX(*Sigma, 6, 5) == 1);
  assert(MATRIX(*Sigma, 6, 6) == 1);
  assert(MATRIX(*Sigma, 6, 7) == 1);
  assert(MATRIX(*Sigma, 7, 0) == 1);
  assert(MATRIX(*Sigma, 7, 1) == 1);
  assert(MATRIX(*Sigma, 7, 2) == 1);
  assert(MATRIX(*Sigma, 7, 3) == 1);
  assert(MATRIX(*Sigma, 7, 4) == 1);
  assert(MATRIX(*Sigma, 7, 5) == 1);
  assert(MATRIX(*Sigma, 7, 6) == 1);
  assert(MATRIX(*Sigma, 7, 7) == 1);

  return 1;
}

