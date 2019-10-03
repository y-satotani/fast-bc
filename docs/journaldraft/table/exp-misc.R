library(readr)
library(tidyr)
library(dplyr)
library(xtable)

out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.tex')

data_time <- read_csv('../../res/data/misc.csv') %>%
  select(c('order', 'size', 'gml', 'time-proposed', 'time-brandes')) %>%
  rename(network = gml) %>%
  mutate(network = sub('.gml', '', basename(network))) %>%
  gather(
    key = 'method', value = 'time',
    `time-proposed`, `time-brandes`
  ) %>%
  mutate(method = sub('time-', '', method)) %>%
  group_by(network, order, size, method) %>%
  summarise(mean = mean(time), max = max(time)) %>%
  gather(Var, val, `mean`, `max`) %>%
  unite(Var1, method, Var, sep = '-') %>%
  spread(Var1, val) %>%
  ungroup()

print(
  xtable(data_time,
         caption = 'Execution time on real networks',
         label = 'tab:exp-real'
         ),
  caption.placement = 'top',
  include.rownames = FALSE,
  file = out_file
  )

