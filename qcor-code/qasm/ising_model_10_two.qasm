OPENQASM 2.0;
include "qelib1.inc";
qreg qrg_nWlrB[10];
creg qrg_nWlrB_c[10];
h qrg_nWlrB[0];
h qrg_nWlrB[1];
h qrg_nWlrB[2];
h qrg_nWlrB[3];
h qrg_nWlrB[4];
h qrg_nWlrB[5];
h qrg_nWlrB[6];
h qrg_nWlrB[7];
h qrg_nWlrB[8];
h qrg_nWlrB[9];
rz(-0.3000000000000000) qrg_nWlrB[0];
rz(0.3000000000000000) qrg_nWlrB[1];
rz(-1.2000000000000000) qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
rz(0.6000000000000000) qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
rz(-0.3600000000000000) qrg_nWlrB[2];
rz(0.3600000000000000) qrg_nWlrB[3];
rz(-1.4399999999999999) qrg_nWlrB[3];
CX qrg_nWlrB[2], qrg_nWlrB[3];
rz(0.7200000000000000) qrg_nWlrB[3];
CX qrg_nWlrB[2], qrg_nWlrB[3];
rz(-0.1200000000000000) qrg_nWlrB[4];
rz(0.1200000000000000) qrg_nWlrB[5];
rz(-0.4800000000000000) qrg_nWlrB[5];
CX qrg_nWlrB[4], qrg_nWlrB[5];
rz(0.2400000000000000) qrg_nWlrB[5];
CX qrg_nWlrB[4], qrg_nWlrB[5];
rz(0.2200000000000000) qrg_nWlrB[6];
rz(-0.2200000000000000) qrg_nWlrB[7];
rz(0.8800000000000000) qrg_nWlrB[7];
CX qrg_nWlrB[6], qrg_nWlrB[7];
rz(-0.4400000000000000) qrg_nWlrB[7];
CX qrg_nWlrB[6], qrg_nWlrB[7];
rz(0.0800000000000000) qrg_nWlrB[8];
rz(-0.0800000000000000) qrg_nWlrB[9];
rz(0.3200000000000000) qrg_nWlrB[9];
CX qrg_nWlrB[8], qrg_nWlrB[9];
rz(-0.1600000000000000) qrg_nWlrB[9];
CX qrg_nWlrB[8], qrg_nWlrB[9];
rz(0.2600000000000000) qrg_nWlrB[1];
rz(-0.2600000000000000) qrg_nWlrB[2];
rz(1.0400000000000000) qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[2];
rz(-0.5200000000000000) qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[2];
rz(-0.2600000000000000) qrg_nWlrB[3];
rz(0.2600000000000000) qrg_nWlrB[4];
rz(-1.0400000000000000) qrg_nWlrB[4];
CX qrg_nWlrB[3], qrg_nWlrB[4];
rz(0.5200000000000000) qrg_nWlrB[4];
CX qrg_nWlrB[3], qrg_nWlrB[4];
rz(0.3800000000000000) qrg_nWlrB[5];
rz(-0.3800000000000000) qrg_nWlrB[6];
rz(1.5200000000000000) qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
rz(-0.7600000000000000) qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
rz(-0.2600000000000000) qrg_nWlrB[7];
rz(0.2600000000000000) qrg_nWlrB[8];
rz(-1.0400000000000000) qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
rz(0.5200000000000000) qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[0];
rz(-1.9199999999999999) qrg_nWlrB[0];
h qrg_nWlrB[0];
h qrg_nWlrB[1];
rz(-1.9199999999999999) qrg_nWlrB[1];
h qrg_nWlrB[1];
h qrg_nWlrB[2];
rz(-1.9199999999999999) qrg_nWlrB[2];
rz(-0.3022302901525382) qrg_nWlrB[3];
rx(2.7772107284643215) qrg_nWlrB[3];
rz(-1.8542629660253118) qrg_nWlrB[3];
rx(1.5565034488683758) qrg_nWlrB[2];
h qrg_nWlrB[3];
cz qrg_nWlrB[2], qrg_nWlrB[3];
rx(-0.9815926535897930) qrg_nWlrB[2];
cz qrg_nWlrB[3], qrg_nWlrB[2];
h qrg_nWlrB[3];
ry(-1.5707963267948966) qrg_nWlrB[3];
rz(0.3101149078944979) qrg_nWlrB[3];
ry(-1.5707963267948970) qrg_nWlrB[2];
rz(0.8554965511316239) qrg_nWlrB[2];
h qrg_nWlrB[4];
rz(-1.9199999999999999) qrg_nWlrB[4];
h qrg_nWlrB[4];
h qrg_nWlrB[5];
rz(-1.9199999999999999) qrg_nWlrB[5];
h qrg_nWlrB[5];
h qrg_nWlrB[6];
rz(-1.9199999999999999) qrg_nWlrB[6];
rz(0.1870343509512486) qrg_nWlrB[7];
rx(2.7866944065162156) qrg_nWlrB[7];
rz(-1.3951717724828128) qrg_nWlrB[7];
rx(2.5606912914823741) qrg_nWlrB[6];
h qrg_nWlrB[7];
cz qrg_nWlrB[6], qrg_nWlrB[7];
rx(-1.3200000000000003) qrg_nWlrB[6];
cz qrg_nWlrB[7], qrg_nWlrB[6];
h qrg_nWlrB[7];
ry(-1.5707963267948961) qrg_nWlrB[7];
rz(3.3274565978210302) qrg_nWlrB[7];
ry(-1.5707963267948966) qrg_nWlrB[6];
rz(6.1969013621074200) qrg_nWlrB[6];
h qrg_nWlrB[8];
rz(-1.9199999999999999) qrg_nWlrB[8];
h qrg_nWlrB[8];
h qrg_nWlrB[9];
rz(-1.9199999999999999) qrg_nWlrB[9];
h qrg_nWlrB[9];
rz(-0.2880000000000000) qrg_nWlrB[0];
rz(0.8640000000000000) qrg_nWlrB[1];
rz(-1.4399999999999999) qrg_nWlrB[4];
rz(-0.5760000000000000) qrg_nWlrB[5];
rz(1.2480000000000000) qrg_nWlrB[8];
rz(-1.8240000000000001) qrg_nWlrB[9];
rz(-0.9000000000000000) qrg_nWlrB[0];
rz(0.9000000000000000) qrg_nWlrB[1];
rz(-3.6000000000000001) qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
rz(1.8000000000000000) qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
rz(-0.3600000000000000) qrg_nWlrB[4];
rz(0.3600000000000000) qrg_nWlrB[5];
rz(-1.4399999999999999) qrg_nWlrB[5];
CX qrg_nWlrB[4], qrg_nWlrB[5];
rz(0.7200000000000000) qrg_nWlrB[5];
CX qrg_nWlrB[4], qrg_nWlrB[5];
rz(0.2400000000000000) qrg_nWlrB[8];
rz(-0.2400000000000000) qrg_nWlrB[9];
rz(0.9600000000000000) qrg_nWlrB[9];
CX qrg_nWlrB[8], qrg_nWlrB[9];
rz(-0.4800000000000000) qrg_nWlrB[9];
CX qrg_nWlrB[8], qrg_nWlrB[9];
rz(0.7800000000000000) qrg_nWlrB[1];
CX qrg_nWlrB[1], qrg_nWlrB[2];
rz(-1.5600000000000001) qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[2];
rz(-0.7800000000000000) qrg_nWlrB[3];
rz(0.7800000000000000) qrg_nWlrB[4];
rz(-3.1200000000000001) qrg_nWlrB[4];
CX qrg_nWlrB[3], qrg_nWlrB[4];
rz(1.5600000000000001) qrg_nWlrB[4];
CX qrg_nWlrB[3], qrg_nWlrB[4];
rz(1.1399999999999999) qrg_nWlrB[5];
CX qrg_nWlrB[5], qrg_nWlrB[6];
rz(-2.2799999999999998) qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
rz(-0.7800000000000000) qrg_nWlrB[7];
rz(0.7800000000000000) qrg_nWlrB[8];
rz(-3.1200000000000001) qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
rz(1.5600000000000001) qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[0];
rz(-0.9600000000000000) qrg_nWlrB[0];
h qrg_nWlrB[0];
h qrg_nWlrB[1];
rz(-0.9600000000000000) qrg_nWlrB[1];
h qrg_nWlrB[1];
h qrg_nWlrB[2];
rz(-0.9600000000000000) qrg_nWlrB[2];
h qrg_nWlrB[2];
h qrg_nWlrB[3];
rz(-0.9600000000000000) qrg_nWlrB[3];
h qrg_nWlrB[3];
h qrg_nWlrB[4];
rz(-0.9600000000000000) qrg_nWlrB[4];
h qrg_nWlrB[4];
h qrg_nWlrB[5];
rz(-0.9600000000000000) qrg_nWlrB[5];
h qrg_nWlrB[5];
h qrg_nWlrB[6];
rz(-0.9600000000000000) qrg_nWlrB[6];
h qrg_nWlrB[6];
h qrg_nWlrB[7];
rz(-0.9600000000000000) qrg_nWlrB[7];
h qrg_nWlrB[7];
rz(7.4687834425982746) qrg_nWlrB[9];
rx(1.0782253924597991) qrg_nWlrB[9];
rz(0.7087678269429900) qrg_nWlrB[9];
rz(-1.1410497340285382) qrg_nWlrB[8];
rx(1.0339790588614888) qrg_nWlrB[8];
rz(-3.8722983760777367) qrg_nWlrB[8];
h qrg_nWlrB[9];
cz qrg_nWlrB[8], qrg_nWlrB[9];
rx(-0.8000000000000000) qrg_nWlrB[8];
cz qrg_nWlrB[9], qrg_nWlrB[8];
h qrg_nWlrB[9];
ry(-1.5707963267948959) qrg_nWlrB[9];
rz(4.0449711166084086) qrg_nWlrB[9];
ry(-1.5707963267948959) qrg_nWlrB[8];
rz(2.7329779238938157) qrg_nWlrB[8];
rz(-0.1440000000000000) qrg_nWlrB[0];
rz(0.4320000000000000) qrg_nWlrB[1];
rz(0.5760000000000000) qrg_nWlrB[2];
rz(-0.5280000000000000) qrg_nWlrB[3];
rz(-0.7200000000000000) qrg_nWlrB[4];
rz(-0.2880000000000000) qrg_nWlrB[5];
rz(0.7680000000000000) qrg_nWlrB[6];
rz(-0.1440000000000000) qrg_nWlrB[7];
rz(-1.5000000000000000) qrg_nWlrB[0];
rz(1.5000000000000000) qrg_nWlrB[1];
rz(-6.0000000000000000) qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
rz(3.0000000000000000) qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
rz(-1.8000000000000000) qrg_nWlrB[2];
rz(1.8000000000000000) qrg_nWlrB[3];
rz(-7.2000000000000002) qrg_nWlrB[3];
CX qrg_nWlrB[2], qrg_nWlrB[3];
rz(3.6000000000000001) qrg_nWlrB[3];
CX qrg_nWlrB[2], qrg_nWlrB[3];
rz(-0.6000000000000000) qrg_nWlrB[4];
rz(0.6000000000000000) qrg_nWlrB[5];
rz(-2.3999999999999999) qrg_nWlrB[5];
CX qrg_nWlrB[4], qrg_nWlrB[5];
rz(1.2000000000000000) qrg_nWlrB[5];
CX qrg_nWlrB[4], qrg_nWlrB[5];
rz(1.1000000000000001) qrg_nWlrB[6];
rz(-1.1000000000000001) qrg_nWlrB[7];
rz(4.4000000000000004) qrg_nWlrB[7];
CX qrg_nWlrB[6], qrg_nWlrB[7];
rz(-2.2000000000000002) qrg_nWlrB[7];
CX qrg_nWlrB[6], qrg_nWlrB[7];
rz(1.3000000000000000) qrg_nWlrB[1];
rz(10.9955742875642759) qrg_nWlrB[2];
ry(1.5707963267948970) qrg_nWlrB[2];
rz(7.8539816339744828) qrg_nWlrB[1];
ry(-1.5707963267948970) qrg_nWlrB[1];
h qrg_nWlrB[2];
cz qrg_nWlrB[1], qrg_nWlrB[2];
rx(-0.5415926535897930) qrg_nWlrB[1];
cz qrg_nWlrB[2], qrg_nWlrB[1];
h qrg_nWlrB[2];
ry(-1.5707963267948966) qrg_nWlrB[2];
rz(2.3292036732051038) qrg_nWlrB[2];
rz(-1.5707963267948966) qrg_nWlrB[1];
rx(1.5707963267948970) qrg_nWlrB[1];
rz(3.1247779607693795) qrg_nWlrB[1];
rz(-1.3000000000000000) qrg_nWlrB[3];
rz(1.3000000000000000) qrg_nWlrB[4];
rz(-5.2000000000000002) qrg_nWlrB[4];
CX qrg_nWlrB[3], qrg_nWlrB[4];
rz(2.6000000000000001) qrg_nWlrB[4];
CX qrg_nWlrB[3], qrg_nWlrB[4];
rz(1.8999999999999999) qrg_nWlrB[5];
rz(-1.8999999999999999) qrg_nWlrB[6];
rz(7.5999999999999996) qrg_nWlrB[6];
ry(1.5707963267948970) qrg_nWlrB[6];
ry(1.5707963267948970) qrg_nWlrB[5];
h qrg_nWlrB[6];
cz qrg_nWlrB[5], qrg_nWlrB[6];
rx(-0.6584073464102063) qrg_nWlrB[5];
cz qrg_nWlrB[6], qrg_nWlrB[5];
h qrg_nWlrB[6];
ry(-1.5707963267948970) qrg_nWlrB[6];
rz(3.1415926535897931) qrg_nWlrB[6];
ry(-1.5707963267948966) qrg_nWlrB[5];
rz(0.6215926535897929) qrg_nWlrB[5];
rz(-1.3000000000000000) qrg_nWlrB[7];
ry(1.5707963267948970) qrg_nWlrB[8];
ry(1.5707963267948970) qrg_nWlrB[7];
h qrg_nWlrB[8];
cz qrg_nWlrB[7], qrg_nWlrB[8];
rx(-0.5415926535897930) qrg_nWlrB[7];
cz qrg_nWlrB[8], qrg_nWlrB[7];
h qrg_nWlrB[8];
ry(-1.5707963267948970) qrg_nWlrB[8];
rz(3.1415926535897931) qrg_nWlrB[8];
ry(-1.5707963267948970) qrg_nWlrB[7];
rz(1.4784073464102061) qrg_nWlrB[7];
h qrg_nWlrB[0];
rz(0.0000000000000000) qrg_nWlrB[0];
h qrg_nWlrB[0];
h qrg_nWlrB[3];
rz(0.0000000000000000) qrg_nWlrB[3];
h qrg_nWlrB[3];
h qrg_nWlrB[4];
rz(0.0000000000000000) qrg_nWlrB[4];
h qrg_nWlrB[4];
h qrg_nWlrB[9];
rz(0.0000000000000000) qrg_nWlrB[9];
h qrg_nWlrB[9];
rz(0.0000000000000000) qrg_nWlrB[0];
rz(0.0000000000000000) qrg_nWlrB[3];
rz(0.0000000000000000) qrg_nWlrB[4];
rz(0.0000000000000000) qrg_nWlrB[9];
rz(-2.1000000000000001) qrg_nWlrB[0];
CX qrg_nWlrB[0], qrg_nWlrB[1];
rz(4.2000000000000002) qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
rz(-2.5200000000000000) qrg_nWlrB[2];
rz(2.5200000000000000) qrg_nWlrB[3];
rz(-10.0800000000000001) qrg_nWlrB[3];
CX qrg_nWlrB[2], qrg_nWlrB[3];
rz(5.0400000000000000) qrg_nWlrB[3];
CX qrg_nWlrB[2], qrg_nWlrB[3];
rz(-0.8400000000000000) qrg_nWlrB[4];
CX qrg_nWlrB[4], qrg_nWlrB[5];
rz(1.6799999999999999) qrg_nWlrB[5];
CX qrg_nWlrB[4], qrg_nWlrB[5];
rz(1.5400000000000000) qrg_nWlrB[6];
CX qrg_nWlrB[6], qrg_nWlrB[7];
rz(-3.0800000000000001) qrg_nWlrB[7];
CX qrg_nWlrB[6], qrg_nWlrB[7];
rz(0.5600000000000001) qrg_nWlrB[8];
rz(-0.5600000000000001) qrg_nWlrB[9];
rz(2.2400000000000002) qrg_nWlrB[9];
CX qrg_nWlrB[8], qrg_nWlrB[9];
rz(-1.1200000000000001) qrg_nWlrB[9];
CX qrg_nWlrB[8], qrg_nWlrB[9];
rz(1.8200000000000001) qrg_nWlrB[1];
rz(-1.8200000000000001) qrg_nWlrB[2];
rz(7.2800000000000002) qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[2];
rz(-3.6400000000000001) qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[2];
rz(-1.8200000000000001) qrg_nWlrB[3];
rz(1.8200000000000001) qrg_nWlrB[4];
rz(-7.2800000000000002) qrg_nWlrB[4];
CX qrg_nWlrB[3], qrg_nWlrB[4];
rz(3.6400000000000001) qrg_nWlrB[4];
CX qrg_nWlrB[3], qrg_nWlrB[4];
rz(2.6600000000000001) qrg_nWlrB[5];
rz(-2.6600000000000001) qrg_nWlrB[6];
rz(10.6400000000000006) qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
rz(-5.3200000000000003) qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
rz(-1.8200000000000001) qrg_nWlrB[7];
rz(1.8200000000000001) qrg_nWlrB[8];
rz(-7.2800000000000002) qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
rz(3.6400000000000001) qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[0];
rz(0.9600000000000000) qrg_nWlrB[0];
h qrg_nWlrB[0];
h qrg_nWlrB[1];
rz(0.9600000000000000) qrg_nWlrB[1];
h qrg_nWlrB[1];
h qrg_nWlrB[2];
rz(0.9600000000000000) qrg_nWlrB[2];
h qrg_nWlrB[2];
h qrg_nWlrB[3];
rz(0.9600000000000000) qrg_nWlrB[3];
h qrg_nWlrB[3];
rz(-1.3081681427750791) qrg_nWlrB[5];
rx(1.9259056420456335) qrg_nWlrB[5];
rz(6.9413726602929122) qrg_nWlrB[5];
rz(7.3111297444624395) qrg_nWlrB[4];
rx(2.2064830340719861) qrg_nWlrB[4];
rz(2.3481893790990815) qrg_nWlrB[4];
h qrg_nWlrB[5];
cz qrg_nWlrB[4], qrg_nWlrB[5];
rx(-0.9815926535897930) qrg_nWlrB[4];
cz qrg_nWlrB[5], qrg_nWlrB[4];
h qrg_nWlrB[5];
ry(-1.5707963267948970) qrg_nWlrB[5];
rz(6.0344411184552200) qrg_nWlrB[5];
ry(-1.5707963267948966) qrg_nWlrB[4];
rz(5.9970795444432676) qrg_nWlrB[4];
h qrg_nWlrB[6];
rz(0.9600000000000000) qrg_nWlrB[6];
h qrg_nWlrB[6];
h qrg_nWlrB[7];
rz(0.9600000000000000) qrg_nWlrB[7];
h qrg_nWlrB[7];
h qrg_nWlrB[8];
rz(0.9600000000000000) qrg_nWlrB[8];
h qrg_nWlrB[8];
h qrg_nWlrB[9];
rz(0.9600000000000000) qrg_nWlrB[9];
h qrg_nWlrB[9];
rz(0.1440000000000000) qrg_nWlrB[0];
rz(-0.4320000000000000) qrg_nWlrB[1];
rz(-0.5760000000000000) qrg_nWlrB[2];
rz(0.5280000000000000) qrg_nWlrB[3];
rz(-0.7680000000000000) qrg_nWlrB[6];
rz(0.1440000000000000) qrg_nWlrB[7];
rz(-0.6240000000000000) qrg_nWlrB[8];
rz(0.9120000000000000) qrg_nWlrB[9];
rz(-2.7000000000000002) qrg_nWlrB[0];
rz(2.7000000000000002) qrg_nWlrB[1];
rz(-10.8000000000000007) qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
rz(5.4000000000000004) qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
rz(-3.2400000000000002) qrg_nWlrB[2];
rz(3.2400000000000002) qrg_nWlrB[3];
rz(-12.9600000000000009) qrg_nWlrB[3];
CX qrg_nWlrB[2], qrg_nWlrB[3];
rz(6.4800000000000004) qrg_nWlrB[3];
CX qrg_nWlrB[2], qrg_nWlrB[3];
rz(1.9800000000000000) qrg_nWlrB[6];
rz(-1.9800000000000000) qrg_nWlrB[7];
rz(7.9199999999999999) qrg_nWlrB[7];
CX qrg_nWlrB[6], qrg_nWlrB[7];
rz(-3.9600000000000000) qrg_nWlrB[7];
CX qrg_nWlrB[6], qrg_nWlrB[7];
rz(0.7200000000000000) qrg_nWlrB[8];
rz(-0.7200000000000000) qrg_nWlrB[9];
rz(2.8799999999999999) qrg_nWlrB[9];
CX qrg_nWlrB[8], qrg_nWlrB[9];
rz(-1.4399999999999999) qrg_nWlrB[9];
CX qrg_nWlrB[8], qrg_nWlrB[9];
rz(2.3399999999999999) qrg_nWlrB[1];
rz(-2.3399999999999999) qrg_nWlrB[2];
rz(9.3599999999999994) qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[2];
rz(-4.6799999999999997) qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[2];
rz(-2.3399999999999999) qrg_nWlrB[3];
CX qrg_nWlrB[3], qrg_nWlrB[4];
rz(4.6799999999999997) qrg_nWlrB[4];
CX qrg_nWlrB[3], qrg_nWlrB[4];
rz(3.4199999999999999) qrg_nWlrB[5];
rz(-3.4199999999999999) qrg_nWlrB[6];
rz(13.6799999999999997) qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
rz(-6.8399999999999999) qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
rz(-2.3399999999999999) qrg_nWlrB[7];
rz(2.3399999999999999) qrg_nWlrB[8];
rz(-9.3599999999999994) qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
rz(4.6799999999999997) qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[0];
rz(1.9199999999999999) qrg_nWlrB[0];
h qrg_nWlrB[0];
h qrg_nWlrB[1];
rz(1.9199999999999999) qrg_nWlrB[1];
h qrg_nWlrB[1];
h qrg_nWlrB[2];
rz(1.9199999999999999) qrg_nWlrB[2];
h qrg_nWlrB[2];
h qrg_nWlrB[3];
rz(1.9199999999999999) qrg_nWlrB[3];
h qrg_nWlrB[3];
h qrg_nWlrB[4];
rz(1.9199999999999999) qrg_nWlrB[4];
h qrg_nWlrB[4];
h qrg_nWlrB[5];
rz(1.9199999999999999) qrg_nWlrB[5];
h qrg_nWlrB[5];
h qrg_nWlrB[6];
rz(1.9199999999999999) qrg_nWlrB[6];
h qrg_nWlrB[6];
h qrg_nWlrB[7];
rz(1.9199999999999999) qrg_nWlrB[7];
h qrg_nWlrB[7];
h qrg_nWlrB[8];
rz(1.9199999999999999) qrg_nWlrB[8];
h qrg_nWlrB[8];
h qrg_nWlrB[9];
rz(1.9199999999999999) qrg_nWlrB[9];
h qrg_nWlrB[9];
rz(0.2880000000000000) qrg_nWlrB[0];
rz(-0.8640000000000000) qrg_nWlrB[1];
rz(-1.1519999999999999) qrg_nWlrB[2];
rz(1.0560000000000000) qrg_nWlrB[3];
rz(1.4399999999999999) qrg_nWlrB[4];
rz(0.5760000000000000) qrg_nWlrB[5];
rz(-1.5360000000000000) qrg_nWlrB[6];
rz(0.2880000000000000) qrg_nWlrB[7];
rz(-1.2480000000000000) qrg_nWlrB[8];
rz(1.8240000000000001) qrg_nWlrB[9];