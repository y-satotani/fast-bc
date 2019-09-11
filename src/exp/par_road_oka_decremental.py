#!/usr/bin/env python3
import os
import networkx as nx

# header spec.:
# gml,vert1,vert2,weight,query,seed,max-bc,max-error,time-proposed,time-brandes,
# updated-path-pairs,updated-deps-pairs,updated-bc-verts,changed-bc-verts
#
dname = os.path.join(os.path.dirname(__file__), '..', 'data', 'road')
gml_file = os.path.join(dname, 'road-oka-integer-distance.gml')
fundamental_args = '--vertex-name id --weight-name distance --real-weight --no-brandes'

G = nx.read_graphml(gml_file)
print('\n'.join(
    '--gml-file {} --query {} --vertex-pair {},{} --weight {} '\
    .format(gml_file, 'delete', *e, G.edges[e]['distance'])\
    + fundamental_args
    for e in G.edges
))
