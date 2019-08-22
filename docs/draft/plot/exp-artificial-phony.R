library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 9, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/artificial.csv') %>%
  filter(name %in% c('ER', 'BA')) %>%
  mutate(
    `time-diff` = `time-brandes` - `time-proposed`,
    `phony-verts` = `updated-bc-verts` - `changed-bc-verts`,
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
    data_time %>% filter(n == 1000, k %in% c(4)),
    aes(`changed-bc-verts`, `updated-bc-verts`, colour = mode)
  ) +
  geom_point(alpha = 0.2) +
  guides(colour = guide_legend(title = '操作', override.aes = list(alpha = 1))) +
  facet_grid(cols = vars(name)) +
  xlab('媒介中心性が変化した頂点数') + ylab('媒介中心性を更新した頂点数') +
  scale_colour_viridis(discrete = TRUE, begin = 0.8, end = 0.2) +
  theme(
    legend.position = 'top',
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 12, height = 8, units = 'cm')
