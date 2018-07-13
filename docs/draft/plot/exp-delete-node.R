library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 10, base_family = 'IPAexGothic'))

data_time <- read_csv('../../res/data/bc-20180419-fixed.csv') %>%
  filter(query == 'delete') %>%
  gather(key = 'calc-type', value = 'time', `time-update`, `time-noupdate`) %>%
  group_by(graph, n, k, `calc-type`) %>%
  summarise(time = mean(time)) %>%
  ungroup()

data_fig1 <- data_time %>% filter(n >= 100, k == 10)
gp <- ggplot(data_fig1, aes(n, time, colour = `calc-type`)) +
  facet_wrap(~graph) + geom_point() +
  xlab('頂点数') + ylab('平均実行時間(s)') +
  scale_colour_viridis('計算方法', labels = c('time-noupdate' = '再計算', 'time-update' = '更新'), discrete = TRUE) +
  theme(strip.background = element_rect(fill = 'white'), strip.text = element_text(colour = 'black'))

ggsave('exp-delete-node.pdf', gp, cairo_pdf, width = 15, height = 5, units = 'cm')
