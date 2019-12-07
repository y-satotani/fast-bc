
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <argp.h>
#include <igraph/igraph.h>
#include "dybc/aug_dist.h"
#include "dybc/betweenness.h"
#include "dybc/incremental.h"
#include "dybc/decremental.h"

enum argkey {
  KEY_ORDER = 10000,
  KEY_DEGREE,
  KEY_TOPOLOGY,
  KEY_FILE,
  KEY_VERTEX_NAME,
  KEY_VERTEX_PAIR,
  KEY_WEIGHT_NAME,
  KEY_WEIGHT_RANGE,
  KEY_WEIGHT,
  KEY_REAL_WEIGHT,
  KEY_QUERY,
  KEY_NO_BRANDES,
  KEY_SEED,
};

static struct argp_option options[] = {
  {"order", KEY_ORDER, "N", 0, "Network size"},
  {"degree", KEY_DEGREE, "N", 0, "Average degree"},
  {"topology", KEY_TOPOLOGY, "(BA|ER|RRG)", 0, "Network topology"},
  {"gml-file", KEY_FILE, "FILE", 0, "Input graphml file"},

  {"vertex-name", KEY_VERTEX_NAME, "NAME", 0, "Vertex id name"},
  {"vertex-pair", KEY_VERTEX_PAIR, "U,V", 0, "Vertices, comma separated"},
  {"weight-name", KEY_WEIGHT_NAME, "NAME", 0, "Weight name"},
  {"weight-range", KEY_WEIGHT_RANGE, "A,B", 0, "Random weight range"},
  {"weight", KEY_WEIGHT, "W", 0, "Edge weight, ignored if query == delete"},
  {"real-weight", KEY_REAL_WEIGHT, 0, 0, "Use real weight on random weight"},

  {"query", KEY_QUERY, "(insert|delete)", 0, "Query"},
  {"no-brandes", KEY_NO_BRANDES, 0, 0, "Disable Brandes' algorithm"},
  {"seed", KEY_SEED, "N", 0, "Seed"},
  {0}
};

struct arguments {
  int does_make_artificial;
  int order;
  int degree;
  char* topology;
  char* gml_file;

  char* vertex_name;
  char* vertex_pair;
  igraph_attribute_type_t vertex_type;
  char* weight_name;
  char* weight;
  double weight_min;
  double weight_max;
  igraph_attribute_type_t weight_type;

  char* query;
  int no_brandes;
  int is_given_seed;
  long int seed;
  char* graph_name;
};

void make_artificial_graph(igraph_t* G, struct arguments* args);
void load_real_graph(igraph_t* G, struct arguments* args);
void select_random_vertex_pair(igraph_t* G,
                               igraph_integer_t* u,
                               igraph_integer_t* v,
                               struct arguments* args);
void select_given_vertex_pair(igraph_t* G,
                              igraph_integer_t* u,
                              igraph_integer_t* v,
                              struct arguments* args);
void get_weight(igraph_t* G,
                igraph_integer_t u,
                igraph_integer_t v,
                igraph_real_t* c,
                struct arguments* args);

double update_igraph(igraph_t* G,
                     const char* mode,
                     igraph_integer_t u,
                     igraph_integer_t v,
                     igraph_real_t c,
                     const char* weight,
                     igraph_vector_t* B,
                     igraph_integer_t* n_change_deps_verts);

double update_proposed(igraph_t* G,
                       const char* mode,
                       igraph_integer_t u,
                       igraph_integer_t v,
                       igraph_real_t c,
                       const char* weight,
                       igraph_vector_t* B,
                       igraph_integer_t* n_update_path_pairs,
                       igraph_integer_t* n_update_deps_pairs,
                       igraph_integer_t* n_update_deps_verts);

static error_t parse_opt(int key, char* arg, struct argp_state* state);

static struct argp argp = {options, parse_opt, 0, 0};

