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
    arrange(topology, `is-weighted`, order, degree)

headers = c(NA, NA, NA, NA, 'Brandes\'', NA, 'proposed', NA, NA)
headers = c(NA, NA, NA, NA, 'Brandes\'', NA, 'proposed', NA, NA)

print(
    xtable(data,
           caption = 'Execution time on synthesized networks (in seconds)',
           label = 'tab:exp-artificial',
           digits = c(0, 0, 0, 0, 0, 3, 3, 3, 3, 2)
           ),
    size = '\\fontsize{6pt}{8pt}\\selectfont',
    caption.placement = 'top',
    include.rownames = FALSE,
    file = out_file
)
