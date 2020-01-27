#!/usr/bin/env python

import numpy as np
import networkx as nx
import matplotlib.pyplot as plt
from matplotlib import rcParams
rcParams['font.family'] = 'IPAexGothic'
import matplotlib.ticker as ticker
from itertools import combinations
from bs4 import BeautifulSoup
from geopy.distance import geodesic

railwayline_names = set([
    '1号線浅草線',
    '6号線三田線',
    '10号線新宿線',
    '12号線大江戸線',
    '2号線日比谷線',
    '3号線銀座線',
    '4号線丸ノ内線',
    '4号線丸ノ内線分岐線',
    '5号線東西線',
    '7号線南北線',
    '8号線有楽町線',
    '9号線千代田線',
    '11号線半蔵門線',
    '13号線副都心線',
])

ksj_xml = 'N02-18.xml'
with open(ksj_xml, 'r') as fp:
    soup = BeautifulSoup(fp.read(), 'lxml')

station_by_id = dict([
    (s['gml:id'], s) for s in soup.find_all('ksj:station')
    if 'gml:id' in s.attrs
])
section_by_id = dict([
    (s['gml:id'], s) for s in soup.find_all('ksj:railroadsection')
    if 'gml:id' in s.attrs
])

tokyo_stations = [
    s for s in station_by_id.values()
    if s.find('ksj:railwaylinename') is not None
    and s.find('ksj:railwaylinename').text in railwayline_names
]
tokyo_sections = list(set([
    s for s in soup.find_all('ksj:railroadsection')
    if s.find('ksj:railwaylinename') is not None
    and s.find('ksj:railwaylinename').text in railwayline_names
] + [
    section_by_id[sid]
    for sid in map(
            lambda s: s.find('ksj:railroadsection')['xlink:href'][1:],
            tokyo_stations
    )
]))

tokyo_station_names = set([
    s.find('ksj:stationname').text for s in tokyo_stations
])

curve_by_id = dict([
    (
        c['gml:id'],
        [
            tuple(map(float, line.strip().split()))
            for line in c.find('gml:poslist').text.splitlines()
            if '.' in line
        ]
    )
    for c in soup.find_all('gml:curve')
])

# generate graph for draw rail
G_draw = nx.Graph()
for section in tokyo_sections:
    line_name = section.find('ksj:railwaylinename').text
    if section.find('ksj:station') is not None:
        station_name = station_by_id[
            section.find('ksj:station')['xlink:href'][1:]
        ].find('ksj:stationname').text
    else:
        station_name = ''
    curve_id = section.find('ksj:location')['xlink:href'][1:]
    curve = curve_by_id[section.find('ksj:location')['xlink:href'][1:]]
    for p in curve:
        G_draw.add_node(
            f'{line_name}:{p}',
            pos=(p[1], p[0])
        )
    for p, q in zip(curve, curve[1:]):
        G_draw.add_edge(
            *[f'{line_name}:{t}' for t in (p, q)],
            line_name=line_name,
            station_name=station_name
        )

# generate graph for calculating distance
G_calc = nx.Graph()
for section in tokyo_sections:
    line_name = section.find('ksj:railwaylinename').text
    if section.find('ksj:station') is not None:
        station_name = station_by_id[
            section.find('ksj:station')['xlink:href'][1:]
        ].find('ksj:stationname').text
    else:
        station_name = ''
    curve_id = section.find('ksj:location')['xlink:href'][1:]
    curve = curve_by_id[section.find('ksj:location')['xlink:href'][1:]]
    length = sum(geodesic(p, q).meters for p, q in zip(curve, curve[1:]))
    v, w = curve[0], curve[-1]
    for x in (v, w):
        node_name = f'{line_name}:{x}'
        pos = (x[1], x[0])
        if node_name not in G_calc.nodes:
            G_calc.add_node(node_name, station_name='', pos=pos)
        if station_name:
            G_calc.nodes[node_name]['station_name'] = station_name
    G_calc.add_edge(
        *[f'{line_name}:{x}' for x in (v, w)],
        station_name=station_name,
        length=length
    )

# reduce 1 (station edges to station nodes)
for station_name in tokyo_station_names:
    nodes_to_remove = set([
        v for v, n in G_calc.nodes(data='station_name') if n == station_name
    ])
    pos_center = tuple(
        np.mean(np.array([
            G_calc.nodes[v]['pos'] for v in nodes_to_remove
        ]), axis=0)
    )
    G_calc.add_node(station_name, station_name=station_name, pos=pos_center)
    edges_to_reduce = [
        (v, w) for v in nodes_to_remove
        for w in G_calc.neighbors(v)
        if w not in nodes_to_remove
    ]
    for v, w in edges_to_reduce:
        #print('reducing', v, w, 'to', station_name, w)
        length = G_calc.edges[v, w]['length']
        if not G_calc.has_edge(station_name, w):
            G_calc.add_edge(station_name, w, length=length)
        if length < G_calc.edges[station_name, w]['length']:
            G_calc.edges[station_name, w]['length'] = length
        G_calc.remove_edge(v, w)
    edges_to_remove = [
        (v, w) for v, w, n in G_calc.edges(data='station_name')
        if n == station_name
    ]
    G_calc.remove_edges_from(edges_to_remove)
    #print('removing', edges_to_remove)
    G_calc.remove_nodes_from(sum(map(list, edges_to_remove), []))

# reduce 2 (reduce non-station node)
for v in [v for v, n in G_calc.nodes(data='station_name') if n == '']:
    vn = list(G_calc.neighbors(v))
    for w, x in combinations(vn, 2):
        length = G_calc.edges[v, w]['length'] + G_calc.edges[v, x]['length']
        if not G_calc.has_edge(w, x):
            G_calc.add_edge(w, x, length=length)
        if length < G_calc.edges[w, x]['length']:
            G_calc.edges[w, x]['length'] = length
    G_calc.remove_edges_from([(v, w) for w in vn])
    G_calc.remove_node(v)

# calculate BC value
for e in G_calc.edges:
    G_calc.edges[e]['length_round'] = int(G_calc.edges[e]['length'] / 100) * 100

bc = nx.betweenness_centrality(G_calc, weight='length_round')

# draw
pos = nx.get_node_attributes(G_draw, 'pos')
pos_min_x = min(p[0] for p in pos.values())
pos_max_x = max(p[0] for p in pos.values())
pos_min_y = min(p[1] for p in pos.values())
pos_max_y = max(p[1] for p in pos.values())
margin_x = (pos_max_x - pos_min_x) * 0.05
margin_y = (pos_max_y - pos_min_y) * 0.05
station_edges = sorted(
    [(u, v) for u, v, n in G_draw.edges(data='station_name') if n],
    key=lambda e: bc[G_draw.edges[e]['station_name']]
)
station_color = [bc[G_draw.edges[e]['station_name']] for e in station_edges]

width = 7
plt.figure(figsize=(width, width * (pos_max_y-pos_min_y) / (pos_max_x-pos_min_x)))
nx.draw_networkx_edges(G_draw, pos, width=0.3)
nx.draw_networkx_edges(
    G_draw, pos, station_edges, width=3, edge_color=station_color,
    edge_cmap=plt.cm.YlGnBu
)
plt.axis([
    pos_min_x - margin_x, pos_max_x + margin_x,
    pos_min_y - margin_y, pos_max_y + margin_y
])
plt.axis('equal')
plt.axis('off')
plt.savefig('tokyo-metro-betweenness.svg', bbox_inchs='tight')

