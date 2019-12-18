
#ifndef _DYBC_UPDATE_STATS_H_
#define _DYBC_UPDATE_STATS_H_

typedef struct dybc_update_stats_t {
  double upd_betw;
  double upd_path;
  unsigned long n_tau_hat;
  double time_betw;
  double time_path;
  double time_full;
} dybc_update_stats_t;

#endif // _DYBC_UPDATE_STATS_H_
