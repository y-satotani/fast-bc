library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 11, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/bc-20190622.csv') %>%
  group_by(name, n, k, mode) %>%
  filter(name %in% c('BA', 'ER')) %>%
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
  )

gp <- ggplot(
    data_time %>% filter(k == 4, mode == 'insert'),
    aes(n, time, colour = method, shape = method, linetype = method)
  ) +
  facet_wrap(~name) + geom_line() + geom_point() +
  xlab('頂点数') + ylab('実行時間(s)') +
  scale_colour_viridis(discrete = TRUE) +
  theme(
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 15, height = 6, units = 'cm')
