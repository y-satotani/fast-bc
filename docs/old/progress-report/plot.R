library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)

data <- read_csv('experiment_data.csv') %>%
  gather(key = 'algorithm', value = 'time',
         `time-brandes`, `time-dm`, `time-es`) %>%
  mutate(algorithm = gsub('time-', '', algorithm)) %>%
  mutate(n = factor(n)) %>%
  rename(param = `p-or-m`) %>%
  select(n, type, param, algorithm, time)

theme_set(theme_light())

gp <- ggplot(
  data %>%
    filter(type == 'random', param == 0.1 | param == 0.4 | param == 0.7) %>%
    rename(p = param),
  aes(n, time, color = algorithm)) +
  facet_grid(. ~ p, labeller = label_both) +
  geom_boxplot() +
  ggtitle('Random graph') +
  scale_y_continuous(trans = 'log10')
ggsave('random.pdf', gp, width = 15, height = 6, units = 'cm')

gp <- ggplot(
  data %>%
    filter(type == 'scale-free', param == 4 | param == 8 | param == 12) %>%
    rename(m = param),
  aes(n, time, color = algorithm)) +
  facet_grid(. ~ m, labeller = label_both) +
  geom_boxplot() +
  ggtitle('Scale-free graph') +
  scale_y_continuous(trans = 'log10')
ggsave('scale-free.pdf', gp, width = 15, height = 6, units = 'cm')
