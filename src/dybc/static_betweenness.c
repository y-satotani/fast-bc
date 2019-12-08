#include "static_betweenness.h"

#include <igraph/igraph_types_internal.h>
#include <igraph/igraph_math.h>

void _betweenness_weighted(igraph_t* G,
                           igraph_matrix_t* D,
                           igraph_matrix_int_t* S,
                           igraph_vector_t* B,
                           igraph_vector_t* weight);

void _betweenness_unweighted(igraph_t* G,
                             igraph_matrix_t* D,
                             igraph_matrix_int_t* S,
                             igraph_vector_t* B);

void betweenness_with_redundant_information(igraph_t* G,
                                            igraph_matrix_t* D,
                                            igraph_matrix_int_t* S,
                                            igraph_vector_t* B,
                                            igraph_vector_t* weight) {
  if(weight) _betweenness_weighted(G, D, S, B, weight);
  else _betweenness_unweighted(G, D, S, B);
}

void _betweenness_weighted(igraph_t* G,
                           igraph_matrix_t* D,
                           igraph_matrix_int_t* S,
                           igraph_vector_t* B,
                           igraph_vector_t* weight) {
  igraph_integer_t n = igraph_vcount(G);
  igraph_integer_t m = igraph_ecount(G);
  igraph_vector_t delta;
  igraph_inclist_t inclist;
  igraph_adjlist_t precedings;
  igraph_2wheap_t queue;
  igraph_stack_t stack;

  igraph_matrix_resize(D, n, n);
  igraph_matrix_int_resize(S, n, n);
  igraph_vector_resize(B, n);
  igraph_vector_init(&delta, n);
  igraph_inclist_init(G, &inclist, IGRAPH_OUT);
  igraph_adjlist_init_empty(&precedings, n);
  igraph_2wheap_init(&queue, n);
  igraph_stack_init(&stack, n);

  for(igraph_integer_t source = 0; source < n; source++) {
    igraph_2wheap_push_with_index(&queue, source, 0);
    MATRIX(*D, source, source) = 0;
    MATRIX(*S, source, source) = 1;

    while(!igraph_2wheap_empty(&queue)) {
      long int u = igraph_2wheap_max_index(&queue);
      igraph_real_t d_u = -igraph_2wheap_delete_max(&queue);

      igraph_stack_push(&stack, u);

      igraph_vector_int_t* neis = igraph_inclist_get(&inclist, u);
      long int nneis = igraph_vector_int_size(neis);
      for(long int ni = 0; ni < nneis; ni++) {
        igraph_integer_t eid = VECTOR(*neis)[ni];
        igraph_integer_t v = IGRAPH_OTHER(G, eid, u);
        igraph_real_t d_v = MATRIX(*D, source, v);
        igraph_real_t d_vp = d_u + VECTOR(*weight)[eid];

        if(d_v == 0. && v != source) {
          // first visit
          igraph_vector_int_t* pv = igraph_adjlist_get(&precedings, v);
          igraph_vector_int_resize(pv, 1);
          VECTOR(*pv)[0] = u;
          MATRIX(*S, source, v) = MATRIX(*S, source, u);

          MATRIX(*D, source, v) = d_vp;
          igraph_2wheap_push_with_index(&queue, v, -d_vp);
        } else if(igraph_cmp_epsilon(d_vp, d_v, IGRAPH_SHORTEST_PATH_EPSILON) < 0) {
          // shorter path was found
          igraph_vector_int_t* pv = igraph_adjlist_get(&precedings, v);
          igraph_vector_int_resize(pv, 1);
          VECTOR(*pv)[0] = u;
          MATRIX(*S, source, v) = MATRIX(*S, source, u);

          MATRIX(*D, source, v) = d_vp;
          igraph_2wheap_modify(&queue, v, -d_vp);
        } else if(igraph_cmp_epsilon(d_vp, d_v, IGRAPH_SHORTEST_PATH_EPSILON) == 0) {
          // new same-length path
          igraph_vector_int_t* pv = igraph_adjlist_get(&precedings, v);
          igraph_vector_int_push_back(pv, u);
          MATRIX(*S, source, v) += MATRIX(*S, source, u);
        }
      }
    } /* !igraph_2wheap_empty(&Q) */

    while(!igraph_stack_empty(&stack)) {
      long int w = (long int) igraph_stack_pop(&stack);
      igraph_vector_int_t* vs = igraph_adjlist_get(&precedings, w);
      long int nvs = igraph_vector_int_size(vs);
      for(igraph_integer_t vi = 0; vi < nvs; vi++) {
        long int v = VECTOR(*vs)[vi];
        VECTOR(delta)[v] += (1 + VECTOR(delta)[w])
          * MATRIX(*S, source, v) / MATRIX(*S, source, w);
      }
      if(w != source) {
        VECTOR(*B)[w] += VECTOR(delta)[w];
      }

      VECTOR(delta)[w] = 0;
      igraph_vector_int_clear(igraph_adjlist_get(&precedings, w));
    }

  } /* source < n */

  igraph_vector_destroy(&delta);
  igraph_inclist_destroy(&inclist);
  igraph_adjlist_destroy(&precedings);
  igraph_2wheap_destroy(&queue);
  igraph_stack_destroy(&stack);
}

