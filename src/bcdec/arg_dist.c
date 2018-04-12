
#include "arg_dist.h"

void arg_dist(igraph_matrix_t* D,
              igraph_matrix_t* Sigma,
              igraph_t* G) {

  igraph_vector_t nrgeo;
  igraph_vector_ptr_t paths;
  int row, pi;

  int n = igraph_vcount(G);
  igraph_matrix_init(D, n, n);
  igraph_matrix_init(Sigma, n, n);

  for(row = 0; row < igraph_vcount(G); row++) {
    igraph_vector_ptr_init(&paths, 0);
    igraph_vector_init(&nrgeo, 0);

    igraph_get_all_shortest_paths(G, &paths, &nrgeo,
                                  row, igraph_vss_all(),
                                  IGRAPH_ALL);

    // calculate row of D and Sigma
    int col = 0, tail = 0;
    while(col < igraph_vcount(G)) {
      igraph_vector_t* path = (igraph_vector_t*)
        igraph_vector_ptr_e(&paths, tail);
      if(igraph_vector_e(&nrgeo, col) > 0)
        igraph_matrix_set(D, row, col, igraph_vector_size(path)-1);
      else
        igraph_matrix_set(D, row, col, IGRAPH_INFINITY);
      igraph_matrix_set(Sigma, row, col, igraph_vector_e(&nrgeo, col));
      tail += igraph_vector_e(&nrgeo, col);
      col++;
    }

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

void edge_set(igraph_vector_ptr_t* E,
              igraph_t* G) {

  igraph_vector_t nrgeo;
  igraph_vector_ptr_t paths;
  int row, pi, vi;

  int n = igraph_vcount(G);

  igraph_vector_ptr_init(E, 0);
  igraph_vector_ptr_set_item_destructor
    (E, (igraph_finally_func_t*)igraph_vector_destroy);

  for(row = 0; row < igraph_vcount(G); row++) {
    igraph_vector_ptr_init(&paths, 0);
    igraph_vector_init(&nrgeo, 0);

    igraph_get_all_shortest_paths(G, &paths, &nrgeo,
                                  row, igraph_vss_all(),
                                  IGRAPH_ALL);

    // calculate elements of E
    igraph_vector_t *edges = (igraph_vector_t*)
      malloc(sizeof(igraph_vector_t));
    igraph_vector_init(edges, 0);
    for(pi = 0; pi < igraph_vector_ptr_size(&paths); pi++) {
      igraph_vector_t* path = (igraph_vector_t*)
        igraph_vector_ptr_e(&paths, pi);
      for(vi = 0; vi < igraph_vector_size(path)-1; vi++) {
        igraph_integer_t e = vi*n*n +
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
