from math import inf
from collections import defaultdict
from itertools import combinations
import networkx as nx
from networkx.algorithms.centrality.betweenness\
    import _single_source_dijkstra_path_basic

def argumented_all_shortest_path_length(G, weight='length'):
    dist = defaultdict(lambda: inf)
    sigma = defaultdict(lambda: 0)
    for v in G.nodes:
        dist[v, v] = 0
        sigma[v, v] = 1
    for v, w in combinations(G.nodes, 2):
        if not nx.has_path(G, v, w):
            continue
        paths = list(nx.all_shortest_paths(G, v, w, weight))
        d = sum(G.edges[e][weight] if weight != None else 1
                for e in zip(paths[0], paths[0][1:]))
        dist[v, w] = dist[w, v] = d
        sigma[v, w] = sigma[w, v] = len(paths)
    return dist, sigma

def pairwise_dependencies(G, weight='length'):
    dependencies = defaultdict(lambda: 0)
    for s in sorted(G.nodes):
        S, P, sigma = _single_source_dijkstra_path_basic(G, s, weight)
        while S:
            w = S.pop()
            coeff = (1.0 + dependencies[s, w]) / sigma[w]
            for v in P[w]:
                dependencies[s, v] += sigma[v] * coeff
        dependencies[s, s] = 0
    return dependencies
