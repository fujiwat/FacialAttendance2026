# FacialAttendance2026
<img width="1004" height="800" alt="image" src="https://github.com/user-attachments/assets/452de1e7-6f53-4fb9-b025-a279f9e1d43d" />
Progress:  Apr 22.<br />
OK:  Face Detection by HaarCascard<br />
OK:  Face Detection by YuNet<br />
OK:  Find center face<br />
<br />
== Not Yet ==<br />
Window Controls<br />
Ring Buffer<br />
Feature Extraction<br />
Person Identification (= Face Recognition)<br />
Save Function<br />
<br />

## New Topics

- It is showing two types of bounding boxes.  Showing HaarCascade and YuNet to compare two tech difference.
  - HaarCascade: Pink, dark pink.
  - YuNet:  Green, dark green.
  - Dark colored bounding boxes are shown when there are two faces or more.  The light colored face is the most centered face in the photo.

## Settings
| Library Setting | Value |
| :--- | :--- |
| OpenCV Version | 4.10 |

| Camera Settings | Value |
| :--- | :--- |
| Size (Width x Height) | 640 x 480 |
| Color/Gray | Color |
| Mirror | Yes |

| HaarCascade Settings | Value |
| :--- | :--- |
| Model | haarcascade_frontalface_default.xml |
| HaarCascadeScaleFactor |  1.2 |
| HaarCascadeMinNeighbors | 10 |
| Detect_HaarDetectionType | cv::CASCADE_DO_CANNY_PRUNING |
| HaarCascadeMinFaceWidth | 110 |
| HaarCascadeMinFaceHeight | 110 |
| HaarCascadeMaxFaceWidth | 0 |
| HaarCascadeMaxFaceHeight | 0 |
| Learning_eigenDistanceThreshold | 3220 (not used)|

| YuNet Settings | Value |
| :--- | :--- |
| Model | face_detection_yunet_2023mar.onnx |
| score_threshold | 0.9 |
| nms_threshold | 0.3 (default) |
| top_k | 5000 (default) |
| backend_id | DNN_BACKEND_DEFAULT (default) |
| target_id | DNN_TARGET_CPU (default) |

## Log
04.19.  Mirror Mode, it is better than the normal mode.
04.22.  New version - remake the project with the new template "C++ MFC App"
