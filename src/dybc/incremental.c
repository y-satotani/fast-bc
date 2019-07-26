
#include "incremental.h"

#include <igraph_types_internal.h>
#include "custom_igraph.h"
#include "betweenness.h"

#include <stdio.h>

void incremental(igraph_t*            G,
                 igraph_integer_t     v,
                 igraph_integer_t     w,
                 igraph_real_t        c,
                 igraph_matrix_t*     D,
                 igraph_matrix_int_t* Sigma,
                 igraph_matrix_t*     Delta,
                 const char*          weight,
                 igraph_integer_t*    n_update_path_pairs,
                 igraph_integer_t*    n_update_dep_pairs,
                 igraph_integer_t*    n_update_dep_verts) {
  // add edge and set weight
  igraph_integer_t eid;
  igraph_add_edge(G, v, w);
  igraph_get_eid(G, &eid, v, w, 0, 0);
  SETEAN(G, weight, eid, c);

  // target loop
  igraph_inclist_t inclist;
  igraph_integer_t z;
  igraph_vector_bool_t update_dep_verts;
  igraph_inclist_init(G, &inclist, IGRAPH_ALL);
  igraph_vector_bool_init(&update_dep_verts, igraph_vcount(G));
  *n_update_path_pairs = *n_update_dep_pairs = 0;
  for(z = 0; z < igraph_vcount(G); z++) {
    igraph_integer_t n_update_path_pairs_sub = 0;
    igraph_integer_t n_update_dep_pairs_sub = 0;
    if(MATRIX(*D, v, z) > MATRIX(*D, w, z))
      incremental_part(G, &inclist, v, w, z, c, D, Sigma, Delta, weight,
                       &n_update_path_pairs_sub, &n_update_dep_pairs_sub,
                       &update_dep_verts);
    else
      incremental_part(G, &inclist, w, v, z, c, D, Sigma, Delta, weight,
                       &n_update_path_pairs_sub, &n_update_dep_pairs_sub,
                       &update_dep_verts);
    *n_update_path_pairs += n_update_path_pairs_sub;
    *n_update_dep_pairs += n_update_dep_pairs_sub;
  }

  *n_update_dep_verts = 0;
  for(z = 0; z < igraph_vcount(G); z++)
    if(igraph_vector_bool_e(&update_dep_verts, z))
      (*n_update_dep_verts)++;

  igraph_inclist_destroy(&inclist);
  igraph_vector_bool_destroy(&update_dep_verts);
}

