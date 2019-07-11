library(readr)
library(tidyr)
library(dplyr)
library(xtable)

out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.tex')

data_time <- read_csv('../../res/data/bc-hep.csv') %>%
  select(-c(name, n, k, seed)) %>%
  gather(
    key = 'method', value = 'time',
    `time-proposed`, `time-brandes`
  ) %>%
  mutate(method = sub('time-', '', method)) %>%
  group_by(method, mode) %>%
  summarise(
    MaxErr = format(max(err), digits = 3, scientific = TRUE),
    Min = min(time), Median = median(time),
    Mean = mean(time), Max = max(time)
  ) %>%
  ungroup() %>%
  mutate(MaxErr = ifelse(method == 'brandes', 0, MaxErr))
names(data_time) <- c('手法', '更新種別', 'brandesとの差の最大値', '最小値(s)', '中央値(s)', '平均値(s)', '最大値(s)')

print(
  xtable(data_time,
         caption = 'HepPhに対する各アルゴリズムの実行時間',
         label = 'tab:exp-hep-time'
         ),
  caption.placement = 'top',
  include.rownames = FALSE,
  file = out_file
  )

