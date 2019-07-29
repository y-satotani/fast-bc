library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
library(cowplot)
theme_set(theme_light(base_size = 11, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data <- read_csv('../../res/data/sfhh.csv') %>%
  rename(`time-idx` = time) %>%
  group_by(`time-idx`) %>%
  summarise(
    `proposed` = mean(`time-proposed`),
    `brandes` = mean(`time-brandes`),
    `incremental` = max(`size-plus`),
    `decremental` = max(`size-minus`)
  ) %>%
  ungroup() %>%
  gather(
    key = 'method', value = 'update-time',
    `proposed`, `brandes`
  ) %>%
  gather(
    key = 'update-type', value = 'update-size',
    `incremental`, `decremental`
  ) %>%
  mutate(
    method = factor(
      method,
      levels = c('proposed', 'brandes'),
      labels = c('更新', '再計算')
    ),
    `update-type` = factor(
      `update-type`,
      levels = c('incremental', 'decremental'),
      labels = c('挿入', '削除')
    )
  )

gp_top <- ggplot(
    data,
    aes(`time-idx`, `update-time`, colour = method)
  ) +
  geom_line() +
  xlab('時間番号') + ylab('実行時間(s)') +
  scale_colour_viridis(discrete = TRUE, begin = 0.2, end = 0.8) +
  theme(
    legend.title = element_blank()
  )

gp_bottom <- ggplot(
    data,
    aes(`time-idx`, `update-size`, fill = `update-type`, colour = `update-type`)
  ) +
  geom_area(position='stack') +
  xlab('時間番号') + ylab('更新辺数') +
  scale_fill_viridis(discrete = TRUE, begin = 0.7, end = 0.3) +
  scale_colour_viridis(discrete = TRUE, begin = 0.7, end = 0.3) +
  theme(
    legend.title = element_blank()
  )

gp <- plot_grid(gp_top, gp_bottom, ncol = 1, align = 'v', axis = 'l')
ggsave(out_file, gp, cairo_pdf, width = 18, height = 12, units = 'cm')
