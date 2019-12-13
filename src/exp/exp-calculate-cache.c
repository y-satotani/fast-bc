
#include <stdio.h>
#include <string.h>
#include <argp.h>
#include <igraph/igraph.h>
#include "dybc/static_betweenness.h"
#include "dybc_io.h"

const char *argp_program_version =
  "exp-calculate-cache beta";
const char *argp_program_bug_address =
  "https://github.com/y-satotani/dynamic-betweenness/issues";
struct arguments {
  char* input_file;
  char* output_file;
  int is_directed;
};
static struct argp_option options[] = {
  {"directed", 'd', 0, 0, "cache for directed graph"},
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
      arguments->output_file = arg;
    else
      argp_usage(state);
    break;
  case ARGP_KEY_END:
    if(state->arg_num < 2)
      argp_usage(state);
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}
static char args_doc[] = "IN OUT";
static char doc[] = "";
static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char* argv[]) {
  struct arguments arguments;
  arguments.input_file = NULL;
  arguments.output_file = NULL;
  arguments.is_directed = 0;
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  FILE* istream;
  if(strcmp(arguments.input_file, "-") == 0) istream = stdin;
  else istream = fopen(arguments.input_file, "r");

  igraph_t G;
  igraph_vector_t weights;
  dybc_read_edgelist(&G, &weights, arguments.is_directed, istream);
  fclose(istream);

  igraph_matrix_t D;
  igraph_matrix_int_t S;
  igraph_vector_t B, *W;
  igraph_matrix_init(&D, igraph_vcount(&G), igraph_vcount(&G));
  igraph_matrix_int_init(&S, igraph_vcount(&G), igraph_vcount(&G));
  igraph_vector_init(&B, igraph_vcount(&G));
  if(igraph_ecount(&G) == igraph_vector_size(&weights)) {
    W = &weights;
  } else {
    if(igraph_vector_size(&weights) > 0)
      printf("Warning: length of weights is different from size. Discarding weights\n");
    W = 0;
  }
  betweenness_with_redundant_information(&G, &D, &S, &B, W);

  FILE* ostream;
  if(strcmp(arguments.output_file, "-") == 0) ostream = stdout;
  else ostream = fopen(arguments.output_file, "wb");
  dybc_dump_cache(&D, &S, &B, ostream);

  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&S);
  igraph_vector_destroy(&B);
  igraph_destroy(&G);
  igraph_vector_destroy(&weights);
  return 0;
}
