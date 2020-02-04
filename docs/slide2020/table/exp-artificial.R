library(readr)
library(tidyr)
library(dplyr)
library(xtable)

out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.tex')

data <- read_csv('../../res/data/artificial-performance-comparison.csv') %>%
    separate(
        'network',
        c('topology', 'order', 'degree', NA, NA, 'net-seed', NA)
    ) %>%
    mutate(order = as.numeric(order), degree = as.numeric(degree)) %>%
    filter(query == 'delete', order %in% c(500, 1000)) %>%
    group_by(topology, order, degree, query, `is-weighted`) %>%
    summarise(
        `Brandes'-mean` = mean(`time-brandes`),
        `Brandes'-max` = max(`time-brandes`),
        `proposed-mean` = mean(`time-proposed`),
        `proposed-max` = max(`time-proposed`),
        speedup = mean(`time-brandes` / `time-proposed`)
    ) %>%
    ungroup() %>%
    select(-c(query)) %>%
    select(c(topology, `is-weighted`, order, degree,
             `Brandes'-mean`, `Brandes'-max`,
             `proposed-mean`, `proposed-max`,
             speedup)) %>%
    arrange(topology, `is-weighted`, order, degree) %>%
    mutate(`is-weighted` = if_else(`is-weighted` == 'unweighted', '重みなし', '重み付き')) %>%
    rename(
        `モデル` = `topology`,
        `重み` = `is-weighted`,
        `頂点数` = `order`,
        `次数` = `degree`,
        `Brandes法-平均` = `Brandes'-mean`,
        `Brandes法-最大` = `Brandes'-max`,
        `提案手法-平均` = `proposed-mean`,
        `提案手法-最大` = `proposed-max`,
        `性能比` = `speedup`
    )

print(
    xtable(data,
           caption = '人工ネットワーク上での性能比較(秒)',
           label = 'tab:exp-artificial',
           digits = c(0, 0, 0, 0, 0, 3, 3, 3, 3, 2)
           ),
    size = '\\fontsize{6pt}{8pt}\\selectfont',
    caption.placement = 'top',
    include.rownames = FALSE,
    file = out_file
)
