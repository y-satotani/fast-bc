#!/usr/bin/env python3

from itertools import chain
import networkx as nx
import sys

def main():
    if len(sys.argv) != 3:
        print(f'usage: {sys.argv[0]} in out', file=sys.stderr)
        exit(1)
    in_file = sys.argv[1]
    out_file = sys.argv[2]
    ifstream = sys.stdin if in_file == '-' else open(in_file, 'r')
    ofstream = sys.stdout if out_file == '-' else open(out_file, 'w')
    edges = list(map(
        lambda l: tuple(map(int, l.split())),
        ifstream.read().splitlines()
    ))
    edges.sort(key=lambda e: e[2])
    vertices = set(chain((e[0] for e in edges), (e[1] for e in edges)))
    vert_mapper = dict(map(lambda p: (p[1], p[0]), enumerate(vertices)))

    edge_lifetime = 3600 * 24
    current_edges = []
    current_eid = 0
    current_time = 0
    while current_eid < len(edges):
        t = edges[current_eid][2]
        while len(current_edges) > 0\
              and current_edges[0][2] + edge_lifetime < t:
            current_edges.pop(0)
        while current_eid < len(edges)\
              and t == edges[current_eid][2]:
            current_edges.append(edges[current_eid])
            current_eid += 1
        print(
            '\n'.join(f'{u} {v} {current_time}' for (u, v, t) in current_edges),
            file=ofstream
        )
        current_time += 1

if __name__ == '__main__':
    main()
