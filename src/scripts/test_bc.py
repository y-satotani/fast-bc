#!/usr/bin/env python3

import random, sys, time
from math import inf, isinf
from itertools import product
import numpy as np
import networkx as nx

from betweenness_updater import BetweennessUpdater

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
    is_digraph = sys.argv[5] == 'directed'
    random.seed(seed)
    #assert(k % 2 == 0)
    if topology == 'erdos-renyi':
        G = nx.erdos_renyi_graph(n, k/(n-1), seed=seed)
        pass
    elif topology == 'random-regular':
        G = nx.random_regular_graph(k, n, seed=seed)
    elif topology == 'barabasi-albert':
        G = nx.barabasi_albert_graph(n, k // 2, seed=seed)
    if is_digraph:
        G, G_ = nx.DiGraph(), G
        for e in G_.edges:
            G.add_edge(*(e if random.random() > 0.5 else (e[1], e[0])))
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
    updater = BetweennessUpdater(G, weight='length')
    # update process
    time_start = time.time()
    if query == 'insert':
        updater.insert_edge(v, w, c)
    elif query == 'delete':
        updater.delete_edge(v, w)
    time_end = time.time()
    time_proposed = time_end - time_start

    updater_ = BetweennessUpdater(G, weight='length')
    dist_true, sigma_true = updater_.dist, updater_.sigma
    deps_true = updater_.delta
    time_start = time.time()
    betweenness_true = nx.betweenness_centrality(
        G, weight=weight, normalized=False
    )
    if not G.is_directed():
        betweenness_true = dict((v, 2*b) for v, b in betweenness_true.items())
    time_end = time.time()
    time_brandes = time_end - time_start

    print('{},{},{},{},{},{},{},{}'.format(
        topology, n, k, seed, query,
        max(abs(updater.betweenness[x] - betweenness_true[x]) for x in G.nodes),
        time_proposed, time_brandes))
    exit()
    keys = sorted(G.nodes)
    D = np.array([[updater.dist[x, y] for y in keys] for x in keys])
    D_true = np.array([[dist_true[x, y] for y in keys] for x in keys])
    print('-----')
    DDelta = D-D_true
    DDelta[np.isnan(DDelta)] = 0
    print(D_true)
    print(DDelta)
    print('-----')
    Sigma = np.array([[updater.sigma[x, y] for y in keys] for x in keys])
    Sigma_true = np.array([[sigma_true[x, y] for y in keys] for x in keys])
    print('-----')
    print(Sigma)
    print(Sigma_true)
    print(Sigma-Sigma_true)
    print('-----')

    B = np.array([updater.betweenness[x] for x in keys])
    B_true = np.array([betweenness_true[x] for x in keys])
    deps = np.array([[updater.delta[x, y] for y in keys] for x in keys])
    deps_true = np.array([[deps_true[x, y] for y in keys] for x in keys])
    print(deps)
    print(deps_true)
    print(deps - deps_true)
    print(B)
    print(B_true)
    print(seed, is_digraph, np.max(np.abs(deps-deps_true)))
    exit()
    import matplotlib.pyplot as plt
    pos = nx.spring_layout(G)
    nx.draw(G, pos, with_labels=True)
    nx.draw_networkx_edge_labels(G, pos, edge_labels=dict((e, G.edges[e]['length']) for e in G.edges))
    plt.show()


if __name__ == '__main__':
    main()