void _betweenness_unweighted(igraph_t* G,
                             igraph_matrix_t* D,
                             igraph_matrix_int_t* S,
                             igraph_vector_t* B) {

  igraph_integer_t n = igraph_vcount(G);
  igraph_integer_t m = igraph_ecount(G);
  igraph_vector_t delta;
  igraph_inclist_t inclist;
  igraph_adjlist_t precedings;
  igraph_dqueue_t queue;
  igraph_stack_t stack;

  igraph_matrix_resize(D, n, n);
  igraph_matrix_int_resize(S, n, n);
  igraph_vector_resize(B, n);
  igraph_vector_init(&delta, n);
  igraph_inclist_init(G, &inclist, IGRAPH_OUT);
  igraph_adjlist_init_empty(&precedings, n);
  igraph_dqueue_init(&queue, n);
  igraph_stack_init(&stack, n);

  for(igraph_integer_t source = 0; source < n; source++) {
    igraph_dqueue_push(&queue, source);
    MATRIX(*D, source, source) = 0;
    MATRIX(*S, source, source) = 1;

    while(!igraph_dqueue_empty(&queue)) {
      long int u = igraph_dqueue_pop(&queue);
      igraph_stack_push(&stack, u);

      igraph_vector_int_t* neis = igraph_inclist_get(&inclist, u);
      long int nneis = igraph_vector_int_size(neis);
      for(long int ni = 0; ni < nneis; ni++) {
        igraph_integer_t eid = VECTOR(*neis)[ni];
        igraph_integer_t v = IGRAPH_OTHER(G, eid, u);
        igraph_real_t d_v = MATRIX(*D, source, v);
        igraph_real_t d_vp = 1 + MATRIX(*D, source, u);

        if(d_v == 0. && v != source) {
          // first visit
          igraph_vector_int_t* pv = igraph_adjlist_get(&precedings, v);
          igraph_vector_int_resize(pv, 1);
          VECTOR(*pv)[0] = u;
          MATRIX(*S, source, v) = MATRIX(*S, source, u);

          MATRIX(*D, source, v) = d_vp;
          igraph_dqueue_push(&queue, v);
        }
        if(d_vp == d_v) {
          // new same-length path
          igraph_vector_int_t* pv = igraph_adjlist_get(&precedings, v);
          igraph_vector_int_push_back(pv, u);
          MATRIX(*S, source, v) += MATRIX(*S, source, u);
        }
      }
    } /* !igraph_dqueue_empty(&Q) */

    while(!igraph_stack_empty(&stack)) {
      long int w = (long int) igraph_stack_pop(&stack);
      igraph_vector_int_t* vs = igraph_adjlist_get(&precedings, w);
      long int nvs = igraph_vector_int_size(vs);
      for(igraph_integer_t vi = 0; vi < nvs; vi++) {
        long int v = VECTOR(*vs)[vi];
        VECTOR(delta)[v] += (1 + VECTOR(delta)[w])
          * MATRIX(*S, source, v) / MATRIX(*S, source, w);
      }
      if(w != source) {
        VECTOR(*B)[w] += VECTOR(delta)[w];
      }

      VECTOR(delta)[w] = 0;
      igraph_vector_int_clear(igraph_adjlist_get(&precedings, w));
    }

  } /* source < n */

  igraph_vector_destroy(&delta);
  igraph_inclist_destroy(&inclist);
  igraph_adjlist_destroy(&precedings);
  igraph_dqueue_destroy(&queue);
  igraph_stack_destroy(&stack);

}
