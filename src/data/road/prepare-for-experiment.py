#!/usr/bin/env python3
import os
import sys
import subprocess
import networkx as nx
from itertools import chain

gml_file = 'road-oka-integer-distance.graphml'
gml_add_file = 'road-oka-additional-integer-distance.graphml'
out_base = os.path.splitext(gml_file)[0]

G = nx.read_graphml(gml_file)
G_add = nx.read_graphml(gml_add_file)

# this script generates...

# 1. .edgelist
v_edgelist = sorted(G.nodes)
edgelist_to_graphml = dict(enumerate(v_edgelist))
graphml_to_edgelist = dict((a, b) for b, a in enumerate(v_edgelist))
g_to_e = lambda v: graphml_to_edgelist[v]
with open(f'{out_base}.edgelist', 'w') as fp:
    contents = '\n'.join(
        f'{g_to_e(u)} {g_to_e(v)} {G.edges[(u,v)]["distance"]}'
        for u, v in G.edges
    )
    print(contents, file=fp)

# 2. .dybccache
args = [
    os.path.join('..', '..', 'release', 'exp-calculate-cache'),
    f'{out_base}.edgelist',
    f'{out_base}.dybccache',
]
subprocess.run(args)

# 3. .txt which maps graphml vertices <-> edgelist vertices
with open(f'{out_base}.map.list', 'w') as fp:
    contents = '\n'.join(
        f'{a} {b}' for a, b in edgelist_to_graphml.items()
    )
    print(contents, file=fp)

# 4. arguments to pass exp-betweenness-stats
with open(f'{out_base}.arg.list', 'w') as fp:
    contents = '\n'.join(chain(
        (f'-c {out_base}.dybccache -u {g_to_e(u)} -v {g_to_e(v)} {out_base}.edgelist delete {out_base}-{g_to_e(u)}-{g_to_e(v)}.out.csv'
         for u, v in G.edges),
        (f'-c {out_base}.dybccache -u {g_to_e(u)} -v {g_to_e(v)} -w {G_add.edges[(u,v)]["distance"]} {out_base}.edgelist insert {out_base}-{g_to_e(u)}-{g_to_e(v)}.out.csv'
         for u, v in G_add.edges),
    ))
    print(contents, file=fp)

