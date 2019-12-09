
#ifndef _UPDATE_INFO_H_
#define _UPDATE_INFO_H_

typedef struct update_info_t {
  // input attributes
  igraph_integer_t u;
  igraph_integer_t v;
  igraph_integer_t eid;
  igraph_real_t weight_before;
  igraph_real_t weight_after;
  // output attributes
  unsigned long int total_size_of_affected_targets;
  unsigned long int total_size_of_affected_sources;
  unsigned long int total_size_of_tau;
  unsigned long int total_size_of_tau_prime;
  unsigned long int size_of_tau_hat;
} update_info_t;

#endif // _UPDATE_INFO_H_
