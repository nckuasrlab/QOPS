from circuit_generator import *
from ini_generator import *
from test_util import *

from singleGatesTest import singleGatesTest
from measureTest import measureTest
from phaseTest import phaseTest
from swapTest import swapTest
from u1Test import u1Test
from u2Test import u2Test
from u3Test import u3Test
from ccxTest import ccxTest

# Test for all gates

tests = [singleGatesTest(), 
        measureTest(),
        phaseTest(),
        swapTest(),
        u1Test(),   u2Test(),   u3Test(),
        ccxTest()]
flag = True
for test in tests:
    try:
        flag = flag and test.test()
    except:
        flag = False

if(flag):
    print("[PASS]", "ALL TEST", ": match with qiskit under 1e-9", flush=True)
    print("===========================")

os.system("rm cir_test test.ini m.out")
os.system("rm -r state")
