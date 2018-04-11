#include "distance_update.h"

/** \private */
void _sort_two_vertices(igraph_integer_t* near,
                        igraph_integer_t* far,
                        igraph_matrix_t* D,
                        igraph_integer_t s,
                        igraph_integer_t u,
                        igraph_integer_t v) {
  if(igraph_matrix_e(D, s, u) == IGRAPH_INFINITY &&
     igraph_matrix_e(D, s, v) == IGRAPH_INFINITY) {
    *near = *far = -1;
  } else if(igraph_matrix_e(D, s, u) <= igraph_matrix_e(D, s, v)) {
    *near = u;
    *far = v;
  } else {
    *near = v;
    *far = u;
  }
}

/** \private */
igraph_integer_t _sigma_ab(igraph_matrix_t* D,
                           igraph_matrix_t* S,
                           igraph_integer_t s,
                           igraph_integer_t t,
                           igraph_integer_t u,
                           igraph_integer_t v) {
  if(igraph_matrix_e(S, s, t) == 0) return 0; // no s-t path
  igraph_integer_t a, b;
  _sort_two_vertices(&a, &b, D, s, u, v);
  if(a < 0) return 0; // no path
  if(igraph_matrix_e(D, s, t) <
     igraph_matrix_e(D, s, a) + igraph_matrix_e(D, b, t) + 1)
    return 0;
  else
    return igraph_matrix_e(S, s, a) * igraph_matrix_e(S, b, t);
}

void update_distance_on_insert(igraph_matrix_t* Dnext,
                               igraph_matrix_t* Snext,
                               igraph_matrix_t* D,
                               igraph_matrix_t* S,
                               igraph_integer_t u,
                               igraph_integer_t v) {
  int s, t;
  igraph_integer_t a, b;
  igraph_real_t dsa, dsb, dbt, dst;
  igraph_real_t ssa, sbt, sst;
  for(s = 0; s < igraph_matrix_nrow(D); s++) {
    for(t = 0; t < igraph_matrix_nrow(D); t++) {
      if(s > t) continue;
      if(s == t) {
        igraph_matrix_set(Dnext, s, s, 0);
        igraph_matrix_set(Snext, s, s, 1);
        continue;
      }
      _sort_two_vertices(&a, &b, D, s, u, v);
      dsa = igraph_matrix_e(D, s, a);
      dsb = igraph_matrix_e(D, s, b);
      dbt = igraph_matrix_e(D, b, t);
      dst = igraph_matrix_e(D, s, t);
      ssa = igraph_matrix_e(S, s, a);
      sbt = igraph_matrix_e(S, b, t);
      sst = igraph_matrix_e(S, s, t);

      if(dsa < dsb && dsa + dbt + 1 < dst) {
        dst = dsa + dbt + 1;
        sst = (igraph_integer_t)ssa * (igraph_integer_t)sbt;
      } else if(dsa < dsb && dsa + dbt + 1 == dst) {
        sst = (igraph_integer_t)sst +
          (igraph_integer_t)ssa * (igraph_integer_t)sbt;
      } else {
      }

      igraph_matrix_set(Dnext, s, t, dst);
      igraph_matrix_set(Dnext, t, s, dst);
      igraph_matrix_set(Snext, s, t, sst);
      igraph_matrix_set(Snext, t, s, sst);
    }
  }
}

