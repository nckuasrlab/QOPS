from PyQt5 import QtWidgets
from gui_controller import main_window
import sys

app = QtWidgets.QApplication(sys.argv)
file_path = ""
if len(sys.argv) > 1:
    file_path = sys.argv[1]
window = main_window(file_path)
window.show()
sys.exit(app.exec_())