int main(int argc, char* argv[]) {
  // Initialization
  char graph_name[1024];
  struct arguments args =
    {-1, 0, 0, 0, NULL, 0, 0, 0, "", NULL, 1, 5, 0, "", 0, 0, 0, graph_name};
  argp_parse(&argp, argc, argv, 0, 0, &args);

  igraph_t G;
  igraph_integer_t u, v;
  igraph_real_t c;
  igraph_i_set_attribute_table(&igraph_cattribute_table);
  if(args.is_given_seed)
    igraph_rng_seed(igraph_rng_default(), args.seed);
  else
    igraph_rng_seed(igraph_rng_default(), time(NULL));
  if(args.does_make_artificial)
    make_artificial_graph(&G, &args);
  else
    load_real_graph(&G, &args);
  if(args.vertex_pair)
    select_given_vertex_pair(&G, &u, &v, &args);
  else
    select_random_vertex_pair(&G, &u, &v, &args);
  get_weight(&G, u, v, &c, &args);

  igraph_vector_t B, Btrue;
  igraph_integer_t n_update_path_pairs, n_update_deps_verts,
    n_update_deps_pairs, n_changed_deps_verts;
  igraph_vector_init(&B, igraph_vcount(&G));
  igraph_vector_init(&Btrue, igraph_vcount(&G));
  double time_igraph = -1;
  if(!args.no_brandes)
    time_igraph
      = update_igraph(&G, args.query, u, v, c, args.weight_name, &Btrue,
                      &n_changed_deps_verts);
  double time_update
    = update_proposed(&G, args.query, u, v, c, args.weight_name, &B,
                      &n_update_path_pairs, &n_update_deps_pairs,
                      &n_update_deps_verts);

  printf("%s,%s,%s,%s,%ld,%e,%e,%e,%e,%d,%d,%d,%d\n",
         args.graph_name, args.vertex_pair, args.weight,
         args.query, args.seed,
         igraph_vector_max(&B),
         igraph_vector_maxdifference(&B, &Btrue),
         time_update, time_igraph,
         n_update_path_pairs, n_update_deps_pairs,
         n_update_deps_verts, n_changed_deps_verts);

  igraph_vector_destroy(&Btrue);
  igraph_vector_destroy(&B);
  igraph_destroy(&G);

  return 0;
}

void make_artificial_graph(igraph_t* G, struct arguments* args) {
  // Generate graph
  assert(args->degree % 2 == 0);
  sprintf(args->graph_name, "%s,%d,%d",
          args->topology, args->order, args->degree);
  if(strcmp(args->topology, "RRG") == 0)
    igraph_k_regular_game(G, args->order, args->degree, 0, 0);
  else if(strcmp(args->topology, "ER") == 0)
    igraph_erdos_renyi_game
      (G, IGRAPH_ERDOS_RENYI_GNM, args->order, args->order*args->degree/2, 0, 0);
  else if(strcmp(args->topology, "BA") == 0)
    igraph_barabasi_game
      (G, args->order, 1, args->degree/2, 0, 1, 1, 0, IGRAPH_BARABASI_BAG, 0);
  else
    assert(0);
  // set weights
  for(igraph_integer_t eid = 0; eid < igraph_ecount(G); eid++) {
    igraph_real_t l;
    if(args->weight_type == IGRAPH_ATTRIBUTE_DEFAULT) {
      l = igraph_rng_get_integer(igraph_rng_default(),
                                 (long int)args->weight_min,
                                 (long int)args->weight_max);
    } else {
      l = igraph_rng_get_unif(igraph_rng_default(),
                              args->weight_min,
                              args->weight_max);
    }
    SETEAN(G, args->weight_name, eid, l);
  }
}

void load_real_graph(igraph_t* G, struct arguments* args) {
  // Load graph
  FILE* instream = fopen(args->gml_file, "r");
  assert(instream);
  igraph_read_graph_graphml(G, instream, 0);
  fclose(instream);
  strcpy(args->graph_name, args->gml_file);

  assert(igraph_cattribute_has_attr
         (G, IGRAPH_ATTRIBUTE_EDGE, args->weight_name));
}

void select_random_vertex_pair(igraph_t* G,
                               igraph_integer_t* u,
                               igraph_integer_t* v,
                               struct arguments* args) {
  if(strcmp(args->query, "insert") == 0) {
    igraph_t C;
    igraph_integer_t eid;
    igraph_complementer(&C, G, 0);
    eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(&C)-1);
    igraph_edge(&C, eid, u, v);
    igraph_destroy(&C);
  } else if(strcmp(args->query, "delete") == 0) {
    igraph_integer_t eid;
    eid = igraph_rng_get_integer
      (igraph_rng_default(), 0, igraph_ecount(G)-1);
    igraph_edge(G, eid, u, v);
  }
}

