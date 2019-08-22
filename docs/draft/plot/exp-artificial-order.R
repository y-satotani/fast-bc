library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 9, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/artificial.csv') %>%
  filter(name %in% c('ER', 'BA')) %>%
  group_by(name, n, k, mode) %>%
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
  )

gp <- ggplot(
    data_time %>% filter(k == 4),
    aes(n, time, colour = method, shape = method, linetype = method)
  ) +
  geom_line() + geom_point() +
  facet_grid(rows = vars(mode), cols = vars(name)) +
  xlab('頂点数') + ylab('実行時間(s)') +
  scale_colour_viridis(discrete = TRUE, begin = 0.1, end = 0.9) +
  theme(
    legend.title = element_blank(),
    legend.position = 'top',
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 12, height = 10, units = 'cm')
