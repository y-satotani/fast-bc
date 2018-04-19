#!/usr/bin/env python3
from itertools import chain, product

print("\n".join([
    "{} {} {} {}".format(gtype, n, k, qtype)
    for _ in range(100)
    for gtype in ('RRG', 'ER', 'BA')
    for n in chain(range(10, 100, 10), range(100, 1000+1, 100))
    for k in chain(range(4, 10, 2), range(10, 100+1, 10))
    for qtype in ('insert', 'delete')
    if n > k
]))
