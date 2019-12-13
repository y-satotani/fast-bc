
#include "dybc_io.h"

int dybc_read_edgelist(igraph_t* G,
                       igraph_vector_t* weights,
                       igraph_bool_t is_directed,
                       FILE* istream) {
  igraph_integer_t u, v;
  igraph_real_t w;
  int n_fields;

  igraph_empty(G, 0, is_directed);
  igraph_vector_init(weights, 0);
  while((n_fields = fscanf(istream, "%d %d %lf\n", &u, &v, &w)) != EOF) {
    igraph_integer_t max_v = u > v ? u : v;
    if(igraph_vcount(G) <= max_v)
      igraph_add_vertices(G, max_v-igraph_vcount(G)+1, 0);
    igraph_add_edge(G, u, v);
    if(n_fields == 3)
      igraph_vector_push_back(weights, w);
  }
  if(igraph_ecount(G) != igraph_vector_size(weights))
    igraph_vector_clear(weights);
  return 1;
}

int dybc_dump_cache(igraph_matrix_t* D,
                    igraph_matrix_int_t* S,
                    igraph_vector_t* B,
                    FILE* ostream) {
  igraph_integer_t N = igraph_vector_size(B);
  igraph_integer_t *N_ =
    (igraph_integer_t*) malloc(sizeof(igraph_integer_t));
  N_[0] = N;
  igraph_real_t* D_ =
    (igraph_real_t*) malloc(sizeof(igraph_real_t)*N*N);
  igraph_matrix_copy_to(D, D_);
  igraph_integer_t* S_ =
    (igraph_integer_t*) malloc(sizeof(igraph_integer_t)*N*N);
  igraph_matrix_int_copy_to(S, S_);
  igraph_real_t* B_ =
    (igraph_real_t*) malloc(sizeof(igraph_real_t)*N);
  igraph_vector_copy_to(B, B_);

  fwrite(N_, sizeof(igraph_integer_t), 1, ostream);
  fwrite(D_, sizeof(igraph_real_t), N*N, ostream);
  fwrite(S_, sizeof(igraph_integer_t), N*N, ostream);
  fwrite(B_, sizeof(igraph_real_t), N, ostream);

  free(N_);
  free(D_);
  free(S_);
  free(B_);
  return 1;
}

int dybc_load_cache(igraph_matrix_t* D,
                    igraph_matrix_int_t* S,
                    igraph_vector_t* B,
                    FILE* istream) {
  igraph_integer_t* N_ =
    (igraph_integer_t*) malloc(sizeof(igraph_integer_t));
  fread(N_, sizeof(igraph_integer_t), 1, istream);
  igraph_integer_t N = N_[0];

  igraph_matrix_resize(D, N, N);
  igraph_matrix_int_resize(S, N, N);
  igraph_vector_resize(B, N);

  igraph_real_t* D_ =
    (igraph_real_t*) malloc(sizeof(igraph_real_t)*N*N);
  fread(D_, sizeof(igraph_real_t), N*N, istream);
  for(long i = 0; i < N*N; i++) VECTOR(D->data)[i] = D_[i];
  igraph_integer_t* S_ =
    (igraph_integer_t*) malloc(sizeof(igraph_integer_t)*N*N);
  fread(S_, sizeof(igraph_integer_t), N*N, istream);
  for(long i = 0; i < N*N; i++) VECTOR(S->data)[i] = S_[i];
  igraph_real_t* B_
    = (igraph_real_t*) malloc(sizeof(igraph_real_t)*N);
  fread(B_, sizeof(igraph_real_t), N, istream);
  for(long i = 0; i < N; i++) VECTOR(*B)[i] = B_[i];

  free(N_);
  free(D_);
  free(S_);
  free(B_);
  return 1;
}
