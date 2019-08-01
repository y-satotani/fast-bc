#!/usr/bin/env python3
from itertools import chain, product

# header spec.:
# topology,order,degree,dummy1,dummy2,query,seed,
# max-bc,max-error,time-proposed,time-brandes,
# updated-path-pairs,updated-deps-pairs,updated-bc-verts,changed-bc-verts
#
print("\n".join([
    "--seed {} --topology {} --order {} --degree {} --query {}"\
    .format(seed, gtype, n, k, qtype)
    for seed in range(100)
    for gtype in ('RRG', 'ER', 'BA')
    for n in range(100, 1000+1, 100)
    for k in (4, 16, 64)
    for qtype in ('insert', 'delete')
    if n > k
]))
