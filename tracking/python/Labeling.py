import cv2
import numpy as np

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

    # t_inv = np.linalg.inv(tm)
    # return t_inv.dot(r).dot(tm)
    # print(r)
    return tm.dot(r)

if __name__ == '__main__':
    # cap = cv2.VideoCapture('../data/Real02.mp4')

    # kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(3,3))
    # fgbg = cv2.createBackgroundSubtractorMOG2()

    p = [-5*np.pi/18, 0, 0]
    # p = [5*np.pi/18, 0, 0]
    t = [0, 0, 0]
    M = affineM(p, t)
    print(M)
    # history_track_data = []
    # while(cap.isOpened()):
    #     # 前準備
    #     ret, frame = cap.read()
    #     src = cv2.resize(frame[26:719, 54:989], (640, 480))
    #
    #     height, width, channels = src.shape[:3]
    #     gray = cv2.cvtColor(src, cv2.COLOR_BGR2GRAY)
    #     depth = np.full((height, width), 3500.0) - gray*np.full((height, width), 3000/255)
    #     # depth_img = depth * 255/3500
    #     # depth_img = np.uint8(depth_img)
    #
    #     # fgmask = fgbg.apply(gray)
    #     # fgmask = cv2.morphologyEx(fgmask, cv2.MORPH_OPEN, kernel)
    #
    #     # ラベリング処理
    #     nlabels, labelimg, contours, CoGs = cv2.connectedComponentsWithStats(gray, 8, cv2.CV_16U)
    #
    #     window_name = "window"
    #     cv2.namedWindow(window_name, cv2.WINDOW_AUTOSIZE)
    #
    #     # X,Y,AREA
    #     track_data = []
    #     if nlabels > 0:
    #         for nlabel in range(1, nlabels):
    #             x, y, w, h, size = contours[nlabel]
    #             xg, yg = CoGs[nlabel]
    #
    #             # 面積フィルタ
    #             if size >= 1500 and h < 300:
    #                 # centroid.append([xg, yg, size])
    #                 track_data.append([xg, yg, depth[y, x], 1])
    #                 cv2.rectangle(src, (x, y), (x + w, y + h), (0, 255, 0), 2)
    #
    #     paint = np.tile(np.uint8([0, 0, 0]), (320, 640, 1))
    #
    #     history_track_data.append(track_data)
    #     if len(history_track_data) >= 20:
    #         history_track_data.pop(0)
    #
    #     for h_t in history_track_data:
    #         for track in h_t:
    #             track = np.array(track).dot(M)
    #             cv2.circle(paint, (int(track[0]), int(track[1])), 5, (0, 255, 0))
    #
    #     cv2.imshow(window_name, src)
    #     cv2.imshow('1', paint)
    #     # cv2.imshow('1', cv2.resize(gray, (640, 480)))
    #     if cv2.waitKey(1) & 0xFF == ord('q'):
    #         break
    #
    # cap.release()
    # cv2.destroyAllWindows()
