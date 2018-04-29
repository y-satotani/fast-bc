
#include "arg_dist.h"

void arg_dist(igraph_matrix_t* D,
              igraph_matrix_t* Sigma,
              igraph_t* G) {

  igraph_integer_t n = igraph_vcount(G);
  igraph_adjlist_t adjlist_out;
  igraph_dqueue_t queue;
  igraph_adjlist_init(G, &adjlist_out, IGRAPH_ALL);
  igraph_dqueue_init(&queue, 0);

  igraph_matrix_init(D, n, n);
  igraph_matrix_init(Sigma, n, n);

  for(igraph_integer_t source = 0; source < n; source++) {
    for(igraph_integer_t t = 0; t < n; t++) {
      MATRIX(*D, source, t) = MATRIX(*D, t, source) = IGRAPH_INFINITY;
      MATRIX(*Sigma, source, t) = MATRIX(*Sigma, t, source) = 0;
    }
    MATRIX(*D, source, source) = 0;
    MATRIX(*Sigma, source, source) = 1;
    igraph_dqueue_push(&queue, source);

    while(!igraph_dqueue_empty(&queue)) {
      igraph_integer_t v = igraph_dqueue_pop(&queue);
      igraph_vector_int_t* neis = igraph_adjlist_get(&adjlist_out, v);
      long int nneis = igraph_vector_int_size(neis);
      for(igraph_integer_t i = 0; i < nneis; i++) {
        igraph_integer_t w = VECTOR(*neis)[i];
        if(MATRIX(*D, source, w) == IGRAPH_INFINITY) {
          igraph_dqueue_push(&queue, w);
          MATRIX(*D, source, w) = MATRIX(*D, w, source) =
            MATRIX(*D, source, v) + 1;
        }
        if(MATRIX(*D, source, w) == MATRIX(*D, source, v) + 1) {
          MATRIX(*Sigma, source, w) = MATRIX(*Sigma, w, source) =
            MATRIX(*Sigma, source, w) + MATRIX(*Sigma, source, v);
        }
      }
    }
  }

  igraph_adjlist_destroy(&adjlist_out);
  igraph_dqueue_destroy(&queue);
}
