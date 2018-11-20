from math import inf, isinf
import bisect

def insert_update(G, e, z, dist, sigma, deps, weight='length'):
    v, w = e
    dist_old = dict((x, dist[x, z]) for x in G.nodes)
    length = lambda a, b: 1 if weight is None else G.edges[a, b][weight]
    SP = lambda a, b, c:\
         dist[a, c] == length(a, b) + dist[b, c] and\
         dist[a, c] != inf
    SP_old = lambda a, b:\
             dist_old[a] == length(a, b) + dist_old[b] and\
             dist_old[a] != inf

    work_set = {(v, w)}
    visited_vertices = {v}
    affected_vertices = set()
    affected_dependencies = set()

    # Phase 1: Select vertex affected under insertion while
    #          updating distance
    while len(work_set) > 0:
        x, u = work_set.pop()
        if length(x, u) + dist[u, z] < dist[x, z]:
            dist[x, z] = length(x, u) + dist[u, z]
            affected_vertices.add(x)
            # Expand selection
            for y in filter(
                    lambda y: y not in visited_vertices, G.neighbors(x)
            ):
                if SP(y, x, v):
                    work_set.add((y, x))
                    visited_vertices.add(y)
            # x -> y --> z is not shortest path anymore
            # so, assume y changes its pairwise dependency
            for y in filter(lambda y: SP_old(x, y), G.neighbors(x)):
                affected_dependencies.add(y)

    # Phase 2: Update sigma
    work_set = [(dist[v, w], (v, w))]
    visited_vertices = {v}
    while len(work_set) > 0:
        _, (x, u) = work_set.pop(0)
        sigma_xz = sum(
            sigma[y, z] for y in G.neighbors(x) if SP(x, y, z)
        )
        is_affected = x in affected_vertices or\
                      x != z and sigma[x, z] != sigma_xz
        sigma[x, z] = sigma_xz if x != z else 1
        # Expand selection
        if is_affected:
            affected_vertices.add(x)
            neis = list(filter(lambda y: SP(y, x, v), G.neighbors(x)))
            for y in filter(lambda y: y not in visited_vertices, neis):
                bisect.insort(work_set, (dist[y, z], (y, x)))
                visited_vertices.add(y)
        # If affected, also pairwise dependency is affected
        if is_affected:
            affected_dependencies.add(x)

    # Phase 3: Update pairwise dependencies
    visited_vertices = set(x for x in affected_dependencies)
    affected_dependencies = sorted([
        (dist[x, z], x) for x in affected_dependencies
    ])
    nupdate = 0 # counter for update pairwise depedencies
    while len(affected_dependencies) > 0:
        _, x = affected_dependencies.pop()
        nupdate += 1
        far = list(filter(lambda y: SP(y, x, z), G.neighbors(x)))
        near = list(filter(lambda y: SP(x, y, z), G.neighbors(x)))
        delta = sum(sigma[x, z] / sigma[y, z] * (1 + deps[z, y]) for y in far)
        deps[z, x] = delta if z != x else 0
        for y in near:
            if not y in visited_vertices:
                bisect.insort(affected_dependencies, (dist[y, z], y))
                visited_vertices.add(y)

    return affected_vertices, nupdate

def insert_edge(G, e, dist, sigma, deps, weight=None, c=1):
    v, w = e
    if weight is None:
        G.add_edge(v, w)
    else:
        G.add_edge(v, w, **{weight: c})

    sum_nupdate = 0
    affected_sinks, nupdate\
        = insert_update(G, (w, v), v, dist, sigma, deps, weight)
    sum_nupdate += nupdate
    affected_sources, nupdate\
        = insert_update(G, (v, w), w, dist, sigma, deps, weight)
    sum_nupdate += nupdate

    for x in affected_sinks:
        _, nupdate = insert_update(G, (v, w), x, dist, sigma, deps, weight)
        sum_nupdate += nupdate
    for x in affected_sources:
        _, nupdate = insert_update(G, (w, v), x, dist, sigma, deps, weight)
        sum_nupdate += nupdate
    return sum_nupdate
