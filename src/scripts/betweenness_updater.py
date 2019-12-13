
from math import isinf, inf
from operator import xor
from itertools import chain
from collections import defaultdict
import networkx as nx
from networkx.algorithms.centrality.betweenness import\
    _single_source_dijkstra_path_basic as path_basic

def update_key(queue, u, v):
    # update element of priority queue which has u
    us = [t for t in queue if t[1] == u]
    assert(len(us) < 2)
    if not us:
        queue.append((v, u))
    elif us[0][0] > v:
        queue.remove(us[0])
        queue.append((v, u))

class BetweennessUpdater:
    def __init__(self, G = None, weight = None):
        self.G = G
        self.weight = weight
        self.dist = defaultdict(lambda: inf)
        self.sigma = defaultdict(lambda: 0)
        self.delta = defaultdict(lambda: 0)
        self.betweenness = defaultdict(lambda: 0)
        if G is not None:
            self.calculate_cache()
        self.updating_content = None

    def calculate_cache(self):
        for s in self.G.nodes:
            dist = nx.shortest_path_length(self.G, s, weight=self.weight)
            for t, d in dist.items():
                self.dist[s, t] = d
            stack = []
            S, P, sigma = path_basic(self.G, s, self.weight)
            for t, sigma_ in sigma.items():
                self.sigma[s, t] = sigma_ // 2
            while S:
                w = S.pop()
                stack.append(w)
                coeff = (1.0 + self.delta[s, w]) / sigma[w]
                for v in P[w]:
                    self.delta[s, v] += sigma[v] * coeff
            self.delta[s, s] = 0
        for v in self.G.nodes:
            self.betweenness[v] = sum(
                self.delta[s, v] for s in self.G.nodes
            )

    def get_weight(self, u, v):
        return inf if not self.G.has_edge(u, v) else\
            1 if self.weight is None else\
            self.G.edges[u, v][self.weight]

    def succs(self, v):
        return self.G.successors(v) if self.G.is_directed() else\
            self.G.neighbors(v)

    def preds(self, v):
        return self.G.predecessors(v) if self.G.is_directed() else\
            self.G.neighbors(v)

    def insert_edge(self, u, v, c = None):
        assert not self.G.has_edge(u, v)
        assert not self.weight is None or c is None
        if self.weight is None:
            self.G.add_edge(u, v)
        else:
            self.G.add_edge(u, v, **{self.weight: (1 if c is None else c)})
        self.updating_content = ((u, v), inf, 1 if c is None else c)
        _incremental_update(self)

    def delete_edge(self, u, v):
        assert self.G.has_edge(u, v)
        c = self.get_weight(u, v)
        self.G.remove_edge(u, v)
        self.updating_content = ((u, v), c, inf)
        _decremental_update(self)

    def update_weight(self, u, v, c):
        assert self.weight
        if self.get_weight(u, v) > c:
            self.decrease_weight(u, v, c)
        elif self.get_weight(u, v) < c:
            self.increase_weight(u, v, c)

    def decrease_weight(self, u, v, c):
        assert self.weight
        assert not self.G.has_edge(u, v) or self.get_weight(u, v) > c
        prev_c = self.get_weight(u, v)
        if not self.G.has_edge(u, v):
            self.G.add_edge(u, v, **{self.weight: (1 if c is None else c)})
        else:
            self.G.edges[u, v][weight] = 1 if c is None else c
        self.updating_content = ((u, v), prev_c, c)
        _incremental_update(self)

    def increase_weight(self, u, v, c):
        assert self.weight and\
            self.G.has_edge(u, v) and self.get_weight(u, v) < c
        prev_c = self.get_weight(u, v)
        self.G.edges[u, v][weight] = c
        self.update_content = ((u, v), prev_c, c)
        _decremental_update(self)

    @staticmethod
    def load():
        raise NotImplementedError

    def dump(self):
        raise NotImplementedError

# edge insertion and weight decrease
def _incremental_update(updater: BetweennessUpdater):
    G = updater.G
    weight = updater.weight
    dist = updater.dist
    sigma = updater.sigma
    delta = updater.delta
    betweenness = updater.betweenness
    (u, v), prev_c, c = updater.updating_content

    affected_sources = _find_affected_vertices_on_incremental(
        updater, u, v, v, rev=False
    )
    affected_targets = _find_affected_vertices_on_incremental(
        updater, v, u, u, rev=(True if G.is_directed() else False)
    )
    affected_targets_many = dict(
        (s, _find_affected_vertices_on_incremental(
            updater, v, u, s, rev=(True if G.is_directed() else False)
        ))
        for s in affected_sources)
    affected_sources_many = dict(
        (t, _find_affected_vertices_on_incremental(
            updater, u, v, t, rev=False
        ))
        for t in affected_targets)
    delta_sources = affected_sources if G.is_directed() else\
        affected_sources | affected_targets
    delta_targets = affected_targets_many if G.is_directed() else\
        dict(
            (k, v) for k, v in chain(
                affected_targets_many.items(),
                affected_sources_many.items()
            )
        )

    if weight is None:
        G.remove_edge(u, v)
    else:
        G.edges[u, v][weight] = prev_c
    _incremental_betweenness_update(
        updater, delta_sources, delta_targets, factor=-1
    )
    if weight is None:
        G.add_edge(u, v)
    else:
        G.edges[u, v][weight] = c

    for x in affected_sources:
        _incremental_apsp_update_part(
            updater, v, u, x, rev=(True if G.is_directed() else False)
        )
    for x in affected_targets:
        _incremental_apsp_update_part(
            updater, u, v, x, rev=False
        )

    _incremental_betweenness_update(
        updater, delta_sources, delta_targets, factor=1
    )

