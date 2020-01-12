library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 9, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_dir <- '../../res/data/'
data_static <- read_csv(paste0(data_dir, 'road-oka-vertex-betweenness.csv'))
bc_max = max(data_static$bc)
data_dynamic <- read_csv(paste0(data_dir, 'road-oka-betweenness-max.csv'))
data_dynamic <- data_dynamic %>%
    mutate(
        query = factor(
            query,
            levels = c('insert', 'delete'),
            labels = c('挿入', '削除')
        )
    )

bc_max
data_dynamic %>% arrange(`bc-after-max`) %>% head(n = 3)
data_dynamic %>% arrange(`bc-after-max`) %>% tail(n = 3)

gp <- ggplot(data_dynamic, aes(query, `bc-after-max`, colour = `query`)) +
    geom_jitter(alpha = 0.2) +
    geom_hline(yintercept = bc_max) +
    xlab('操作') +
    ylab('操作後の媒介中心性の最大値') +
    scale_colour_viridis(discrete = TRUE) +
    theme(
        legend.title = element_blank(),
        legend.position = 'none',
    )

ggsave(out_file, gp, cairo_pdf, width = 7, height = 7, units = 'cm')
