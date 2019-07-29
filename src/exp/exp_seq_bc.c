
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <argp.h>
#include <igraph.h>
#include "dybc/custom_igraph.h"
#include "dybc/aug_dist.h"
#include "dybc/betweenness.h"
#include "dybc/incremental.h"
#include "dybc/decremental.h"

static struct argp_option options[] = {
  {0, 'f', "FILE", 0, "Input file"},
  {0, 'w', "WEIGHT", 0, "Weight"},
  {0}
};

struct arguments {
  char* file;
  char* weight;
};

double update_igraph(igraph_t* G,
                     igraph_vector_t* ipairs,
                     igraph_vector_t* dpairs,
                     igraph_vector_t* B) {
  // Update graph
  igraph_vector_t eids;
  igraph_vector_init(&eids, 0);
  igraph_get_eids(G, &eids, dpairs, 0, IGRAPH_UNDIRECTED, 1);
  igraph_delete_edges(G, igraph_ess_vector(&eids));
  igraph_vector_destroy(&eids);

  igraph_add_edges(G, ipairs, 0);

  // Calculate
  clock_t start, end;
  double time_igraph;
  start = clock();
  igraph_betweenness(G, B, igraph_vss_all(), 0, 0, 0);
  end = clock();
  time_igraph = (double)(end - start) / CLOCKS_PER_SEC;

  return time_igraph;
}

double update_proposed(igraph_t* G,
                       igraph_matrix_t* D,
                       igraph_matrix_int_t* Sigma,
                       igraph_matrix_t* Delta,
                       igraph_vector_t* ipairs,
                       igraph_vector_t* dpairs,
                       igraph_vector_t* B) {
  // Update
  clock_t start, end;
  double time_update;
  igraph_integer_t n_update_path_pairs;
  igraph_integer_t n_update_deps_pairs;
  igraph_integer_t n_update_deps_verts;

  start = clock();
  for(int i = 0; i < igraph_vector_size(dpairs) / 2; i++) {
    igraph_integer_t u = igraph_vector_e(dpairs, 2*i);
    igraph_integer_t v = igraph_vector_e(dpairs, 2*i+1);
    decremental(G, u, v, D, Sigma, Delta, "weight",
                &n_update_path_pairs, &n_update_deps_pairs,
                &n_update_deps_verts);
  }
  for(int i = 0; i < igraph_vector_size(ipairs) / 2; i++) {
    igraph_integer_t u = igraph_vector_e(ipairs, 2*i);
    igraph_integer_t v = igraph_vector_e(ipairs, 2*i+1);
    incremental(G, u, v, 1., D, Sigma, Delta, "weight",
                &n_update_path_pairs, &n_update_deps_pairs,
                &n_update_deps_verts);
  }
  end = clock();

  igraph_matrix_colsum(Delta, B);
  igraph_vector_scale(B, 0.5);
  time_update = (double)(end - start) / CLOCKS_PER_SEC;

  return time_update;
}

static error_t parse_opt(int key, char* arg, struct argp_state* state);

static struct argp argp = {options, parse_opt, 0, 0};

