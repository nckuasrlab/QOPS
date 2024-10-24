import os
import argparse


parser = argparse.ArgumentParser(description="Setting NGQB NSQB NLQB")
parser.add_argument(
    "--N",
    type=int,
    default=12,
    help="Number of total bit",
)
parser.add_argument(
    "--NGQB",
    type=int,
    default=3,
    help="Number of global Qubit",
)
parser.add_argument(
    "--NSQB",
    type=int,
    default=6,
    help="Number of thread Qubit",
)
parser.add_argument(
    "--NLQB",
    type=int,
    default=3,
    help="Number of local Qubit",
)
parser.add_argument(
    "--qc",
    type=str,
    default="test/qc.txt",
    help="Path to qc",
)


args=parser.parse_args()
N    = args.N
NGQB = args.NGQB
NSQB = args.NSQB
NLQB = args.NLQB

NUMFD = 1 << NGQB

os.system(f"python3 setup_state.py {NGQB}")
os.system("rm -r test")
os.system("mkdir test")

os.system("touch test/qc.txt")
os.system("touch test/path.txt")

paths = []
for i in range(NUMFD):
    paths.append(f"state/state{i:02}")

for path in paths:
    os.system(f"echo {path} >> test/path.txt")

test_files = ["test_all_single_gate",
              "test_all_ctrl_targ_gate"]
# test_files = ["test_all_single_gate"]

for test in test_files:
    os.system(f"python3 {test}.py test/qc.txt test/path.txt {N} {NGQB} {NSQB} {NLQB} > test/{test}_N{N}_F{NGQB}_S{NSQB}_C{NLQB}.out")