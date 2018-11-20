from math import inf, isinf
import bisect

def delete_update(G, e, z, dist, sigma, deps, weight=None):
    v, w = e
    dist_old = dict((x, dist[x, z]) for x in G.nodes)
    length = lambda a, b: 1 if weight is None else G.edges[a, b][weight]
    SP = lambda a, b, c:\
         dist[a, c] == length(a, b) + dist[b, c] and\
         dist[a, c] != inf
    SP_old = lambda a, b, c:\
             c == z and\
             dist_old[a] == length(a, b) + dist_old[b] and\
             dist_old[a] != inf

    affected_vertices = set()

    # Phase 1: Identify affected vertices then determine new distances to z
    work_set = set() if any(SP(v, x, z) for x in G.neighbors(v)) else {v}
    while work_set:
        u = work_set.pop()
        affected_vertices.add(u)
        for x in filter(lambda x: SP(x, u, z), G.neighbors(u)):
            if all(lambda y: not SP(x, y, z) or y in affected_vertices
                   for y in G.neighbors(x)):
                work_set.add(x)

    priority_queue = []
    for a in affected_vertices:
        dist[a, z] = min((
            length(a, b) + dist[b, z]
            for b in G.neighbors(a) if not b in affected_vertices
        ), default=inf)
        if not isinf(dist[a, z]):
            bisect.insort(priority_queue, (dist[a, z], a))
    while priority_queue:
        _, a = priority_queue.pop(0)
        for c in filter(
                lambda c: length(c, a) + dist[a, z] < dist[c, z],
                G.neighbors(a)
        ):
            dist_cz_old = dist[c, z]
            dist[c, z] = length(c, a) + dist[a, z]
            if (dist_cz_old, c) in priority_queue:
                idx = priority_queue.index((dist_cz_old, c))
                del priority_queue[idx]
            bisect.insort(priority_queue, (dist[c, z], c))

    # Phase 2: Identify affected vertices then determine new sigma to z
    work_set = set() if dist_old[v] == dist_old[w] else {v}
    while work_set:
        u = work_set.pop()
        affected_vertices.add(u)
        for x in filter(lambda x: SP(x, u, z), G.neighbors(u)):
            if any(lambda y: not SP(x, y, z) or y in affected_vertices
                   for y in G.neighbors(x)):
                work_set.add(x)

    priority_queue = sorted([(dist[x, z], x) for x in affected_vertices])
    for _, a in priority_queue:
        sigma_az = sum(
            sigma[b, z] for b in G.neighbors(a) if SP(a, b, z)
        )
        sigma[a, z] = sigma_az

    # Phase 3: Update pairwise dependencies
    if dist_old[v] != dist_old[w]:
        bisect.insort(priority_queue, (dist[w, z], w))
    visited_vertices = set(x for _, x in priority_queue)
    nupdate = 0
    while len(priority_queue) > 0:
        _, x = priority_queue.pop()
        nupdate += 1
        far = list(filter(lambda y: SP(y, x, z), G.neighbors(x)))
        near = list(filter(lambda y: SP(x, y, z), G.neighbors(x)))
        delta = sum(sigma[x, z] / sigma[y, z] * (1 + deps[z, y]) for y in far)
        deps[z, x] = delta if z != x else 0
        for y in near:
            if not y in visited_vertices:
                bisect.insort(priority_queue, (dist[y, z], y))
                visited_vertices.add(y)

    return affected_vertices, nupdate

def delete_edge(G, e, dist, sigma, deps, weight=None):
    v, w = e
    G.remove_edge(v, w)

    sum_nupdate = 0
    affected_sinks, nupdate\
        = delete_update(G, (w, v), v, dist, sigma, deps, weight)
    sum_nupdate += nupdate
    affected_sources, nupdate\
        = delete_update(G, (v, w), w, dist, sigma, deps, weight)
    sum_nupdate += nupdate

    for x in affected_sinks:
        delete_update(G, (v, w), x, dist, sigma, deps, weight)
        sum_nupdate += nupdate
    for x in affected_sources:
        delete_update(G, (w, v), x, dist, sigma, deps, weight)
        sum_nupdate += nupdate
    return sum_nupdate
