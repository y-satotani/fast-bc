from itertools import combinations
import numpy as np
import networkx as nx

def update_on_delete(D, S, paths, e):
    N = D.shape[0]
    Dnew, Snew = np.copy(D), np.copy(S)
    update_pair = []
    for s, t in combinations(range(D.shape[0]), 2):
        a, b = e if D[s, e[0]] < D[s, e[1]] else (e[1], e[0])
        if D[s, a] + D[b, t] + 1 > D[s, t]:
            continue
        else: # D[s, a] + D[b, t] + 1 == D[s, t]
            Snew[s, t] = Snew[t, s] = S[s, t] - S[s, a] * S[b, t]
            if Snew[s, t] == 0:
                # set default value for no st-path
                Dnew[s, t] = Dnew[t, s] = np.infty
                update_pair.append((s, t))

    def no_ab_path_num(s, t):
        # determine st shortest path must go through ab
        a, b = e if D[s, e[0]] < D[s, e[1]] else (e[1], e[0])
        if D[s, a] == np.infty:
            return 0
        if D[s, a] + D[b, t] + 1 > D[s, t]:
            return S[s, t]
        else: # D[s, a] + D[b, t] + 1 == D[s, t]
            return S[s, t] - S[s, a] * S[b, t]

    for s, t in update_pair:
        min_dist = np.infty
        for v in range(D.shape[0]):
            if no_ab_path_num(s, v) == 0 or\
               no_ab_path_num(v, t) == 0:
                continue # have to go alpha-beta

            dist = D[s, v] + D[v, t]
            if dist < min_dist:
                min_dist = dist
        Dnew[t, s] = Dnew[s, t] = min_dist

    # sort to calculate S in right order
    update_pair.sort(key=lambda t: Dnew[t[0], t[1]])

    for s, t in update_pair:
        sigma = 0
        for v in range(D.shape[0]):
            dist = Dnew[s, v] + Dnew[v, t]
            if dist == Dnew[s, t]:
                sigma += Snew[s, v] * Snew[v, t]
        # divide by (d_st - 1). every v on shortest path
        # shows (d_st - 1) times
        Snew[s, t] = Snew[t, s] = sigma // (Dnew[s, t] - 1)

    Pnew = [path.copy() for path in paths]
    update_vertices = [[] for _ in range(len(paths))]
    for s, t in update_pair:
        update_vertices[s].append(t)
        update_vertices[t].append(s)
    for s, path in enumerate(Pnew):
        a, b = e if D[s, e[0]] < D[s, e[1]] else (e[1], e[0])
        if D[s, a] == D[s, b]:
            continue
        path.remove((a, b))
        if len(update_vertices[s]) == 0:
            continue # no distance update (no further edge process)
        # delete collapsed edge
        path.difference_update(
            [(i, j) for i, j in path if
             D[s, i] > D[s, a] and (
                 Snew[s, i] == 0 or Dnew[s, i] + 1 != Dnew[s, j]
             )]
        )
        # add new edge
        for t in update_vertices[s]:
            for v in range(N):
                if Dnew[v, t] == 1 and Dnew[s, v] + 1 == Dnew[s, t] and\
                   Snew[s, t] > 0:
                    path.add((v, t))

    return Dnew, Snew, Pnew
