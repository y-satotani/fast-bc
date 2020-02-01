#!/usr/bin/env python3
import sys
import random
import networkx as nx
from matplotlib import rcParams
rcParams['font.family'] = 'IPAexGothic'
import matplotlib.pyplot as plt
from geopy.distance import geodesic

try:
    filename = sys.argv[1]
except:
    raise RuntimeError('usage: {} '.format(sys.argv[0]))

G = nx.read_graphml(filename)
print(f'read {G.order()} nodes / {G.size()} edges')

pos = dict(
    (node, (G.nodes[node]['lon'], G.nodes[node]['lat']))
    for node in G.nodes
)

for u, v, d in G.edges(data='distance'):
    G.edges[u, v]['distance'] = int(d)


E = list(nx.complement(G).edges)
random.shuffle(E)
u, v = E[0]
distance = geodesic(
    (G.nodes[u]['lat'], G.nodes[u]['lon']),
    (G.nodes[v]['lat'], G.nodes[v]['lon'])
).meters
bc_before = nx.betweenness_centrality(G, weight='distance', normalized=False)
G.add_edge(u, v, distance=int(distance))
bc_after = nx.betweenness_centrality(G, weight='distance', normalized=False)
labels = dict((n, f'{bc_before[n]}→{bc_after[n]}') for n in G.nodes)
node_color = '#3d3b6f'
red = '#c73634'

fig, ax = plt.subplots(1, 1, figsize=(7, 7))
nx.draw_networkx_nodes(
    G, pos,
    node_color=[
        node_color if bc_before[n] == bc_after[n] else red
        for n in G.nodes
    ],
    node_size=100,
    ax=ax
)
nx.draw_networkx_edges(
    G, pos,
    edgelist=[
        (x, y) for x, y in G.edges
        if not (x == u and y == v or x == v and y == u)
    ],
    ax=ax
)
nx.draw_networkx_edges(G, pos, [(u, v)], style='dashed', ax=ax)
nx.draw_networkx_edge_labels(
    G, pos,
    edge_labels={(u, v): '挿入'},
    font_family='IPAexGothic',
    font_size=24,
    ax=ax
)

xlim = (min([p[0] for p in pos.values()]), max(p[0] for p in pos.values()))
ylim = (min([p[1] for p in pos.values()]), max(p[1] for p in pos.values()))
ax.set_xlim(
    xlim[0] - (xlim[1]-xlim[0]) * 0.05,
    xlim[1] + (xlim[1]-xlim[0]) * 0.05
)
ax.set_ylim(
    ylim[0] - (ylim[1]-ylim[0]) * 0.05,
    ylim[1] + (ylim[1]-ylim[0]) * 0.05
)
ax.axis('off')

plt.savefig('{}.svg'.format('.'.join(filename.split('.')[:-1])))
