#!/usr/bin/env python

import os
import random
import networkx as nx
from itertools import chain, product
from multiprocessing import Pool

def worker(args):
    topo, n, k, weighted, directed, seed, path = args
    if(topo == 'RRG'):
        G = nx.random_regular_graph(k, n, seed=seed)
    elif(topo == 'BA'):
        G = nx.barabasi_albert_graph(n, k//2, seed=seed)
    random.seed(seed)
    if weighted:
        weights = [random.randint(1, 5) for _ in range(G.size())]
    else:
        weights = [None] * G.size()
    if directed:
        E = [e if random.random() < 0.5 else (e[1], e[0]) for e in G.edges]
    else:
        E = list(G.edges)
    contents = '\n'.join(
        f'{u} {v} {w}' if w else f'{u} {v}' for (u, v), w in zip(E, weights)
    )
    with open(path, 'w') as fp:
        print(contents, file=fp)

def make_path(topo, n, k, weighted, directed, seed):
    w_label = "weighted" if weighted else "unweighted"
    d_label = "directed" if directed else "undirected"
    return f'{topo}-{n}-{k}-{w_label}-{d_label}-{seed:04d}.edgelist'

def main():
    args = [
        (topo, n, k, weighted, directed, seed,
         make_path(topo, n, k, weighted, directed, seed))
        for seed in range(1, 11)
        for topo in ('RRG', 'BA')
        for n in chain(range(10, 100, 10), range(100, 1000, 100), (1000, 2000))
        for k in (4, 8, 16, 32, 64)
        for weighted in (True, False)
        for directed in (True,)
        if n > k
    ]
    p = Pool()
    list(p.map(worker, args))

if __name__ == '__main__':
    main()
