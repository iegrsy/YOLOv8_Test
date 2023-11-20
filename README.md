# Object Detection and Tracking with YOLO

This Python script utilizes the YOLO (You Only Look Once) object detection algorithm to detect and track objects in a video feed. It offers options for real-time preview, object tracking, and exporting detected objects.

## Features

- **Object Detection**: Detects various objects in frames using YOLO models.
- **Object Tracking**: Enables object tracking for detected objects across frames.
- **Export**: Option to export cropped images of detected objects.
- **Real-time Preview**: Display real-time frame previews while processing.

## Requirements

- Python 3.x
- OpenCV (`cv2`)
- Ultralytics
- Windows OS (for `msvcrt` usage)

## Installation

1. Clone this repository:

    ```bash
    git clone https://github.com/iegrsy/YOLOv8_Test.git
    ```

2. Install required packages:

    ```bash
    pip install -r requirements.txt
    ```

## Usage

When running the application, you can use the following parameters:

- `-s, --video_source`: File name or path of the video source. Default value: `aoe.mp4`.
- `-e, --export`: Option to export the processed video (True/False). Default value: `False`.
- `-p, --preview`: Option to enable preview mode (True/False). Default value: `False`.
- `--skip_frame_count`: Number of frames to skip. Default value: `1`.

## Example Usage

An example command to run the application:

```bash
python object_detector.py -s video.mp4 -p True -e True --skip_frame_count 2
```

This command runs the script on the 'aoe.mp4' video, enabling both object export and real-time preview.

## Example training

Create dataset folder

```bash
cd build
mkdir datasets
cp -r person datasets/train
cp -r person datasets/test
cp -r person datasets/valid
```

Run training command

```bash
yolo task=classify mode=train data=datasets model=yolov8m-cls.pt epochs=2
```

## License

This project is licensed under the [MIT License](LICENSE).
