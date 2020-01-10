library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
library(cowplot)
theme_set(theme_light(base_size = 8, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data <- read_csv('../../res/data/temporal-sfhh.csv') %>%
    rename(`time-real` = time) %>%
    group_by(`time-real`) %>%
    summarise(
        `proposed` = mean(`time-proposed`),
        `brandes` = mean(`time-brandes`),
        `incremental` = max(`n-insert`),
        `decremental` = -max(`n-delete`)
    ) %>%
    ungroup() %>%
    gather(
        key = 'method', value = 'time-execute',
        `proposed`, `brandes`
    ) %>%
    gather(
        key = 'query', value = 'update-size',
        `incremental`, `decremental`
    ) %>%
    mutate(
        method = factor(
            method,
            levels = c('proposed', 'brandes'),
            labels = c('提案手法', 'Brandes法')
        ),
        query = factor(
            query,
            levels = c('incremental', 'decremental'),
            labels = c('挿入', '削除')
        )
    )

gp_top <- ggplot(data, aes(`time-real`, `time-execute`, colour = method)) +
    geom_line(size = 0.1) +
    xlab('実時間(s)') +
    ylab('実行時間(s)') +
    scale_colour_manual(
        breaks = c('提案手法', 'Brandes法'),
        values = c('royalblue', 'firebrick')
    ) +
    theme(
        legend.title = element_blank()
    )

gp_bottom <- ggplot() +
    geom_bar(
        data = data,
        aes(`time-real`, `update-size`, fill = query),
        stat = 'identity'
    ) +
    xlab('実時間(s)') +
    ylab('更新辺数') +
    scale_y_continuous(labels = function(x) abs(x)) +
    scale_fill_manual(
        breaks = c('挿入', '削除'),
        values = c('forestgreen', 'firebrick')
    ) +
    theme(
        legend.title = element_blank()
    )

gp <- plot_grid(gp_top, gp_bottom, ncol = 1, align = 'v', axis = 'lr')
ggsave(out_file, gp, cairo_pdf, width = 14, height = 7, units = 'cm')
