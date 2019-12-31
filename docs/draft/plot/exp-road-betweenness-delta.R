library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 9, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_dir <- '../../res/data/'
data_static <- read_csv(paste0(data_dir, 'road-oka-edge-betweenness.csv'))
data_dynamic <- read_csv(paste0(data_dir, 'road-oka-betweenness-delta.csv'))
data_dynamic <- data_dynamic %>%
    filter(query == 'delete') %>%
    gather(
        key = `agg-func`,
        value = `bc-delta`,
        `bc-delta-min`, `bc-delta-max`
    ) %>%
    mutate(
        `agg-func` = factor(
            `agg-func`,
            levels = c('bc-delta-max', 'bc-delta-mean', 'bc-delta-min'),
            labels = c('最大', '平均', '最小')
        )) %>%
    inner_join(data_static, by = c('u', 'v')) %>%
    arrange(bc)

gp <- ggplot(data_dynamic, aes(bc, `bc-delta`, colour = `agg-func`)) +
    geom_point(size = 0.1, alpha = 0.3) +
    xlab('辺媒介中心性') +
    ylab('媒介中心性の変化量') +
    theme(
        legend.title = element_blank(),
        legend.position = 'top',
        strip.text = element_text(colour = 'black'),
        strip.background = element_blank()
    )

ggsave(out_file, gp, cairo_pdf, width = 9, height = 7, units = 'cm')
