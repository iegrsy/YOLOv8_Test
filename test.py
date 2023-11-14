import cv2
import time
from ultralytics import YOLO, checks


def current_milli_time():
    return round(time.time() * 1000)


class Opts:
    def __init__(self, conf, verbose, showpreview, withtrack):
        self.conf = conf
        self.verbose = verbose
        self.showpreview = showpreview
        self.withtrack = withtrack


class ObjModel:
    def __init__(self, oid, name, conf, point):
        self.oid = oid
        self.name = name
        self.conf = conf
        self.point = point

    def __str__(self) -> str:
        return f"[{self.oid}, {self.name}] c: {self.conf}, p: {self.point}"


# Print check
checks()

model = YOLO('yolov8s.pt')
print(f"{model.info()}")
# model.to("cpu")

classes_filter = ["person"]
__classes = [key for key, value in model.names.items()
             if value in classes_filter]

opts = Opts(0.2, False, True, False)
sources = {
    1: "rtsp://192.168.1.128:8554/cam",
    2: "video5000_640.h264",
    3: "video5000.h264",
    4: "test4.h264",
    5: "tokyo.mp4",
    6: "trucks.mp4",
}

source = sources.get(3)
cap = cv2.VideoCapture(source)

frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
print(
    f"Test video name: '{source}', resolution: [{frame_width} x {frame_height}]")

start_time = current_milli_time()
frame_count = 1

try:
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break

        frame_count = frame_count + 1

        if opts.withtrack:
            detections = model.track(
                frame, persist=True, conf=opts.conf, verbose=opts.verbose, classes=__classes)
        else:
            detections = model(frame, conf=opts.conf,
                               verbose=opts.verbose, classes=__classes)
            if opts.verbose:
                for detection in detections[0].boxes:
                    o = ObjModel(detection.cls.item(), model.names[detection.cls.item()],
                                 detection.conf.item(), detection.xyxyn.tolist()[0])
                    print(f"obj: {o}")

        if opts.showpreview:
            frame = detections[0].plot()
            cv2.imshow('Frame', frame)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

        # if frame_count > 2500:
        #     break

except KeyboardInterrupt:
    pass

end_time = current_milli_time()
ellapse_time = end_time-start_time
cycle_avg_time = ellapse_time/frame_count
process_fps = 1000.0/cycle_avg_time
print(f"[{start_time}, {end_time}] Processed frame count: {frame_count}, Ellapse time: {ellapse_time} ms, AVG process cycle time: {cycle_avg_time} ms, Process FPS: {process_fps}")

cap.release()
cv2.destroyAllWindows()
