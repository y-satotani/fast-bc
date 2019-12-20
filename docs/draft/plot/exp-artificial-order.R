library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 9, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/artificial-performance-comparison.csv') %>%
    separate(
        'network',
        c('topology', 'order', 'degree', NA, 'net-seed', NA)
    ) %>%
    mutate(order = as.numeric(order), degree = as.numeric(degree)) %>%
    filter(degree == 4, `is-weighted` == 'weighted') %>%
    group_by(topology, order, degree, query) %>%
    summarise(
        `proposed-mean` = mean(`time-proposed`),
        `brandes-mean` = mean(`time-brandes`),
        ) %>%
    ungroup() %>%
    gather(
        key = 'method', value = 'time',
        `proposed-mean`, `brandes-mean`
    ) %>%
    mutate(
        method = factor(
            method,
            levels = c('proposed-mean', 'brandes-mean'),
            labels = c('提案手法', 'Brandes法')
        ),
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
    )

gp <- ggplot(
    data_time,
    aes(order, time, colour = method, shape = method, linetype = method)
) +
    geom_line() + geom_point() +
    facet_grid(rows = vars(query), cols = vars(topology)) +
    xlab('頂点数') + ylab('実行時間(s)') +
    scale_x_log10() + scale_y_log10() +
    scale_colour_viridis(discrete = TRUE, begin = 0.1, end = 0.9) +
    theme(
        legend.title = element_blank(),
        legend.position = 'top',
        strip.text = element_text(colour = 'black'),
        strip.background = element_blank()
    )

ggsave(out_file, gp, cairo_pdf, width = 12, height = 10, units = 'cm')
