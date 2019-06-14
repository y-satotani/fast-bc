#!/usr/bin/env python3

import random, sys, time
from math import inf, isinf
import numpy as np
import networkx as nx

from bc_helper import\
    augmented_all_shortest_path_length, pairwise_dependencies
from bc_insert import insert_edge
from bc_delete import delete_edge

# First experiment code for fast update method of
# pairwise dependencies
# input: topology n k seed query
# output: topology,n,k,seed,query,max-err, (continues)
# (contd) update-rate,time-proposed,time-brandes

def main():
    # generate initial graph
    topology = sys.argv[1]
    n, k = int(sys.argv[2]), int(sys.argv[3])
    seed = int(sys.argv[4])
    assert(k % 2 == 0)
    if topology == 'erdos-renyi':
        G = nx.erdos_renyi_graph(n, k/(n-1), seed=seed)
        pass
    elif topology == 'random-regular':
        G = nx.random_regular_graph(k, n, seed=seed)
    elif topology == 'barabasi-albert':
        G = nx.barabasi_albert_graph(n, k // 2, seed=seed)
    # add weight to graph
    weight = 'length'
    length = dict([(e, random.randint(1, 5)) for e in G.edges])
    nx.set_edge_attributes(G, length, weight)
    # find insert/delete edge
    query = sys.argv[5]
    assert(query in ('insert', 'delete'))
    if query == 'insert':
        v, w = random.choice([
            (v, w)
            for v in G.nodes for w in G.nodes
            if not G.has_edge(v, w) and v != w
        ])
        c = random.randint(1, 5)
    elif query == 'delete':
        v, w = random.choice(list(G.edges))

    # calculate APSP with sigma of original graph
    dist, sigma = augmented_all_shortest_path_length(G, weight)
    deps = pairwise_dependencies(G, weight)

    # update process
    time_start = time.time()
    if query == 'insert':
        insert_edge(G, v, w, c, dist, sigma, deps, weight)
    elif query == 'delete':
        delete_edge(G, v, w, dist, sigma, deps, weight)
    bc = dict((x, sum(deps[z, x] for z in G.nodes) / 2) for x in G.nodes)
    time_end = time.time()
    time_proposed = time_end - time_start

    #dist_true, sigma_true = argumented_all_shortest_path_length(G, weight)
    #deps_true = pairwise_dependencies(G, weight)
    time_start = time.time()
    bc_true = nx.betweenness_centrality(G, weight=weight, normalized=False)
    time_end = time.time()
    time_brandes = time_end - time_start

    print('{},{},{},{},{},{},{},{}'.format(
        topology, n, k, seed, query,
        max(abs(bc[x] - bc_true[x]) for x in G.nodes),
        time_proposed, time_brandes))

if __name__ == '__main__':
    main()
