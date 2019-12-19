library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 9, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/result-performance-comparison.csv') %>%
  filter(`is-weighted` == 'weighted') %>%
  separate('network', c('topology', 'order', 'degree', NA, 'net-seed', NA)) %>%
  mutate(order = as.numeric(order), degree = as.numeric(degree)) %>%
  group_by(topology, order, degree, query) %>%
  summarise(
    `proposed-max` = max(`time-proposed`),
    `proposed-mean` = mean(`time-proposed`),
    `brandes-mean` = mean(`time-brandes`),
    `brandes-max` = max(`time-brandes`)
  ) %>%
  ungroup() %>%
  gather(
    key = 'method', value = 'time',
    `proposed-max`, `proposed-mean`, `brandes-mean`, `brandes-max`
  ) %>%
  mutate(
    method = factor(
      method,
      levels = c('proposed-max', 'proposed-mean', 'brandes-max', 'brandes-mean'),
      labels = c('更新-最大値', '更新-平均値', '再計算-最大値', '再計算-平均値')
    ),
    topology = factor(
      topology,
      levels = c('RRG', 'BA'),
      labels = c('ランダム正則グラフ', 'Barabási–Albertモデル')
    ),
    query = factor(
      query,
      levels = c('insert', 'delete'),
      labels = c('挿入', '削除')
    ),
  )

gp <- ggplot(
    data_time %>% filter(degree == 4),
    aes(order, time, colour = method, shape = method, linetype = method)
  ) +
  geom_line() + geom_point() +
  facet_grid(rows = vars(query), cols = vars(topology)) +
    xlab('頂点数') + ylab('実行時間(s)') +
    #scale_x_log10() + scale_y_log10() +
  scale_colour_viridis(discrete = TRUE, begin = 0.1, end = 0.9) +
  theme(
    legend.title = element_blank(),
    legend.position = 'top',
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 12, height = 10, units = 'cm')
