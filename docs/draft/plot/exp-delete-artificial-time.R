library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 11, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/bc-20190622.csv') %>%
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
      levels=c('proposed-max', 'proposed-mean', 'brandes-max', 'brandes-mean'),
      labels=c('更新-最大値', '更新-平均値', '再計算-最大値', '再計算-平均値')
    )
  )

gp <- ggplot(
    data_time %>% filter(k == 4, mode == 'delete'),
    aes(n, time, colour = method, shape = method, linetype = method)
  ) +
  facet_wrap(~name) + geom_line() + geom_point() +
  xlab('頂点数') + ylab('実行時間(s)') +
  scale_colour_viridis(discrete = TRUE) +
  theme(
    legend.title = element_blank(),
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 18, height = 6, units = 'cm')
