#pragma once

#pragma region Application_Constants
#	define APP_NAME_SHORT "FacialAttendance2026"
#	define wAPP_NAME_SHORT L"FacialAttendance2026"
#	define APP_NAME_LONG "FacialAttendance2026 - YuNet/HaarCascade Face Detection"
#	define wAPP_NAME_LONG L"FacialAttendance2026 - YuNet/HaarCascade Face Detection"
#pragma endregion

#pragma region Camera_Constants
#	define CaptureWidth 640
#	define CaptureHeight 480
#pragma endregion

#pragma region HaarCascade_Constants
#	define HaarCascadeXml "haarcascade_frontalface_default.xml"
#   define HaarCascadeScaleFactor 1.2
#   define HaarCascadeMinNeighbors 10
#   define HaarCascadeMinFaceWidth 110
#   define HaarCascadeMinFaceHeight 110
#   define HaarCascadeMaxFaceWidth 0
#   define HaarCascadeMaxFaceHeight 0
#   define HaarCascadeFlags cv::CASCADE_DO_CANNY_PRUNING
#   define HaarCascadeMinSize cv::Size(HaarCascadeMinFaceWidth, HaarCascadeMinFaceHeight)
#   define HaarCascadeMaxSize cv::Size(HaarCascadeMaxFaceWidth, HaarCascadeMaxFaceHeight)
#pragma endregion

#pragma region YuNet_Constants
#	define YuNetModelPath "face_detection_yunet_2023mar.onnx"
#pragma endregion

