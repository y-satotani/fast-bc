#include "igraph_bheap.h"
#include <igraph/igraph.h>

void igraph_bheap_init(igraph_bheap_t* heap, igraph_integer_t bsize) {
  igraph_vector_ptr_init(&heap->bptr, bsize);
  heap->bi_min = bsize;
  heap->bi_max = -1;
  heap->n_elem = 0;
}

igraph_bool_t igraph_bheap_empty(igraph_bheap_t* heap) {
  return heap->n_elem == 0;
}

igraph_integer_t igraph_bheap_size(igraph_bheap_t* heap) {
  return heap->n_elem;
}

void igraph_bheap_destroy(igraph_bheap_t* heap) {
  for(igraph_integer_t i = 0; i < igraph_vector_ptr_size(&heap->bptr); i++) {
    igraph_vector_int_t* bucket = VECTOR(heap->bptr)[i];
    if(bucket) {
      igraph_vector_int_destroy(bucket);
      free(bucket);
    }
  }
  igraph_vector_ptr_destroy(&heap->bptr);
}

void igraph_bheap_push_with_index(igraph_bheap_t* heap,
                                  igraph_integer_t elem,
                                  igraph_integer_t bi) {
  if(!VECTOR(heap->bptr)[bi]) {
    igraph_vector_int_t* bucket
      = (igraph_vector_int_t*) malloc(sizeof(igraph_vector_int_t));
    igraph_vector_int_init(bucket, 0);
    VECTOR(heap->bptr)[bi] = bucket;
  }
  igraph_vector_int_push_back(VECTOR(heap->bptr)[bi], elem);
  if(heap->bi_max < bi) heap->bi_max = bi;
  if(heap->bi_min > bi) heap->bi_min = bi;
  heap->n_elem++;
}

igraph_integer_t igraph_bheap_max_index(igraph_bheap_t* heap) {
  return igraph_vector_int_tail(VECTOR(heap->bptr)[heap->bi_max]);
}

igraph_integer_t igraph_bheap_delete_max(igraph_bheap_t* heap) {
  igraph_integer_t res = heap->bi_max;
  igraph_vector_int_pop_back(VECTOR(heap->bptr)[heap->bi_max]);
  while(heap->bi_max >= 0
        && (!VECTOR(heap->bptr)[heap->bi_max]
            || igraph_vector_int_empty(VECTOR(heap->bptr)[heap->bi_max]))
        )
    heap->bi_max--;
  heap->n_elem--;
  return res;
}

igraph_integer_t igraph_bheap_min_index(igraph_bheap_t* heap) {
  return igraph_vector_int_tail(VECTOR(heap->bptr)[heap->bi_min]);
}

igraph_integer_t igraph_bheap_delete_min(igraph_bheap_t* heap) {
  igraph_integer_t res = heap->bi_min;
  igraph_vector_int_pop_back(VECTOR(heap->bptr)[heap->bi_min]);
  while(heap->bi_min < igraph_vector_ptr_size(&heap->bptr)
        && (!VECTOR(heap->bptr)[heap->bi_min]
            || igraph_vector_int_empty(VECTOR(heap->bptr)[heap->bi_min]))
        )
    heap->bi_min++;
  heap->n_elem--;
  return res;
}

void igraph_2wbheap_init(igraph_2wbheap_t* heap,
                         igraph_integer_t esize,
                         igraph_integer_t bsize) {
  igraph_vector_ptr_init(&heap->bptr, bsize);
  igraph_vector_int_init(&heap->bids, esize);
  heap->bi_min = bsize;
  heap->bi_max = -1;
  heap->n_elem = 0;
}

igraph_bool_t igraph_2wbheap_empty(igraph_2wbheap_t* heap) {
  return heap->n_elem == 0;
}

igraph_integer_t igraph_2wbheap_size(igraph_2wbheap_t* heap) {
  return heap->n_elem;
}

void igraph_2wbheap_destroy(igraph_2wbheap_t* heap) {
  for(igraph_integer_t i = 0; i < igraph_vector_ptr_size(&heap->bptr); i++) {
    igraph_vector_int_t* bucket = VECTOR(heap->bptr)[i];
    if(bucket) {
      igraph_vector_int_destroy(bucket);
      free(bucket);
    }
  }
  igraph_vector_ptr_destroy(&heap->bptr);
  igraph_vector_int_destroy(&heap->bids);
}

