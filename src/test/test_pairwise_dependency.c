
#include <math.h>
#include <assert.h>
#include <igraph/igraph.h>

#include "dybc/aug_dist.h"
#include "dybc/betweenness.h"
#include "minigraph.h"

int check_pairwise_dependency_of_minigraph(igraph_matrix_t*);

int main(int argc, char* argv[]) {
  igraph_i_set_attribute_table(&igraph_cattribute_table);

  igraph_t G;
  make_test_graph(&G, "length");

  igraph_matrix_t Delta;
  pairwise_dependency(&G, &Delta, "length");

  assert(check_pairwise_dependency_of_minigraph(&Delta));

  igraph_matrix_destroy(&Delta);
  igraph_destroy(&G);

  return 0;
}