def _incremental_apsp_update_part(updater, u, v, t, rev=False):
    G = updater.G
    dist = updater.dist
    sigma = updater.sigma
    _, prev_c, c = updater.updating_content
    l = lambda x, y: updater.get_weight(x, y) if not rev\
        else updater.get_weight(y, x)
    i = lambda x, y: (x, y) if not rev else (y, x)
    if dist[i(u, t)] < c + dist[i(v, t)] or isinf(dist[i(v, t)]):
        return
    queue = [(c + dist[i(v, t)], u)]
    while queue:
        queue.sort()
        d_xt_d, x = queue.pop(0)
        dist[i(x, t)] = d_xt_d
        sigma[i(x, t)] = 0
        for y in (updater.succs(x) if not rev else updater.preds(x)):
            if dist[i(x, t)] == l(x, y) + dist[i(y, t)]:
                sigma[i(x, t)] += sigma[i(y, t)]
        for y in (updater.preds(x) if not rev else updater.succs(x)):
            if dist[i(y, t)] >= l(y, x) + dist[i(x, t)]:
                update_key(queue, y, l(y, x) + dist[i(x, t)])
        pass # end while

def _incremental_betweenness_update(updater, sources, targets, factor):
    G = updater.G
    weight = updater.weight
    dist = updater.dist
    sigma = updater.sigma
    delta = updater.delta
    betweenness = updater.betweenness
    (u, v), prev_c, c = updater.updating_content

    for s in sources:
        Delta = dict.fromkeys(G.nodes, 0)
        queue = [(-dist[s, x], x) for x in targets[s]]
        while len(queue) > 0:
            queue.sort()
            _, x = queue.pop(0)
            betweenness[x] += factor * Delta[x]
            delta[s, x] += factor * Delta[x]
            if isinf(dist[s, x]):
                continue
            for y in updater.preds(x):
                if y == s or dist[s, x] < dist[s, y] + updater.get_weight(y, x):
                    continue
                if x in targets[s]:
                    Delta[y] += sigma[s, y] / sigma[s, x] * (1 + Delta[x])
                else:
                    Delta[y] += sigma[s, y] / sigma[s, x] * Delta[x]
                if y not in {e[1] for e in queue}:
                    queue.append((-dist[s, y], y))
            pass # end for
        pass #end while

def _find_affected_vertices_on_incremental(updater, u, v, t, rev=False):
    G = updater.G
    dist = updater.dist
    c = updater.get_weight(u, v) if not rev else updater.get_weight(v, u)
    l = lambda x, y: updater.get_weight(x, y) if not rev\
        else updater.get_weight(y, x)
    i = lambda x, y: (x, y) if not rev else (y, x)
    affected = set()
    if dist[i(u, t)] < c + dist[i(v, t)] or isinf(dist[i(v, t)]):
        return affected
    visited = {u}
    stack = [u]
    while stack:
        x = stack.pop()
        affected.add(x)
        for y in (updater.preds(x) if not rev else updater.succs(x)):
            if dist[i(y, t)] >= l(y, x) + dist[i(x, u)] + c + dist[i(v, t)]\
               and not y in visited:
                stack.append(y)
                visited.add(y)
        pass # end while
    return affected

