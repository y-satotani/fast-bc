
from math import inf, isinf
from operator import xor

from bc_helper import update_key

def delete_edge(G, v, w, dist, sigma, delta, weight=None):
    c = G.edges[v, w][weight]
    G.remove_edge(v, w)

    for z in G.nodes:
        if dist[v, z] > dist[w, z]:
            delete_update_part(G, v, w, c, z, dist, sigma, delta, weight)
        else:
            delete_update_part(G, w, v, c, z, dist, sigma, delta, weight)

def delete_update_part(G, v, w, c, z, dist, sigma, delta, weight):
    # assert failed on 664
    if dist[v, z] < c + dist[w, z] or isinf(dist[w, z]):
        return
    d_z = dict((x, dist[x, z]) for x in G.nodes)
    s_z = dict((x, sigma[x, z]) for x in G.nodes)
    l = lambda x, y: G.edges[x, y][weight]\
        if (x != v or y != w) and (x != w or y != v) else inf
    work_set = {v}
    affected = {v}
    while work_set:
        x = work_set.pop()
        for y in G.neighbors(x):
            if dist[y, z] == l(y, x) + dist[x, z] and y not in affected:
                affected.add(y)
                work_set.add(y)
            pass # end for
        pass # end while

    queue = []
    for x in affected:
        d_xz_d = min(
            [l(x, y) + dist[y, z] for y in G.neighbors(x) if y not in affected]\
            +[inf]
        )
        if isinf(d_xz_d):
            dist[x, z] = inf
            sigma[x, z] = 0
        else:
            update_key(queue, x, d_xz_d)
        pass # end for
    delta_set = {w} # v -> w --> z
    while queue:
        queue.sort()
        d_xz_d, x = queue.pop(0)
        affected.remove(x)
        d_xz, dist[x, z] = dist[x, z], d_xz_d
        s_xz, sigma[x, z] = sigma[x, z], 0
        for y in G.neighbors(x):
            if y in affected:
                update_key(queue, y, l(y, x) + dist[x, z])
            elif dist[x, z] == l(x, y) + dist[y, z]:
                sigma[x, z] += sigma[y, z]
            if xor(
                    d_z[y] == l(y, x) + d_z[x],
                    dist[y, z] == l(y, x) + dist[x, z]
            ):
                # edge y -> x --> z changed
                delta_set.add(x)
            if xor(
                    d_z[x] == l(x, y) + d_z[y],
                    dist[x, z] == l(x, y) + dist[y, z]
            ):
                # edge x -> y --> z changed
                delta_set.add(y)
            pass # end for
        if sigma[x, z] != s_z[x]:
            delta_set.add(x)
            pass
        pass # end while
    while affected:
        x = affected.pop()
        delta_set.add(x)

    delta_queue = []
    for y in delta_set:
        update_key(delta_queue, y, dist[y, z])
    while len(delta_queue) > 0:
        delta_queue.sort()
        _, x = delta_queue.pop()
        delta[z, x] = 0
        if x == z:
            continue
        for y in G.neighbors(x):
            if sigma[y, z] == 0:
                continue
            if dist[y, z] == G.edges[y, x][weight] + dist[x, z]:
                delta[z, x] += sigma[x, z] / sigma[y, z] * (1 + delta[z, y])
            elif dist[x, z] == G.edges[x, y][weight] + dist[y, z]:
                update_key(delta_queue, y, dist[y, z])


