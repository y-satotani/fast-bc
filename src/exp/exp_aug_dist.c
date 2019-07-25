
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

static struct argp_option options[] = {
  {0, 'n', "N", 0, "Network size"},
  {0, 'k', "N", 0, "Average degree"},
  {0, 't', "(BA|ER|RRG)", 0, "Network topology"},
  {0, 'f', "FILE", 0, "Input file"},
  {0, 'w', "WEIGHT", 0, "Weight"},
  {0, 'q', "(insert|delete)", 0, "Query"},
  {0, 's', "N", 0, "Seed"},
  {0}
};

struct arguments {
  int n;
  int k;
  char* name;
  char* file;
  char* weight;
  char* mode;
  long int seed;
};

void make_graph(igraph_t* G, struct arguments* args);
void choice_noadjacent_pair(igraph_t* G, int* u, int* v);

static error_t parse_opt(int key, char* arg, struct argp_state* state);

static struct argp argp = {options, parse_opt, 0, 0};

int main(int argc, char* argv[]) {
  // Initialization
  struct arguments args;
  args.file = 0;
  args.weight = "length";
  argp_parse(&argp, argc, argv, 0, 0, &args);
  igraph_t G;
  const char* weight = args.weight;
  igraph_i_set_attribute_table(&igraph_cattribute_table);
  igraph_rng_seed(igraph_rng_default(), args.seed);
  make_graph(&G, &args);

  // Make augmented distance
  igraph_matrix_t D;
  igraph_matrix_int_t Sigma;
  aug_dist(&G, &D, &Sigma, weight);

  // Choose edge to insert/delete
  igraph_integer_t u, v;
  igraph_real_t c;
  if(strcmp(args.mode, "insert") == 0) {
    long int weight_min = -1;
    long int weight_max = -1;
    for(igraph_integer_t eid = 0; eid < igraph_ecount(&G); eid++) {
      if(weight_min < 0 || weight_min > EAN(&G, weight, eid))
        weight_min = (long int)EAN(&G, weight, eid);
      if(weight_max < 0 || weight_max < EAN(&G, weight, eid))
        weight_max = (long int)EAN(&G, weight, eid);
    }
    choice_noadjacent_pair(&G, &u, &v);
    c = igraph_rng_get_integer(igraph_rng_default(), weight_min, weight_max);
  } else if(strcmp(args.mode, "delete") == 0) {
    igraph_integer_t eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&G)-1);
    igraph_edge(&G, eid, &u, &v);
    c = EAN(&G, weight, eid);
  } else assert(0);

  // Update
  clock_t start, end;
  double time_update;
  igraph_integer_t n_update_path_pairs, n_update_dep_verts;
  if(strcmp(args.mode, "insert") == 0) {
    start = clock();
    incremental(&G, u, v, c, &D, &Sigma, 0, weight,
                &n_update_path_pairs, &n_update_dep_verts);
    end = clock();
  } else if(strcmp(args.mode, "delete") == 0) {
    start = clock();
    decremental(&G, u, v, &D, &Sigma, 0, weight,
                &n_update_path_pairs, &n_update_dep_verts);
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

  printf("%s,%d,%d,%s,%ld,%e,%e,%e\n",
         args.name, args.n, args.k, args.mode, args.seed,
         igraph_matrix_maxdifference(&D, &Dtrain),
         time_update, time_igraph);

  igraph_vector_destroy(&weights);
  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&Sigma);
  igraph_matrix_destroy(&Dtrain);
  igraph_destroy(&G);

  return 0;
}

void make_graph(igraph_t* G, struct arguments* args) {
  if(!(args->file)) {
    // Generate graph
    assert(args->k % 2 == 0);
    if(strcmp(args->name, "RRG") == 0)
      igraph_k_regular_game(G, args->n, args->k, 0, 0);
    else if(strcmp(args->name, "ER") == 0)
      igraph_erdos_renyi_game
        (G, IGRAPH_ERDOS_RENYI_GNM, args->n, args->n*args->k/2, 0, 0);
    else if(strcmp(args->name, "BA") == 0)
      igraph_barabasi_game
        (G, args->n, 1, args->k/2, 0, 1, 1, 0, IGRAPH_BARABASI_BAG, 0);
    else
      assert(0);
    // set weights
    for(igraph_integer_t eid = 0; eid < igraph_ecount(G); eid++) {
      long int l = igraph_rng_get_integer(igraph_rng_default(), 1, 5);
      SETEAN(G, args->weight, eid, l);
    }
  } else {
    // Load graph
    FILE* instream = fopen(args->file, "r");
    assert(instream);
    igraph_read_graph_graphml(G, instream, 0);
    fclose(instream);
    args->name = args->file;
    args->n = igraph_vcount(G);
    args->k = (2*igraph_ecount(G)) / igraph_vcount(G);
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
  case 'f':
    arguments->file = arg;
    break;
  case 'w':
    arguments->weight = arg;
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