void select_given_vertex_pair(igraph_t* G,
                              igraph_integer_t* u,
                              igraph_integer_t* v,
                              struct arguments* args) {
  if(args->vertex_name) {
    igraph_attribute_type_t attr_type;
    assert(igraph_cattribute_has_attr
           (G, IGRAPH_ATTRIBUTE_VERTEX, args->vertex_name)
           && "vertex name not found");
    igraph_cattribute_table.gettype
      (G, &attr_type, IGRAPH_ATTRIBUTE_VERTEX, args->vertex_name);

    *v = -1, *u = -1;
    switch(attr_type) {
    case IGRAPH_ATTRIBUTE_NUMERIC: {
      igraph_real_t ua, va;
      igraph_vector_t attr_vec;
      long int up, vp;
      sscanf(args->vertex_pair, "%lf,%lf", &ua, &va);
      igraph_vector_init(&attr_vec, igraph_vcount(G));
      igraph_cattribute_VANV
        (G, args->vertex_name, igraph_vss_all(), &attr_vec);
      assert(igraph_vector_search(&attr_vec, 0, ua, &up));
      assert(igraph_vector_search(&attr_vec, 0, va, &vp));
      igraph_vector_destroy(&attr_vec);
      *u = (igraph_integer_t)up;
      *v = (igraph_integer_t)vp;
      break;
    }
    case IGRAPH_ATTRIBUTE_STRING: {
      char ua[1024], va[1024];
      igraph_strvector_t attr_vec;
      sscanf(args->vertex_pair, "%[^,],%[^,]", ua, va);
      igraph_strvector_init(&attr_vec, igraph_vcount(G));
      igraph_cattribute_VASV
        (G, args->vertex_name, igraph_vss_all(), &attr_vec);
      for(int i = 0; i < igraph_vcount(G); i++)
        if(strcmp(STR(attr_vec, i), ua) == 0) {
          *u = i;
          break;
        }
      for(int i = 0; i < igraph_vcount(G); i++)
        if(strcmp(STR(attr_vec, i), va) == 0) {
          *v = i;
          break;
        }
      assert(*u >= 0 && *v >= 0 && "vertex pair not found");
      break;
    }
    default:
      assert(0 && "invalid query type");
    }
  } else {
    sscanf(args->vertex_pair, "%d,%d", u, v);
    assert(0 <= *u && *u < igraph_vcount(G) &&
           0 <= *v && *v < igraph_vcount(G) &&
           "vertex pair not found");
  }

  igraph_integer_t eid;
  igraph_get_eid(G, &eid, *u, *v, IGRAPH_ALL, 0);
  if(strcmp(args->query, "insert") == 0)
    assert(eid < 0 && "edge to insert already exists");
  else if(strcmp(args->query, "delete") == 0)
    assert(eid >= 0 && "edge to delete does not exist");
  else
    assert(0);
}

void get_weight(igraph_t* G,
                igraph_integer_t u,
                igraph_integer_t v,
                igraph_real_t* c,
                struct arguments* args) {
  if(strcmp(args->query, "insert") == 0) {
    if(!args->weight)
      *c = igraph_rng_get_unif
        (igraph_rng_default(), args->weight_min, args->weight_max);
    else
      *c = atof(args->weight);
    if(args->weight_type == IGRAPH_ATTRIBUTE_DEFAULT)
      *c = (long int)(*c);
  } else if(strcmp(args->query, "delete") == 0) {
    igraph_integer_t eid;
    igraph_get_eid(G, &eid, u, v, IGRAPH_ALL, 0);
    assert(eid >= 0);
    *c = EAN(G, args->weight_name, eid);
  }
}

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments *arguments = state->input;
  switch(key) {
  case KEY_ORDER:
    if(arguments->does_make_artificial == 0)
      argp_usage(state);
    arguments->order = atoi(arg);
    arguments->does_make_artificial = 1;
    break;
  case KEY_DEGREE:
    if(arguments->does_make_artificial == 0)
      argp_usage(state);
    arguments->degree = atoi(arg);
    arguments->does_make_artificial = 1;
    break;
  case KEY_TOPOLOGY:
    if(arguments->does_make_artificial == 0)
      argp_usage(state);
    arguments->topology = arg;
    arguments->does_make_artificial = 1;
    break;

  case KEY_FILE:
    if(arguments->does_make_artificial == 1)
      argp_usage(state);
    arguments->gml_file = arg;
    arguments->does_make_artificial = 0;
    break;

  case KEY_VERTEX_NAME:
    arguments->vertex_name = arg;
    break;
  case KEY_VERTEX_PAIR:
    arguments->vertex_pair = arg;
    break;

  case KEY_WEIGHT_NAME:
    arguments->weight_name = arg;
    break;
  case KEY_WEIGHT:
    arguments->weight = arg;
    break;
  case KEY_WEIGHT_RANGE:
    sscanf(arg, "%lf,%lf", &arguments->weight_min, &arguments->weight_max);
    break;
  case KEY_REAL_WEIGHT:
    arguments->weight_type = IGRAPH_ATTRIBUTE_NUMERIC;
    break;

  case KEY_QUERY:
    arguments->query = arg;
    break;
  case KEY_NO_BRANDES:
    arguments->no_brandes = 1;
    break;
  case KEY_SEED:
    arguments->seed = atoll(arg);
    arguments->is_given_seed = 1;
    break;

  case ARGP_KEY_END:
    if(arguments->does_make_artificial < 0)
      argp_usage(state);
    break;

  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}


