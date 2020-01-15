library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
library(latex2exp)
library(cowplot)
theme_set(theme_light(base_size = 8, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/artificial-update-statistics.csv') %>%
    separate(
        'network',
        c('topology', 'order', 'degree', NA, 'net-seed', NA)
    ) %>%
    select(-c(`is-directed`)) %>%
    mutate(order = as.numeric(order), degree = as.numeric(degree)) %>%
    filter(topology == 'RRG', order == 1000, degree == 4) %>%
    mutate(
        `pred-time-path` = if_else(
            `is-weighted` == 'weighted',
            (`n-aff-src`+`n-aff-tgt`)*(degree*`upd-path`*log(`upd-path`)),
            (`n-aff-src`+`n-aff-tgt`)*(degree*`upd-path`)
        )
    ) %>%
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

linear_models <- data_time %>%
    group_by(topology, order, degree, query, `is-weighted`) %>%
    do(broom::tidy(lm(`time-path` ~ `pred-time-path`, .))) %>%
    ungroup() %>%
    select(c(topology, order, degree, query, `is-weighted`, term, estimate)) %>%
    spread(key = term, value = estimate) %>%
    rename(intercept = `(Intercept)`, slope = `\`pred-time-path\``) %>%
    left_join(
        data_time %>%
        group_by(topology, order, degree, query, `is-weighted`) %>%
        summarise(x = max(`pred-time-path`, na.rm = TRUE)) %>%
        ungroup(),
        by = c("topology", "order", "degree", "query", "is-weighted")
    )
linear_models

linear_models_glance <- data_time %>%
    group_by(topology, order, degree, query, `is-weighted`) %>%
    do(broom::glance(lm(`time-path` ~ `pred-time-path`, .)))
linear_models_glance

x_label = '計算ステップ数'
y_label = '実行時間(s)'

make_gp = function(is_weighted_, query_) {
    gp <- ggplot(
        data_time %>% filter(`is-weighted` == is_weighted_, query == query_),
        aes(`pred-time-path`, `time-path`)
    ) +
        geom_point(colour = 'royalblue', alpha = 0.2) +
        geom_abline(
            aes(slope = slope, intercept = intercept),
            data = linear_models %>%
                filter(`is-weighted` == is_weighted_, query == query_),
            size = 0.25
        ) +
        guides(
            colour = guide_legend(
                title = '次数', override.aes = list(alpha = 1)
            )
        ) +
        xlab(x_label) +
        ylab(y_label) +
        theme(
            legend.position = 'none'
        )
    return(gp)
}

gp1 = make_gp('重み付き', '挿入')
gp2 = make_gp('重み付き', '削除')
gp3 = make_gp('重みなし', '挿入')
gp4 = make_gp('重みなし', '削除')
gp <- plot_grid(
    gp1, gp2, gp3, gp4,
    ncol = 2,
    label_size = 10,
    labels = c('a', 'b', 'c', 'd')
)
ggsave(out_file, gp, cairo_pdf, width = 10, height = 10, units = 'cm')
