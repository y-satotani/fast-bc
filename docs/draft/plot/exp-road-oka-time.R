library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 11, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data <- read_csv(
  '../../res/data/road-oka-dynamic.csv',
  col_types = list(`time-proposed` = col_double(), `time-brandes` = col_double())
  ) %>%
  select(c(`time-proposed`, `time-brandes`)) %>%
  mutate(
    `edge-id` = row_number(),
    `time-proposed` = cumsum(`time-proposed`),
    `time-brandes` = cumsum(`time-brandes`)
  ) %>%
  gather(key = 'method', value = 'time', `time-proposed`, `time-brandes`) %>%
  mutate(
    method = factor(
      method,
      levels = c('time-proposed', 'time-brandes'),
      labels = c('更新', '再計算')
    )
  )

gp <- ggplot(data, aes(`edge-id`, `time`, colour = method)) +
  geom_step() +
  xlab('辺の数') +
  ylab('積算計算時間[s]') +
  scale_colour_viridis(discrete = TRUE, begin = 0.3, end = 0.7) +
  theme(
    legend.title = element_blank(),
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 18, height = 12, units = 'cm')
