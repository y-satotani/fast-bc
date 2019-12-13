
#include "aug_dist.h"

#include <igraph/igraph_types_internal.h>
#include <igraph/igraph_math.h>

void aug_dist(igraph_t*            G,
              igraph_matrix_t*     D,
              igraph_matrix_int_t* Sigma,
              const char*          weight) {
  igraph_integer_t n = igraph_vcount(G);
  igraph_inclist_t inclist;
  igraph_2wheap_t queue;
  igraph_inclist_init(G, &inclist, IGRAPH_ALL);
  igraph_2wheap_init(&queue, n);

  igraph_matrix_init(D, n, n);
  igraph_matrix_int_init(Sigma, n, n);
  igraph_matrix_fill(D, IGRAPH_INFINITY);

  for(igraph_integer_t source = 0; source < n; source++) {
    igraph_2wheap_push_with_index(&queue, source, -0);
    MATRIX(*D, source, source) = 0.;
    MATRIX(*Sigma, source, source) = 1;

    while(!igraph_2wheap_empty(&queue)) {
      long int u = igraph_2wheap_max_index(&queue);
      igraph_real_t d_u = -igraph_2wheap_delete_max(&queue);

      igraph_vector_int_t* neis = igraph_inclist_get(&inclist, u);
      long int nneis = igraph_vector_int_size(neis);
      for(long int ni = 0; ni < nneis; ni++) {
        igraph_integer_t eid = VECTOR(*neis)[ni];
        igraph_integer_t v = IGRAPH_OTHER(G, eid, u);
        igraph_real_t d_v = MATRIX(*D, source, v);
        igraph_real_t d_vp = d_u + EAN(G, weight, eid);

        if(d_v == IGRAPH_INFINITY && v != source) {
          // first visit
          MATRIX(*D, source, v) = d_vp;
          MATRIX(*Sigma, source, v) = MATRIX(*Sigma, source, u);
          igraph_2wheap_push_with_index(&queue, v, -d_vp);
        } else if(igraph_cmp_epsilon(d_vp, d_v, IGRAPH_SHORTEST_PATH_EPSILON) < 0) {
          // shorter path was found
          MATRIX(*D, source, v) = d_vp;
          MATRIX(*Sigma, source, v) = MATRIX(*Sigma, source, u);
          igraph_2wheap_modify(&queue, v, -d_vp);
        } else if(igraph_cmp_epsilon(d_vp, d_v, IGRAPH_SHORTEST_PATH_EPSILON) == 0) {
          // new same-length path
          MATRIX(*Sigma, source, v) += MATRIX(*Sigma, source, u);
        }
      }
    } /* !igraph_2wheap_empty(&Q) */
  } /* source < n */

  igraph_inclist_destroy(&inclist);
  igraph_2wheap_destroy(&queue);
}