# edge deletion and weight increase
def _decremental_update(updater: BetweennessUpdater):
    G = updater.G
    weight = updater.weight
    dist = updater.dist
    sigma = updater.sigma
    delta = updater.delta
    betweenness = updater.betweenness
    (u, v), prev_c, c = updater.updating_content

    #print('call root')

    G = updater.G
    weight = updater.weight
    dist = updater.dist
    sigma = updater.sigma
    delta = updater.delta
    betweenness = updater.betweenness
    (u, v), prev_c, c = updater.updating_content

    affected_sources = _find_affected_vertices_on_decremental(
        updater, u, v, v, rev=False
    )
    affected_targets = _find_affected_vertices_on_decremental(
        updater, v, u, u, rev=(True if G.is_directed() else False)
    )
    affected_targets_many = dict(
        (s, _find_affected_vertices_on_decremental(
            updater, v, u, s, rev=(True if G.is_directed() else False)
        ))
        for s in affected_sources)
    affected_sources_many = dict(
        (t, _find_affected_vertices_on_decremental(
            updater, u, v, t, rev=False
        ))
        for t in affected_targets)
    delta_sources = affected_sources if G.is_directed() else\
        affected_sources | affected_targets
    delta_targets = affected_targets_many if G.is_directed() else\
        dict(
            (k, v) for k, v in chain(
                affected_targets_many.items(),
                affected_sources_many.items()
            )
        )

    if weight is None:
        G.add_edge(u, v)
    elif not G.has_edge(u, v):
        G.add_edge(u, v, **{weight: prev_c})
    else:
        G.edges[u, v][weight] = prev_c

    _incremental_betweenness_update(
        updater, delta_sources, delta_targets, factor=-1
    )
    if weight is None or isinf(c):
        G.remove_edge(u, v)
    else:
        G.edges[u, v][weight] = c

    for x in affected_sources:
        _decremental_apsp_update_part(
            updater, v, u, x, rev=(True if G.is_directed() else False)
        )
    for x in affected_targets:
        _decremental_apsp_update_part(
            updater, u, v, x, rev=False
        )
    _incremental_betweenness_update(
        updater, delta_sources, delta_targets, factor=1
    )

def _decremental_apsp_update_part(updater, u, v, t, rev=False):
    G = updater.G
    dist = updater.dist
    sigma = updater.sigma
    _, c_prev, c = updater.updating_content
    l = lambda x, y: updater.get_weight(x, y) if not rev\
        else updater.get_weight(y, x)
    i = lambda x, y: (x, y) if not rev else (y, x)
    #print('call apsp', u, v, t, rev)
    #print('check 1:', dist[i(u, t)], '<', c_prev, '+', dist[i(v, t)], 'or', isinf(dist[i(v, t)]))
    if dist[i(u, t)] < c_prev + dist[i(v, t)] or isinf(dist[i(v, t)]):
        return
    affected = set()
    visited = {u}
    stack = [u]
    while stack:
        x = stack.pop()
        affected.add(x)
        for y in (updater.preds(x) if not rev else updater.succs(x)):
            if dist[i(y, t)] == l(y, x) + dist[i(x, u)] + c_prev + dist[i(v, t)]\
               and not y in visited:
                stack.append(y)
                visited.add(y)
        pass # end while

    queue = []
    for x in affected:
        d_xt_d = min(
            (
                l(x, y) + dist[i(y, t)] for y in
                (updater.succs(x) if not rev else updater.preds(x))
                if y not in affected
            ),
            default=inf
        )
        if isinf(d_xt_d):
            dist[i(x, t)] = inf
            sigma[i(x, t)] = 0
        else:
            update_key(queue, x, d_xt_d)
        pass # end for

    while queue:
        queue.sort()
        d_xt_d, x = queue.pop(0)
        affected.remove(x)
        dist[i(x, t)] = d_xt_d
        sigma[i(x, t)] = 0
        #print('select:', x)
        for y in updater.succs(x) if not rev else updater.preds(x):
            #print('check 2:', (x, y), dist[i(x, t)], '==', l(x, y), '+', dist[i(y, t)])
            if dist[i(x, t)] == l(x, y) + dist[i(y, t)] and y not in affected:
                #print('subs. 2:', 'sigma', i(x, t), '+=', sigma[i(y, t)])
                sigma[i(x, t)] += sigma[i(y, t)]
        for y in updater.preds(x) if not rev else updater.succs(x):
            if y in affected:
                update_key(queue, y, l(y, x) + dist[i(x, t)])
            pass # end for
        pass # end while

def _find_affected_vertices_on_decremental(updater, u, v, t, rev=False):
    G = updater.G
    dist = updater.dist
    (u_, v_), c_prev, c = updater.updating_content
    if G.is_directed():
        c = c if xor(u_ == u and v_ == v, rev) else inf
        c_prev = c_prev if xor(u_ == u and v_ == v, rev) else inf
    #c = updater.get_weight(u, v) if not rev else updater.get_weight(v, u)
    l = lambda x, y: updater.get_weight(x, y) if not rev\
        else updater.get_weight(y, x)
    i = lambda x, y: (x, y) if not rev else (y, x)
    affected = set()
    if dist[i(u, t)] < c_prev + dist[i(v, t)] or isinf(dist[i(v, t)]):
        return affected
    visited = {u}
    stack = [u]
    while stack:
        x = stack.pop()
        affected.add(x)
        for y in (updater.preds(x) if not rev else updater.succs(x)):
            if dist[i(y, t)] == l(y, x) + dist[i(x, u)] + c_prev + dist[i(v, t)]\
               and not y in visited:
                stack.append(y)
                visited.add(y)
        pass # end while
    return affected

