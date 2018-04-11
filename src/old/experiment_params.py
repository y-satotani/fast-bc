#!/usr/bin/env python
from __future__ import print_function
from random import randint

N = (100, 300, 500)
Net = ('random', 'scale-free')
P = {
    'random': (0.1*i for i in range(1, 10)),
    'scale-free': (2*i+2 for i in range(10))
}

print('\n'.join([
    '{} {} {} {}'.format(net, n, p, randint(0, 2**32-1))
    for net in Net
    for p in P[net]
    for n in N
    for _ in range(10)
]))

