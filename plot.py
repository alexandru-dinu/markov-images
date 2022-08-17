#!/usr/bin/env python3

"""
Get verbose output during markov training and plot it.
"""

import re
import sys

import matplotlib.pyplot as plt


def stdin_gen():
    while x := sys.stdin.readline():
        if m := re.search(r"\(states: (\d+)\)", x.strip()):
            yield int(m.groups()[0])


xs = list(stdin_gen())

plt.figure()
plt.plot(xs)
plt.show()
