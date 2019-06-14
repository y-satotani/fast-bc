
from math import inf, isinf
from operator import xor

from bc_helper import update_key

def insert_edge(G, v, w, c, dist, sigma, delta, weight=None):
    if weight is None:
        G.add_edge(v, w)
    else:
        G.add_edge(v, w, **{weight: c})

    for z in G.nodes:
        if dist[v, z] > dist[w, z]:
            insert_update_part(G, v, w, c, z, dist, sigma, delta, weight)
        else:
            insert_update_part(G, w, v, c, z, dist, sigma, delta, weight)

def insert_update_part(G, v, w, c, z, dist, sigma, delta, weight='length'):
    if dist[v, z] < c + dist[w, z] or isinf(dist[w, z]):
        return
    d_z = dict((x, dist[x, z]) for x in G.nodes)
    l = lambda x, y: G.edges[x, y][weight]
    queue = [(c + dist[w, z], v)]
    delta_set = set()
    while queue:
        queue.sort()
        d_xz_d, x = queue.pop(0)
        d_xz, dist[x, z] = dist[x, z], d_xz_d
        s_xz, sigma[x, z] = sigma[x, z], 0
        for u in G.neighbors(x):
            #l_xu, l_ux = G.edges[x, u][weight], G.edges[u, x][weight]
            if dist[u, z] >= l(u, x) + dist[x, z]:
                update_key(queue, u, l(u, x) + dist[x, z])
            if dist[x, z] == l(x, u) + dist[u, z]:
                sigma[x, z] += sigma[u, z]
            if xor(
                    d_z[x] == l(x, u) + d_z[u] and not (u == w and x == v),
                    dist[x, z] == l(x, u) + dist[u, z]
            ):
                # edge x -> u --> z changed
                delta_set.add(u)
            pass # end for
        if sigma[x, z] != s_xz:
            # sigma x --> z changed
            delta_set.add(x)
        pass # end while

    delta_queue = []
    for u in delta_set:
        update_key(delta_queue, u, dist[u, z])
    while len(delta_queue) > 0:
        delta_queue.sort()
        _, x = delta_queue.pop()
        delta[z, x] = 0
        if x == z:
            continue
        for y in G.neighbors(x):
            if dist[y, z] == G.edges[y, x][weight] + dist[x, z]:
                delta[z, x] += sigma[x, z] / sigma[y, z] * (1 + delta[z, y])
            elif dist[x, z] == G.edges[x, y][weight] + dist[y, z]:
                update_key(delta_queue, y, dist[y, z])
    pass # end while

