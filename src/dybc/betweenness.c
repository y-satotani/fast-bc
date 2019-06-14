#include "betweenness.h"

#include <igraph_types_internal.h>

#include "custom_igraph_math.h"

void pairwise_dependency(igraph_matrix_t* Delta,
                         igraph_t* G, const char* weight) {
  igraph_integer_t n = igraph_vcount(G);
  igraph_vector_t dist;
  igraph_vector_int_t sigma;
  igraph_inclist_t inclist;
  igraph_adjlist_t precedings;
  igraph_2wheap_t queue;
  igraph_stack_t stack;
  const igraph_real_t eps = IGRAPH_SHORTEST_PATH_EPSILON;

  igraph_vector_init(&dist, n);
  igraph_vector_int_init(&sigma, n);
  igraph_inclist_init(G, &inclist, IGRAPH_ALL);
  igraph_adjlist_init_empty(&precedings, n);
  igraph_2wheap_init(&queue, n);
  igraph_stack_init(&stack, n);

  igraph_matrix_init(Delta, n, n);

  for(igraph_integer_t source = 0; source < n; source++) {
    igraph_2wheap_push_with_index(&queue, source, 0);
    VECTOR(dist)[source] = 0.;
    VECTOR(sigma)[source] = 1;

    while(!igraph_2wheap_empty(&queue)) {
      long int u = igraph_2wheap_max_index(&queue);
      igraph_real_t d_u = -igraph_2wheap_delete_max(&queue);

      igraph_stack_push(&stack, u);

      igraph_vector_t* neis = igraph_inclist_get(&inclist, u);
      long int nneis = igraph_vector_size(neis);
      for(long int ni = 0; ni < nneis; ni++) {
        igraph_integer_t eid = VECTOR(*neis)[ni];
        igraph_integer_t v = IGRAPH_OTHER(G, eid, u);
        igraph_real_t d_v = VECTOR(dist)[v];
        igraph_real_t d_vp = d_u + EAN(G, weight, eid);

        if(d_v == 0. && v != source) {
          // first visit
          igraph_vector_int_t* pv = igraph_adjlist_get(&precedings, v);
          igraph_vector_int_resize(pv, 1);
          VECTOR(*pv)[0] = u;
          VECTOR(sigma)[v] = VECTOR(sigma)[u];

          VECTOR(dist)[v] = d_vp;
          igraph_2wheap_push_with_index(&queue, v, -d_vp);
        } else if(igraph_cmp_epsilon(d_vp, d_v, eps) < 0) {
          // shorter path was found
          igraph_vector_int_t* pv = igraph_adjlist_get(&precedings, v);
          igraph_vector_int_resize(pv, 1);
          VECTOR(*pv)[0] = u;
          VECTOR(sigma)[v] = VECTOR(sigma)[u];

          VECTOR(dist)[v] = d_vp;
          igraph_2wheap_modify(&queue, v, -d_vp);

        } else if(igraph_cmp_epsilon(d_vp, d_v, eps) == 0) {
          // new same-length path
          igraph_vector_int_t* pv = igraph_adjlist_get(&precedings, v);
          igraph_vector_int_push_back(pv, u);
          VECTOR(sigma)[v] += VECTOR(sigma)[u];
        }
      }
    } /* !igraph_2wheap_empty(&Q) */

    while(!igraph_stack_empty(&stack)) {
      long int w = (long int) igraph_stack_pop(&stack);
      igraph_vector_int_t* vs = igraph_adjlist_get(&precedings, w);
      long int nvs = igraph_vector_int_size(vs);
      for(igraph_integer_t vi = 0; vi < nvs; vi++) {
        long int v = VECTOR(*vs)[vi];
        MATRIX(*Delta, source, v)
          += (1 + MATRIX(*Delta, source, w))
          * VECTOR(sigma)[v] / VECTOR(sigma)[w];
      }
      if(w == source) {
        MATRIX(*Delta, source, w) = 0;
      }

      VECTOR(dist)[w] = 0;
      VECTOR(sigma)[w] = 0;
      igraph_vector_int_clear(igraph_adjlist_get(&precedings, w));
    }

  } /* source < n */

  igraph_vector_destroy(&dist);
  igraph_vector_int_destroy(&sigma);
  igraph_inclist_destroy(&inclist);
  igraph_adjlist_destroy(&precedings);
  igraph_2wheap_destroy(&queue);
  igraph_stack_destroy(&stack);
}

