# input path to the file of path of states
# path.txt
# ./state/state1
# ./state/state2
# ./state/state3
# ./state/state4
# 
# then check the states with qiskit to make sure the result is the same.s
#
# 
# usage: python3 test_by_qiskit.py path_to_path_files.txt 

from test_util import *

# NSQB >= NGQB 確保一個file至少有一個thread處理
# NSQB + NLQB <= N 確保不會有多個thread處理到同一個chunk
N    = 20 if len(sys.argv) < 4 else int(sys.argv[3])
NGQB =  3 if len(sys.argv) < 5 else int(sys.argv[4])
NSQB =  6 if len(sys.argv) < 6 else int(sys.argv[5])
NLQB =  12 if len(sys.argv) < 7 else int(sys.argv[6])

print_header(N, NGQB, NSQB, NLQB, False)
simple_test("Match test", False, sys.argv[1], sys.argv[2], N, NGQB)
print("===========================")
    