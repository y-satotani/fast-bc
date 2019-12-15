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
    E = list(G.edges)
    random.seed(seed)
    if weighted:
        weights = [random.randint(1, 5) for _ in range(G.size())]
    else:
        weights = [None] * G.size()
    steps = random.randint(100, 101)
    diffs = [0]+[random.randint(0, 10) for _ in range(steps)]
    fp = open(path, 'w')
    for step, diff in enumerate(diffs, 0):
        for _ in range(diff):
            if random.random() > 0.5: # incremental
                G = nx.Graph()
                G.add_edges_from(E)
                H = nx.complement(G)
                eid = random.randint(0, H.size()-1)
                E.append(list(H.edges)[eid])
                weights.append(random.randint(1, 5) if weighted else None)
            else: # decremental
                eid = random.randint(0, len(E)-1)
                del E[eid]
                del weights[eid]
        contents = '\n'.join(
            f'{u} {v} {step} {w}' if w else f'{u} {v} {step}'
            for (u, v), w in zip(E, weights)
        )
        print(contents, file=fp)
    fp.close()

def make_path(topo, n, k, weighted, seed):
    w_label = "weighted" if weighted else "unweighted"
    return f'{topo}-{n}-{k}-{w_label}-{seed:04d}.edgelist'

def main():
    topo, n, k = 'RRG', 1000, 4
    args = [
        (topo, n, k, weighted, seed, make_path(topo, n, k, weighted, seed))
        for weighted in (True, False)
        for seed in range(1, 11)
    ]
    p = Pool()
    list(p.map(worker, args))

if __name__ == '__main__':
    main()
