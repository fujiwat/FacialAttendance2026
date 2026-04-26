# FacialAttendance2026
<img width="1852" height="1082" alt="image" src="https://github.com/user-attachments/assets/d318e16e-7f6f-4daf-be4f-ef732c29eeb4" />

## Progress:  Apr 26.
- OK:  Face Detection by HaarCascard
- OK:  Face Detection by YuNet
- OK:  Find center face
- OK:  Adaptive Screen Light
- OK:  Multiple Camera/Hardware.  Laptop PC, Desktop PC.
- OK:  Window Controls
- OK:  Ring Buffer
- == Not Yet ==
- Feature Extraction
- Person Identification (= Face Recognition)
- Save Function

## New Topics
- Yellow guide frame is added.  Caution message is added when there is no face.
- Multiple Camera support, Camera is tested and code modified for laptop PC, desktop PC.
- The project is converted to "C++ MFC App" to make easier the window controls.  It took 2-3 days...
- It is showing two types of bounding boxes.  Showing HaarCascade and YuNet to compare two tech difference.
  - HaarCascade: Pink, dark pink.
  - YuNet:  Green, dark green.
  - Dark colored bounding boxes are shown when there are two faces or more.  The light colored face is the most centered face in the photo.

## Settings
| Camera Settings | Value |
| :--- | :--- |
| &ensp; Size (Width x Height) | 640 x 360 |
| &ensp; Color/Gray | Color |
| &ensp; Mirror | Yes |
| **Programming Environment** | **Value** |
| &ensp; IDE | Visual Studio 2022 Community Edition |
| &ensp; Project Template | MFC App |
| &ensp; Programming Language | C++ |
| &ensp; OpenCV Version | 4.10 |
| **Facial Attendance Technologies** | **Value** |
| &ensp; Facial Detection | Haar Cascade / YuNet |
| &ensp; Facial Recognition | SFace |
| **HaarCascade Settings** | **Value** |
| &ensp; Model | haarcascade_frontalface_default.xml |
| &ensp; HaarCascadeScaleFactor |  1.1 |
| &ensp; HaarCascadeMinNeighbors | 8 |
| &ensp; Detect_HaarDetectionType | cv::CASCADE_DO_CANNY_PRUNING |
| &ensp; HaarCascadeMinFaceWidth | 110 |
| &ensp; HaarCascadeMinFaceHeight | 110 |
| &ensp; HaarCascadeMaxFaceWidth | 0 |
| &ensp; HaarCascadeMaxFaceHeight | 0 |
| &ensp; Learning_eigenDistanceThreshold | 3220 (not used)|
| **YuNet Settings** | **Value** |
| &ensp; Model | face_detection_yunet_2023mar.onnx |
| &ensp; score_threshold | 0.9 |
| &ensp; nms_threshold | 0.3 (default) |
| &ensp; top_k | 5000 (default) |
| &ensp; backend_id | DNN_BACKEND_DEFAULT (default) |
| &ensp; target_id | DNN_TARGET_CPU (default) |

## Issue
1. There is a acase that the score of closing eyes was better than opening eyes.
<img width="1185" height="693" alt="image" src="https://github.com/user-attachments/assets/663d3405-ecae-4a59-9d90-63f0d843c231" />

## Log
- 04.26.  Ring Buffer showing best face and worst face with score.
- 04.25.  Change the view full color -> gray scale to enphasize the bounding box, guide frame.
- 04.24.  Adaptive Screen Light, Slider completed. Changed the HaarCascade parameter (ScaleFaceor1.2->1.1, MinNeibors 10->8). 
- 04.23.  Implimenting Adaptive Screen Light, not completed.
- 04.22.  New version - remake the project with the new template "C++ MFC App"
- 04.19.  Mirror Mode, it is better than the normal mode.

