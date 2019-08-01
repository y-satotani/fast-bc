#!/usr/bin/env python3
import os
import networkx as nx

# header spec.:
# gml,vert1,vert2,weight,query,seed,max-bc,max-error,time-proposed,time-brandes,
# updated-path-pairs,updated-deps-pairs,updated-bc-verts,changed-bc-verts
#
gml_file = os.path.join('data', 'road', 'road-oka.gml')
additional_file = os.path.join('data', 'road', 'road-oka-addition.gml')
fundamental_args = '--vertex-name id --weight-name distance --real-weight'
G = nx.read_graphml(additional_file)
print('\n'.join(
    '--gml-file {} --query {} --vertex-pair {},{} --weight {} '\
    .format(gml_file, 'insert', *e, G.edges[e]['distance'])\
    + fundamental_args
    for e in G.edges
))
