from PyQt5 import QtWidgets
from PyQt5.QtSvg import QGraphicsSvgItem, QSvgRenderer
from PyQt5.QtCore import QRectF

class SvgItem(QGraphicsSvgItem): # class for a svg item
    def __init__(self, id, renderer, window, parent=None):
        super().__init__(parent)
        self.id = id
        self.setSharedRenderer(renderer)
        self.setElementId(id)
        self.window = window
        bounds = renderer.boundsOnElement(id)
        self.setPos(bounds.topLeft())

    def mousePressEvent(self, event: 'QtWidgets.QGraphicsSceneMouseEvent'):
        i = 0
        for i in range(len(self.id)):
            if self.id[i].isdigit():
                break
        if self.id[0] == 'U':
            i+=1
        try:
            num_tmp = int(self.id[i:])
        except ValueError:
            return
        self.window.svg_press(int(self.id[i:]))
        super().mousePressEvent(event)

def svg_init(qt_view):
    qt_view._scene = QtWidgets.QGraphicsScene(qt_view)
    qt_view._renderer = QSvgRenderer()
    qt_view.setScene(qt_view._scene)

def set_svg(qt_view, window, data, str_data): # add svg item into qt
    qt_view.resetTransform()
    qt_view._scene.clear()
    qt_view._renderer.load(data)
    start = 0
    while True: # find svg item in svg data string
        start = str_data.find("id=\"", start)
        end = str_data.find("\"", start+5)
        if start == -1:
            break;
        name = str_data[start+4:end]
        if name[:4] == "text" or name[:4] == "time": # the prefix of time stamp and some text
            id = name[4:]
            pos_start = str_data.find("x=\"", end)
            pos_end = str_data.find("\"", pos_start+3)
            x = int(str_data[pos_start+3:pos_end])
            pos_start = str_data.find("y=\"", end)
            pos_end = str_data.find("\"", pos_start+3)
            y = int(str_data[pos_start+3:pos_end])
            text = qt_view._scene.addText(id)
            f = text.font()
            f.setPixelSize(25)
            text.setFont(f)
            text.setPos(x,y)
        else: # add other item, e.g: rectangle, line, circle...
            item = SvgItem(name, qt_view._renderer, window)
            qt_view._scene.addItem(item)
        start = end + 1
