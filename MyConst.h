#pragma once

#pragma region Application_Constants
#	define APP_NAME_SHORT "FacialAttendance2026"                                    /**< Short ANSI name of the application. */
#	define wAPP_NAME_SHORT L"FacialAttendance2026"                                  /**< Short wide-char name of the application. */
#	define APP_NAME_LONG "FacialAttendance2026 - YuNet/HaarCascade Face Detection"  /**< Descriptive ANSI long name of the application. */
#	define wAPP_NAME_LONG L"FacialAttendance2026 - YuNet/HaarCascade Face Detection"/**< Descriptive wide-char long name of the application. */
#pragma endregion

#pragma region Camera_Constants
#	define CaptureWidth 640  /**< Default width targeted during video frame capture. */
#	define CaptureHeight 360 /**< Default height targeted during video frame capture. */
#pragma endregion

#pragma region HaarCascade_Constants
#	define HaarCascadeXml "haarcascade_frontalface_default.xml" /**< The file path referencing the utilized Haar Cascade classifier XML. */
#   define HaarCascadeScaleFactor 1.1                           /**< Factor expressing how much the image size is reduced at each image scale. */
#   define HaarCascadeMinNeighbors 8                            /**< Specifies how many neighbors each candidate rectangle should have to retain it. */
#   define HaarCascadeMinFaceWidth 110                          /**< Minimal bounded width allocated during cascade evaluations natively. */
#   define HaarCascadeMinFaceHeight 110                         /**< Minimal bounded height allocated during cascade evaluations natively. */
#   define HaarCascadeMaxFaceWidth 0                            /**< Optional bounded max width limits (zero means unbound). */
#   define HaarCascadeMaxFaceHeight 0                           /**< Optional bounded max height limits (zero means unbound). */
#   define HaarCascadeFlags cv::CASCADE_DO_CANNY_PRUNING        /**< Optimization flag trimming non-viable regions via Canny approximations. */
#   define HaarCascadeMinSize cv::Size(HaarCascadeMinFaceWidth, HaarCascadeMinFaceHeight) /**< OpenCV structured format reflecting minimal scanning metrics. */
#   define HaarCascadeMaxSize cv::Size(HaarCascadeMaxFaceWidth, HaarCascadeMaxFaceHeight) /**< OpenCV structured format reflecting maximum scanning limits. */
#pragma endregion

#pragma region YuNet_Constants
#	define YuNetModelPath "face_detection_yunet_2023mar.onnx" /**< Sourced pre-trained definition for accurate ONNX inference mapping correctly. */
#pragma endregion

#pragma region UI_Constants
#	define UI_ID_TEXT_MAX_LENGTH 8          /**< Maximum allowed character length representing unique numerical identifiers. */
#	define UI_COMMENT_TEXT_MAX_LENGTH 200   /**< Maximum character input limits applied inside user observation logging. */
#   define UI_LIST_FONT_SIZE 16             /**< Structural font sizing applied rendering sequential grids. */
#	define UI_FIELD_HEIGHT_SMALL 20         /**< Adjusted height configurations limiting distinct visual structural elements. */
#	define UI_FIELD_HEIGHT 28               /**< Master sizing limit resolving main visual typefaces. */
#   define UI_FONT_NAME L"Segoe UI"         /**< Central Windows GUI standard font. */
#   define UI_FONT_NAME_FIXED L"Tahoma"     /**< Legacy styled metric font for UI fields. */
#pragma endregion


#pragma region FACE_RECOGNITION
const int MAX_FACE_RING_BUFFER = 300;	                  /**< The maximum amount of retained tracking arrays maintained simultaneously (roughly 10s at 30 fps). */
const int FACE_RING_BUFFER_TIMEOUT_MS = 3000;             /**< Absolute logical lifespan (ms) restricting persistence securely preventing invalid memory bloats. */
const double FACE_TARGET_JUMP_RATIO = 0.6;                /**< Frame transition coordinate ratio defining when a jump resets tracking. */
const int FACE_NORM_SIZE = 112;			                  /**< Uniform target structural array size normalized properly. */

const double FACE_WEIGHT_CONFIDENCE = 0.6;               /**< Evaluated weight for AI confidence levels. */
const double FACE_WEIGHT_SHARPNESS = 0.2;                /**< Evaluated weight for image sharpness levels. */
const double FACE_WEIGHT_CONTRAST = 0.2;                 /**< Evaluated weight for image contrast levels. */
#pragma endregion

#pragma region Evaluation_Constants
#	define wEVALUATION_FOLDER_NAME L"Evaluation"                             /**< Primary diagnostic root directory strictly. */
#	define wFACE_DETECTION_LATENCY_FOLDER_NAME L"FaceDetection"              /**< Subfolder for face detection telemetry. */
#	define wFACE_IDENTIFICATION_LATENCY_FOLDER_NAME L"FaceIdentification"    /**< Subfolder for face identification telemetry. */
#   define wFACE_DETECTION_METHOD_HAARCASCADES L"HaarCascades"               /**< Literal constant for Haar Cascades evaluation output. */
#   define wFACE_DETECTION_METHOD_YUNET L"YuNet"                             /**< Literal constant for YuNet evaluation output. */
#   define wFACE_DETECTION_METHOD_BOTH L"Both"                               /**< Literal constant for dual-mode evaluation output. */
#   define wFACE_IDENTIFICATION_METHOD_Eigenfaces L"Eigenfaces"              /**< Literal constant for Eigenfaces evaluation output. */
#   define wFACE_IDENTIFICATION_METHOD_LBPH L"LBPH"                          /**< Literal constant for LBPH evaluation output. */
#   define wFACE_IDENTIFICATION_METHOD_SFace L"SFace"                        /**< Literal constant for SFace evaluation output. */
#pragma endregion

#pragma region Registry_Constants
#	define REG_SECTION_SETTINGS _T("Settings")                               /**< Application setting section registry key. */
#	define REG_KEY_FACE_DETECTION_MODE _T("FaceDetectionMode")               /**< Face detection setting registry key. */
#	define REG_KEY_FACE_IDENTIFICATION_MODE _T("FaceIdentificationMode")     /**< Face identification setting registry key. */
#   define REG_KEY_OPTION_REQUIREDS_ID _T("OptionRequiresID")                /**< Require ID setting registry key. */
#   define REG_KEY_OPTION_MIRROR_VIEW _T("OptionMirrorView")                 /**< Camera mirror view setting registry key. */
#   define DEF_KEY_OPTION_TRUE 1                                             /**< Default logical true value for registry settings. */
#pragma endregion
