library(readr)
library(tidyr)
library(dplyr)
library(xtable)

out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.tex')

data_net_info <- read_csv('../../res/data/real-network-info.csv') %>%
    rename(nodes = order, links = size)

data_time <- read_csv('../../res/data/real-performance-comparison.csv') %>%
    select(c('network', 'query', 'time-proposed', 'time-brandes')) %>%
    mutate(network = sub('.edgelist', '', network)) %>%
    filter(query == 'delete') %>%
    group_by(network, query) %>%
    summarise(
        `Brandes` = median(`time-brandes`),
        `proposed` = median(`time-proposed`),
        speedup = median(`time-brandes` / `time-proposed`)
    ) %>%
    ungroup() %>%
    inner_join(data_net_info) %>%
    select(c('network', 'nodes', 'links', 'is-directed', 'Brandes', 'proposed', 'speedup'))

geomean = function(x) { exp(sum(log(x)) / length(x)) }
geomean_of_speedups = c(
    network = 'geometric mean',
    nodes = NA,
    links = NA,
    `is-directed` = NA,
    `Brandes` = NA,
    `proposed` = NA,
    speedup = geomean(data_time$speedup)
)

data_time <- rbind(data_time, t(as.data.frame(geomean_of_speedups))) %>%
    mutate(
        nodes = as.numeric(nodes),
        links = as.numeric(links),
        `Brandes` = as.numeric(`Brandes`),
        `proposed` = as.numeric(`proposed`),
        speedup = as.numeric(speedup)
    )

hlines = c(-1, 0, nrow(data_time)-1, nrow(data_time))
print(
    xtable(data_time,
           caption = 'Execution time on real networks (in seconds)',
           label = 'tab:exp-real',
           digits = c(0, 0, 0, 0, 0, 2, 2, 2)
           ),
    size = '\\fontsize{6pt}{8pt}\\selectfont',
    caption.placement = 'top',
    include.rownames = FALSE,
    hline.after = hlines,
    file = out_file
)
