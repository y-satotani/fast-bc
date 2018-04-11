#!/usr/bin/env python
from __future__ import division, print_function
from math import sqrt
from itertools import combinations, product
import random
import sys
import networkx as nx
import numpy as np
from update_path import *

def make_test_graph():
    G = nx.Graph()
    G.add_edges_from([(0, 1), (0, 2), (2, 3), (2, 4), (4, 5)])
    return G

def orig_bc(G):
    _, B = zip(*sorted(
        nx.betweenness_centrality(G, normalized=False).items()
    ))
    return np.array(B)

def get_data(G):
    paths = [set() for _ in range(len(G))]
    D = np.empty((len(G), len(G)))
    S = np.empty((len(G), len(G)))
    for s in xrange(len(G)):
        D[s, s] = 0
        S[s, s] = 1
    for s, t in combinations(xrange(len(G)), 2):
        D[s, t] = np.infty
        S[s, t] = 0
        if nx.has_path(G, s, t):
            for p in nx.all_shortest_paths(G, s, t):
                paths[s].update(zip(p[-2::-1], p[:0:-1]))
                paths[t].update(zip(p[1:], p[:-1]))
                D[s, t] = len(p)-1
                S[s, t] += 1
        D[t, s] = D[s, t]
        S[t, s] = S[s, t]
    return D, S, paths

def my_bc(D, S, paths):
    B = np.zeros((len(paths),))
    for s in range(len(paths)):
        delta = np.zeros(len(paths))
        for i, j in sorted(paths[s], key=lambda e: D[s, e[0]], reverse=True):
            if i == s: continue
            delta[i] += S[s, i] / S[s, j] * (1 + delta[j])
        B += delta
    B /= 2
    return B

def main(**kwargs):
    seed = kwargs['seed']
    random.seed(seed)
    import matplotlib.pyplot as plt
    #G = make_test_graph()
    n, d = 12, 3
    #G = nx.random_regular_graph(d, n)
    G = nx.barabasi_albert_graph(n, d)
    while not nx.is_connected(G):
        G = nx.random_regular_graph(d, n)
    D, S, paths = get_data(G)

    e = random.choice(list(G.edges()))
    G.remove_edge(*e)
    Dnext, Snext, Pnext = update_on_delete(D, S, paths, e)
    Dtrain, Strain, Ptrain = get_data(G)

    B1 = orig_bc(G)
    B2 = my_bc(Dnext, Snext, Pnext)
    print('{},{},{},{},{}'.format(
        seed,
        np.all(Dtrain == Dnext),
        np.all(Strain == Snext),
        Ptrain == Pnext,
        sqrt(np.sum((B1-B2)**2))
    ))

    if kwargs.get('show_net', False):
        s = 0
        if Ptrain != Pnext:
            s = [p1 == p2 for p1, p2 in zip(Ptrain, Pnext)].index(False)
        print('test1:', np.all(Dtrain==Dnext), np.all(Strain==Snext))
        print('test2:', [p1 == p2 for p1, p2 in zip(Ptrain, Pnext)])
        print('test3:', Ptrain[s]-Pnext[s], Pnext[s]-Ptrain[s])
        print('sigma update:', not np.all(np.equal(S, Strain), axis=0)[s])
        print('d update:', not np.all(np.equal(D, Dtrain), axis=0)[s])
        print('removed edge:', e)
        Gb = nx.DiGraph()
        Gb.add_edges_from(paths[s])
        Ga = nx.DiGraph()
        Ga.add_nodes_from(range(len(paths)))
        Ga.add_edges_from(Ptrain[s])
        Gc = nx.DiGraph()
        Gc.add_nodes_from(range(len(paths)))
        Gc.add_edges_from(Pnext[s])
        pos = nx.spring_layout(Gb)
        plt.subplot(1, 3, 1)
        nx.draw_networkx(Gb, pos, with_labels=True)
        plt.axis('off')
        plt.subplot(1, 3, 2)
        nx.draw_networkx(Ga, pos, with_labels=True)
        plt.axis('off')
        plt.subplot(1, 3, 3)
        nx.draw_networkx(Gc, pos, with_labels=True)
        plt.axis('off')
        plt.show()

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print('Seed,Dtest,Stest,Ptest,Err')
        for seed in [random.randint(0, 2**32-1) for s in range(10000)]:
            main(seed=seed, show_net=False)
    else:
        main(seed=int(sys.argv[1]), show_net=True)
