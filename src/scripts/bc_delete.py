from math import inf, isinf
import bisect

def delete_update_dist(G, e, z, trio, weight=None):
    v, w = e
    dist, sigma, deps = trio
    length = lambda a, b: 1 if weight is None else G.edges[a, b][weight]
    SP = lambda a, b, c:\
         dist[a, c] == length(a, b) + dist[b, c] and\
         dist[a, c] != inf

    do_sigma = dist[v, z] != dist[w, z]
    affected_vertices = set()

    # Phase 1: Identify affected vertices then determine new distances to z
    if any(SP(v, x, z) for x in G.neighbors(v)):
        return affected_vertices, do_sigma

    work_set = {v}
    while work_set:
        u = work_set.pop()
        affected_vertices.add(u)
        for x in filter(lambda x: SP(x, u, z), G.neighbors(u)):
            if all(lambda y: y in affected_vertices
                   for y in G.neighbors(x) if SP(x, y, z)):
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

    return affected_vertices, do_sigma

def delete_update_sigma(G, e, z, trio, affected_dist, do_sigma, weight=None):
    v, w = e
    dist, sigma, deps = trio
    length = lambda a, b: 1 if weight is None else G.edges[a, b][weight]
    SP = lambda a, b, c:\
         dist[a, c] == length(a, b) + dist[b, c] and\
         dist[a, c] != inf

    # Phase 2: Identify affected vertices then determine new sigma to z
    if not do_sigma:
        return affected_dist

    affected_vertices = affected_dist
    priority_queue = sorted([(dist[a, z], a) for a in affected_dist])
    work_set = {v}
    while work_set:
        u = work_set.pop()
        if u not in affected_dist:
            affected_vertices.add(u)
            bisect.insort(priority_queue, (dist[u, z], u))
        for x in filter(lambda x: SP(x, u, z), G.neighbors(u)):
            if any(lambda y: y in affected_vertices
                   for y in G.neighbors(x) if SP(x, y, z)):
                work_set.add(x)

    for _, a in priority_queue:
        sigma[a, z] = sum(
            sigma[b, z] for b in G.neighbors(a) if SP(a, b, z)
        )

    return affected_vertices

def delete_update_delta(G, e, z, trio, affected_vertices, weight=None):
    v, w = e
    dist, sigma, deps = trio
    length = lambda a, b: 1 if weight is None else G.edges[a, b][weight]
    SP = lambda a, b, c:\
         dist[a, c] == length(a, b) + dist[b, c] and\
         dist[a, c] != inf

    # Phase 3: Update pairwise dependencies
    nupdate = 0
    if len(affected_vertices) == 0:
        return nupdate

    priority_queue = sorted([(dist[a, z], a) for a in affected_vertices])
    #if not (dist[w, z], w) in priority_queue:
    if w not in affected_vertices:
        bisect.insort(priority_queue, (dist[w, z], w))
    visited_vertices = affected_vertices.copy()

    while len(priority_queue) > 0:
        _, x = priority_queue.pop()
        if x == z:
            break
        far = filter(lambda y: SP(y, x, z), G.neighbors(x))
        near = filter(lambda y: SP(x, y, z), G.neighbors(x))
        a = deps[z, x]
        deps[z, x] = sum(
            sigma[x, z] / sigma[y, z] * (1 + deps[z, y]) for y in far)
        nupdate += 1
        for y in near:
            if not y in visited_vertices:
                bisect.insort(priority_queue, (dist[y, z], y))
                visited_vertices.add(y)

    return nupdate

def delete_update(G, e, z, dist, sigma, deps, weight=None):
    affected_vertices, do_sigma\
        = delete_update_dist(G, e, z, (dist, sigma, deps), weight)
    affected_vertices\
        = delete_update_sigma(G, e, z, (dist, sigma, deps),
                              affected_vertices, do_sigma, weight)
    nupdate\
        = delete_update_delta(G, e, z, (dist, sigma, deps),
                              affected_vertices, weight)
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
        if x == w: continue
        delete_update(G, (v, w), x, dist, sigma, deps, weight)
        sum_nupdate += nupdate
    for x in affected_sources:
        if x == v: continue
        delete_update(G, (w, v), x, dist, sigma, deps, weight)
        sum_nupdate += nupdate
    return sum_nupdate
