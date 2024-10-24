# ops.txt
the opcode list and circuit format to each gate.

# ini file
There's corresponding example ini file in ini folder.
circuit/1.txt -> ini/1.ini

# 1.txt 1.ini
N = 20
H gate on each qubit

# 2.txt (for old version simulator)
N = 20
H gate on each qubit

# 3.txt 1.ini
Goal: CX gate test for ctrl-global targ-global situation.
N = 20
H gate on each qubit except qubit#0
then Y gate on qubit#5 (for put some value into imaginary part)
CX(ctrl = qubit#0, targ = qubit#1) at last

# 4.txt 1.ini
Goal: CX gate test for ctrl-local targ-local situation.
N = 20
H gate on each qubit except qubit#19
then Y gate on qubit#5 (for put some value into imaginary part)
CX(ctrl = qubit#18, targ = qubit#19) at last

# 5.txt 2.ini
Goal: demo for U2(unitary 2-qubit gate) on global qubits.
N NGQB NSQB NLQB = 8 3 4 2
H gate on each qubit except qubit#2
then CX(ctrl = qubit#0, targ = qubit#2)
U2(qubit#0, qubit#1) with a unitary
[[ 0.65396611,  0.04869761, -0.02532728, -0.75452992],
 [ 0.38302748, -0.80093799, -0.35552298,  0.29221858],
 [-0.55736732, -0.57963226,  0.27007049, -0.52955646],
 [ 0.33905744, -0.14196239,  0.89444053,  0.25468188]]

 # 6.txt 6_1.ini/6_2.ini
 \[TODO\]
 Goal: With 6_1 ini and 6_2 ini, the exe time is drastically decrease.

 # 7.txt 1.ini
 Goal: demo for U3(unitary 3-qubit gate) on Local-Local-Local case.
 N NGQB NSQB NLQB = 20 3 6 2
H gate on each qubit
U3(qubit#17, qubit#18, qubit#19) with a unitary
[[ 0.031703890  0.546264264  0.606622689  0.157559501 -0.489839459 -0.130269192  0.225524119  0.000497503]
 [ 0.336453272  0.194472042 -0.010234904  0.379841454  0.078144359  0.626291867 -0.225809712  0.505230114]
 [-0.042591163 -0.638293120  0.047380086  0.022477337 -0.711385751  0.047626132 -0.109284987  0.260262636]
 [ 0.648273427 -0.156774036  0.245046606 -0.544822507  0.155249384 -0.115105068  0.280183730  0.287105540]
 [ 0.534060505 -0.265819672 -0.026228826  0.524108087 -0.018330603  0.015750946  0.243690314 -0.555673437]
 [ 0.306821215  0.068454057  0.175635896 -0.071019654 -0.029700506 -0.287238077 -0.861745829 -0.198215333]
 [-0.015578235  0.056470711  0.091709997 -0.480167498 -0.186309267  0.688456735 -0.051712912 -0.496224530]
 [-0.290032849 -0.391464911  0.727780740  0.152527946  0.433416004  0.136178297 -0.055114850 -0.016582608]]
