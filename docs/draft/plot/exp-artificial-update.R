library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 9, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/artificial-update-stats.csv') %>%
  filter(topology == 'RRG') %>%
  filter(`is-weighted` == 'weighted') %>%
  gather(
    key = 'update-index', value = 'update-amount',
    `upd-path`, `upd-betw`
  ) %>%
  mutate(
    `update-index` = factor(
      `update-index`,
      levels = c('upd-path', 'upd-betw'),
      labels = c('最短経路', 'ペア依存度')
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
    degree = factor(degree)
  )

gp <- ggplot(
    data_time %>% filter(order == 1000, degree %in% c(4, 16, 64)),
    aes(`update-amount`, `time`, colour = degree)
  ) +
  geom_point(alpha = 0.2) +
  guides(colour = guide_legend(title = '次数', override.aes = list(alpha = 1))) +
  facet_grid(rows = vars(`update-index`), cols = vars(query)) +
  xlab('更新数') + ylab('実行時間(s)') +
  scale_colour_viridis(discrete = TRUE, begin = 0.8, end = 0.2) +
  theme(
    legend.position = 'top',
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 12, height = 8, units = 'cm')
