import svgwrite
import os
from svg_core import *

def preprocess_delete_irrelevant(cpp_source): # delete irrelevant code
    begin = cpp_source.find('qalloc(')
    end = cpp_source.find(')', begin)
    gate_num = int(cpp_source[begin+7:end])
    begin = cpp_source.find('using qcor::openqasm;') + len('using qcor::openqasm;')
    end = cpp_source.find('}')
    text = cpp_source[begin:end].splitlines()
    text = [s.lstrip() for s in text]
    data = []
    for i in text:
        pos = i.find(' ')
        gate_type = i[:pos]
        tmp = [gate_type]
        while i.find('[', pos + 1) != -1:
            pos = i.find('[', pos+1)
            right_pos = i.find(']', pos)
            tmp.append(int(i[pos + 1:right_pos]))
        if gate_type != "":
            data.append(tmp)
    return gate_num, data

def svg_cpp(cpp_source):
    gate_cnt, data = preprocess_delete_irrelevant(cpp_source)
    prof_data = read_file()
    svg_size_width = len(data)*150+150
    svg_size_height = gate_cnt*165
    dwg = svgwrite.Drawing('circ.svg', (svg_size_width, svg_size_height), debug=False)
    dwg.add(dwg.rect(insert=(0, 0), size=('100%', '100%'), rx=None, ry=None, fill='white'))
    draw_qbit_line(dwg, svg_size_width, gate_cnt)

    pos = 50 # x-axis position for every qubit
    gate_num = 5 # for naming uniqueness, also be used as index when click the svg graph
    last_time_stamp = 0
    prof_head = 0 # two pointer for the range of composite gate
    prof_tail = 0
    for i in data:
        if prof_head != 0: # calculation for time stamp
            earlier = int(prof_data[prof_tail][len(prof_data[prof_tail]) - 1])
            older = int(prof_data[prof_head][len(prof_data[prof_head]) - 1])
            time_tmp = format((older - earlier)/1000, ".1f")
            dwg.add(dwg.text(time_tmp, id="time"+time_tmp, insert=(pos-25, gate_cnt*150+150), fill='black', font_size='20px'))
            prof_tail = prof_head
        else:
            dwg.add(dwg.text("timeTime(us)", id="timeTime(us)", insert=(pos-25, gate_cnt*150+150), fill='black', font_size='20px'))
        if i[0] == "ccx":
            draw_ctrl(dwg, pos, i[1], i[3], str(gate_num)+'ccx')  # +'ccx' is for naming uniqueness. Qt need different
            draw_ctrl(dwg, pos, i[2], i[3], str(gate_num)+'ccx2') # name to identify different item
            draw_rect(dwg, pos, i[3]*150+100, i[0], str(gate_num), gate_cnt)
            prof_head += 15
        elif i[0][0] == "c":
            draw_ctrl(dwg, pos, i[1], i[2], str(gate_num))
            draw_rect(dwg, pos, i[2]*150+100, i[0], str(gate_num), gate_cnt)
            prof_head += 1
        else:
            draw_rect(dwg, pos, i[1]*150+100, i[0], str(gate_num), gate_cnt)
            prof_head += 1
        gate_num+=1
        pos+=150
    # for the last time stamp
    earlier = int(prof_data[prof_tail][len(prof_data[prof_tail]) - 1])
    older = int(prof_data[prof_head][len(prof_data[prof_head]) - 1])
    time_diff = format((older - earlier)/1000, ".1f")
    dwg.add(dwg.text(time_diff, id="time"+time_diff, insert=(pos-25, gate_cnt*150+150), fill='black', font_size='20px'))
    dwg.save()
