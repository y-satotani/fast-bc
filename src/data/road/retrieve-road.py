#!/usr/bin/env python3
import sys
from math import cos, radians, ceil
from numpy import linspace
from itertools import product
import requests
from bs4 import BeautifulSoup
from geopy.distance import geodesic
import networkx as nx

try:
    locname, radius, filename = sys.argv[1], float(sys.argv[2]), sys.argv[3]
except:
    raise RuntimeError('usage: {} locname radius filename'.format(sys.argv[0]))

latlon = None
try:
    lat_center, lon_center = tuple(map(float, ','.split(locname)))
except:
    print('searching location...', file=sys.stderr)
    r = requests.get('https://nominatim.openstreetmap.org/search/',
                     params={'q': locname, 'format': 'xml', 'limit': '1', 'bounded': '1'})
    s = BeautifulSoup(r.text, 'lxml')
    print('found location {}'.format(s.place['display_name']), file=sys.stderr)
    lat_center, lon_center = float(s.place['lat']), float(s.place['lon'])

lat_delta, lon_delta = radius / 110.574, radius / (111.320*cos(radians(lat_center)))
lat_min, lat_max = lat_center-lat_delta, lat_center+lat_delta
lon_min, lon_max = lon_center-lon_delta, lon_center+lon_delta



lat_s = linspace(lat_min, lat_max, 2*ceil(radius)+1)
lon_s = linspace(lon_min, lon_max, 2*ceil(radius)+1)
N = (len(lat_s)-1)*(len(lon_s)-1)
G = nx.Graph(name='{} 周辺 約{}km'.format(locname, radius))
for i, (lat_p, lon_p) in\
    enumerate(product(zip(lat_s, lat_s[1:]), zip(lon_s, lon_s[1:]))):
    bbox = [lon_p[0], lat_p[0], lon_p[1], lat_p[1]]
    print('retrieving {}/{} patch...'.format(i+1, N), file=sys.stderr)
    r = requests.get('https://api.openstreetmap.org/api/0.6/map',
                     params={'bbox': ','.join(map(str, bbox))})
    print('extracting {}/{} patch...'.format(i+1, N), file=sys.stderr)
    s = BeautifulSoup(r.text, 'lxml')
    G.add_nodes_from(map(
        lambda node: (node['id'], {'lat': float(node['lat']), 'lon': float(node['lon'])}),
        filter(lambda tag: tag.name == 'node', s.osm)
    ))
    for way in filter(lambda elem: elem.name == 'way' and\
                      any(map(lambda p: p['k'] == 'highway', elem.find_all('tag'))),
                      s.osm):
        waytype = next(p['v'] for p in way.find_all('tag') if p['k'] == 'highway')
        nodes = [nd['ref'] for nd in way.find_all('nd')]
        for u, v in zip(nodes, nodes[1:]):
            distance = geodesic(
                (G.nodes[u]['lat'], G.nodes[u]['lon']),
                (G.nodes[v]['lat'], G.nodes[v]['lon'])).meters
            G.add_edge(u, v, waytype=waytype, distance=distance)

G.remove_nodes_from([node for node in G.nodes if
                     G.degree(node) == 0 or\
                     G.nodes[node]['lat'] < lat_min or\
                     G.nodes[node]['lat'] > lat_max or\
                     G.nodes[node]['lon'] < lon_min or\
                     G.nodes[node]['lon'] > lon_max])
nx.write_graphml(G, filename)
print('wrote {} nodes / {} edges'.format(G.number_of_nodes(), G.number_of_edges()))
