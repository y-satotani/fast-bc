library(readr)
library(dplyr)
library(tidyr)
library(ggplot2)
library(viridis)
theme_set(theme_light(base_size = 9, base_family = 'IPAexGothic'))
out_file <- paste0(sub('^--file=(.+)\\.R$', '\\1', basename(commandArgs()[4])), '.pdf')

data_static <- read_csv(
  '../../res/data/road-oka-static.csv',
  col_types = list(vert = col_character(), bc = col_double())
)

data_dynamic <- read_csv(
  '../../res/data/road-oka-dynamic.csv',
  col_types = list(
    vert1 = col_character(),
    vert2 = col_character(),
    `max-bc` = col_double()
  )
) %>%
  select(c(vert1, vert2, query, `max-bc`))

data <- data_dynamic %>%
  filter(!is.nan(`max-bc`)) %>%
  inner_join(data_static, by = c('vert1' = 'vert')) %>%
  rename('bc1' = 'bc') %>%
  inner_join(data_static, by = c('vert2' = 'vert')) %>%
  rename('bc2' = 'bc') %>%
  mutate(`min-bc-on-pair` = pmin(bc1, bc2)) %>%
  mutate(query = factor(
    query, levels = c('insert', 'delete'), labels = c('挿入', '削除')
  )) %>%
  arrange(`min-bc-on-pair`)

gp <- ggplot(data, aes(`min-bc-on-pair`, `max-bc`)) +
  facet_grid(cols = vars(`query`)) +
  geom_point(alpha = 0.2, colour = 'royalblue') +
  xlab('操作辺と接続する頂点の媒介中心性の最小値') +
  ylab('操作後の全体の媒介中心性の最大値') +
  theme(
    legend.title = element_blank(),
    legend.position = 'none',
    strip.text = element_text(colour = 'black'),
    strip.background = element_blank()
  )

ggsave(out_file, gp, cairo_pdf, width = 12, height = 7, units = 'cm')
