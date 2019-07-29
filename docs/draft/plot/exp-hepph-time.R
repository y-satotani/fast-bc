library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 11, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/ca-hepph.csv') %>%
  select(-c(name, n, k, seed)) %>%
  gather(
    key = 'method', value = 'time',
    `time-proposed`, `time-brandes`
  ) %>%
  mutate(method = sub('time-', '', method))

gp <- ggplot(data_time, aes(mode, time, colour = method)) +
  geom_boxplot() + geom_jitter() +
  xlab('操作種類') + ylab('実行時間(s)') +
  scale_colour_viridis(discrete = TRUE, begin = 0.3, end = 0.6) +
  theme(legend.position = c(0.75, 0.5))

ggsave(out_file, gp, cairo_pdf, width = 10, height = 15, units = 'cm')

