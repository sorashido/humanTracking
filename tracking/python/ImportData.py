import cv2
import numpy as np

# range: 500mm ~ 3500mm
if __name__ == '__main__':
    # cap = cv2.VideoCapture('./data/Real02.mp4')
    win_name = '1'
    cv2.namedWindow(win_name)

    fs = cv2.FileStorage('../data/test.xml', cv2.FILE_STORAGE_READ)

    for num in range(0, 1000):
        fn = fs.getNode("d"+str(num))
        depth = fn.mat()

        # print(depth)
        depth_img = cv2.resize(np.uint8(depth), (640, 480))

        # ラベリング処理
        nlabels, labelimg, contours, CoGs = cv2.connectedComponentsWithStats(depth_img, 8, cv2.CV_16U)

        # X,Y,AREA
        track_data = []
        if nlabels > 0:
            for nlabel in range(1, nlabels):
                x, y, w, h, size = contours[nlabel]
                xg, yg = CoGs[nlabel]

                # 面積フィルタ
                if size >= 1500 and h < 300:
                    cv2.rectangle(depth_img, (x, y), (x + w, y + h), (255, 255, 255), 2)

        cv2.imshow(win_name, depth_img)
        if cv2.waitKey(30) & 0xFF == ord('q'):
            break
