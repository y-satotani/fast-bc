
#ifndef _DYBC_IO_H_
#define _DYBC_IO_H_

#include <stdio.h>
#include <igraph/igraph.h>

int dybc_read_edgelist(igraph_t* G,
                       igraph_vector_t* weights,
                       igraph_bool_t is_directed,
                       FILE* istream);

int dybc_dump_cache(igraph_matrix_t* D,
                    igraph_matrix_int_t* S,
                    igraph_vector_t* B,
                    FILE* ostream);

int dybc_load_cache(igraph_matrix_t* D,
                    igraph_matrix_int_t* S,
                    igraph_vector_t* B,
                    FILE* istream);

#endif // _DYBC_IO_H_
