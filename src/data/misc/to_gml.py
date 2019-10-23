#!/usr/bin/env python3
import networkx as nx
import os

def to_gml(filename):
  out_file = os.path.splitext(filename)[0]+'.gml'
  G = nx.read_edgelist(filename)
  nx.set_edge_attributes(G, 1, name='weight')
  nx.write_graphml(G, out_file)

filenames = list(filter(lambda f: os.path.splitext(f)[1] == '.txt', os.listdir()))
for filename in filenames:
  to_gml(filename)

