
#ifndef _DYBC_UPDATE_STATS_H_
#define _DYBC_UPDATE_STATS_H_

typedef struct dybc_update_stats_t {
  double upd_betw;
  double upd_path;
  unsigned long n_tau_hat;
  double update_time;
} dybc_update_stats_t;

#endif // _DYBC_UPDATE_STATS_H_
