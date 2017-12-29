import sys
from collections import defaultdict

callgraph = {}
not_roots = []

with open(sys.argv[1]) as f:
    for l in f.readlines():
        if l.startswith('--'):
            caller = l[len('-------'):-1]
            callgraph[caller] = []
        elif ' call' in l or ' jp' in l or ' rst' in l:
            callee = l.split()[2]
            callgraph[caller].append(callee)
            not_roots.append(callee)

def printg(cg, so_far):
    cur = so_far[-1]
    if cur in so_far[:-1]:
        return

    if cur not in cg:
        return

    for f in cg[cur]:
        print(len(so_far) * "    " + f)
        so_far.append(f)
        printg(cg, so_far)
        so_far.pop()

for k in callgraph.keys():
    if k in not_roots:
        continue
    print(k)
    printg(callgraph, [k])
    print()
