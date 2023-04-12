from circuit_generator import *
from ini_generator import *
from test_util import *

from HGateTest import HGateTest
# Test for all gates

tests = [HGateTest()]
flag = True
for test in tests:
    try:
        flag = flag and test.test()
    except:
        flag = False

if(flag):
    print("[PASS]", "ALL TEST", ": match with qiskit under 1e-9", flush=True)
    print("===========================")

os.system("rm cir_test test.ini")
os.system("rm -r state")
