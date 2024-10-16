import svgwrite
import os
from svg_core import *

def svg_qasm(gate_cnt):
    data = read_file()
    svg_size_width = len(data)*150
    svg_size_height = gate_cnt*165
    dwg = svgwrite.Drawing('circ.svg', (svg_size_width, svg_size_height), debug=False)
    dwg.add(dwg.rect(insert=(0, 0), size=('100%', '100%'), rx=None, ry=None, fill='white'))
    draw_qbit_line(dwg, svg_size_width, gate_cnt)
    pos = 50 # x-axis position for every qubit
    gate_num = 5 # for naming uniqueness, also be used as index when click the svg graph
    last_time_stamp = 0
    for i in data:
        if len(i) == 5:
            draw_ctrl_line(dwg, pos+150, i[2]*150+100, i[1]*150+150, str(gate_num))
            dwg.add(dwg.circle(id="circle"+str(gate_num), center=(pos+150, i[1]*150+150), r=13))
            draw_rect(dwg, pos, i[2]*150+100, INV_GATE_OPS[i[3]], str(gate_num), gate_cnt)
        elif len(i) == 4:
            draw_rect(dwg, pos, i[1]*150+100, INV_GATE_OPS[i[2]], str(gate_num), gate_cnt)
        if last_time_stamp != 0: # calculation for time stamp
            time_tmp = format((int(i[len(i)-1])-last_time_stamp)/1000, ".1f")
            dwg.add(dwg.text(time_tmp, id="time"+time_tmp, insert=(pos-25, gate_cnt*150+150), fill='black', font_size='20px'))
        else:
            dwg.add(dwg.text("text", id="timeTime(us)", insert=(pos-25, gate_cnt*150+150), fill='black', font_size='20px'))
        last_time_stamp=int(i[len(i)-1])
        gate_num+=1
        pos+=150
    dwg.save()
