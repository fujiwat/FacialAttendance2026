#pragma once

#pragma region Application_Constants
#	define APP_NAME_SHORT "FacialAttendance2026"
#	define wAPP_NAME_SHORT L"FacialAttendance2026"
#	define APP_NAME_LONG "FacialAttendance2026 - YuNet/HaarCascade Face Detection"
#	define wAPP_NAME_LONG L"FacialAttendance2026 - YuNet/HaarCascade Face Detection"
#pragma endregion

#pragma region Camera_Constants
#	define CaptureWidth 640
#	define CaptureHeight 360
#pragma endregion

#pragma region HaarCascade_Constants
#	define HaarCascadeXml "haarcascade_frontalface_default.xml"
#   define HaarCascadeScaleFactor 1.1
#   define HaarCascadeMinNeighbors 8
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

#pragma region UI_Constants
#	define UI_ID_TEXT_MAX_LENGTH 8
#	define UI_COMMENT_TEXT_MAX_LENGTH 200
#	define UI_FIELD_HEIGHT 32
#pragma endregion


#pragma region FACE_RECOGNITION
	const int MAX_FACE_RING_BUFFER = 300;	// 顔写真のリングバッファの最大保存数 (1フレーム1枚として約10秒分)
	const int FACE_NORM_SIZE = 112;			// 顔の正規化サイズ (SFace, Eigenfaces, LBPH 共通)
	// 顔評価用スコアの重み (合計 1.0 になるように設定)
	const double FACE_WEIGHT_CONFIDENCE = 0.6; // 確度（正面、隠れなし）
	const double FACE_WEIGHT_SHARPNESS = 0.2; // シャープネス（ブレ）
	const double FACE_WEIGHT_CONTRAST = 0.2; // コントラスト（白飛び/黒つぶれ）

#pragma endregion

#pragma region Evaluation_Constants
#	define wEVALUATION_FOLDER_NAME L"Evaluation"
#	define wFACE_DETECTION_LATENCY_FOLDER_NAME L"FaceDetectionLatency"
#pragma endregion

#pragma region Registry_Constants
#	define REG_SECTION_SETTINGS _T("Settings")
#	define REG_KEY_FACE_DETECTION_MODE _T("FaceDetectionMode")
#pragma endregion