int main(int argc, char* argv[]) {
  // Initialization
  struct arguments args;
  args.file = 0;
  args.weight = "length";
  argp_parse(&argp, argc, argv, 0, 0, &args);
  const char* weight = "weight";
  igraph_i_set_attribute_table(&igraph_cattribute_table);

  FILE* fp = fopen(args.file, "r");
  int time_idx, query_num;
  int max_graph_idx = 0;
  while(fscanf(fp, "%d %d", &time_idx, &query_num) != EOF) {
    for(int qi = 0; qi < query_num; qi++) {
      char qtype[8];
      int v, w;
      assert(fscanf(fp, "%d %s %d %d", &time_idx, qtype, &v, &w) != EOF);
      if(max_graph_idx < v)
        max_graph_idx = v;
      if(max_graph_idx < w)
        max_graph_idx = w;
    }
  }
  fclose(fp);
  igraph_t G, Gmod;
  igraph_empty(&G, max_graph_idx+1, IGRAPH_UNDIRECTED);
  igraph_empty(&Gmod, max_graph_idx+1, IGRAPH_UNDIRECTED);

  // Make argumented distance and pair dependency
  igraph_matrix_t D;
  igraph_matrix_int_t Sigma;
  igraph_matrix_t Delta;
  aug_dist(&G, &D, &Sigma, weight);
  pairwise_dependency(&G, &Delta, weight);

  igraph_vector_t B, Btrue;
  igraph_vector_init(&B, igraph_vcount(&G));
  igraph_vector_init(&Btrue, igraph_vcount(&G));

  // Read query list from file
  fp = fopen(args.file, "r");
  while(fscanf(fp, "%d %d", &time_idx, &query_num) != EOF) {
    // Build query list
    igraph_vector_t ipairs, dpairs;
    igraph_vector_init(&ipairs, 0);
    igraph_vector_init(&dpairs, 0);
    for(int qi = 0; qi < query_num; qi++) {
      char qtype[8];
      int v, w;
      assert(fscanf(fp, "%d %s %d %d", &time_idx, qtype, &v, &w) != EOF);
      if(strcmp(qtype, "insert") == 0) {
        igraph_vector_push_back(&ipairs, v);
        igraph_vector_push_back(&ipairs, w);
      }
      if(strcmp(qtype, "delete") == 0) {
        igraph_vector_push_back(&dpairs, v);
        igraph_vector_push_back(&dpairs, w);
      }
    }

    // Calculate and print
    double time_igraph
      = update_igraph(&G, &ipairs, &dpairs, &Btrue);
    double time_update
      = update_proposed(&Gmod, &D, &Sigma, &Delta, &ipairs, &dpairs, &B);

    /*
    igraph_matrix_t Dtrue;
    igraph_matrix_int_t Sigmatrue;
    igraph_matrix_t Deltatrue;
    aug_dist(&Gmod, &Dtrue, &Sigmatrue, weight);
    pairwise_dependency(&Gmod, &Deltatrue, weight);
    printf("%f %d %f\n",
           igraph_matrix_maxdifference(&D, &Dtrue),
           igraph_matrix_int_maxdifference(&Sigma, &Sigmatrue),
           igraph_matrix_maxdifference(&Delta, &Deltatrue));
    igraph_matrix_destroy(&Dtrue);
    igraph_matrix_int_destroy(&Sigmatrue);
    igraph_matrix_destroy(&Deltatrue);
    */

    igraph_vector_t degree;
    igraph_integer_t active_order = 0;
    igraph_vector_init(&degree, igraph_vcount(&G));
    igraph_degree(&G, &degree, igraph_vss_all(), IGRAPH_ALL, 0);
    for(int i = 0; i < igraph_vector_size(&degree); i++)
      if(igraph_vector_e(&degree, i) > 0)
        active_order++;
    igraph_vector_destroy(&degree);
    // time, order, order-delta-plus, order-delta-minus,
    // error, time-proposed, time-igraph,
    printf("%d,%d,%ld,%ld,%e,%e,%e\n",
           time_idx,
           active_order,
           igraph_vector_size(&ipairs) / 2,
           igraph_vector_size(&dpairs) / 2,
           igraph_vector_maxdifference(&B, &Btrue),
           time_update,
           time_igraph);

    igraph_vector_destroy(&ipairs);
    igraph_vector_destroy(&dpairs);
  }

  fclose(fp);

  igraph_matrix_destroy(&Delta);
  igraph_matrix_destroy(&D);
  igraph_matrix_int_destroy(&Sigma);

  igraph_vector_destroy(&Btrue);
  igraph_vector_destroy(&B);
  igraph_destroy(&G);
  igraph_destroy(&Gmod);

  return 0;
}

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments *arguments = state->input;
  switch(key) {
  case 'f':
    arguments->file = arg;
    break;
  case 'w':
    arguments->weight = arg;
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

