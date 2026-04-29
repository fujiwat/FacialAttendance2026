# FacialAttendance2026
<img width="1852" height="1082" alt="image" src="https://github.com/user-attachments/assets/d318e16e-7f6f-4daf-be4f-ef732c29eeb4" />

## Progress:  Apr 28.
- OK:  Face Detection by HaarCascard
- OK:  Face Detection by YuNet
- OK:  Find center face
- OK:  Adaptive Screen Light
- OK:  Multiple Camera/Hardware.  Laptop PC, Desktop PC.
- OK:  Window Controls
- OK:  Ring Buffer
- OK:  Feature Extraction
- 50%  Person Identification (= Face Identification)
- == Not Yet ==
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
| Camera Settings | Value | Remarks |
| :--- | :--- | :--- |
| &ensp; Size (Width x Height) | 640 x 360 |  |
| &ensp; Color/Gray | Color |  |
| &ensp; Mirror | Yes |  |
| **Programming Environment** | **----------------** |  |
| &ensp; IDE | Visual Studio 2022 Community Edition |  |
| &ensp; Project Template | MFC App |  |
| &ensp; Programming Language | C++ |  |
| &ensp; OpenCV Version | 4.10 |  |
| **Facial Attendance Technologies** | |  |
| &ensp; Facial Detection | Haar Cascade / YuNet |  |
| &ensp; Facial Identification | SFace |  |
| **[Detection] HaarCascade Settings** | |  |
| &ensp; Model | haarcascade_frontalface_default.xml |  |
| &ensp; HaarCascadeScaleFactor |  1.1 |  |
| &ensp; HaarCascadeMinNeighbors | 8 |  |
| &ensp; Detect_HaarDetectionType | cv::CASCADE_DO_CANNY_PRUNING |  |
| &ensp; HaarCascadeMinFaceWidth | 110 |  |
| &ensp; HaarCascadeMinFaceHeight | 110 |  |
| &ensp; HaarCascadeMaxFaceWidth | 0 |  |
| &ensp; HaarCascadeMaxFaceHeight | 0 |  |
| &ensp; Learning_eigenDistanceThreshold | 3220 (not used)|  |
| **[Detection] YuNet Settings** | |  |
| &ensp; Model | face_detection_yunet_2023mar.onnx |  |
| &ensp; score_threshold | 0.9 |  |
| &ensp; nms_threshold | 0.3 (default) |  |
| &ensp; top_k | 5000 (default) |  |
| &ensp; backend_id | DNN_BACKEND_DEFAULT (default) |  |
| &ensp; target_id | DNN_TARGET_CPU (default) |  |
| **[Identification] SFace Settings** | |  |
| &ensp; Model | face_recognition_sface_2021dec.onnx |  |
| &ensp; FACE_NORM_SIZE |112 x 112 |  |
| &ensp; Mathing method | cv::FaceRecognizerSF::DisType::FR_COSINE |  |
| &ensp; Threshold | 0.70 |  |
| **Ring Buffer Settings** | |  |
| &ensp; Number of Buffers | 300 |  |
| &ensp; Buffer Time out | 3000 ms |  |
| &ensp; FACE_TARGET_JUMP_RATIO | 60% |  |
| **Weights Finding Best Shot** | |  |
| &ensp; FACE_WEIGHT_CONFIDENCE | 0.6 |  |
| &ensp; FACE_WEIGHT_SHARPNESS | 0.2 |  |
| &ensp; FACE_WEIGHT_CONTRAST | 0.2 |  |

## Issue
1. Font Size on MFC
2. There is a case that the score of closing eyes was better than opening eyes.
<img width="1185" height="693" alt="image" src="https://github.com/user-attachments/assets/663d3405-ecae-4a59-9d90-63f0d843c231" />

## Log
- 04.28.  Bug fix for Face Identification.
- 04.27.  Feature extraction and Identification (SFace only)
- 04.26.  Ring Buffer showing best face and worst face with score.
- 04.25.  Change the view full color -> gray scale to enphasize the bounding box, guide frame.
- 04.24.  Adaptive Screen Light, Slider completed. Changed the HaarCascade parameter (ScaleFaceor1.2->1.1, MinNeibors 10->8). 
- 04.23.  Implimenting Adaptive Screen Light, not completed.
- 04.22.  New version - remake the project with the new template "C++ MFC App"
- 04.19.  Mirror Mode, it is better than the normal mode.

