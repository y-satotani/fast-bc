library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
library(latex2exp)
theme_set(theme_light(base_size = 8, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/artificial-performance-comparison.csv') %>%
    separate(
        'network',
        c('topology', 'order', 'degree', NA, NA, 'net-seed', NA)
    ) %>%
    mutate(order = as.numeric(order), degree = as.numeric(degree)) %>%
    filter(degree == 4, topology == 'RRG') %>%
    group_by(topology, order, degree, query, `is-weighted`) %>%
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
            labels = c('RRG', 'BA')
        ),
        query = factor(
            `query`,
            levels = c('insert', 'delete'),
            labels = c('挿入', '削除')
        ),
        `is-weighted` = factor(
            `is-weighted`,
            levels = c('unweighted', 'weighted'),
            labels = c('重みなし', '重み付き')
        )
    ) %>%
    mutate(
        setting = paste(`is-weighted`, topology, method)
    ) %>%
    mutate(
        setting = factor(
            setting,
            levels = c(
                '重み付き RRG Brandes法',
                '重みなし RRG Brandes法',
                '重み付き RRG 提案手法',
                '重みなし RRG 提案手法'
            )
        )
    )

gp <- ggplot(
    data_time,
    aes(order, time, colour = setting, shape = setting, linetype = setting)
) +
    geom_line() + geom_point() +
    facet_grid(cols = vars(query)) +
    xlab('頂点数') + ylab('実行時間(s)') +
    scale_x_log10() + scale_y_log10() +
    scale_colour_viridis(discrete = TRUE, begin = 0.1, end = 0.9) +
    guides(colour = guide_legend(nrow = 2, byrow = TRUE)) +
    theme(
        legend.title = element_blank(),
        legend.position = 'top',
        strip.text = element_text(colour = 'black'),
        strip.background = element_blank()
    )

ggsave(out_file, gp, cairo_pdf, width = 10, height = 6.5, units = 'cm')
