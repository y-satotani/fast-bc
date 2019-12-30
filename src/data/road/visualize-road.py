#!/usr/bin/env python3
import sys
import networkx as nx
from matplotlib import rcParams
rcParams['font.family'] = 'IPAexGothic'
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

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

ebc = nx.edge_betweenness_centrality(G, weight='distance')

fig, (ax_road, ax_bar) = plt.subplots(2, 1, figsize=(7, 7.5), gridspec_kw={'height_ratios': [7, 0.5]})
ecol = nx.draw_networkx_edges(
    G, pos,
    edge_color=[ebc[e] for e in G.edges],
    edge_cmap=plt.cm.YlGnBu,
    ax=ax_road
)
col = plt.colorbar(ecol, cax=ax_bar, orientation='horizontal')
col.set_label('Normaized betweenness value')

ax_road.set_title(G.name)
ax_road.set_xlabel('経度')
ax_road.set_ylabel('緯度')
ax_road.set_xlim(
    min([p[0] for p in pos.values()]), max(p[0] for p in pos.values())
)
ax_road.set_ylim(
    min([p[1] for p in pos.values()]), max(p[1] for p in pos.values())
)
ax_road.yaxis.set_major_formatter(ticker.FormatStrFormatter('%0.3f'))
ax_road.xaxis.set_major_formatter(ticker.FormatStrFormatter('%0.3f'))
ax_road.axis('equal')

plt.savefig('{}.svg'.format('.'.join(filename.split('.')[:-1])))

