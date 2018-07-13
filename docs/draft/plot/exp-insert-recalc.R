library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 10, base_family = 'IPAexGothic'))

data_ratio <- read_csv('../../res/data/bc-20180419-fixed.csv') %>%
  filter(query == 'insert') %>%
  mutate(`time-ratio` = `time-update` / `time-noupdate`)

data_fig4 <- data_ratio %>% filter(n == 100, k == 10)
gp <- ggplot(data_fig4, aes(`update-frac`, `time-update`, colour = query)) +
  facet_wrap(~graph) + geom_point() +
  xlab('更新頂点割合') + ylab('実行時間') +
  scale_colour_viridis(discrete = TRUE) +
  theme(legend.position = 'none')

ggsave('exp-insert-recalc.pdf', gp, cairo_pdf, width = 15, height = 5, units = 'cm')
