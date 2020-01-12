library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
library(latex2exp)
library(cowplot)
theme_set(theme_light(base_size = 8, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data <- read_csv('../../res/data/artificial-update-statistics.csv') %>%
    separate(
        'network',
        c('topology', 'order', 'degree', NA, 'net-seed', NA)
    ) %>%
    select(-c(`is-directed`)) %>%
    mutate(order = as.numeric(order), degree = as.numeric(degree)) %>%
    filter(order == 1000, degree == 4) %>% # TODO: use all order
    mutate(
        topology = factor(
            topology,
            levels = c('RRG', 'BA'),
            labels = c('ランダム正則グラフ', 'Barabási–Albertモデル')
        ),
        `is-weighted` = factor(
            `is-weighted`,
            levels = c('unweighted', 'weighted'),
            labels = c('重みなし', '重み付き')
        ),
        query = factor(
            query,
            levels = c('insert', 'delete'),
            labels = c('挿入', '削除')
        )
    )

max_slope <- data %>%
    group_by(topology, order, degree, query, `is-weighted`) %>%
    summarise(slope = max(`tau-hat` / `tau-ast`, na.rm = TRUE)) %>%
    ungroup() %>%
    left_join(
        data %>%
        group_by(topology, order, degree, query, `is-weighted`) %>%
        summarise(x = max(`tau-ast`, na.rm = TRUE)) %>%
        ungroup(),
        by = c("topology", "order", "degree", "query", "is-weighted")
    )
max_slope

make_gp = function(topology_, is_weighted_, query_) {
    gp <- ggplot(
        data %>%
        filter(
            topology == topology_,
            `is-weighted` == is_weighted_,
            query == query_
        ),
        aes(`tau-ast`, `tau-hat`, colour = factor(degree))
    ) +
        geom_point(alpha = 0.2) +
        geom_abline(
            aes(slope = slope, intercept = 0),
            data = max_slope %>%
                filter(
                    topology == topology_,
                    `is-weighted` == is_weighted_,
                    query == query_
                ),
            size = 0.25
        ) +
        xlab(expression(tau*"*")) +
        ylab(TeX('$\\mathit{\\hat{\\tau}}$')) +
        xlim(0, max(data$order)) +
        scale_colour_viridis(discrete = TRUE, begin = 0.8, end = 0.2) +
        theme(
            legend.position = 'none',
            plot.title = element_text(size = 10),
            axis.title.x = element_text(family = 'Times New Roman'),
            axis.title.y = element_text(family = 'Times New Roman')
        )
    return(gp)
}

                                        #is_weighted = '重み付き'
                                        #is_weighted = '重みなし'
is_weighted = '重み付き'
gp1 = make_gp('ランダム正則グラフ', is_weighted, '挿入')
gp2 = make_gp('ランダム正則グラフ', is_weighted, '削除')
gp3 = make_gp('Barabási–Albertモデル', is_weighted, '挿入')
gp4 = make_gp('Barabási–Albertモデル', is_weighted, '削除')
gp <- plot_grid(
    gp1, gp2, gp3, gp4,
    ncol = 2,
    label_size = 10,
    labels = c('a', 'b', 'c', 'd')
)
ggsave(out_file, gp, cairo_pdf, width = 9, height = 9, units = 'cm')
