
library(parallel)
library(readr)
library(dplyr)

find_statistics <- function(file) {
    library(readr)
    library(dplyr)
    untar(tarfile, files=c(file))
    data <- read_csv(
        file,
        col_names = c('query', 'ev1', 'ev2', 'ex', 'bc'),
        col_types = cols()
    ) %>%
        mutate(eu = pmin(ev1, ev2), ev = pmax(ev1, ev2)) %>%
        select(-c(ev1, ev2)) %>%
        inner_join(elist_to_graphml, by = c('eu' = 'ev')) %>% rename(u = gv) %>%
        inner_join(elist_to_graphml, by = c('ev' = 'ev')) %>% rename(v = gv) %>%
        inner_join(elist_to_graphml, by = c('ex' = 'ev')) %>% rename(x = gv) %>%
        select(-c(eu, ev, ex)) %>%
        mutate(bc = bc / ((n() - 1) * (n() - 2))) %>%
        rename(`bc-after` = bc) %>%
        inner_join(vertex_betweenness, by = c('x' = 'vert')) %>%
        group_by(query, u, v) %>%
        summarise(
            `bc-after-max` = max(`bc-after`)
        ) %>%
        ungroup()
    file.remove(file)
    return(data)
}

data_dir <- '../../../docs/res/data/'
vertex_betweenness <- read_csv(
    paste0(data_dir, 'road-oka-vertex-betweenness.csv')
)
elist_to_graphml <- read_csv(
    paste0(data_dir, 'road-oka-edgelist-to-graphml.csv'),
    col_names = c('ev', 'gv')
)
tarfile <- paste0(data_dir, 'road-oka-dynamic.tar.gz')

cl <- makeCluster(detectCores())
clusterExport(cl, c('tarfile', 'vertex_betweenness', 'elist_to_graphml'))

files <- untar(tarfile, list = TRUE)
data <- parLapply(cl, files, find_statistics) %>% bind_rows()

stopCluster(cl)
write_csv(data, 'road-oka-betweenness-max.csv')
