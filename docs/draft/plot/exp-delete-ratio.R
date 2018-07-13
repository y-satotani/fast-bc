library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 10, base_family = 'IPAexGothic'))

data_ratio <- read_csv('../../res/data/bc-20180419-fixed.csv') %>%
  filter(query == 'delete') %>%
  mutate(`time-ratio` = `time-update` / `time-noupdate`)

data_fig3 <- data_ratio %>% filter(n >= 100, k >= 10) %>%
  group_by(graph, n, k) %>%
  summarise(`time-ratio` = mean(`time-ratio`)) %>%
  ungroup()
gp <- ggplot(data_fig3, aes(n, k, fill = `time-ratio`)) +
  facet_wrap(~graph) + geom_tile() +
  xlab('頂点数') + ylab('平均次数') +
  scale_fill_gradientn('平均実行時間比', colours = rev(viridis(10))) +
  theme(strip.background = element_rect(fill = 'white'), strip.text = element_text(colour = 'black'))

ggsave('exp-delete-ratio.pdf', gp, cairo_pdf, width = 15, height = 5, units = 'cm')
