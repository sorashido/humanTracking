import cv2

def draw_detections(img, rects, thickness = 1):
    for x, y, w, h in rects:
        # the HOG detector returns slightly larger rectangles than the real objects.
        # so we slightly shrink the rectangles to get a nicer output.
        pad_w, pad_h = int(0.15*w), int(0.05*h)
        cv2.rectangle(img, (x+pad_w, y+pad_h), (x+w-pad_w, y+h-pad_h), (0, 255, 0), thickness)

if __name__ == '__main__':
    cap = cv2.VideoCapture('./data/Real02.mp4')

    hog = cv2.HOGDescriptor()
    hog.setSVMDetector( cv2.HOGDescriptor_getDefaultPeopleDetector() )

    while(cap.isOpened()):
        ret, frame = cap.read()
        src = cv2.resize(frame[26:719, 54:989], (640, 480))
        height, width, channels = src.shape[:3]
        dst = src

        gray = cv2.cvtColor(src, cv2.COLOR_BGR2GRAY)
        found, w = hog.detectMultiScale(gray, winStride=(8,8), padding=(32,32), scale=1.05)
        draw_detections(dst, found)

        cv2.imshow('1', cv2.resize(dst, (640, 480)))
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()