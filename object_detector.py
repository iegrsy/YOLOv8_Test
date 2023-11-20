import argparse
import threading
import cv2
import time
import uuid
import os
from ultralytics import YOLO, checks

# Check for Ultralytics installation and configurations
checks()


def current_milli_time():
    return round(time.time() * 1000)


class DetectedObject:
    def __init__(self, track_id, object_id, object_name, class_id, class_name, confidence, point, pointn):
        self.track_id = track_id
        self.object_id = object_id
        self.object_name = object_name
        self.class_id = class_id
        self.class_name = class_name
        self.confidence = confidence
        self.point = point
        self.pointn = pointn

    def __str__(self) -> str:
        return f"[{self.object_id}, {self.object_name}] [{self.class_id}, {self.class_name}] c: {self.confidence}, p: {self.point}"


class ObjectDetector:
    """
    ObjectDetector class for detecting and processing objects in frames.

    Args:
    - detection_confidence: Confidence threshold for object detection.
    - classification_confidence: Confidence threshold for object classification.
    - verbose: Flag for verbose output.
    - show_preview: Flag to display real-time frame previews.
    - with_track: Flag for object tracking.
    - export: Flag to enable export of cropped images.
    - split_for_classes: Flag to split exported images into folders based on classes.
    """
    def max_value_key(dictionary):
        return max(dictionary, key=lambda k: dictionary[k])

    def __init__(self, detection_confidence=0.4, classification_confidence=0.7,
                 verbose=False, show_preview=False, with_track=False, export=False,
                 split_for_classes=True):
        """
        Initializes the ObjectDetector class with default parameters and models.
        """
        self.detection_confidence = detection_confidence
        self.classification_confidence = classification_confidence
        self.verbose = verbose
        self.show_preview = show_preview
        self.with_track = with_track
        self.export = export
        self.split_for_classes = split_for_classes
        self.model_yolo = YOLO('yolov8m.pt')
        self.model_cls_yolo = YOLO('vest-cls.pt')
        self.classes_filter = ["person"]
        self.colors = {0: (0, 0, 255), 1: (0, 255, 0)}
        self.selected_classes = [key for key, value in self.model_yolo.names.items(
        ) if value in self.classes_filter]

    def detect_objects(self, frame):
        """
        Detects objects in a frame and performs classification and tracking if enabled.

        Args:
        - frame: Input frame for object detection.

        Returns:
        - List of DetectedObject instances representing the detected objects.
        """
        if self.with_track:
            detections = self.model_yolo.track(
                frame, persist=True, conf=self.detection_confidence, verbose=self.verbose, classes=self.selected_classes)
        else:
            detections = self.model_yolo.predict(
                frame, conf=self.detection_confidence, verbose=self.verbose, classes=self.selected_classes)

        detect_objects = []
        for detection in detections[0].boxes:
            x1, y1, x2, y2 = detection.xyxy.tolist()[0]
            cropped_image = frame[int(y1):int(y2), int(x1):int(x2)]
            obj_cls = self.model_cls_yolo.predict(
                cropped_image, conf=self.classification_confidence, verbose=self.verbose)
            if detection.id is not None:
                track_id = detection.id[0].item()
            else:
                track_id = uuid.uuid4()
            class_id = obj_cls[0].probs.top1
            class_name = obj_cls[0].names[class_id]
            obj = DetectedObject(track_id, detection.cls.item(), self.model_yolo.names[detection.cls.item()],
                                 class_id, class_name, detection.conf.item(),
                                 detection.xyxy.tolist()[0], detection.xyxyn.tolist()[0])
            detect_objects.append(obj)
            for obj in detect_objects:
                if self.show_preview:
                    self.draw_box(frame, obj)
                    cv2.imshow('Frame', frame)
                    cv2.waitKey(1)
                if self.export:
                    self.export_cropped_image(frame, obj, cropped_image)

        return detect_objects

    def draw_box(self, frame, detected_object):
        """
        Draws a bounding box around the detected object on the frame.

        Args:
        - frame: Input frame to draw the bounding box.
        - detected_object: DetectedObject instance representing the object to be drawn.
        """
        x1, y1, x2, y2 = detected_object.point
        color = self.colors[detected_object.class_id]
        cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), color, 2)
        cv2.putText(frame, f"[{detected_object.track_id}] {detected_object.class_name}", (int(x1), int(y1) - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)

    def export_cropped_image(self, frame, detected_object, cropped_image):
        """
        Exports the cropped image of the detected object to a specified folder.

        Args:
        - frame: Original frame containing the detected object.
        - detected_object: DetectedObject instance representing the object to be exported.
        - cropped_image: Cropped image of the detected object.
        """
        folder = os.path.join(f"build", f"{detected_object.object_name}")
        if self.split_for_classes:
            folder = os.path.join(f"{folder}", f"{detected_object.class_name}")
        if not os.path.exists(folder):
            os.makedirs(folder)
        file_name = os.path.join(
            folder, f"{current_milli_time()}_{uuid.uuid4()}.jpg")
        cv2.imwrite(file_name, cropped_image)
        if self.verbose:
            print(f"export file: {file_name}")


class VideoSource:
    def __init__(self, video_source, skip_frame_count=1):
        """
        Initializes the VideoSource class.

        Args:
        - video_source: The path or index of the video source (file or camera index).
        - skip_frame_count: The number of frames to skip between processed frames.
        """
        self.video_source = video_source
        self.skip_frame_count = skip_frame_count
        self.video_capture = cv2.VideoCapture(self.video_source)
        self.frame_width = int(
            self.video_capture.get(cv2.CAP_PROP_FRAME_WIDTH))
        self.frame_height = int(
            self.video_capture.get(cv2.CAP_PROP_FRAME_HEIGHT))
        print(
            f"Test video name: '{self.video_source}', resolution: [{self.frame_width} x {self.frame_height}]")
        self.player_thread = threading.Thread(target=self._get_frame)

    def start(self, on_frame_callback):
        """
        Starts the video source, enabling the retrieval of frames.

        Args:
        - on_frame_callback: A function to handle each retrieved frame.
        """
        self._is_running = True
        self.on_frame_callback = on_frame_callback
        self.player_thread.start()

    def stop(self):
        """
        Stops the video source and releases the video capture resources.
        """
        if not self._is_running:
            return
        self._is_running = False
        self.video_capture.release()

    def is_running(self):
        return self._is_running

    def _get_frame(self):
        """
        Internal method to retrieve frames from the video source.
        """
        frame_index = 0
        while self.video_capture.isOpened():
            ret, frame = self.video_capture.read()
            if not ret:
                break
            frame_index += 1
            if frame_index % self.skip_frame_count != 0:
                continue
            if not self._is_running:
                break

            if self.on_frame_callback is not None:
                self.on_frame_callback(frame)

        self._is_running = False


if __name__ == "__main__":
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description='Video processing application')
    parser.add_argument('-s', '--video_source', default='aoe.mp4', help='File name or path of the video source')
    parser.add_argument('-e', '--export', default=False, help='Option to export the processed video (True/False)')
    parser.add_argument('-p', '--preview', default=False, help='Option to enable preview mode (True/False)')
    parser.add_argument('--skip_frame_count', default=1, help='Number of frames to skip')
    args = parser.parse_args()

    try:
        # Initialize ObjectDetector instance
        object_detector = ObjectDetector(
            with_track=True, show_preview=args.preview, export=args.export)

        # Define the frame processing callback function
        def on_frame(frame):
            object_detector.detect_objects(frame)

        # Initialize VideoSource instance with specified video source and skip frame count
        video_source = VideoSource(args.video_source, int(args.skip_frame_count))
        video_source.start(on_frame)

        # Wait for cancel to exit the loop
        try:
            while video_source.is_running():
                time.sleep(1)
        except KeyboardInterrupt:
            pass
    except Exception as e:
        print(f"Error: {e}")
    finally:
        # Stop the video source capture
        video_source.stop()
