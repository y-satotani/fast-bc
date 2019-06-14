#!/usr/bin/env python3
import networkx as nx

def make_path_minigraph():
    G = nx.Graph()
    G.add_weighted_edges_from([
        ('A', 'B', 1),
        ('A', 'E', 4),
        ('B', 'C', 2),
        ('B', 'D', 1),
        ('C', 'E', 1),
        ('D', 'E', 2),
        ('E', 'F', 2),
        ('E', 'G', 1),
        ('F', 'H', 2),
        ('G', 'H', 3),
    ])
    return G

def make_incremental_minigraph_and_edge():
    G = nx.Graph()
    G.add_weighted_edges_from([
        ('A', 'D', 3),
        ('B', 'C', 1),
        ('B', 'G', 2),
        ('C', 'D', 3),
        ('C', 'F', 2),
        ('E', 'F', 1),
        ('F', 'G', 1),
        ('G', 'H', 3),
    ])
    e = ('E', 'H', 2)
    return G, e

def make_decremental_minigraph_and_edge():
    G, e = make_incremental_minigraph_and_edge()
    G.add_weighted_edges_from([e])
    return G, e
