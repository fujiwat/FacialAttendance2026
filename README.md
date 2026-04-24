# FacialAttendance2026
<img width="886" height="503" alt="image" src="https://github.com/user-attachments/assets/d1f14509-7529-4c7a-a1b3-e1c7ea3b434f" />

Progress:  Apr 24.
- OK:  Face Detection by HaarCascard
- OK:  Face Detection by YuNet
- OK:  Find center face
- OK:  Adaptive Screen Light
- OK:  Multiple Camera/Hardware.  Laptop PC, Desktop PC.
- == Not Yet ==
- Window Controls
- Ring Buffer
- Feature Extraction
- Person Identification (= Face Recognition)
- Save Function

## New Topics
- Multiple Camera support, Camera is tested and code modified for laptop PC, desktop PC.
- The project is converted to "C++ MFC App" to make easier the window controls.  It took 2-3 days...
- It is showing two types of bounding boxes.  Showing HaarCascade and YuNet to compare two tech difference.
  - HaarCascade: Pink, dark pink.
  - YuNet:  Green, dark green.
  - Dark colored bounding boxes are shown when there are two faces or more.  The light colored face is the most centered face in the photo.

## Settings
| [Camera Settings] | [Value] |
| :--- | :--- |
| &ensp; Size (Width x Height) | 640 x 360 |
| &ensp; Color/Gray | Color |
| &ensp; Mirror | Yes |
| **[Programming Environment]** | **[Value]** |
| IDE | Visual Studio 2022 Community Edition |
| Project Template | MFC App |
| Programming Language | C++ |
| OpenCV Version | 4.10 |
| **[Facial Attendance Technologies]** | **[Value]** |
| Facial Detection | Haar Cascade / YuNet |
| Facial Recognition | SFace |
| **[HaarCascade Settings]** | **[Value]** |
| Model | haarcascade_frontalface_default.xml |
| HaarCascadeScaleFactor |  1.1 |
| HaarCascadeMinNeighbors | 8 |
| Detect_HaarDetectionType | cv::CASCADE_DO_CANNY_PRUNING |
| HaarCascadeMinFaceWidth | 110 |
| HaarCascadeMinFaceHeight | 110 |
| HaarCascadeMaxFaceWidth | 0 |
| HaarCascadeMaxFaceHeight | 0 |
| Learning_eigenDistanceThreshold | 3220 (not used)|
| **[YuNet Settings]** | **[Value]** |
| Model | face_detection_yunet_2023mar.onnx |
| score_threshold | 0.9 |
| nms_threshold | 0.3 (default) |
| top_k | 5000 (default) |
| backend_id | DNN_BACKEND_DEFAULT (default) |
| target_id | DNN_TARGET_CPU (default) |

## Log
- 04.24.  Adaptive Screen Light, Slider completed. Changed the HaarCascade parameter (ScaleFaceor1.2->1.1, MinNeibors 10->8). 
- 04.23.  Implimenting Adaptive Screen Light, not completed.
- 04.22.  New version - remake the project with the new template "C++ MFC App"
- 04.19.  Mirror Mode, it is better than the normal mode.

