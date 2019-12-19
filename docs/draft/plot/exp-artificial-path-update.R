library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
library(latex2exp)
theme_set(theme_light(base_size = 9, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/result-update-stats.csv') %>%
  separate('network', c('topology', 'order', 'degree', NA, 'net-seed', NA)) %>%
  mutate(order = as.numeric(order), degree = as.numeric(degree)) %>%
  filter(topology == 'RRG') %>%
  filter(`is-weighted` == 'weighted') %>%
  filter(order == 1000) %>%
  mutate(
    topology = factor(
      topology,
      levels = c('RRG', 'BA'),
      labels = c('ランダム正則グラフ', 'Barabási–Albertモデル')
    ),
    query = factor(
      query,
      levels = c('insert', 'delete'),
      labels = c('挿入', '削除')
    ),
    ) %>%
    mutate(`pred-time-path` = (`n-aff-src` + `n-aff-tgt`) * (degree * `upd-path` * log(`upd-path`)))

linear_models <- data_time %>%
    group_by(topology, order, degree, query, `is-weighted`) %>%
    do(broom::tidy(lm(`time-path` ~ `pred-time-path`, .))) %>%
    ungroup() %>%
    select(c(topology, order, degree, degree, query, `is-weighted`, term, estimate)) %>%
    spread(key = term, value = estimate) %>%
    rename(intercept = `(Intercept)`, slope = `\`pred-time-path\``) %>%
    mutate(slope = sprintf('%.3e', slope)) %>%
    mutate(intercept = sprintf('%.3e', intercept)) %>%
    mutate(`line-label` = as.character(TeX(paste0('$\\mathit{y}=', slope, '\\mathit{x}+', intercept, '$'), output = 'expression')))
linear_models_glance <- data_time %>%
    group_by(topology, order, degree, query, `is-weighted`) %>%
    do(broom::glance(lm(`time-path` ~ `pred-time-path`, .)))

gp <- ggplot(
    data_time %>% filter(degree %in% c(4, 16, 64)),
    aes(`pred-time-path`, `time-path`, colour = factor(degree))
  ) +
    geom_point(alpha = 0.2) +
    geom_text(data = linear_models, aes(x = 100000, y = 0, label = `line-label`), parse = TRUE, size = 3, colour = 'black', family = 'Times New Roman', face = 'italic') +
    geom_smooth(method = 'lm', se = FALSE, colour = 'black', size = 0.2) +
    guides(colour = guide_legend(title = '次数', override.aes = list(alpha = 1))) +
    facet_grid(cols = vars(query)) +
    xlab(TeX('$\\mathit{x}=(|\\mathit{S}(\\mathit{v})|+|\\mathit{T}(\\mathit{u})|)(\\mathit{k}\\bar{\\mathit{A}}+\\bar{\\mathit{A}}\\log\\bar{\\mathit{A}})$')) +
    ylab(TeX('$\\mathit{y}$：実行時間(s)')) +
  scale_colour_viridis(discrete = TRUE, begin = 0.8, end = 0.2) +
  theme(
    legend.position = 'top',
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank(),
    axis.title.x = element_text(family = 'Times New Roman', face = 'italic')
  )

ggsave(out_file, gp, cairo_pdf, width = 12, height = 8, units = 'cm')
