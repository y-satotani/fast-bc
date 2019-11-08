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
  ungroup() %>%
  mutate(`order` = as.integer(`order`), `size` = as.integer(`size`))
names(data_time) <- c('ネットワーク名', '頂点数', '辺数', '再計算-最大(s)', '再計算-平均(s)', '更新-最大(s)', '更新-平均(s)')

print(
  xtable(data_time,
         caption = '実ネットワークでの実行時間',
         label = 'tab:exp-real',
         format.args = list(digits = 2, format = c('d', 'd', 'd', 'f', 'f', 'f', 'f'))
         ),
  size="\\fontsize{9pt}{10pt}\\selectfont",
  table.placement = 'tb',
  caption.placement = 'top',
  include.rownames = FALSE,
  file = out_file
  )

