library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
library(latex2exp)
library(cowplot)
theme_set(theme_light(base_size = 9, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_time <- read_csv('../../res/data/artificial-update-stats-time.csv') %>%
    separate('network', c('topology', 'order', 'degree', NA, 'net-seed', NA)) %>%
    select(-c(`is-directed`)) %>%
    mutate(order = as.numeric(order), degree = as.numeric(degree)) %>%
    filter(topology == 'RRG', order == 1000, degree == 4) %>%
    mutate(
        `pred-time-betw` = if_else(
            `is-weighted` == 'weighted',
            `n-aff-src` * (degree * `upd-betw` + `upd-betw` * log(`upd-betw`)),
            `n-aff-src` * (degree * `upd-betw`)
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
    do(broom::tidy(lm(`time-betw` ~ `pred-time-betw`, .))) %>%
    ungroup() %>%
    select(c(topology, order, degree, query, `is-weighted`, term, estimate)) %>%
    spread(key = term, value = estimate) %>%
    rename(intercept = `(Intercept)`, slope = `\`pred-time-betw\``) %>%
    mutate(
        `line-label` = as.character(TeX(paste0(
            '$\\mathit{y}=',
            sprintf('%.3e', slope),
            '\\mathit{x}+',
            sprintf('%.3e', intercept),
            '$'), output = 'expression'))
    ) %>%
    left_join(
        data_time %>%
        group_by(topology, order, degree, query, `is-weighted`) %>%
        summarise(x = max(`pred-time-betw`, na.rm = TRUE)) %>%
        ungroup(),
        by = c("topology", "order", "degree", "query", "is-weighted")
    )

linear_models_glance <- data_time %>%
    group_by(topology, order, degree, query, `is-weighted`) %>%
    do(broom::glance(lm(`time-betw` ~ `pred-time-betw`, .)))
linear_models_glance

x_label_weighted = TeX(paste0(
    '$\\mathit{x}',
    '=|\\mathit{S}(\\mathit{v})|',
    '(\\mathit{k}\\bar{\\mathit{\\tau}}',
    '+\\bar{\\mathit{\\tau}}\\log\\bar{\\mathit{\\tau}})$'
))
x_label_unweighted = TeX(paste0(
    '$\\mathit{x}',
    '=|\\mathit{S}(\\mathit{v})|',
    '\\mathit{k}\\bar{\\mathit{\\tau}}'
))
y_label = TeX('$\\mathit{y}$：実行時間(s)')

make_gp = function(is_weighted_, query_) {
    gp <- ggplot(
        data_time %>% filter(`is-weighted` == is_weighted_, query == query_),
        aes(`pred-time-betw`, `time-betw`, colour = factor(degree))
    ) +
        geom_point(alpha = 0.2) +
        geom_text(
            aes(x = x, y = 0, label = `line-label`),
            data = linear_models %>%
                filter(`is-weighted` == is_weighted_, query == query_),
            parse = TRUE,
            size = 3,
            colour = 'black',
            family = 'Times New Roman',
            vjust = 0,
            hjust = 1
        ) +
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
        xlab(ifelse(
            is_weighted_ == '重み付き', x_label_weighted, x_label_unweighted
        )) +
        ylab(y_label) +
        ggtitle(paste(is_weighted_, query_)) +
        scale_colour_viridis(discrete = TRUE, begin = 0.8, end = 0.2) +
        theme(
            legend.position = 'none',
            plot.title = element_text(size = 10),
            axis.title.x = element_text(family = 'Times New Roman'),
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
    label_size = 10
)
ggsave(out_file, gp, cairo_pdf, width = 15, height = 10, units = 'cm')
