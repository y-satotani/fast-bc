library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 9, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/result-update-stats.csv') %>%
  separate('network', c('topology', 'order', 'degree', NA, 'net-seed', NA)) %>%
  mutate(order = as.numeric(order), degree = as.numeric(degree)) %>%
  filter(topology == 'RRG') %>%
  filter(`is-weighted` == 'weighted') %>%
  filter(order == 1000) %>%
  mutate(
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
  ) %>%
  mutate(`pred-time-betw` = `n-aff-src` * (degree * `upd-betw` + `upd-betw` * log(`upd-betw`)))

summary(data_time)

gp <- ggplot(
    data_time %>% filter(degree %in% c(4, 16, 64)),
                                        #aes(`upd-betw`, `time-betw`, colour = degree)
    aes(`pred-time-betw`, `time-betw`, colour = factor(degree))
  ) +
  geom_point(alpha = 0.2) +
  guides(colour = guide_legend(title = '次数', override.aes = list(alpha = 1))) +
  facet_grid(cols = vars(query)) +
  xlab('更新数') + ylab('実行時間(s)') +
  scale_colour_viridis(discrete = TRUE, begin = 0.8, end = 0.2) +
  theme(
    legend.position = 'top',
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 12, height = 8, units = 'cm')
