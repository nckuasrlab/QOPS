from PyQt5 import *
from PyQt5.QtCore import QThread, pyqtSignal
from PyQt5.QtGui import QTextCharFormat, QColor, QTextCursor
from gui_ui import Ui_MainWindow
from text_track import *
from svg_mouse import *
from svg_qasm import svg_qasm
from svg_cpp import svg_cpp
import subprocess
import os

def read_file(file_name):
    buffer = ""
    with open(file_name, "r") as file:
        buffer = file.read()
    return buffer

def store_file(context, file_name):
    with open(file_name, "w") as file:
        file.write(context)

def get_newest_qasm():
    pwd = os.getcwd()
    qasm_files = []
    for file_name in os.listdir(pwd):
        if file_name[-5:] == ".qasm":
            qasm_files.append(file_name)
    if len(qasm_files) == 0:
        return ""
    newest_qasm = max(qasm_files, key=lambda file_name: os.path.getmtime(os.path.join(pwd, file_name)))
    return newest_qasm
    
class compile_thread(QThread): # thread run compile command
    finish = pyqtSignal(str)
    def __init__(self):
        super().__init__()

    def run(self):
        command_tmp = ["python3", "command.py", "init", "tmp.cpp"]
        terminal_message = subprocess.run(command_tmp, capture_output=True) # some error messages will appear in stdout
        if terminal_message.stderr:
            self.finish.emit(terminal_message.stderr.decode("utf-8") + terminal_message.stdout.decode("utf-8"))
            return
        name = get_newest_qasm()
        buffer = read_file(name)
        self.finish.emit("") # continue compile task

class sim_thread(QThread): # thread run simulation command
    finish = pyqtSignal(str)
    def __init__(self, qasm, sim_mode="context"):
        super().__init__()
        self.sim_mode = sim_mode
        self.qasm = qasm

    def run(self):
        store_file(self.qasm, "tmp.qasm")
        command_tmp = ["python3", "command.py", "sim", "tmp.qasm", self.sim_mode]
        terminal_message = subprocess.run(command_tmp, stderr=subprocess.PIPE)
        os.remove("tmp.qasm")
        if terminal_message.stderr :
            self.finish.emit(terminal_message.stderr.decode("utf-8"))
            return
        self.finish.emit("")

class svg_thread(QThread): # thread create svg file
    svg_sig = pyqtSignal()
    def __init__(self, src, num = 0):
        super().__init__()
        self.gate_num = num
        self.src = src

    def run(self):
        if self.src == "qasm":
            svg_qasm(self.gate_num)
        else:
            self.sleep(1)
            svg_cpp(self.src)
        self.svg_sig.emit()

class pgo_thread(QThread):
    finish = pyqtSignal(int)
    
    def __init__(self, cpp_source, qasm):
        super().__init__()
        self.source = cpp_source
        self.qasm = qasm
        self.sim_thread = sim_thread(qasm, "counter")
    
    def run(self):
        self.sim_thread.run()
        self.sim_thread.wait()
        store_file(self.qasm+'\n', "before_pgo.qasm") # +'\n' prevent diff from 'no newline at end of file' warning
        store_file(self.source, "tmp.cpp");
        subprocess.run(["python3 command.py pgo tmp.cpp"], shell = True)
        qasm_file_name = get_newest_qasm()
        subprocess.run(["mv", qasm_file_name, "after_pgo.qasm"])
        subprocess.run(["diff -Naur before_pgo.qasm after_pgo.qasm | tail -n +4 >> qasm.diff"], shell = True)
        self.finish.emit(1)