void update_on_delete(igraph_matrix_t* D,
                      igraph_matrix_t* S,
                      igraph_vector_ptr_t* E,
                      igraph_vector_t* B_update,
                      igraph_t* G,
                      igraph_integer_t u,
                      igraph_integer_t v) {
  int a, b, i, j, ei, e, d, ni, p, s, t, n = igraph_matrix_nrow(D);
  long int pos;
  igraph_integer_t w;
  igraph_vector_t update_pair;
  igraph_vector_t update_dist;
  igraph_vector_init(&update_pair, 0);
  igraph_vector_init(&update_dist, 0);

  igraph_matrix_t Dtmp, Stmp;
  igraph_matrix_copy(&Dtmp, D);
  igraph_matrix_copy(&Stmp, S);

  for(s = 0; s < n; s++) {
    for(t = s+1; t < n; t++) {
      if(igraph_matrix_e(&Stmp, s, t) == 0) continue;
      igraph_integer_t sigma_ab = _sigma_ab(&Dtmp, &Stmp, s, t, u, v);
      if(sigma_ab < igraph_matrix_e(&Stmp, s, t)) {
        igraph_integer_t sigma = (igraph_integer_t)
          igraph_matrix_e(&Stmp, s, t) - sigma_ab;
        igraph_matrix_set(S, s, t, sigma);
        igraph_matrix_set(S, t, s, sigma);
        continue;
      }
      // update distance
      igraph_real_t mindist = IGRAPH_INFINITY;
      for(w = 0; w < n; w++) {
        if(igraph_matrix_e(&Stmp, s, w) ==
           _sigma_ab(&Dtmp, &Stmp, s, w, u, v) ||
           igraph_matrix_e(&Stmp, w, t) ==
           _sigma_ab(&Dtmp, &Stmp, w, t, u, v))
          continue;
        igraph_integer_t dist =
          igraph_matrix_e(D, s, w) + igraph_matrix_e(D, w, t);
        if(dist < mindist)
          mindist = dist;
      }

      if(mindist == IGRAPH_INFINITY) {
        igraph_matrix_set(D, s, t, IGRAPH_INFINITY);
        igraph_matrix_set(D, t, s, IGRAPH_INFINITY);
        igraph_matrix_set(S, s, t, 0);
        igraph_matrix_set(S, t, s, 0);
      } else {
        igraph_matrix_set(D, s, t, (igraph_integer_t)mindist);
        igraph_matrix_set(D, t, s, (igraph_integer_t)mindist);
        // add to update list to appropriate place
        igraph_vector_binsearch(&update_dist, mindist, &pos);
        igraph_vector_insert(&update_dist, pos, mindist);
        igraph_vector_insert(&update_pair, pos, s * n + t);
      }
    }
  }

  for(i = 0; i < igraph_vector_size(&update_dist); i++) {
    p = (igraph_integer_t)igraph_vector_e(&update_pair, i);
    s = p / n; t = p % n;
    igraph_integer_t dist = (igraph_integer_t)
      igraph_vector_e(&update_dist, i);
    igraph_integer_t sigma = 0;
    for(w = 0; w < igraph_matrix_nrow(D); w++) {
      igraph_integer_t dsw = (igraph_integer_t)igraph_matrix_e(D, s, w);
      igraph_integer_t dwt = (igraph_integer_t)igraph_matrix_e(D, w, t);
      if(w != s && w != t && dist == dsw + dwt)
        sigma += (igraph_integer_t)igraph_matrix_e(S, s, w) *
          (igraph_integer_t)igraph_matrix_e(S, w, t);
    }
    sigma /= (dist - 1);
    igraph_matrix_set(S, s, t, sigma);
    igraph_matrix_set(S, t, s, sigma);
  }

  // update edge set
  if(!E) {
    igraph_matrix_destroy(&Dtmp);
    igraph_matrix_destroy(&Stmp);
    igraph_vector_destroy(&update_pair);
    igraph_vector_destroy(&update_dist);
    return;
  }
  if(B_update)
    igraph_vector_null(B_update);

  igraph_vector_ptr_t update_vertex;
  igraph_vector_ptr_init(&update_vertex, n);
  for(i = 0; i < n; i++) {
    igraph_vector_t* vec = malloc(sizeof(igraph_vector_t));
    igraph_vector_init(vec, 0);
    igraph_vector_ptr_set(&update_vertex, i, vec);
  }
  igraph_vector_t neighbors;
  igraph_vector_init(&neighbors, 0);

  for(i = 0; i < igraph_vector_size(&update_pair); i++) {
    s = (igraph_integer_t)igraph_vector_e(&update_pair, i) / n;
    t = (igraph_integer_t)igraph_vector_e(&update_pair, i) % n;
    igraph_vector_push_back((igraph_vector_t*)
                            igraph_vector_ptr_e(&update_vertex, s), t);
    igraph_vector_push_back((igraph_vector_t*)
                            igraph_vector_ptr_e(&update_vertex, t), s);
  }

  for(s = 0; s < n; s++) {
    igraph_vector_t *edges = igraph_vector_ptr_e(E, s);
    igraph_vector_t *ts = igraph_vector_ptr_e(&update_vertex, s);
    _sort_two_vertices(&a, &b, &Dtmp, s, u, v);

    if(igraph_matrix_e(&Dtmp, s, a) == igraph_matrix_e(&Dtmp, s, b))
      continue;

    if(B_update)
      igraph_vector_set(B_update, s, 1);

    igraph_vector_update(&neighbors, edges);
    igraph_vector_clear(edges);
    for(ei = 0; ei < igraph_vector_size(&neighbors); ei++) {
      e = (igraph_integer_t)igraph_vector_e(&neighbors, ei) % (n*n);
      i = e / n; j = e % n;
      if((i == u && j == v) || (i == v && j == u))
        continue;
      int is_valid = igraph_matrix_e(S, s, i) > 0 &&
        (igraph_integer_t)igraph_matrix_e(D, s, i) + 1 ==
        (igraph_integer_t)igraph_matrix_e(D, s, j);
      int is_flip = igraph_matrix_e(S, s, j) > 0 &&
        (igraph_integer_t)igraph_matrix_e(D, s, j) + 1 ==
        (igraph_integer_t)igraph_matrix_e(D, s, i);
      if(is_valid) {
        e = (igraph_integer_t)igraph_matrix_e(D, s, i)*n*n + i*n + j;
        igraph_vector_binsearch(edges, e, &pos);
        igraph_vector_insert(edges, pos, e);
      } else if(is_flip) {
        e = (igraph_integer_t)igraph_matrix_e(D, s, j)*n*n + j*n + i;
        igraph_vector_binsearch(edges, e, &pos);
        igraph_vector_insert(edges, pos, e);
      }
    }

    for(i = 0; i < igraph_vector_size(ts); i++) {
      t = igraph_vector_e(ts, i);
      if(igraph_matrix_e(S, s, t) == 0)
        continue;
      igraph_neighbors(G, &neighbors, t, IGRAPH_ALL);
      for(ni = 0; ni < igraph_vector_size(&neighbors); ni++) {
        w = igraph_vector_e(&neighbors, ni);
        if(igraph_matrix_e(D, w, t) == 1 &&
           igraph_matrix_e(S, s, t) > 0 &&
           (igraph_integer_t)igraph_matrix_e(D, s, w) + 1 ==
           (igraph_integer_t)igraph_matrix_e(D, s, t)) {
          d = igraph_matrix_e(D, s, w);
          if(!igraph_vector_binsearch(edges, d*n*n+w*n+t, &pos))
            igraph_vector_insert(edges, pos, d*n*n+w*n+t);
        }
      }
    }
  }

  igraph_matrix_destroy(&Dtmp);
  igraph_matrix_destroy(&Stmp);
  igraph_vector_destroy(&update_pair);
  igraph_vector_destroy(&update_dist);
  for(i = 0; i < n; i++) {
    igraph_vector_destroy((igraph_vector_t*)
                          igraph_vector_ptr_e(&update_vertex, i));
    free(igraph_vector_ptr_e(&update_vertex, i));
  }
  igraph_vector_ptr_destroy(&update_vertex);
  igraph_vector_destroy(&neighbors);
}


