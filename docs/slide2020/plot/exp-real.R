library(readr)
library(tidyr)
library(dplyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 8, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

geomean = function(x) { exp(sum(log(x)) / length(x)) }

data_net_info <- read_csv('../../res/data/real-network-info.csv') %>%
    rename(nodes = order, links = size)

data_time <- read_csv('../../res/data/real-performance-comparison.csv') %>%
    select(c('network', 'query', 'time-proposed', 'time-brandes')) %>%
    mutate(network = sub('.edgelist', '', network)) %>%
    filter(grepl('^ca-|^musae-|^soc-sign-bitcoin|^sx-', network)) %>%
    filter(query == 'delete') %>%
    group_by(network, query) %>%
    summarise(
        `brandes-median` = median(`time-brandes`),
        `proposed-median` = median(`time-proposed`)
    ) %>%
    ungroup() %>%
    mutate(
        `brandes-ratio` = `brandes-median` / `brandes-median`,
        `proposed-ratio` = `proposed-median` / `brandes-median`
    ) %>%
    select(c('network', 'brandes-ratio', 'proposed-ratio'))

data_time <- rbind(
    data_time,
    t(as.data.frame(c(
        network = '幾何平均',
        `brandes-ratio` = 1.0,
        `proposed-ratio` = geomean(data_time$`proposed-ratio`)
    )))
)

data_time <- data_time %>% gather(
    key = 'method', value = 'ratio',
    `brandes-ratio`, `proposed-ratio`
) %>%
    mutate(network = factor(network)) %>%
    mutate(ratio = as.numeric(ratio)) %>%
    mutate(
        method = factor(
            method,
            levels = c('brandes-ratio', 'proposed-ratio'),
            labels = c('Brandes法', '提案手法')
        )
    )

summary(data_time %>% filter(method == '提案手法'))
summary(data_time %>% filter(network == '幾何平均'))

gp <- ggplot(
    data_time, aes(network, ratio, fill = method)
) +
    geom_bar(stat = 'identity', position = position_dodge()) +
    scale_fill_viridis(discrete = TRUE, begin = 0.1, end = 0.9) +
    scale_x_discrete(limits = rev(levels(data_time$network))) +
    coord_flip() +
    ylab('相対実行時間') +
    theme(
        legend.title = element_blank(),
        legend.position = 'top',
        axis.title.y = element_blank(),
        axis.text.y = element_text(hjust = 0),
        legend.key.size = unit(0.2, 'cm')
    )

ggsave(out_file, gp, cairo_pdf, width = 6, height = 8, units = 'cm')
