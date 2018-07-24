#!/usr/bin/env python3
import sys
import networkx as nx
from matplotlib import rcParams
rcParams['font.family'] = 'IPAexGothic'
import matplotlib.pyplot as plt

try:
    filename = sys.argv[1]
except:
    raise RuntimeError('usage: {} '.format(sys.argv[0]))

G = nx.read_gml(filename)
print('read {} nodes / {} edges'.format(G.number_of_nodes(), G.number_of_edges()))

pos = dict((node, (G.nodes[node]['lon'], G.nodes[node]['lat'])) for node in G.nodes)

nx.draw_networkx_edges(G, pos)
plt.title(G.name)
plt.xlabel('経度')
plt.ylabel('緯度')
plt.xlim(min([p[0] for p in pos.values()]), max(p[0] for p in pos.values()))
plt.ylim(min([p[1] for p in pos.values()]), max(p[1] for p in pos.values()))
plt.savefig('{}.svg'.format('.'.join(filename.split('.')[:-1])))