void igraph_2wbheap_push_with_index(igraph_2wbheap_t* heap,
                                  igraph_integer_t elem,
                                  igraph_integer_t bi) {
  if(!VECTOR(heap->bptr)[bi]) {
    igraph_vector_int_t* bucket
      = (igraph_vector_int_t*) malloc(sizeof(igraph_vector_int_t));
    igraph_vector_int_init(bucket, 0);
    VECTOR(heap->bptr)[bi] = bucket;
  }
  igraph_vector_int_push_back(VECTOR(heap->bptr)[bi], elem);
  igraph_vector_int_set(&heap->bids, elem, bi+1);
  if(heap->bi_max < bi) heap->bi_max = bi;
  if(heap->bi_min > bi) heap->bi_min = bi;
  heap->n_elem++;
}

igraph_integer_t igraph_2wbheap_max_index(igraph_2wbheap_t* heap) {
  return igraph_vector_int_tail(VECTOR(heap->bptr)[heap->bi_max]);
}

igraph_integer_t igraph_2wbheap_delete_max(igraph_2wbheap_t* heap) {
  igraph_integer_t res = heap->bi_max;
  igraph_integer_t elem
    = igraph_vector_int_pop_back(VECTOR(heap->bptr)[heap->bi_max]);
  igraph_vector_int_set(&heap->bids, elem, 0);
  while(heap->bi_max >= 0
        && (!VECTOR(heap->bptr)[heap->bi_max]
            || igraph_vector_int_empty(VECTOR(heap->bptr)[heap->bi_max]))
        )
    heap->bi_max--;
  heap->n_elem--;
  return res;
}

igraph_integer_t igraph_2wbheap_min_index(igraph_2wbheap_t* heap) {
  return igraph_vector_int_tail(VECTOR(heap->bptr)[heap->bi_min]);
}

igraph_integer_t igraph_2wbheap_delete_min(igraph_2wbheap_t* heap) {
  igraph_integer_t res = heap->bi_min;
  igraph_integer_t elem
    = igraph_vector_int_pop_back(VECTOR(heap->bptr)[heap->bi_min]);
  igraph_vector_int_set(&heap->bids, elem, 0);
  while(heap->bi_min < igraph_vector_ptr_size(&heap->bptr)
        && (!VECTOR(heap->bptr)[heap->bi_min]
            || igraph_vector_int_empty(VECTOR(heap->bptr)[heap->bi_min]))
        )
    heap->bi_min++;
  heap->n_elem--;
  return res;
}

igraph_bool_t igraph_2wbheap_has_elem(igraph_2wbheap_t* heap,
                                      igraph_integer_t elem) {
  return VECTOR(heap->bids)[elem] > 0;
}

igraph_integer_t igraph_2wbheap_get(igraph_2wbheap_t* heap,
                                    igraph_integer_t elem) {
  return VECTOR(heap->bids)[elem] - 1;
}

void igraph_2wbheap_modify(igraph_2wbheap_t* heap,
                           igraph_integer_t elem,
                           igraph_integer_t bi) {
  igraph_integer_t old_bi = VECTOR(heap->bids)[elem] - 1;
  igraph_vector_int_t* bucket = VECTOR(heap->bptr)[old_bi];
  for(int i = igraph_vector_int_size(bucket) - 1; i >= 0; i--)
    if(igraph_vector_int_e(bucket, i) == elem) {
      igraph_vector_int_remove(bucket, i);
      break;
    }
  while(heap->bi_max >= 0
        && (!VECTOR(heap->bptr)[heap->bi_max]
            || igraph_vector_int_empty(VECTOR(heap->bptr)[heap->bi_max]))
        )
    heap->bi_max--;
  while(heap->bi_min < igraph_vector_ptr_size(&heap->bptr)
        && (!VECTOR(heap->bptr)[heap->bi_min]
            || igraph_vector_int_empty(VECTOR(heap->bptr)[heap->bi_min]))
        )
    heap->bi_min++;
  if(!VECTOR(heap->bptr)[bi]) {
    igraph_vector_int_t* bucket
      = (igraph_vector_int_t*) malloc(sizeof(igraph_vector_int_t));
    igraph_vector_int_init(bucket, 0);
    VECTOR(heap->bptr)[bi] = bucket;
  }
  igraph_vector_int_push_back(VECTOR(heap->bptr)[bi], elem);
  igraph_vector_int_set(&heap->bids, elem, bi+1);
  if(heap->bi_max < bi) heap->bi_max = bi;
  if(heap->bi_min > bi) heap->bi_min = bi;
}

