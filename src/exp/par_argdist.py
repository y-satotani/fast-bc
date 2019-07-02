#!/usr/bin/env python3
from itertools import chain, product

print("\n".join([
    "-s {} -t {} -n {} -k {} -q {}".format(seed, gtype, n, k, qtype)
    for _ in range(10)
    for seed in range(100)
    for gtype in ('RRG', 'ER', 'BA')
    for n in (10, 50, 100, 500, 1000)
    for k in (4, 16, 64)
    for qtype in ('insert', 'delete')
    if n > k
]))
