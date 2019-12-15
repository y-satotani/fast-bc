
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <argp.h>
#include <igraph/igraph.h>
#include "dybc_exp_common.h"
#include "dybc_io.h"
#include "dybc_update_query.h"
#include "dybc_update_stats.h"

const char *argp_program_version =
  "exp-compare-performance beta";
const char *argp_program_bug_address =
  "https://github.com/y-satotani/dynamic-betweenness/issues";
struct arguments {
  char* input_file;
  char* cache_file;
  int is_directed;
  dybc_update_query_t query;
  igraph_integer_t u, v;
  igraph_real_t w;
  unsigned long int seed;
};
static struct argp_option options[] = {
  {"directed", 'd', 0, 0, "compute for directed graph"},
  {"cache", 'c', "CACHE_FILE", 0, "cache file generated by cache computer"},
  {"v1", 'u', "V", 0, "first vertex id"},
  {"v2", 'v', "V", 0, "second vertex id"},
  {"weight", 'w', "N", 0, "activated only on inserting an edge to weighted network"},
  {"seed", 's', "N", 0, "seed to select endpoints"},
  {0}
};
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
  case 'd':
    arguments->is_directed = 1;
    break;
  case 'c':
    arguments->cache_file = arg;
    break;
  case 'u':
    arguments->u = atol(arg);
    break;
  case 'v':
    arguments->v = atol(arg);
    break;
  case 'w':
    arguments->w = atof(arg);
    break;
  case 's':
    arguments->seed = atoll(arg);
    break;
  case ARGP_KEY_ARG:
    if(state->arg_num == 0)
      arguments->input_file = arg;
    else if(state->arg_num == 1) {
      if(strcmp(arg, "insert") == 0)
        arguments->query = QUERY_INSERT;
      else if(strcmp(arg, "delete") == 0)
        arguments->query = QUERY_DELETE;
      else
        argp_usage(state);
    } else
      argp_usage(state);
    break;
  case ARGP_KEY_END:
    if(state->arg_num < 1)
      argp_usage(state);
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}
static char args_doc[] = "edgelist (insert|delete)";
static char doc[] = "";
static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char* argv[]) {
  // parsing options
  struct arguments arguments;
  arguments.input_file = NULL;
  arguments.is_directed = 0;
  arguments.query = QUERY_NONE;
  arguments.cache_file = NULL;
  arguments.u = arguments.v = -1;
  arguments.w = -1;
  arguments.seed = 0;
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  // reading graph
  igraph_t G;
  igraph_vector_t* weights
    = (igraph_vector_t*) malloc(sizeof(igraph_vector_t));
  FILE* istream;
  if(strcmp(arguments.input_file, "-") == 0) istream = stdin;
  else istream = fopen(arguments.input_file, "r");
  dybc_read_edgelist(&G, weights, arguments.is_directed, istream);
  fclose(istream);
  if(igraph_vector_size(weights) != igraph_ecount(&G)) {
    if(igraph_vector_size(weights) > 0) {
      fprintf(stderr, "Warning: length of weights is different from size. ");
      fprintf(stderr, "Discarding weights\n");
    }
    igraph_vector_destroy(weights);
    free(weights);
    weights = NULL;
  }

  // calculating/loading pre-operation information
  igraph_matrix_t D;
  igraph_matrix_int_t S;
  igraph_vector_t B;
  igraph_matrix_init(&D, igraph_vcount(&G), igraph_vcount(&G));
  igraph_matrix_int_init(&S, igraph_vcount(&G), igraph_vcount(&G));
  igraph_vector_init(&B, igraph_vcount(&G));
  if(arguments.cache_file) {
    FILE* cistream;
    if(strcmp(arguments.cache_file, "-") == 0) cistream = stdin;
    else cistream = fopen(arguments.cache_file, "r");
    dybc_load_cache(&D, &S, &B, cistream);
    fclose(cistream);
  } else {
    betweenness_with_redundant_information(&G, &D, &S, &B, weights);
  }

  // prepare for update
  igraph_integer_t eid, u, v;
  igraph_real_t w;
  if(arguments.u >= 0 && arguments.v >= 0) {
    u = arguments.u; v = arguments.v;
  } else {
    choose_random_edge_for(&G, arguments.query, &u, &v, arguments.seed);
  }
  igraph_get_eid(&G, &eid, u, v, arguments.is_directed, 0);
  assert(arguments.query != QUERY_INSERT || eid < 0
         && "invalid query (inserting existing edge)");
  assert(arguments.query != QUERY_DELETE || eid >= 0
         && "invalid query (deleting not existing edge)");
  if(!weights) {
    w = 1.0;
  } else if(arguments.w > 0) {
    w = arguments.w;
  } else {
    igraph_real_t v_min, v_max;
    igraph_vector_minmax(weights, &v_min, &v_max);
    w = igraph_rng_get_integer(igraph_rng_default(), v_min, v_max);
  }

  // update
  clock_t start, end;
  double time_proposed, time_igraph;
  start = clock();
  if(arguments.query == QUERY_INSERT)
    incremental_update(&G, &D, &S, &B, u, v, weights, w, NULL);
  else if(arguments.query == QUERY_DELETE)
    decremental_update(&G, &D, &S, &B, u, v, weights, w, NULL);
  else
    assert(0 && "query must be insert or delete");
  if(!arguments.is_directed) igraph_vector_scale(&B, 0.5);
  end = clock();
  time_proposed = (double)(end - start) / CLOCKS_PER_SEC;
  igraph_vector_t B_true;
  igraph_vector_init(&B_true, igraph_vcount(&G));
  start = clock();
  igraph_betweenness
    (&G, &B_true, igraph_vss_all(), arguments.is_directed, weights, 0);
  end = clock();
  time_igraph = (double)(end - start) / CLOCKS_PER_SEC;

  FILE* ostream = stdout;
  // input-name,query,directed,weighted,max-diff,time-proposed,time-igraph
  fprintf(stdout, "%s,%s,%s,%s,%10e,%10.10f,%10.10f\n",
          arguments.input_file,
          arguments.query == QUERY_INSERT ? "insert" : "delete",
          arguments.is_directed ? "directed" : "undirected",
          weights ? "weighted" : "unweighted",
          igraph_vector_maxdifference(&B, &B_true),
          time_proposed,
          time_igraph);
  fclose(ostream);

  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&S);
  igraph_vector_destroy(&B);
  igraph_vector_destroy(&B_true);
  igraph_destroy(&G);
  if(weights) {
    igraph_vector_destroy(weights);
    free(weights);
  }
  return 0;
}
