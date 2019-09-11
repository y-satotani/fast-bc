library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 9))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/artificial.csv') %>%
  filter(name %in% c('ER', 'BA')) %>%
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
      levels = c('proposed-max', 'proposed-mean', 'brandes-max', 'brandes-mean'),
      labels = c('proposed-max', 'proposed-mean', 'brandes-max', 'brandes-mean')
    ),
    name = factor(
      name,
      levels = c('RRG', 'ER', 'BA'),
      labels = c('Ramdom regular graph', 'Erdős–Rényi model', 'Barabási–Albert model')
    ),
    mode = factor(
      mode,
      levels = c('insert', 'delete'),
      labels = c('insert', 'delete')
    ),
  )

gp <- ggplot(
    data_time %>% filter(k == 4, mode == 'delete'),
    aes(n, time, colour = method, shape = method, linetype = method)
  ) +
  geom_line() + geom_point() +
  facet_grid(cols = vars(name)) +
  xlab('Order') + ylab('Execution time(s)') +
  scale_colour_viridis(discrete = TRUE, begin = 0.1, end = 0.9) +
  guides(colour = guide_legend(nrow = 2, byrow = TRUE)) +
  theme(
    legend.title = element_blank(),
    legend.position = 'top',
    legend.margin = margin(t = 0, b = 0, unit = 'cm'),
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 8, height = 6, units = 'cm')
