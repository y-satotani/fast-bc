library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
library(latex2exp)
theme_set(theme_light(base_size = 9))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')



data_time <- read_csv('../../res/data/artificial-performance-comparison.csv') %>%
    separate(
        'network',
        c('topology', 'order', 'degree', NA, NA, 'net-seed', NA)
    ) %>%
    mutate(order = as.numeric(order), degree = as.numeric(degree)) %>%
    filter(degree == 4, `is-weighted` == 'unweighted') %>%
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
            labels = c('proposed', 'Brandes')
        ),
        topology = factor(
            topology,
            levels = c('RRG', 'BA'),
            labels = c('RRG', 'BA')
        ),
        query = factor(
            `query`,
            levels = c('insert', 'delete'),
            labels = c('insert', 'delete')
        ),
        `is-weighted` = factor(
            `is-weighted`,
            levels = c('unweighted', 'weighted'),
            labels = c('unweighted', 'weighted')
        )
    )

gp <- ggplot(
    data_time,
    aes(order, time, colour = method, shape = method, linetype = method)
) +
    geom_line() + geom_point() +
    facet_grid(rows = vars(query), cols = vars(topology)) +
    xlab(TeX('$n$')) + ylab('Execution time(s)') +
    scale_x_log10() + scale_y_log10() +
    scale_colour_viridis(discrete = TRUE) +
    theme(
        legend.title = element_blank(),
        legend.position = 'top',
        strip.text = element_text(colour = 'black'),
        strip.background = element_blank()
    )

ggsave(out_file, gp, cairo_pdf, width = 10, height = 8, units = 'cm')
