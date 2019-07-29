library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 11, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/artificial.csv') %>%
  gather(
    key = 'update-index', value = 'update-number',
    `updated-path-pairs`, `updated-deps-pairs`
  ) %>%
  mutate(
    `update-index` = factor(
      `update-index`,
      levels = c('updated-path-pairs', 'updated-deps-pairs'),
      labels = c('最短経路', 'ペア依存度')
    ),
    name = factor(
      name,
      levels = c('RRG', 'ER', 'BA'),
      labels = c('ランダム正則グラフ', 'Erdős–Rényiモデル', 'Barabási–Albertモデル')
    ),
    mode = factor(
      mode,
      levels = c('insert', 'delete'),
      labels = c('挿入', '削除')
    ),
    k = factor(k)
  )

gp <- ggplot(
    data_time %>% filter(n == 1000, k %in% c(4, 64)),
    aes(`update-number`, `time-proposed`, colour = mode, shape = k)
  ) +
  geom_point(alpha = 0.2) +
  guides(colour = guide_legend(title = '操作', override.aes = list(alpha = 1))) +
  guides(shape = guide_legend(title = '次数', override.aes = list(alpha = 1))) +
  facet_grid(rows = vars(`update-index`), cols = vars(name)) +
  xlab('更新数') + ylab('実行時間(s)') +
  scale_colour_viridis(discrete = TRUE, begin = 0.8, end = 0.2) +
  theme(
    legend.position = 'top',
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 18, height = 12, units = 'cm')
