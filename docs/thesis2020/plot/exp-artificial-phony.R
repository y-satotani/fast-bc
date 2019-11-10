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
    aes(`changed-bc-verts`, `updated-bc-verts`)
  ) +
  geom_point(colour = 'royalblue', alpha = 0.2) +
  geom_abline(slope = 1, intercept = 1, size = 0.2) +
  annotate(
      #'text', x = 600, y = 500, label = paste('|', 'italic(V)', '|', 'italic(x)'),
      'text', x = 600, y = 500, label = expression('|'*italic(V)[italic(delta)]*'|'=='|'*italic(V)[italic(delta)]^{'*'}*'|'),
      parse = TRUE, family = 'Times New Roman', size = 3, hjust = 0.3
  ) +
  guides(colour = guide_legend(title = '操作', override.aes = list(alpha = 1))) +
  facet_grid(cols = vars(name), rows = vars(mode)) +
  xlab(expression('媒介中心性が変化した頂点数'~'|'*V[delta]^{'*'}*'|')) +
  ylab(expression('媒介中心性を更新した頂点数'~'|'*V[delta]*'|')) +
  theme(
    legend.position = 'top',
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 12, height = 10, units = 'cm')
