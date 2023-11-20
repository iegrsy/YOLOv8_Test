import cv2
import time
import uuid
import os
from ultralytics import YOLO, checks


def current_milli_time():
    return round(time.time() * 1000)


class Opts:
    def __init__(self, conf=0.2, verbose=False, showpreview=False, withtrack=False, export=False):
        self.conf = conf
        self.verbose = verbose
        self.showpreview = showpreview
        self.withtrack = withtrack
        self.export = export


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

model = YOLO('yolov8m.pt')
model_cls = YOLO('yolov8m-cls.pt')
print(f"{model.info()}")
# model.to("cpu")

classes_filter = ["person"]
__classes = [key for key, value in model.names.items()
             if value in classes_filter]

opts = Opts(verbose=False, export=True)
sources = {
    1: "rtsp://192.168.1.128:8554/cam",
    2: "video5000_640.h264",
    3: "video5000.h264",
    4: "test4.h264",
    5: "tokyo.mp4",
    6: "trucks.mp4",
    7: "aoe.mp4",
}

source = sources.get(7)
cap = cv2.VideoCapture(source)

frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
print(
    f"Test video name: '{source}', resolution: [{frame_width} x {frame_height}]")

start_process_time = current_milli_time()
process_total = 0
frame_count = 1

try:
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break

        start_time = current_milli_time()
        frame_count = frame_count + 1

        if opts.withtrack:
            detections = model.track(
                frame, persist=True, conf=opts.conf, verbose=opts.verbose, classes=__classes)
        else:
            detections = model.predict(frame, conf=opts.conf,
                                       verbose=opts.verbose, classes=__classes)
            for detection in detections[0].boxes:
                o = ObjModel(detection.cls.item(), model.names[detection.cls.item()],
                             detection.conf.item(), detection.xyxyn.tolist()[0])

                if opts.verbose:
                    print(f"obj: {o}")

                if opts.export:
                    x1, y1, x2, y2 = detection.xyxy.tolist()[0]
                    cropped_image = frame[int(y1):int(y2), int(x1):int(x2)]

                    obj_cls = model_cls.predict(
                        cropped_image, conf=opts.conf, verbose=opts.verbose, classes=__classes)
                    clsid = obj_cls[0].probs.top1
                    box_label = obj_cls[0].names[clsid]
                    print(f"[{clsid}] {box_label}")

                    if opts.showpreview:
                        color = (0, 255, 0)
                        if clsid == 0:
                            color = (0, 0, 255)

                        cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), color, 2)
                        cv2.putText(frame, box_label, (int(x1), int(y1) - 10),
                                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)

                    ffolder = os.path.join(f"build", f"{o.name}")
                    if not os.path.exists(ffolder):
                        os.makedirs(ffolder)
                    fname = os.path.join(ffolder, f"{uuid.uuid4()}_{current_milli_time()}.jpg")
                    cv2.imwrite(fname, cropped_image)
                    if opts.verbose:
                        print(f"export file: {fname}")

        end_time = current_milli_time()
        ellapse_time = end_time-start_time
        process_total = process_total + ellapse_time

        if opts.showpreview:
            # frame = detections[0].plot()
            cv2.imshow('Frame', frame)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

        # if frame_count > 5:
        #     break
except Exception as e:
    print(f"Error: {e}")
except KeyboardInterrupt:
    pass

end_process_time = current_milli_time()
cycle_avg_time = process_total/frame_count
process_fps = 1000.0/cycle_avg_time
print(f"[{start_process_time}, {end_process_time}] Processed frame count: {frame_count}, Ellapse time: {end_process_time-start_process_time} ms, AVG process cycle time: {cycle_avg_time} ms, Process FPS: {process_fps}")

cap.release()
cv2.destroyAllWindows()
