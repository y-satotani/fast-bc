
#ifndef _IGRAPH_BHEAP_H_
#define _IGRAPH_BHEAP_H_

#include <igraph/igraph.h>

typedef struct igraph_bheap_t {
  igraph_vector_ptr_t bptr;
  igraph_integer_t bi_min;
  igraph_integer_t bi_max;
  igraph_integer_t n_elem;
} igraph_bheap_t;

void igraph_bheap_init(igraph_bheap_t* heap, igraph_integer_t bsize);
void igraph_bheap_destroy(igraph_bheap_t* heap);
igraph_bool_t igraph_bheap_empty(igraph_bheap_t* heap);
igraph_integer_t igraph_bheap_size(igraph_bheap_t* heap);
void igraph_bheap_push_with_index(igraph_bheap_t* heap,
                                  igraph_integer_t elem,
                                  igraph_integer_t bi);
igraph_integer_t igraph_bheap_max_index(igraph_bheap_t* heap);
igraph_integer_t igraph_bheap_delete_max(igraph_bheap_t* heap);
igraph_integer_t igraph_bheap_min_index(igraph_bheap_t* heap);
igraph_integer_t igraph_bheap_delete_min(igraph_bheap_t* heap);

typedef struct igraph_2wbheap_t {
  igraph_vector_ptr_t bptr;
  igraph_vector_int_t b_n_elem;
  igraph_vector_int_t bids;
  igraph_vector_int_t ibids;
  igraph_integer_t bi_min;
  igraph_integer_t bi_max;
  igraph_integer_t n_elem;
} igraph_2wbheap_t;

void igraph_2wbheap_init(igraph_2wbheap_t* heap,
                         igraph_integer_t esize,
                         igraph_integer_t bsize);
void igraph_2wbheap_destroy(igraph_2wbheap_t* heap);
igraph_bool_t igraph_2wbheap_empty(igraph_2wbheap_t* heap);
igraph_integer_t igraph_2wbheap_size(igraph_2wbheap_t* heap);
void igraph_2wbheap_push_with_index(igraph_2wbheap_t* heap,
                                    igraph_integer_t elem,
                                    igraph_integer_t bi);
igraph_integer_t igraph_2wbheap_max_index(igraph_2wbheap_t* heap);
igraph_integer_t igraph_2wbheap_delete_max(igraph_2wbheap_t* heap);
igraph_integer_t igraph_2wbheap_min_index(igraph_2wbheap_t* heap);
igraph_integer_t igraph_2wbheap_delete_min(igraph_2wbheap_t* heap);
igraph_bool_t igraph_2wbheap_has_elem(igraph_2wbheap_t* heap,
                                      igraph_integer_t elem);
igraph_integer_t igraph_2wbheap_get(igraph_2wbheap_t* heap,
                                    igraph_integer_t elem);
void igraph_2wbheap_modify(igraph_2wbheap_t* heap,
                           igraph_integer_t elem,
                           igraph_integer_t bi);

#endif // _IGRAPH_BHEAP_H_