void arg_apsp(igraph_matrix_t* D,
              igraph_matrix_t* S,
              igraph_vector_ptr_t* E,
              igraph_t* G) {

  igraph_vector_t nrgeo;
  igraph_vector_ptr_t paths;
  int row, pi, vi, v;

  int n = igraph_vcount(G);
  igraph_matrix_init(D, n, n);
  igraph_matrix_init(S, n, n);
  if(E) {
    igraph_vector_ptr_init(E, 0);
    igraph_vector_ptr_set_item_destructor
      (E, (igraph_finally_func_t*)igraph_vector_destroy);
  }

  for(row = 0; row < igraph_vcount(G); row++) {
    igraph_vector_ptr_init(&paths, 0);
    igraph_vector_init(&nrgeo, 0);

    igraph_get_all_shortest_paths(G, &paths, &nrgeo,
                                  row, igraph_vss_all(),
                                  IGRAPH_ALL);

    // calculate row of D and E
    int col = 0, tail = 0;
    while(col < igraph_vcount(G)) {
      igraph_vector_t* path = (igraph_vector_t*)
        igraph_vector_ptr_e(&paths, tail);
      if(igraph_vector_e(&nrgeo, col) > 0)
        igraph_matrix_set(D, row, col, igraph_vector_size(path)-1);
      else
        igraph_matrix_set(D, row, col, IGRAPH_INFINITY);
      igraph_matrix_set(S, row, col, igraph_vector_e(&nrgeo, col));
      tail += igraph_vector_e(&nrgeo, col);
      col++;
    }

    // calculate elements of E
    if(!E) {
      for(pi = 0; pi < igraph_vector_ptr_size(&paths); pi++) {
        igraph_vector_destroy((igraph_vector_t*)
                              igraph_vector_ptr_e(&paths, pi));
        free(igraph_vector_ptr_e(&paths, pi));
        igraph_vector_ptr_set(&paths, pi, 0);
      }
      igraph_vector_ptr_destroy(&paths);
      igraph_vector_destroy(&nrgeo);
      continue;
    }

    igraph_vector_t *edges = (igraph_vector_t*)
      malloc(sizeof(igraph_vector_t));
    igraph_vector_init(edges, 0);
    for(pi = 0; pi < igraph_vector_ptr_size(&paths); pi++) {
      igraph_vector_t* path = (igraph_vector_t*)
        igraph_vector_ptr_e(&paths, pi);
      for(vi = 0; vi < igraph_vector_size(path)-1; vi++) {
        v = (igraph_integer_t)igraph_vector_e(path, vi);
        int e = (igraph_integer_t)igraph_matrix_e(D, row, v)*n*n +
          (igraph_integer_t)igraph_vector_e(path, vi)*n +
          (igraph_integer_t)igraph_vector_e(path, vi+1);
        long int pos;
        if(!igraph_vector_binsearch(edges, e, &pos))
          igraph_vector_insert(edges, pos, e);
      }
    }
    igraph_vector_ptr_push_back(E, edges);

    for(pi = 0; pi < igraph_vector_ptr_size(&paths); pi++) {
      igraph_vector_destroy((igraph_vector_t*)
                            igraph_vector_ptr_e(&paths, pi));
      free(igraph_vector_ptr_e(&paths, pi));
      igraph_vector_ptr_set(&paths, pi, 0);
    }
    igraph_vector_ptr_destroy(&paths);
    igraph_vector_destroy(&nrgeo);
  }
}
