// 標準ライブラリ
#include "pch.h"
#include <string>
#include <vector>

// Windows 固有の設定（windows.h の前に置く）
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <sal.h> // _In_, _Out_ 等のアノテーション

#include "OpenCV_without_warning.h"
#include "MyFunctions.h"
#include "FaceDetector.h"
#include "MyConst.h"

// Initialize camera: outputs frameWidth, frameHeight and cap, returns success flag.
bool InitCamera(int& frameWidth, int& frameHeight, cv::VideoCapture& cap)
{
    cap.open(0);
    if (!cap.isOpened()) {
        // ★エラーメッセージを削除
        // MyMessageBoxA(NULL, MB_OK | MB_ICONERROR, APP_NAME_SHORT, "Can not find PC Camera.");
        frameWidth = 0;
        frameHeight = 0;
        return false;
    }

    // Reduce resolution to lower computation
    cap.set(cv::CAP_PROP_FRAME_WIDTH, CaptureWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, CaptureHeight);

    frameWidth = int(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    frameHeight = int(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    return true;
}

// FaceDetector implementation
FaceDetector::FaceDetector()
    : yunet_(nullptr)
    , haarCascade_()
    , windowName_(APP_NAME_LONG)
	, cap_()
	, frameWidth_(CaptureWidth)  // ★デフォルト値を設定
	, frameHeight_(CaptureHeight) // ★デフォルト値を設定
	, cameraInitialized_(false)
	, yunetInitialized_(false)
	, haarInitialized_(false)
{
    // 1) Initialize camera
    cameraInitialized_ = InitCamera(frameWidth_, frameHeight_, cap_);

    // 2) Initialize YuNet detector
    const std::string modelPath = YuNetModelPath;

    try {
        yunet_ = cv::FaceDetectorYN::create(modelPath, "", cv::Size(frameWidth_, frameHeight_),
            0.9f,                                 // score_threshold
            0.3f,                                 // nms_threshold
            5000,                                 // top_k
            cv::dnn::DNN_BACKEND_DEFAULT,         // backend_id
            cv::dnn::DNN_TARGET_CPU               // target_id
        );
        if (yunet_) {
            yunet_->setInputSize(cv::Size(frameWidth_, frameHeight_));
            yunetInitialized_ = true;
        } else {
            yunetInitialized_ = false;
        }
    }
    catch (const cv::Exception& /*e*/) {
        MyMessageBoxA(NULL, MB_OK | MB_ICONERROR, APP_NAME_SHORT, "Can not find YuNet model\n%s", modelPath.c_str());
        yunet_.release();
        yunetInitialized_ = false;
    }

    // 3) Initialize HaarCascade
    const std::string haarPath = HaarCascadeXml;
    if (haarCascade_.load(haarPath)) {
        haarInitialized_ = true;
    }
    else {
        haarInitialized_ = false;
        MyMessageBoxA(NULL, MB_OK | MB_ICONWARNING, APP_NAME_SHORT, "Haar cascade not found xml\n%s", haarPath.c_str());
    }
}

FaceDetector::~FaceDetector()
{
    if (cap_.isOpened()) {
        cap_.release();
    }
}

void FaceDetector::DetectFacesYunet(const cv::Mat& frame, cv::Mat& faces)
{
    if (yunet_ && yunetInitialized_) {
        yunet_->detect(frame, faces);
    } else {
        faces.release();
    }
}

void FaceDetector::DrawBoundingBoxesYunet(cv::Mat& frame, const cv::Mat& faces)
{
    // YuNet draw bounding boxes and confidence scores
	size_t centerFaceIndex = 0;
	int minDistance2 = std::numeric_limits<int>::max(); // large initial value
    if (!faces.empty()) {
        // find the center face
        for (int i = 0; i < faces.rows; i++) {
            int x = int(faces.at<float>(i, 0));
            int y = int(faces.at<float>(i, 1));
            int w = int(faces.at<float>(i, 2));
            int h = int(faces.at<float>(i, 3));

            int faceCenterX = x + (w / 2);
            int faceCenterY = y + (h / 2);
            int dx = faceCenterX - GetFrameWidth() / 2;
            int dy = faceCenterY - GetFrameHeight() / 2;

            if ( (dx*dx + dy*dy) < minDistance2 ) {
                centerFaceIndex = i;
                minDistance2 = dx*dx + dy*dy;
			}
        }
        for (int i = 0; i < faces.rows; i++) {
            int x = int(faces.at<float>(i, 0));
            int y = int(faces.at<float>(i, 1));
            int w = int(faces.at<float>(i, 2));
            int h = int(faces.at<float>(i, 3));
            float confidence = faces.at<float>(i, 14);
			cv::Scalar bgrColor = (i == centerFaceIndex) ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 100, 0);

            cv::rectangle(frame, cv::Rect(x, y, w, h), bgrColor, 2);
            cv::putText(frame, cv::format("%.2f", confidence), cv::Point(x, y - 5),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, bgrColor, 1);
        }
    }
}

void FaceDetector::DetectFacesHaar(const cv::Mat& frame, std::vector<cv::Rect>& outRects)
{
    outRects.clear();
    if (!haarInitialized_) return;

    cv::Mat gray;
    int channel = frame.channels();
    if ( channel == 3 || channel == 4) {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
		gray = frame;
    }

    cv::equalizeHist(gray, gray);   //　Uniformize the histogram (robust to lighting)

    // パラメータは必要に応じて調整
    std::vector<cv::Rect> faces;
    std::vector<int> rejectLevels;
    std::vector<double> levelWeights;

    haarCascade_.detectMultiScale(gray, faces, rejectLevels, levelWeights,
        HaarCascadeScaleFactor,                                         // scaleFactor (1.2)
        HaarCascadeMinNeighbors,                                        // minNeighbors (10)
        HaarCascadeFlags,                                               // flags (DO_CANNY_PRUNING)
        cv::Size(HaarCascadeMinFaceWidth, HaarCascadeMinFaceHeight),    // minSize (110, 110)
		cv::Size(HaarCascadeMaxFaceWidth, HaarCascadeMaxFaceHeight),    // maxSize (default: no limit)
		true                                // outputRejectLevels = true 
                                            // (to get rejectLevels and levelWeights)
    );

    // levelWeights[i] will be the weight
    for (size_t i = 0; i < faces.size(); ++i) {
        double raw = levelWeights[i];
        // Example: raw around 1 gives score 0.5, change is relaxed by 1/5
        double scale = 2.0;   // Increasing this makes score changes more gradual
        double score = 1.0 / (1.0 + std::exp(-(raw - 1.0) / scale));
        // Normalize to 0..1 (e.g., sigmoid style or min-max)
        // double score = 1.0 / (1.0 + std::exp(-(raw - 1.0))); // Example: convert raw using sigmoid
        // Or if min/max are known: (raw - min)/(max-min)
		// display the score on the top-left corner of the bounding box
        cv::putText(frame, cv::format("%.2f", score), faces[i].tl() + cv::Point(2, 14),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(203, 192, 255), 1);
    }

    outRects = std::move(faces);
}

void FaceDetector::DrawBoundingBoxesHaar(cv::Mat& frame, const std::vector<cv::Rect>& haarRects)
{
    int centerFaceIndex = 0;
    int minDistance2 = std::numeric_limits<int>::max(); // large initial value

    // 4b. Haar draw bounding boxes
    for (int i = 0; i < haarRects.size(); ++i) {
        int x = haarRects[i].x;
		int y = haarRects[i].y;
		int w = haarRects[i].width;
		int h = haarRects[i].height;
		int faceCenterX = x + (w / 2);
		int faceCenterY = y + (h / 2);
		int dx = faceCenterX - GetFrameWidth() / 2;
		int dy = faceCenterY - GetFrameHeight() / 2;
		if ( (dx*dx + dy*dy) < minDistance2 ) {
			centerFaceIndex = i;
			minDistance2 = dx*dx + dy*dy;
		}
    }
    for (int i = 0; i < haarRects.size(); ++i) {
        cv::Scalar bgrColor = (i == centerFaceIndex) ? cv::Scalar(255, 0, 255) : cv::Scalar(100, 50, 150);
        cv::rectangle(frame, haarRects[i], bgrColor, 2);
    }
}

const char* FaceDetector::GetWindowName() const
{
    return windowName_.c_str();
}

void FaceDetector::SetupWindow()
{
    static bool firstFrame = true;
    if (firstFrame) {
        cv::namedWindow(windowName_.c_str(), cv::WINDOW_AUTOSIZE);
        firstFrame = false;
    }
}

// Accessors
bool FaceDetector::IsCameraInitialized() const
{
    return cameraInitialized_;
}

bool FaceDetector::IsYunetInitialized() const
{
    return yunetInitialized_;
}

bool FaceDetector::IsHaarInitialized() const
{
    return haarInitialized_;
}

bool FaceDetector::IsDetectorInitialized() const
{
    return cameraInitialized_ && (yunetInitialized_ || haarInitialized_);
}

cv::VideoCapture& FaceDetector::GetCapture()
{
    return cap_;
}

int FaceDetector::GetFrameWidth() const
{
    return frameWidth_;
}

int FaceDetector::GetFrameHeight() const
{
    return frameHeight_;
}