class main_window(QtWidgets.QMainWindow):
    def __init__(self, file_path):
        super(main_window, self).__init__()

        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.ui.action_compile.triggered.connect(self.compile)
        self.ui.action_add_file.triggered.connect(self.add_file)
        self.ui.action_simulation.triggered.connect(self.prof_data_display)
        self.ui.action_pgo.triggered.connect(self.pgo)
        self.ui.action_qasm_graph.triggered.connect(self.disp_qasm_circ)
        self.ui.action_draw_qasm.triggered.connect(self.svg_run_qasm)
        self.ui.action_draw_cpp.triggered.connect(self.svg_run_cpp)

        self.ui.pgo_left_display.verticalScrollBar().valueChanged.\
            connect(self.ui.pgo_right_display.verticalScrollBar().setValue)
        self.ui.pgo_right_display.verticalScrollBar().valueChanged.\
            connect(self.ui.pgo_left_display.verticalScrollBar().setValue)
        self.ui.progbar.setVisible(False)
        self.ui.profile_file_display.setReadOnly(True)
        self.init_object(file_path)
        self.ui.svg_view.setVisible(False)
        self.ui.svg_view.setDragMode(1)

        self.source = ""
        self.qasm = ""
        self.qasm_map = []
        self.tracker_old_data = 0 # used by tracker function
        self.tracker_old_key = (0,0)
        self.draw_type = 0 # for svg, 0 for qasm, 1 for cpp

    def tracker(self, item_num=0):
        if item_num == 0: # click profile data display
            cursor = self.ui.profile_file_display.textCursor()
            cur_pos = cursor.blockNumber() + 1
        elif item_num == -1: # click source display
            cursor = self.ui.code_input_display.textCursor()
            cur_pos = cursor.blockNumber() + 1
        else: # click svg
            cur_pos = item_num
            if self.draw_type == 1: # draw cpp file
                cur_pos -= 1 # offset

        # if user click another line, reset the background color of last clicked line.
        self.set_backcolor("white", self.tracker_old_data, self.tracker_old_data, self.ui.code_input_display)
        if self.tracker_old_key != (0,0):
            self.set_backcolor("white", self.tracker_old_key[0], self.tracker_old_key[1], self.ui.profile_file_display)

        # click source display or svg of source
        if item_num == -1 or self.draw_type == 1:
            # reverse map so that can simply use index to get value
            new_map = {v: k for k, v in self.qasm_map.items()}
            if cur_pos not in new_map:
                return
            self.set_backcolor("#C7C7E2", cur_pos, cur_pos, self.ui.code_input_display)
            position = self.set_backcolor("#C7C7E2", new_map[cur_pos][0], new_map[cur_pos][1], self.ui.profile_file_display)
            # adjust scroll bar position(by ratio)
            self.ui.profile_file_display.verticalScrollBar().setValue\
                (position / (len(self.qasm) / self.ui.profile_file_display.verticalScrollBar().maximum()))
            self.tracker_old_data = cur_pos
            self.tracker_old_key = new_map[cur_pos]
            return position
        # click on prof display or svg of prof data
        for key in self.qasm_map:
            if key[0] <= cur_pos <= key[1]:
                position = self.set_backcolor("#C7C7E2", self.qasm_map[key], self.qasm_map[key], self.ui.code_input_display)
                # adjust scroll bar position(by ratio)
                self.ui.code_input_display.verticalScrollBar().setValue\
                    (position / (len(self.source) / self.ui.code_input_display.verticalScrollBar().maximum()))
                # self.tracker_old_data = self.qasm_map[key]
                position = self.set_backcolor("#C7C7E2", key[0], key[1], self.ui.profile_file_display)
                self.set_backcolor("#FFAF60", cur_pos, cur_pos, self.ui.profile_file_display)
                self.tracker_old_data = self.qasm_map[key]
                self.tracker_old_key = key
                return position

    def init_object(self, file_path):
        buffer = ""
        if file_path != "":
            buffer = read_file(file_path)
            self.ui.code_input_display.setText(buffer)

    def compile(self):
        self.set_bar_val(0)
        self.comp_th = compile_thread()
        self.comp_th.finish.connect(self.compile_finish)
        self.set_bar_val(40)
       
        self.ui.svg_view.resetTransform()
        self.ui.profile_file_display.clear()
        self.ui.profile_file_display.setTextBackgroundColor(QColor("white"))
        self.source = self.ui.code_input_display.toPlainText()
        if self.source == "":
            self.ui.profile_file_display.setText("source data is empty\n")
            return
        store_file(self.source, "tmp.cpp")
        self.comp_th.start()

    def compile_finish(self, value): # remain part of compile phase, called by thread
        if value != "":
            self.ui.progbar.setVisible(False)
            self.ui.profile_file_display.setText(value)
            self.check_connect_and_disconnect()
            return
        self.pre_qasm()
        self.ui.profile_file_display.setText(self.qasm)
        self.set_bar_val(70)
        output = self.ui.profile_file_display.cursorPositionChanged.connect(self.tracker)
        self.ui.code_input_display.cursorPositionChanged.connect(lambda: self.tracker(-1))
        os.remove("tmp.cpp")
        os.remove("a.out")
        self.set_bar_val(100)

    def pre_qasm(self): # handle comment string in qasm file
        qasm_file_name = get_newest_qasm()
        self.qasm = read_file(qasm_file_name)
        self.qasm_map, self.qasm = map_source_and_qasm(self.source, self.qasm)
        str_separator = "\n"
        self.qasm = str_separator.join(self.qasm)
        store_file(self.qasm, qasm_file_name)

    def add_file(self):
        options = QtWidgets.QFileDialog.Options()
        file_path, tmp= QtWidgets.QFileDialog.getOpenFileName(self, "select a file", "", "All Files (*)", options=options)
        if file_path == "":
            return
        self.source = read_file(file_path)
        self.ui.code_input_display.setText(self.source)
        self.compile()

    def simulation(self, sim_mode="context", handle_qasm=True):
        if handle_qasm == True:
            self.pre_qasm()
        self.sim_thread = sim_thread(self.qasm, sim_mode)
        self.sim_thread.finish.connect(self.sim_error)
        self.sim_thread.start()

    def sim_error(self, sig_val): # triggered by simulation thread signal
        if sig_val != "":
            self.ui.progbar.setVisible(False)
            self.ui.profile_file_display.setText(sig_val)
            self.check_connect_and_disconnect()
            
    def prof_data_display(self):
        self.simulation("context", False)
        self.ui.profile_file_display.clear()
        self.ui.profile_file_display.setTextBackgroundColor(QColor("white"))
        self.sim_thread.wait()
        buffer = read_file(os.path.expanduser("~/Quokka/src/correctness/xxx.out"))
        self.ui.profile_file_display.setText(buffer)
        self.check_connect_and_disconnect()

    def pgo(self, cpp_file):
        self.set_bar_val(0)
        self.source = str(self.ui.code_input_display.toPlainText()) # update source data
        self.pgo_th = pgo_thread(self.source, self.qasm)
        self.pgo_th.finish.connect(self.pgo_finish)
        self.ui.pgo_left_display.clear()
        self.ui.pgo_right_display.clear()
        self.set_bar_val(30)
        self.pgo_th.start()

    def pgo_finish(self): # remain part of pgo phase, called by thread
        self.set_bar_val(70)
        os.remove("tmp.cpp")
        self.print_diff()
        subprocess.run(["rm qasm.diff before_pgo.qasm after_pgo.qasm"], shell = True)
        self.set_bar_val(100)

    def disp_qasm_circ(self):
        if self.ui.svg_view.isVisible():
            self.ui.svg_view.setVisible(False)
        else:
            self.ui.svg_view.setVisible(True)

    def svg_run_qasm(self):
        self.draw_type = 0
        self.set_bar_val(0)
        self.simulation("context", False)
        self.set_bar_val(30)
        pos = self.qasm.find('[')
        pos += 1
        gate_num = 0
        while self.qasm[pos] != ']':
            gate_num = gate_num * 10 + int(self.qasm[pos])
            pos += 1
        self.svg_thread = svg_thread("qasm", gate_num)
        self.svg_thread.svg_sig.connect(self.svg_to_qt)
        self.set_bar_val(50)
        svg_init(self.ui.svg_view) # refresh QGraphicsScene
        self.sim_thread.wait() # wait simulation done, or the svg_thread won't get profile data
        self.svg_thread.start()

    def svg_run_cpp(self):
        self.draw_type = 1
        self.set_bar_val(0)
        self.simulation("context", False)
        self.set_bar_val(30)
        self.svg_thread = svg_thread(self.source, 0)
        self.svg_thread.svg_sig.connect(self.svg_to_qt)
        self.set_bar_val(50)
        svg_init(self.ui.svg_view) # refresh QGraphicsScene
        self.sim_thread.wait() # wait simulation done, or the svg_thread won't get profile data
        self.svg_thread.start()

    def svg_to_qt(self): # called by thread
        buffer = read_file("circ.svg")
        self.set_bar_val(70)
        img = buffer.encode('utf-8')
        set_svg(self.ui.svg_view, self, img, buffer)
        self.set_bar_val(100)

    def svg_press(self, num):
        pos = self.tracker(num)
        # adjust scroll bar position(by ratio)
        self.ui.profile_file_display.verticalScrollBar().setValue\
            (pos / (len(self.qasm) / self.ui.profile_file_display.verticalScrollBar().maximum()))
        
    def print_diff(self):
        counter = 70
        with open("before_pgo.qasm", 'r') as header: # print header info
            for i in range(4):
                buffer = header.readline()
                self.insert_color_text(buffer, "white", self.ui.pgo_left_display)
                self.insert_color_text(buffer, "white", self.ui.pgo_right_display)
        with open("qasm.diff", 'r') as diff_file:
            counter += 1
            self.set_bar_val(counter)
            for line in diff_file:
                if line[0] == '-':
                    line = line[1:]
                    self.insert_color_text(line, "#EB8988", self.ui.pgo_left_display)
                    self.insert_color_text("\n", "white", self.ui.pgo_right_display)
                elif line[0] == '+':
                    line = line[1:]
                    self.insert_color_text("\n", "white", self.ui.pgo_left_display)
                    self.insert_color_text(line, "#A0DB4D", self.ui.pgo_right_display)
                elif line[0] == '@':
                    continue
                else:
                    line = line[1:]
                    self.insert_color_text(line, "white", self.ui.pgo_right_display)
                    self.insert_color_text(line, "white", self.ui.pgo_left_display)

    def check_connect_and_disconnect(self):
        # check wether the signal is connected to function
        # only check profile_file_display because they always link simultaneously
        if self.ui.profile_file_display.receivers(self.ui.profile_file_display.cursorPositionChanged):
            self.ui.profile_file_display.cursorPositionChanged.disconnect()
            self.ui.code_input_display.cursorPositionChanged.disconnect()

    def insert_color_text(self, text, color, obj):
        cursor = obj.textCursor()
        fmt = QTextCharFormat()
        fmt.setBackground(QColor(color))
        cursor.insertText(text, fmt)
    
    # set background color for multiple lines
    def set_backcolor(self, color, begin_line, end_line, obj):
        cursor = obj.textCursor()
        cursor.movePosition(QTextCursor.Start)
        for i in range(begin_line - 1):
            cursor.movePosition(QTextCursor.Down)
        lines = end_line - begin_line + 1
        for i in range(lines):
            cursor.select(QTextCursor.LineUnderCursor)
            fmt = cursor.charFormat()
            fmt.setBackground(QColor(color))
            cursor.setCharFormat(fmt)
            cursor.movePosition(QTextCursor.Down)
        return cursor.position()

    def set_bar_val(self, value):
        if value == 0:
            self.ui.progbar.setVisible(True)
        elif value == 100:
            self.ui.progbar.setVisible(False)
        self.ui.progbar.setValue(value)
