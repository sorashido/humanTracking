import cv2
import copy
import numpy as np

def mouse_event(event, x, y, flags, param):
    if event == cv2.EVENT_LBUTTONDBLCLK:
        print(x, y, res[y, x])

def rotM(p):
    # 回転行列を計算する
    px = p[0]
    py = p[1]
    pz = p[2]

    # 物体座標系の 1->2->3 軸で回転させる
    Rx = np.array([[1, 0, 0, 0],
                   [0, np.cos(px), np.sin(px), 0],
                   [0, -np.sin(px), np.cos(px), 0],
                   [0, 0, 0, 1]])
    Ry = np.array([[np.cos(py), 0, -np.sin(py), 0],
                   [0, 1, 0, 0],
                   [np.sin(py), 0, np.cos(py), 0],
                   [0, 0, 0, 1]])
    Rz = np.array([[np.cos(pz), np.sin(pz), 0, 0],
                   [-np.sin(pz), np.cos(pz), 0, 0],
                   [0, 0, 1, 0],
                   [0, 0, 0, 1]])
    R = Rz.dot(Ry).dot(Rx)
    return R

def affineM(p, t):
    r = rotM(p)

    tx = t[0]
    ty = t[1]
    tz = t[2]
    tm = np.array([[1, 0, 0, 0],
                  [0, 1, 0, 0],
                  [0, 0, 1, 0],
                  [-tx, -ty, -tz, 1]])

    t_inv = np.linalg.inv(tm)

    return tm.dot(r).dot(t_inv)

# range: 500mm ~ 3500mm
if __name__ == '__main__':
    cap = cv2.VideoCapture('./data/Real02.mp4')

    win_name = '1'
    cv2.namedWindow(win_name)
    cv2.setMouseCallback(win_name, mouse_event)

    while(cap.isOpened()):
        ret, frame = cap.read()
        src = cv2.resize(frame[26:719, 54:989], (640, 480))
        height, width, channels = src.shape[:3]
        # dst = copy.deepcopy(src)

        gray = cv2.cvtColor(src, cv2.COLOR_BGR2GRAY)
        depth = np.full((height, width), 3500.0) - gray*np.full((height, width), 3000/255)

        pz = -np.pi/4
        Rz = np.array([[np.cos(pz), np.sin(pz), 0],
                       [-np.sin(pz), np.cos(pz), 0],
                       [0, 0, 1]])
        px = np.pi/4
        Rx = np.array([[1, 0, 0],
                       [0, np.cos(px), np.sin(px)],
                       [0, -np.sin(px), np.cos(px)]])

        py = -np.pi/1000
        Ry = np.array([[np.cos(py), 0, -np.sin(py)],
                       [0, 1, 0],
                       [np.sin(py), 0, np.cos(py)]])

        depth = cv2.warpPerspective(depth, Rx, (640, 480))

        res = depth * 255/3500
        res = np.uint8(res)
        # res = np.(depth)
        # perspective = cv2.warpPerspective(gray, A, (640, 480))
        # cv2.estimateAffine3D()
        # X = cv2.reprojectImageTo3D(depth, M)
        # cv2.perspectiveTransform()
        cv2.imshow(win_name, res)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()