void incremental_part(igraph_t*            G,
                      igraph_inclist_t*    inclist,
                      igraph_integer_t     v,
                      igraph_integer_t     w,
                      igraph_integer_t     z,
                      igraph_real_t        c,
                      igraph_matrix_t*     D,
                      igraph_matrix_int_t* Sigma,
                      igraph_matrix_t*     Delta,
                      const char*          weight,
                      igraph_integer_t*     n_update_path_pairs,
                      igraph_integer_t*     n_update_dep_pairs,
                      igraph_vector_bool_t* update_dep_verts) {
  if(igraph_cmp_epsilon(MATRIX(*D, v, z), c + MATRIX(*D, w, z)) < 0
     || MATRIX(*Sigma, w, z) == 0)
    return;
  igraph_vector_t dp_z;
  igraph_2wheap_t queue;
  igraph_vector_long_t delta_set;

  igraph_vector_init(&dp_z, igraph_vcount(G));
  igraph_matrix_get_col(D, &dp_z, z);
  igraph_2wheap_init(&queue, igraph_vcount(G));
  igraph_2wheap_push_with_index(&queue, v, -(c+MATRIX(*D, w, z)));
  igraph_vector_long_init(&delta_set, 0);

  //printf("%d %d %d:\n", v, w, z);

  if(n_update_path_pairs)
    *n_update_path_pairs = 0;
  while(!igraph_2wheap_empty(&queue)) {
    igraph_integer_t x = igraph_2wheap_max_index(&queue);
    igraph_real_t d_xz = -igraph_2wheap_delete_max(&queue);
    igraph_real_t dp_xz = VECTOR(dp_z)[x];
    igraph_integer_t sp_xz = MATRIX(*Sigma, x, z);

    MATRIX(*D, x, z) = d_xz;
    MATRIX(*Sigma, x, z) = 0;

    igraph_vector_t* neis = igraph_inclist_get(inclist, x);
    igraph_integer_t ni;
    for(ni = 0; ni < igraph_vector_size(neis); ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      igraph_real_t d_yz = MATRIX(*D, y, z);
      igraph_real_t dp_yz = VECTOR(dp_z)[y];
      igraph_real_t l_yx = EAN(G, weight, eid);

      //printf("--debug1-- %d %d %f %f %f %f %f\n", x, y, l_yx, dp_xz, dp_yz, d_xz, d_yz);

      if(igraph_cmp_epsilon(d_yz, l_yx + d_xz) >= 0) {
        igraph_2wheap_update(&queue, y, -(l_yx+d_xz));
      }
      if(igraph_cmp_epsilon(d_xz, l_yx + d_yz) == 0) {
        MATRIX(*Sigma, x, z) += MATRIX(*Sigma, y, z);
      }
      if((igraph_cmp_epsilon(dp_xz, l_yx + dp_yz) == 0
          && !(y == w && x == v))
         != (igraph_cmp_epsilon(d_xz, l_yx + d_yz) == 0)) {
        igraph_vector_long_push_back(&delta_set, y);
        //printf("--debug1-- %d added\n", y);
      }
    } /* for ni < nnei */
    if(MATRIX(*Sigma, x, z) != sp_xz) {
      igraph_vector_long_push_back(&delta_set, x);
      //printf("--debug1-- %d added\n", x);
    }
    if(n_update_path_pairs)
      (*n_update_path_pairs)++;
  }

  igraph_2wheap_t delta_queue;
  igraph_2wheap_init(&delta_queue, igraph_vcount(G));
  for(int i = 0; i < igraph_vector_long_size(&delta_set); i++) {
    igraph_integer_t x = VECTOR(delta_set)[i];
    igraph_2wheap_update(&delta_queue, x, MATRIX(*D, x, z));
  }

  if(n_update_dep_pairs)
    *n_update_dep_pairs = 0;
  while(Delta && !igraph_2wheap_empty(&delta_queue)) {
    igraph_integer_t x = igraph_2wheap_max_index(&delta_queue);
    igraph_real_t d_xz = igraph_2wheap_delete_max(&delta_queue);
    igraph_integer_t s_xz = MATRIX(*Sigma, x, z);
    //printf("--debug2-- %d %d %f\n", z, x, d_xz);

    MATRIX(*Delta, z, x) = 0;
    if(x == z) continue;

    igraph_vector_t* neis = igraph_inclist_get(inclist, x);
    igraph_integer_t ni;
    for(ni = 0; ni < igraph_vector_size(neis); ni++) {
      igraph_integer_t eid = VECTOR(*neis)[ni];
      igraph_integer_t y = IGRAPH_OTHER(G, eid, x);
      igraph_real_t d_yz = MATRIX(*D, y, z);
      igraph_integer_t s_yz = MATRIX(*Sigma, y, z);
      igraph_real_t l_yx = EAN(G, weight, eid);

      if(igraph_cmp_epsilon(d_yz, l_yx + d_xz) == 0) {
        MATRIX(*Delta, z, x) += (1 + MATRIX(*Delta, z, y)) * s_xz / s_yz;
      } else if(igraph_cmp_epsilon(d_xz, l_yx + d_yz) == 0) {
        igraph_2wheap_update(&delta_queue, y, d_yz);
      }
    }
    if(n_update_dep_pairs)
      (*n_update_dep_pairs)++;
    if(update_dep_verts)
      igraph_vector_bool_set(update_dep_verts, x, 1);
  }

  igraph_vector_destroy(&dp_z);
  igraph_2wheap_destroy(&queue);
  igraph_vector_long_destroy(&delta_set);
  igraph_2wheap_destroy(&delta_queue);
}

