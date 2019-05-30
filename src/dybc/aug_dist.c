
#include "aug_dist.h"
#include <stdio.h>

void aug_dist(igraph_matrix_t* DS, igraph_t* G, const char* weight) {
  igraph_integer_t n = igraph_vcount(G);
  igraph_adjlist_t adjlist_out;
  igraph_dqueue_t queue;
  igraph_vector_bool_t visited;
  igraph_adjlist_init(G, &adjlist_out, IGRAPH_ALL);
  igraph_dqueue_init(&queue, 0);
  igraph_vector_bool_init(&visited, n);

  igraph_matrix_init(DS, n, n);

  for(igraph_integer_t source = 0; source < n; source++) {
    for(igraph_integer_t t = 0; t < n; t++) {
      igraph_vector_bool_set(&visited, t, 0);
      SETDIST(*DS, source, t, IGRAPH_INFINITY);
      SETSIGMA(*DS, source, t, 0);
    }
    igraph_vector_bool_set(&visited, source, 1);
    igraph_dqueue_push(&queue, source);

    while(!igraph_dqueue_empty(&queue)) {
      igraph_integer_t v = igraph_dqueue_pop(&queue);
      igraph_vector_int_t* neis = igraph_adjlist_get(&adjlist_out, v);
      long int nneis = igraph_vector_int_size(neis);

      for(igraph_integer_t i = 0; i < nneis; i++) {
        igraph_integer_t w = VECTOR(*neis)[i];
        igraph_integer_t eid;
        igraph_get_eid(G, &eid, v, w, 0, 1);
        igraph_real_t l = EAN(G, weight, eid);
        if(DIST(*DS, source, w) > DIST(*DS, source, v) + l) {
          if(!igraph_vector_bool_e(&visited, w)) {
            igraph_dqueue_push(&queue, w);
            igraph_vector_bool_set(&visited, w, 1);
          }
          SETDIST(*DS, source, w, DIST(*DS, source, v) + l);
          SETSIGMA(*DS, source, w, 0);
        }
        if(DIST(*DS, source, w) == DIST(*DS, source, v) + l) {
          SETSIGMA(*DS, source, w,
                   SIGMA(*DS, source, w) + SIGMA(*DS, source, v));
        }
      }
    }
  }

  igraph_adjlist_destroy(&adjlist_out);
  igraph_dqueue_destroy(&queue);
  igraph_vector_bool_destroy(&visited);
}

