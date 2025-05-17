import os
import sys
import warnings

warnings.filterwarnings("ignore", category=DeprecationWarning)
from PyQt5.QtWidgets import QApplication, QMainWindow, QFileDialog

sys.path.append('UIProgram')
from UIProgram.UiMain import Ui_MainWindow
import sys
from PyQt5.QtCore import QTimer, Qt, QCoreApplication
import detect_tools as tools
import Config
from PyQt5.QtGui import QPixmap
from DenseNet121 import *


# interface function design
class MainWindow(QMainWindow):
    def __init__(self, parent=None):
        super(QMainWindow, self).__init__(parent)
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.initMain()
        self.signalconnect()

    # Initiation
    def initMain(self):
        self.labeldict = {0: 'Angry', 1: 'Disgust', 2: 'Fear', 3: 'Happy', 4: 'Sad', 5: 'Surprised', 6: 'Normal'}

    self.labelchinese = {0: '生氣', 1: '厭惡', 2: '害怕', 3: '高興', 4: '傷心', 5: '驚訝', 6: '平淡'}
    inputs = keras.Input(shape=(48, 48, 1), batch_size=64)


x = create_dense_net(7, inputs, include_top=True, depth=121, nb_dense_block=4, growth_rate=16, nb_filter=-1,
                     nb_layers_per_block=[6, 12, 32, 32], bottleneck=True, reduction=0.5, dropout_rate=0.2,
                     activation='softmax')
self.model = tf.keras.Model(inputs, x, name='densenet121')
filepath = Config.model_path
self.model.load_weights(filepath)
self.model.predict(np.zeros((1, 48, 48, 1)))
self.show_width = 770
self.show_height = 460
self.org_path = None
self.is_camera_open = False
self.cap = None
self.timer_camera = QTimer()


# button function connection
def signalconnect(self):
    self.ui.PicBtn.clicked.connect(self.open_img)
    self.ui.VideoBtn.clicked.connect(self.vedio_show)
    self.ui.CapBtn.clicked.connect(self.camera_show)
    self.ui.exitBtn.clicked.connect(QCoreApplication.quit)


def open_img(self):
    if self.cap:
        self.video_stop()
        self.is_camera_open = False
        self.ui.CapBtn.setText('打開攝像頭')
        self.cap = None

    # file_path, _ = QFileDialog.getOpenFileName(self.ui.centralwidget, '打開圖片', './', "Image files (*.jpg *.gif)")
    file_path, _ = QFileDialog.getOpenFileName(None, '打開圖片', './', "Image files (*.jpg *.jepg *.png)")
    if not file_path:
        return
    self.org_path = file_path
    self.cv_img = tools.img_cvread(self.org_path)

    face_cvimg, faces, locations = face_detect(self.cv_img)

    if faces is not None:
        for i in range(len(faces)):  ：
        top, right, bottom, left = locations[i]
        face = cv2.cvtColor(faces[i], cv2.COLOR_BGR2GRAY)
        face = cv2.resize(face, (48, 48))
        face = face / 255.0
        conf_res = self.model.predict(
            np.reshape(face, (-1, 48, 48, 1)))
        num = np.argmax(conf_res)
        label = self.labeldict[num]

        max_conf = max(conf_res[0]) * 100
        self.ui.confLb.setText('{:.2f}%'.format(max_conf))
        face_cvimg = cv2.putText(face_cvimg, label, (left, top - 10), cv2.FONT_ITALIC, 0.8, (0, 0, 250), 2,
                                 cv2.LINE_AA)

        self.ui.resLb.setText(self.labeldict[num] + '--' + self.labelchinese[num])
        icon_name = self.labeldict[num] + '.png'
        icon_path = os.path.join('UIProgram/ui_imgs', icon_name)
        pix = QPixmap(icon_path)
        self.ui.resIcon.setPixmap(pix)
        self.ui.resIcon.setScaledContents(True)


self.img_width, self.img_height = self.get_resize_size(face_cvimg)

resize_cvimg = cv2.resize(face_cvimg, (self.img_width, self.img_height))

pix_img = tools.cvimg_to_qpiximg(resize_cvimg)

self.ui.label_show.setPixmap(pix_img)
self.ui.label_show.setAlignment(Qt.AlignCenter)


def get_video_path(self):
    file_path, _ = QFileDialog.getOpenFileName(None, '打開視頻', './', "Image files (*.avi *.mp4)")
    if not file_path:
        return None
    self.org_path = file_path
    return file_path


def video_start(self):
    self.timer_camera.start(30)
    self.timer_camera.timeout.connect(self.open_frame)


def video_stop(self):
    self.is_camera_open = False
    if self.cap is not None:
        self.cap.release()
    self.timer_camera.stop()
    self.ui.label_show.clear()


def open_frame(self):
    ret, image = self.cap.read()
    if ret:
        face_cvimg, faces, locations = face_detect(image)
        if faces is not None:
            for i in range(len(faces)):
                top, right, bottom, left = locations[i]
                face = cv2.cvtColor(faces[i], cv2.COLOR_BGR2GRAY)
                face = cv2.resize(face, (48, 48))
                face = face / 255.0
                conf_res = self.model.predict(np.reshape(face, (-1, 48, 48, 1)))
                num = np.argmax(conf_res)
                label = self.labeldict[num]
                face_cvimg = cv2.putText(face_cvimg, label, (left, top - 10), cv2.FONT_ITALIC, 0.8, (0, 0, 250), 2,
                                         cv2.LINE_AA)
                self.ui.resLb.setText(self.labeldict[num] + '--' + self.labelchinese[num])
                icon_name = self.labeldict[num] + '.png'
                icon_path = os.path.join('UIProgram/ui_imgs', icon_name)
                pix = QPixmap(icon_path)
                self.ui.resIcon.setPixmap(pix)
                self.ui.resIcon.setScaledContents(True)
                max_conf = max(conf_res[0]) * 100
                self.ui.confLb.setText('{:.2f}%'.format(max_conf))

        self.img_width, self.img_height = self.get_resize_size(face_cvimg)
        resize_cvimg = cv2.resize(face_cvimg, (self.img_width, self.img_height))

        pix_img = tools.cvimg_to_qpiximg(resize_cvimg)
        self.ui.label_show.setPixmap(pix_img)
        self.ui.label_show.setAlignment(Qt.AlignCenter)
    else:
        self.cap.release()
        self.timer_camera.stop()


def vedio_show(self):
    if self.is_camera_open:
        self.is_camera_open = False
        self.ui.CapBtn.setText('打開攝像頭')

    video_path = self.get_video_path()
    if not video_path:
        return None
    self.cap = cv2.VideoCapture(video_path)
    self.video_start()


def camera_show(self):
    self.is_camera_open = not self.is_camera_open
    if self.is_camera_open:
        self.ui.CapBtn.setText('關閉攝像頭')
        self.cap = cv2.VideoCapture(0)
        self.video_start()
    else:
        self.ui.CapBtn.setText('打開攝像頭')
        self.ui.label_show.setText('')
        if self.cap:
            self.cap.release()
            cv2.destroyAllWindows()
        self.ui.label_show.clear()


def get_resize_size(self, img):
    _img = img.copy()
    img_height, img_width, depth = _img.shape
    ratio = img_width / img_height
    if ratio >= self.show_width / self.show_height:
        self.img_width = self.show_width
        self.img_height = int(self.img_width / ratio)
    else:
        self.img_height = self.show_height
        self.img_width = int(self.img_height * ratio)
    return self.img_width, self.img_height


if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = MainWindow()
    win.show()
    sys.exit(app.exec_())
