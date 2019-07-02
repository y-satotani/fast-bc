
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <argp.h>
#include <igraph.h>
#include "dybc/aug_dist.h"
#include "dybc/incremental.h"
#include "dybc/decremental.h"

void make_graph(igraph_t* G, char* name, int n, int k, const char* weight);
void choice_noadjacent_pair(igraph_t* G, int* u, int* v);

static struct argp_option options[] = {
  {0, 'n', "N", 0, "Network size"},
  {0, 'k', "N", 0, "Average degree"},
  {0, 't', "(BA|ER|RRG)", 0, "Network topology"},
  {0, 'q', "(insert|delete)", 0, "Query"},
  {0, 's', "N", 0, "Seed"},
  {0}
};

struct arguments {
  int n;
  int k;
  char* name;
  char* mode;
  long int seed;
};

static error_t parse_opt(int key, char* arg, struct argp_state* state);

static struct argp argp = {options, parse_opt, 0, 0};

int main(int argc, char* argv[]) {
  // Initialization
  struct arguments args;
  argp_parse(&argp, argc, argv, 0, 0, &args);
  igraph_t G;
  const char* weight = "length";
  igraph_i_set_attribute_table(&igraph_cattribute_table);
  igraph_rng_seed(igraph_rng_default(), args.seed);
  make_graph(&G, args.name, args.n, args.k, weight);

  // Make augmented distance
  igraph_matrix_t D;
  igraph_matrix_int_t Sigma;
  aug_dist(&G, &D, &Sigma, weight);

  // Choose edge to insert/delete
  igraph_integer_t u, v;
  igraph_real_t c;
  if(strcmp(args.mode, "insert") == 0) {
    choice_noadjacent_pair(&G, &u, &v);
    c = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
  } else if(strcmp(args.mode, "delete") == 0) {
    igraph_integer_t eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&G)-1);
    igraph_edge(&G, eid, &u, &v);
    c = EAN(&G, weight, eid);
  } else assert(0);

  // Update
  clock_t start, end;
  double time_update;
  if(strcmp(args.mode, "insert") == 0) {
    start = clock();
    incremental(&G, u, v, c, &D, &Sigma, 0, weight);
    end = clock();
  } else if(strcmp(args.mode, "delete") == 0) {
    start = clock();
    decremental(&G, u, v, &D, &Sigma, 0, weight);
    end = clock();
  } else assert(0);
  time_update = (double)(end - start) / CLOCKS_PER_SEC;

  // Calculate training data
  igraph_matrix_t Dtrain;
  double time_igraph;
  igraph_vector_t weights;
  igraph_matrix_init(&Dtrain, igraph_vcount(&G), igraph_vcount(&G));
  igraph_vector_init(&weights, igraph_ecount(&G));
  for(igraph_integer_t eid = 0; eid < igraph_ecount(&G); eid++) {
    VECTOR(weights)[eid] = EAN(&G, weight, eid);
  }
  start = clock();
  igraph_shortest_paths_dijkstra
    (&G, &Dtrain, igraph_vss_all(), igraph_vss_all(), &weights, IGRAPH_ALL);
  end = clock();
  time_igraph = (double)(end - start) / CLOCKS_PER_SEC;

  printf("%ld,%s,%d,%d,%s,%f,%f,%f\n",
         args.seed, args.name, args.n, args.k, args.mode,
         igraph_matrix_maxdifference(&D, &Dtrain),
         time_update, time_igraph);

  igraph_vector_destroy(&weights);
  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&Sigma);
  igraph_matrix_destroy(&Dtrain);
  igraph_destroy(&G);

  return 0;
}

void make_graph(igraph_t* G, char* name, int n, int k, const char* weight) {
  assert(k % 2 == 0);
  if(strcmp(name, "RRG") == 0)
    igraph_k_regular_game(G, n, k, 0, 0);
  else if(strcmp(name, "ER") == 0)
    igraph_erdos_renyi_game(G, IGRAPH_ERDOS_RENYI_GNM, n, n*k/2, 0, 0);
  else if(strcmp(name, "BA") == 0)
    igraph_barabasi_game(G, n, 1, k/2, 0, 1, 1, 0, IGRAPH_BARABASI_BAG, 0);
  else
    assert(0);
  // set weights
  for(igraph_integer_t eid = 0; eid < igraph_ecount(G); eid++) {
    long int l = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
    SETEAN(G, weight, eid, l);
  }
}

void choice_noadjacent_pair(igraph_t* G,
                            igraph_integer_t* u,
                            igraph_integer_t* v) {
  igraph_vector_int_t ulist, vlist;
  igraph_integer_t w, x, eid;
  igraph_vector_int_init(&ulist, 0);
  igraph_vector_int_init(&vlist, 0);
  for(w = 0; w < igraph_vcount(G); w++)
    for(x = w+1; x < igraph_vcount(G); x++) {
      igraph_get_eid(G, &eid, w, x, 0, 0);
      if(eid < 0) {
        igraph_vector_int_push_back(&ulist, w);
        igraph_vector_int_push_back(&vlist, x);
      }
    }
  eid = igraph_rng_get_integer
    (igraph_rng_default(), 0, igraph_vector_int_size(&ulist)-1);
  *u = igraph_vector_int_e(&ulist, eid);
  *v = igraph_vector_int_e(&vlist, eid);
  igraph_vector_int_destroy(&ulist);
  igraph_vector_int_destroy(&vlist);
}
static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments *arguments = state->input;
  switch(key) {
  case 'n':
    arguments->n = atoi(arg);
    break;
  case 'k':
    arguments->k = atoi(arg);
    break;
  case 't':
    arguments->name = arg;
    break;
  case 'q':
    arguments->mode = arg;
    break;
  case 's':
    arguments->seed = atoll(arg);
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

