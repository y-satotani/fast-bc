
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <argp.h>
#include <igraph/igraph.h>
#include <dybc/static_betweenness.h>
#include "dybc_update_query.h"
#include "dybc_batch_update.h"

const char *argp_program_version =
  "exp-sequential-update beta";
const char *argp_program_bug_address =
  "https://github.com/y-satotani/dynamic-betweenness/issues";
struct arguments {
  char* input_file;
  char* output_file;
  igraph_integer_t n_vertices;
  igraph_bool_t is_directed;
};
static struct argp_option options[] = {
  {"directed", 'd', 0, 0, "compute for directed graph"},
  {0}
};
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
  case 'd':
    arguments->is_directed = 1;
    break;
  case ARGP_KEY_ARG:
    if(state->arg_num == 0)
      arguments->input_file = arg;
    else if(state->arg_num == 1)
      arguments->n_vertices = atoi(arg);
    else if(state->arg_num == 2)
      arguments->output_file = arg;
    else
      argp_usage(state);
    break;
  case ARGP_KEY_END:
    if(state->arg_num < 3)
      argp_usage(state);
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}
static char args_doc[] = "temporal_edgelist n_vertices out_file";
static char doc[] = "";
static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char* argv[]) {
  // parsing options
  struct arguments arguments;
  arguments.input_file = NULL;
  arguments.output_file = NULL;
  arguments.is_directed = 0;
  arguments.n_vertices = 0;
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  igraph_t G;
  igraph_vector_t weights;
  igraph_vector_int_t queries;
  igraph_vector_int_t endpoints;
  igraph_empty(&G, arguments.n_vertices, arguments.is_directed);
  igraph_vector_init(&weights, 0);
  igraph_vector_int_init(&queries, 0);
  igraph_vector_int_init(&endpoints, 0);
  FILE* istream;
  if(strcmp(arguments.input_file, "-") == 0) istream = stdin;
  else istream = fopen(arguments.input_file, "r");
  FILE* ostream;
  if(strcmp(arguments.output_file, "-") == 0) ostream = stdout;
  else ostream = fopen(arguments.output_file, "w");

  igraph_matrix_t D;
  igraph_matrix_int_t S;
  igraph_vector_t B, B_true;
  igraph_matrix_init(&D, igraph_vcount(&G), igraph_vcount(&G));
  igraph_matrix_int_init(&S, igraph_vcount(&G), igraph_vcount(&G));
  igraph_vector_init(&B, igraph_vcount(&G));
  igraph_vector_init(&B_true, igraph_vcount(&G));

  igraph_vector_bool_t is_to_keep;
  igraph_vector_int_t to_add_endpoints;
  igraph_vector_t weights_to_add, weights_;
  igraph_vector_bool_init(&is_to_keep, 0);
  igraph_vector_int_init(&to_add_endpoints, 0);
  igraph_vector_init(&weights_to_add, 0);
  igraph_vector_init(&weights_, 0);
  igraph_integer_t t_pred = -1, t_last = -1;
  igraph_integer_t t, u, v;
  igraph_real_t w;
  char line[1024];
  int n_fields;
  while(1) {
    char* lineresult = fgets(line, 1024, istream);
    n_fields = sscanf(line, "%d %d %d %lf", &u, &v, &t, &w);
    if(lineresult == NULL || (t_pred >= 0 && t != t_pred)) {
      // update
      for(igraph_integer_t eid = 0;
          eid < igraph_vector_bool_size(&is_to_keep); eid++) {
        if(!VECTOR(is_to_keep)[eid]) {
          igraph_integer_t u_, v_;
          igraph_edge(&G, eid, &u_, &v_);
          igraph_vector_int_push_back(&endpoints, u_);
          igraph_vector_int_push_back(&endpoints, v_);
          igraph_vector_int_push_back(&queries, QUERY_DELETE);
          igraph_vector_push_back(&weights_, 0);
        }
      }
      for(igraph_integer_t i = 0;
          i < igraph_vector_int_size(&to_add_endpoints) / 2; i++) {
        igraph_integer_t u_ = VECTOR(to_add_endpoints)[2*i];
        igraph_integer_t v_ = VECTOR(to_add_endpoints)[2*i+1];
        igraph_vector_int_push_back(&endpoints, u_);
        igraph_vector_int_push_back(&endpoints, v_);
        igraph_vector_int_push_back(&queries, QUERY_INSERT);
        igraph_vector_push_back(&weights_, VECTOR(weights_to_add)[i]);
      }

      // update
      clock_t start, end;
      double time_proposed, time_igraph;
      long n_insert = 0, n_delete = 0;
      if(t_last < 0) {
        for(igraph_integer_t i = 0; i < igraph_vector_int_size(&queries); i++) {
          igraph_integer_t u_ = VECTOR(endpoints)[2*i];
          igraph_integer_t v_ = VECTOR(endpoints)[2*i+1];
          igraph_add_edge(&G, u_, v_);
          igraph_vector_push_back(&weights, VECTOR(weights_to_add)[i]);
        }
        start = clock();
        if(n_fields == 4)
          betweenness_with_redundant_information(&G, &D, &S, &B, &weights);
        else
          betweenness_with_redundant_information(&G, &D, &S, &B, 0);
        end = clock();
      } else {
        start = clock();
        if(n_fields == 4)
          batch_update
            (&G, &D, &S, &B, &endpoints, &weights, &weights_, &queries);
        else
          batch_update
            (&G, &D, &S, &B, &endpoints, 0, 0, &queries);
        end = clock();
      }
      time_proposed = (double)(end - start) / CLOCKS_PER_SEC;
      start = clock();
      if(n_fields == 4)
        igraph_betweenness
          (&G, &B_true, igraph_vss_all(), arguments.is_directed, &weights, 0);
      else
        igraph_betweenness
          (&G, &B_true, igraph_vss_all(), arguments.is_directed, 0, 0);
      end = clock();
      time_igraph = (double)(end - start) / CLOCKS_PER_SEC;
      if(!igraph_is_directed(&G)) igraph_vector_scale(&B_true, 2);

      for(long int i = 0; i < igraph_vector_int_size(&queries); i++)
        if(VECTOR(queries)[i] == QUERY_INSERT)
          n_insert++;
        else if(VECTOR(queries)[i] == QUERY_DELETE)
          n_delete++;
      fprintf(ostream, "%s,%d,%ld,%ld,%10e,%10.10lf,%10.10lf\n",
              arguments.input_file,
              t_pred, n_insert, n_delete,
              igraph_vector_maxdifference(&B, &B_true),
              time_proposed, time_igraph);

      igraph_vector_int_clear(&endpoints);
      igraph_vector_int_clear(&queries);
      igraph_vector_clear(&weights_);
      igraph_vector_bool_resize(&is_to_keep, igraph_ecount(&G));
      igraph_vector_bool_null(&is_to_keep);
      igraph_vector_int_clear(&to_add_endpoints);
      igraph_vector_clear(&weights_to_add);
      t_last = t_pred;
    }
    if(lineresult == NULL) break;
    if(n_fields < 3) continue;
    igraph_integer_t eid;
    igraph_get_eid(&G, &eid, u, v, arguments.is_directed, 0);
    if(eid >= 0) VECTOR(is_to_keep)[eid] = 1;
    else {
      igraph_vector_int_push_back(&to_add_endpoints, u);
      igraph_vector_int_push_back(&to_add_endpoints, v);
      igraph_vector_push_back(&weights_to_add, w);
    }
    t_pred = t;
  }
  fclose(istream);
  fclose(ostream);

  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&S);
  igraph_vector_destroy(&B);
  igraph_vector_destroy(&B_true);

  igraph_vector_int_destroy(&queries);
  igraph_vector_int_destroy(&endpoints);
  igraph_vector_bool_destroy(&is_to_keep);
  igraph_vector_int_destroy(&to_add_endpoints);
  igraph_vector_destroy(&weights_to_add);
  igraph_vector_destroy(&weights_);

  igraph_destroy(&G);
  igraph_vector_destroy(&weights);
  return 0;
}
