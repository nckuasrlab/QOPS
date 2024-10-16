import svgwrite
import os

GATE_OPS = {
    "H": 0,
    "S": 1,
    "T": 2,
    "X": 3,
    "Y": 4,
    "Z": 5,
    "Phase": 6,
    "U1": 7,
    "CX": 8,
    "CY": 9,
    "CZ": 10,
    "CPhase": 11,
    "CU1": 12,
    "SWAP": 13,
    "TOFFOLI": 14,
    "M": 20,
    "OPS_MEASURE_MULTI": 21,
    "OPS_COPY": 22,
    "U2": 31,
    "U3": 32,
}
INV_GATE_OPS = {v: k for k, v in GATE_OPS.items()}

def read_file():
    with open(os.path.expanduser("~/Quokka/src/correctness/xxx.out")) as fin:
        data = [[int(di) for di in line.split()] for line in fin]
    return data

def draw_rect(dwg, p_x, p_y, gate, gate_num, gate_cnt): # draw gate icon
    draw_dash_line(dwg, p_x+150, gate_cnt*150+130)
    dwg.add(dwg.rect(insert=(p_x+100, p_y), size=(100, 100), id=gate+gate_num, fill='white', stroke='black', stroke_width=5))
    while gate[0] == 'c' or gate[0] == 'C':
        gate = gate[1:]
    dwg.add(dwg.text(gate, id="text"+gate, insert=(p_x+135, p_y+38), fill='black'))
    dwg.save()

def draw_qbit_line(dwg, svg_size_width, num): # draw qubit line(horizontal)
    g = dwg.g(stroke="black")
    for i in range(num):
        tmp = "q_" + str(i)
        dwg.add(dwg.text(tmp, id="text"+tmp, insert=(25, i*150+130), fill='black', font_size='20px'))
        g.add(dwg.line(id=tmp+"line", start=(85, i*150+150), end=(svg_size_width, i*150+150), stroke_width=5))
    dwg.add(g)

def draw_dash_line(dwg, start_x, end_y): # draw qubit line(vertical, dashline) 
    g = dwg.g(stroke="black")
    g.add(dwg.line(id="1dash"+str(start_x), start=(start_x, 50), end=(start_x, end_y), stroke_width=5, stroke_dasharray=8))
    dwg.add(g)

def draw_ctrl_line(dwg, start_x, start_y, end_y, num): # for control bit, black line
    g = dwg.g(stroke="black")
    name=num+"line"
    g.add(dwg.line(id=name, start=(start_x, start_y), end=(start_x, end_y), stroke_width=10))
    dwg.add(g)

def draw_ctrl(dwg, pos, ctrl, gate, gate_num): # for control bit
    draw_ctrl_line(dwg, pos+150, gate*150+100, ctrl*150+150, gate_num)
    dwg.add(dwg.circle(id="circle"+gate_num, center=(pos+150, ctrl*150+150), r=13))
