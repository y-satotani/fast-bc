library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 9))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/artificial.csv') %>%
  filter(name %in% c('ER', 'BA')) %>%
  gather(
    key = 'update-index', value = 'update-number',
    `updated-path-pairs`, `updated-deps-pairs`
  ) %>%
  filter(`update-index` == 'updated-deps-pairs') %>%
  mutate(
    `update-index` = factor(
      `update-index`,
      levels = c('updated-path-pairs', 'updated-deps-pairs'),
      labels = c('updated-path-pairs', 'updated-deps-pairs')
    ),
    name = factor(
      name,
      levels = c('RRG', 'ER', 'BA'),
      labels = c('Ramdom regular graph', 'Erdős–Rényi model', 'Barabási–Albert model')
    ),
    mode = factor(
      mode,
      levels = c('insert', 'delete'),
      labels = c('insert', 'delete'),
    ),
    k = factor(k)
  )

gp <- ggplot(
    data_time %>% filter(n == 1000, k %in% c(4, 64), mode == 'delete'),
    aes(`update-number`, `time-proposed`, colour = k)
  ) +
  geom_point(alpha = 0.2) +
  guides(colour = guide_legend(title = 'Average degree', override.aes = list(alpha = 1))) +
  facet_grid(cols = vars(name)) +
  scale_x_continuous(breaks = c(0, 40000), limits = c(0, 80000)) +
  xlab('Update amount') + ylab('Execution time(s)') +
  scale_colour_viridis(discrete = TRUE, begin = 0.8, end = 0.2) +
  theme(
    legend.position = 'top',
    legend.margin = margin(t = 0, b = 0, unit = 'cm'),
    axis.text.x = element_text(vjust = 0.),
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 8, height = 6, units = 'cm')
