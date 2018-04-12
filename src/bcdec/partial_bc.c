
#include "partial_bc.h"

int bc_partial(igraph_t* graph, igraph_vector_t* Bi, long int source) {

  // from https://github.com/igraph/igraph/blob/master/src/centrality.c

  long int no_of_nodes = igraph_vcount(graph);
  igraph_dqueue_t q = IGRAPH_DQUEUE_NULL;
  long int *distance;
  unsigned long long int *nrgeo=0;  /* must be long long; consider grid
                                       graphs for example */
  double *tmpscore;
  igraph_stack_t stack = IGRAPH_STACK_NULL;
  long int j, k, nneis;
  igraph_vector_int_t *neis;
  igraph_vit_t vit;

  igraph_adjlist_t adjlist_out, adjlist_in;
  igraph_adjlist_t *adjlist_out_p, *adjlist_in_p;

  IGRAPH_CHECK(igraph_adjlist_init(graph, &adjlist_out, IGRAPH_ALL));
  IGRAPH_FINALLY(igraph_adjlist_destroy, &adjlist_out);
  IGRAPH_CHECK(igraph_adjlist_init(graph, &adjlist_in, IGRAPH_ALL));
  IGRAPH_FINALLY(igraph_adjlist_destroy, &adjlist_in);
  adjlist_out_p = &adjlist_out;
  adjlist_in_p = &adjlist_in;

  for(j = 0; j < no_of_nodes; j++) {
    igraph_vector_int_clear(igraph_adjlist_get(adjlist_in_p, j));
  }

  distance = igraph_Calloc(no_of_nodes, long int);
  if(distance == 0) {
    IGRAPH_ERROR("betweenness failed", IGRAPH_ENOMEM);
  }
  IGRAPH_FINALLY(igraph_free, distance);
  nrgeo = igraph_Calloc(no_of_nodes, unsigned long long int);
  if (nrgeo == 0) {
    IGRAPH_ERROR("betweenness failed", IGRAPH_ENOMEM);
  }
  IGRAPH_FINALLY(igraph_free, nrgeo);
  tmpscore = igraph_Calloc(no_of_nodes, double);
  if (tmpscore == 0) {
    IGRAPH_ERROR("betweenness failed", IGRAPH_ENOMEM);
  }
  IGRAPH_FINALLY(igraph_free, tmpscore);

  IGRAPH_DQUEUE_INIT_FINALLY(&q, 100);
  igraph_stack_init(&stack, no_of_nodes);
  IGRAPH_FINALLY(igraph_stack_destroy, &stack);

  IGRAPH_CHECK(igraph_dqueue_push(&q, source));
  nrgeo[source] = 1;
  distance[source] = 1;

  while(!igraph_dqueue_empty(&q)) {
    long int actnode = (long int) igraph_dqueue_pop(&q);
    IGRAPH_CHECK(igraph_stack_push(&stack, actnode));

    neis = igraph_adjlist_get(adjlist_out_p, actnode);
    nneis = igraph_vector_int_size(neis);
    for(j = 0; j < nneis; j++) {
      long int neighbor = (long int) VECTOR(*neis)[j];
      if(distance[neighbor] == 0) {
        distance[neighbor] = distance[actnode] + 1;
        IGRAPH_CHECK(igraph_dqueue_push(&q, neighbor));
      }

      if(distance[neighbor] == distance[actnode] + 1) {
        igraph_vector_int_t *v = igraph_adjlist_get(adjlist_in_p, neighbor);
        igraph_vector_int_push_back(v, actnode);
        nrgeo[neighbor] += nrgeo[actnode];
      }
    }
  } /* while !igraph_dqueue_empty */

  /* Ok, we've the distance of each node and also the number of
     shortest paths to them. Now we do an inverse search, starting
     with the farthest nodes. */
  igraph_vector_resize(Bi, no_of_nodes);
  igraph_vector_null(Bi);
  while(!igraph_stack_empty(&stack)) {
    long int actnode = (long int) igraph_stack_pop(&stack);
    neis = igraph_adjlist_get(adjlist_in_p, actnode);
    nneis = igraph_vector_int_size(neis);
    for(j = 0; j < nneis; j++) {
        long int neighbor = (long int) VECTOR(*neis)[j];
        tmpscore[neighbor] +=  (tmpscore[actnode]+1) *
          ((double)(nrgeo[neighbor])) / nrgeo[actnode];
    }

    if(actnode != source) {
      VECTOR(*Bi)[actnode] += tmpscore[actnode];
    }

    distance[actnode]=0;
    nrgeo[actnode]=0;
    tmpscore[actnode]=0;
    igraph_vector_int_clear(igraph_adjlist_get(adjlist_in_p, actnode));
  }

  /* clean  */
  igraph_Free(distance);
  igraph_Free(nrgeo);
  igraph_Free(tmpscore);

  igraph_dqueue_destroy(&q);
  igraph_stack_destroy(&stack);
  IGRAPH_FINALLY_CLEAN(5);

  igraph_adjlist_destroy(&adjlist_out);
  igraph_adjlist_destroy(&adjlist_in);
  IGRAPH_FINALLY_CLEAN(2);

  return 0;
}
