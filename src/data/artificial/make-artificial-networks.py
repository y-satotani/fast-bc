#!/usr/bin/env python

import os
import random
import networkx as nx
from itertools import chain, product
from multiprocessing import Pool

def worker(args):
    topo, n, k, weighted, seed, path = args
    if(topo == 'RRG'):
        G = nx.random_regular_graph(k, n, seed=seed)
    elif(topo == 'BA'):
        G = nx.barabasi_albert_graph(n, k//2, seed=seed)
    random.seed(seed)
    if weighted:
        weights = [random.randint(1, 5) for _ in range(G.size())]
    else:
        weights = [None] * G.size()
    contents = '\n'.join(
        f'{u} {v} {weights[i]}' if weights[i] else f'{u} {v}'
        for i, (u, v) in enumerate(G.edges)
    )
    with open(path, 'w') as fp:
        print(contents, file=fp)

def make_path(topo, n, k, weighted, seed):
    w_label = "weighted" if weighted else "unweighted"
    return f'{topo}-{n}-{k}-{w_label}-{seed:04d}.edgelist'

def main():
    args = [
        (topo, n, k, weighted, seed, make_path(topo, n, k, weighted, seed))
        for topo, n, k, weighted, seed in product(
                ('RRG', 'BA'),
                chain(range(10, 100, 10), range(100, 1000, 100), (1000, 2000)),
                (4, 8, 16, 32, 64),
                (True, False),
                range(1, 11),
        ) if n > k
    ]
    p = Pool()
    list(p.map(worker, args))

if __name__ == '__main__':
    main()
