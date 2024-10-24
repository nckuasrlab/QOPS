# usage:
# python3 setup_state.py NGQB

import os, sys

NGQB = int(sys.argv[1])
NUMFD = 1 << NGQB

os.system("ls -lh state")
os.system("rm -r state")
os.system("mkdir state")

paths = []
for i in range(NUMFD):
    paths.append(f"state/state{i:02}")

for path in paths:
    os.system("touch " + path)