double update_igraph(igraph_t* G,
                     const char* mode,
                     igraph_integer_t u,
                     igraph_integer_t v,
                     igraph_real_t c,
                     const char* weight,
                     igraph_vector_t* B,
                     igraph_integer_t* n_change_deps_verts) {
  // Calculate training data
  igraph_vector_t Bold;
  igraph_vector_t weights;
  igraph_vector_init(&Bold, igraph_vcount(G));
  igraph_vector_init(&weights, igraph_ecount(G));
  for(igraph_integer_t eid = 0; eid < igraph_ecount(G); eid++) {
    VECTOR(weights)[eid] = EAN(G, weight, eid);
  }
  igraph_betweenness(G, &Bold, igraph_vss_all(), 0, &weights, 0);

  // Calculate
  clock_t start, end;
  double time_igraph;
  start = clock();
  // Update graph
  if(strcmp(mode, "insert") == 0) {
    igraph_integer_t eid;
    igraph_add_edge(G, u, v);
    igraph_get_eid(G, &eid, u, v, 0, 0);
    SETEAN(G, weight, eid, c);
  } else if(strcmp(mode, "delete") == 0) {
    igraph_integer_t eid;
    igraph_get_eid(G, &eid, u, v, 0, 0);
    igraph_delete_edges(G, igraph_ess_1(eid));
  } else assert(0);
  igraph_vector_resize(&weights, igraph_ecount(G));
  for(igraph_integer_t eid = 0; eid < igraph_ecount(G); eid++) {
    VECTOR(weights)[eid] = EAN(G, weight, eid);
  }

  igraph_betweenness(G, B, igraph_vss_all(), 0, &weights, 0);
  igraph_vector_scale(B, 2./((igraph_vcount(G)-1) * (igraph_vcount(G)-2)));
  end = clock();
  time_igraph = (double)(end - start) / CLOCKS_PER_SEC;
  *n_change_deps_verts = 0;
  for(igraph_integer_t x = 0; x < igraph_vcount(G); x++)
    if(fabs(igraph_vector_e(B, x) - igraph_vector_e(&Bold, x)) > 1e-6) {
      (*n_change_deps_verts)++;
    }

  // Reset update
  if(strcmp(mode, "insert") == 0) {
    igraph_integer_t eid;
    igraph_get_eid(G, &eid, u, v, 0, 0);
    igraph_delete_edges(G, igraph_ess_1(eid));
  } else if(strcmp(mode, "delete") == 0) {
    igraph_integer_t eid;
    igraph_add_edge(G, u, v);
    igraph_get_eid(G, &eid, u, v, 0, 0);
    SETEAN(G, weight, eid, c);
  } else assert(0);
  igraph_vector_destroy(&Bold);
  igraph_vector_destroy(&weights);

  return time_igraph;
}

double update_proposed(igraph_t* G,
                       const char* mode,
                       igraph_integer_t u,
                       igraph_integer_t v,
                       igraph_real_t c,
                       const char* weight,
                       igraph_vector_t* B,
                       igraph_integer_t* n_update_path_pairs,
                       igraph_integer_t* n_update_deps_pairs,
                       igraph_integer_t* n_update_deps_verts) {
  // Make argumented distance and pair dependency
  igraph_matrix_t D;
  igraph_matrix_int_t Sigma;
  igraph_matrix_t Delta;
  aug_dist(G, &D, &Sigma, weight);
  pairwise_dependency(G, &Delta, weight);

  // Update
  clock_t start, end;
  double time_update;
  start = clock();
  if(strcmp(mode, "insert") == 0) {
    incremental(G, u, v, c, &D, &Sigma, &Delta, weight,
                n_update_path_pairs, n_update_deps_pairs,
                n_update_deps_verts);
  } else if(strcmp(mode, "delete") == 0) {
    decremental(G, u, v, &D, &Sigma, &Delta, weight,
                n_update_path_pairs, n_update_deps_pairs,
                n_update_deps_verts);
  } else assert(0);

  igraph_matrix_colsum(&Delta, B);
  //igraph_vector_scale(B, 0.5);
  igraph_vector_scale(B, 1./((igraph_vcount(G)-1) * (igraph_vcount(G)-2)));
  end = clock();
  time_update = (double)(end - start) / CLOCKS_PER_SEC;

  // Reset update
  if(strcmp(mode, "insert") == 0) {
    igraph_integer_t eid;
    igraph_get_eid(G, &eid, u, v, 0, 0);
    igraph_delete_edges(G, igraph_ess_1(eid));
  } else if(strcmp(mode, "delete") == 0) {
    igraph_integer_t eid;
    igraph_add_edge(G, u, v);
    igraph_get_eid(G, &eid, u, v, 0, 0);
    SETEAN(G, weight, eid, c);
  } else assert(0);
  igraph_matrix_destroy(&Delta);
  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&Sigma);

  return time_update;
}
