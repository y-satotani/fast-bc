
#include "update_on_delete.h"

void update_on_delete(igraph_matrix_t* D, igraph_matrix_t* Sigma,
                      igraph_integer_t u, igraph_integer_t v) {
  long int N = igraph_matrix_nrow(D);
  igraph_vector_int_t update_dist_s, update_dist_t,
    update_sigma_s, update_sigma_t, update_sigma;
  igraph_vector_t update_dist;
  igraph_vector_int_init(&update_dist_s, 0);
  igraph_vector_int_init(&update_dist_t, 0);
  igraph_vector_init(&update_dist, 0);
  igraph_vector_int_init(&update_sigma_s, 0);
  igraph_vector_int_init(&update_sigma_t, 0);
  igraph_vector_int_init(&update_sigma, 0);

  igraph_integer_t s, t;
  for(s = 0; s < N; s++) {

    igraph_integer_t a, b, ssa, dsa;
    if(MATRIX(*D, s, u) == MATRIX(*D, s, v))
      continue;
    if(MATRIX(*D, s, u) < MATRIX(*D, s, v)) {
      a = u; b = v;
    } else {
      a = v; b = u;
    }

    dsa = MATRIX(*D, s, a);
    ssa = MATRIX(*Sigma, s, a);

    for(t = s+1; t < N; t++) {
      if(MATRIX(*D, s, t) == IGRAPH_INFINITY) continue;

      igraph_integer_t dbt, dst;
      igraph_integer_t sbt, sst;
      dbt = MATRIX(*D, b, t);
      dst = MATRIX(*D, s, t);
      sbt = MATRIX(*Sigma, b, t);
      sst = MATRIX(*Sigma, s, t);

      if(dst < dsa + dbt + 1) continue;
      else if(sst > ssa * sbt) {
        // queue update sigma
        igraph_vector_int_push_back(&update_sigma_s, s);
        igraph_vector_int_push_back(&update_sigma_t, t);
        igraph_vector_int_push_back(&update_sigma, sst - ssa * sbt);
        continue;
      }

      // queue update distance
      igraph_integer_t w;
      igraph_real_t mindist = IGRAPH_INFINITY;
      for(w = 0; w < N; w++) {
        if(MATRIX(*Sigma, s, w) == 0 || MATRIX(*Sigma, w, t) == 0)
          continue;
        // find s -> a -> b -> w
        igraph_integer_t dbw, dsw;
        igraph_integer_t sbw, ssw;
        dbw = MATRIX(*D, b, w);
        dsw = MATRIX(*D, s, w);
        sbw = MATRIX(*Sigma, b, w);
        ssw = MATRIX(*Sigma, s, w);
        if(dsa + dbw + 1 == dsw && ssa * sbw == ssw)
          continue; // no path

        // find w -> aw -> bw -> t
        if(MATRIX(*D, w, u) != MATRIX(*D, w, v)) {
          igraph_integer_t aw, bw;
          aw = MATRIX(*D, w, u) < MATRIX(*D, w, v) ? u : v;
          bw = MATRIX(*D, w, u) < MATRIX(*D, w, v) ? v : u;
          igraph_integer_t dwaw, dbwt, dwt;
          igraph_integer_t swaw, sbwt, swt;
          dwaw = MATRIX(*D, w, aw);
          dbwt = MATRIX(*D, bw, t);
          dwt = MATRIX(*D, w, t);
          swaw = MATRIX(*Sigma, w, aw);
          sbwt = MATRIX(*Sigma, bw, t);
          swt = MATRIX(*Sigma, w, t);
          if(dwaw + dbwt + 1 == dwt && swaw * sbwt == swt)
            continue; // no path
        }
        igraph_integer_t dist = MATRIX(*D, s, w) + MATRIX(*D, w, t);
        if(dist < mindist) mindist = dist;
      }

      // add to update list to appropriate place
      if(mindist == IGRAPH_INFINITY) {
        igraph_vector_int_push_back(&update_dist_s, s);
        igraph_vector_int_push_back(&update_dist_t, t);
        igraph_vector_push_back(&update_dist, IGRAPH_INFINITY);
      } else {
        long int pos;
        igraph_vector_binsearch(&update_dist, mindist, &pos);
        igraph_vector_int_insert(&update_dist_s, pos, s);
        igraph_vector_int_insert(&update_dist_t, pos, t);
        igraph_vector_insert(&update_dist, pos, mindist);
      }
    }
  }

  // update sigma
  long int i;
  for(i = 0; i < igraph_vector_int_size(&update_sigma); i++) {
    igraph_integer_t sigma;
    s = VECTOR(update_sigma_s)[i];
    t = VECTOR(update_sigma_t)[i];
    sigma = VECTOR(update_sigma)[i];
    MATRIX(*Sigma, s, t) = MATRIX(*Sigma, t, s) = sigma;
  }
  // update distance (also sigma, if not connected)
  for(i = 0; i < igraph_vector_size(&update_dist); i++) {
    igraph_real_t dist;
    s = VECTOR(update_dist_s)[i];
    t = VECTOR(update_dist_t)[i];
    dist = VECTOR(update_dist)[i];
    MATRIX(*D, s, t) = MATRIX(*D, t, s) = dist;
    if(dist == IGRAPH_INFINITY)
      MATRIX(*Sigma, s, t) = MATRIX(*Sigma, t, s) = 0;
  }
  // update sigma
  for(i = 0; i < igraph_vector_size(&update_dist); i++) {
    if(VECTOR(update_dist)[i] == IGRAPH_INFINITY)
      break; // no more update
    igraph_integer_t dist;
    s = VECTOR(update_dist_s)[i];
    t = VECTOR(update_dist_t)[i];
    dist = VECTOR(update_dist)[i];
    igraph_integer_t w, sigma = 0;
    for(w = 0; w < N; w++) {
      if(w == s || w == t) continue;
      igraph_integer_t dsw, dwt, ssw, swt;
      dsw = MATRIX(*D, s, w);
      dwt = MATRIX(*D, w, t);
      ssw = MATRIX(*Sigma, s, w);
      swt = MATRIX(*Sigma, w, t);
      if(dist == dsw + dwt)
        sigma += ssw * swt;
    }
    sigma /= (dist - 1);
    MATRIX(*Sigma, s, t) = MATRIX(*Sigma, t, s) = sigma;
  }

  igraph_vector_int_destroy(&update_dist_s);
  igraph_vector_int_destroy(&update_dist_t);
  igraph_vector_destroy(&update_dist);
  igraph_vector_int_destroy(&update_sigma_t);
  igraph_vector_int_destroy(&update_sigma_s);
  igraph_vector_int_destroy(&update_sigma);
}
