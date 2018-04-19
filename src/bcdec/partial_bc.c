
#include "partial_bc.h"

void partial_bc_bfs(igraph_t* G, igraph_matrix_t* Delta, igraph_vs_t vs) {

  igraph_integer_t n = igraph_vcount(G);
  igraph_adjlist_t adjlist_in, adjlist_out;
  igraph_vector_int_t dist, sigma;
  igraph_dqueue_t queue;
  igraph_stack_t stack;
  igraph_adjlist_init(G, &adjlist_out, IGRAPH_ALL);
  igraph_adjlist_init_empty(&adjlist_in, n);
  igraph_vector_int_init(&dist, n);
  igraph_vector_int_init(&sigma, n);
  igraph_dqueue_init(&queue, 0);
  igraph_stack_init(&stack, 0);

  igraph_vit_t vit;
  igraph_vit_create(G, vs, &vit);

  while(!IGRAPH_VIT_END(vit)) {
    igraph_integer_t source = IGRAPH_VIT_GET(vit);
    IGRAPH_VIT_NEXT(vit);

    igraph_adjlist_clear(&adjlist_in);
    for(igraph_integer_t t = 0; t < n; t++) {
      VECTOR(dist)[t] = -1;
      VECTOR(sigma)[t] = 0;
    }
    VECTOR(sigma)[source] = 1;
    VECTOR(dist)[source] = 0;
    igraph_dqueue_push(&queue, source);

    while(!igraph_dqueue_empty(&queue)) {
      igraph_integer_t v = igraph_dqueue_pop(&queue);
      igraph_vector_int_t* neis = igraph_adjlist_get(&adjlist_out, v);
      long int nneis = igraph_vector_int_size(neis);
      igraph_stack_push(&stack, v);
      for(igraph_integer_t i = 0; i < nneis; i++) {
        igraph_integer_t w = VECTOR(*neis)[i];
        if(VECTOR(dist)[w] < 0) {
          igraph_dqueue_push(&queue, w);
          VECTOR(dist)[w] = VECTOR(dist)[v] + 1;
        }
        if(VECTOR(dist)[w] == VECTOR(dist)[v] + 1) {
          VECTOR(sigma)[w] += VECTOR(sigma)[v];
          igraph_vector_int_push_back
            (igraph_adjlist_get(&adjlist_in, w), v);
        }
      }
    }

    for(igraph_integer_t v = 0; v < n; v++)
      MATRIX(*Delta, source, v) = 0;
    while(!igraph_stack_empty(&stack)) {
      igraph_integer_t w = igraph_stack_pop(&stack);
      igraph_vector_int_t *neis = igraph_adjlist_get(&adjlist_in, w);
      long int nneis = igraph_vector_int_size(neis);
      for(igraph_integer_t i = 0; i < nneis; i++) {
        igraph_integer_t v = VECTOR(*neis)[i];
        if(v != source)
          MATRIX(*Delta, source, v) +=
            (igraph_real_t) VECTOR(sigma)[v] / VECTOR(sigma)[w] *
            (1 + MATRIX(*Delta, source, w));
      }
    }
  }

  igraph_adjlist_destroy(&adjlist_out);
  igraph_adjlist_destroy(&adjlist_in);
  igraph_vector_int_destroy(&dist);
  igraph_vector_int_destroy(&sigma);
  igraph_dqueue_destroy(&queue);
  igraph_stack_destroy(&stack);
  igraph_vit_destroy(&vit);
}
