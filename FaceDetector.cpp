#include "pch.h"
#include <string>
#include <vector>
#include <thread>  
#include <chrono>  

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <sal.h>

#include "OpenCV_without_warning.h"
#include "MyFunctions.h"
#include "FaceDetector.h"
#include "MyConst.h"

bool FaceDetector::OpenCamera(int cameraIdx)
{
    if (cap_.isOpened()) {
        cap_.release();
    }

    int targetW = frameWidth_;
    int targetH = frameHeight_;

    cap_.open(cameraIdx, cv::CAP_DSHOW);
    if (!cap_.isOpened()) {
        return false;
    }

    cap_.set(cv::CAP_PROP_FRAME_WIDTH, targetW);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, targetH);

    return true;
}

bool FaceDetector::OpenCamera_ok(int cameraIdx)
{
    if (cap_.isOpened()) {
        cap_.release();
    }

    int targetW = frameWidth_;
    int targetH = frameHeight_;

    cap_.open(cameraIdx);
    if (!cap_.isOpened()) {
        return false;
    }

    cap_.set(cv::CAP_PROP_FRAME_WIDTH, targetW);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, targetH);

    cv::Mat testFrame;
    int i;
    const int MAX_ATTEMPTS = 1;

    for (i = 0; i < MAX_ATTEMPTS; i++) {
        if (cap_.read(testFrame) && !testFrame.empty()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    if (i == MAX_ATTEMPTS) {
        cap_.release();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        cap_.open(cameraIdx);
        if (!cap_.isOpened()) {
            return false;
        }
    }

    return true;
}

FaceDetector::FaceDetector()
    : yunet_(nullptr)
    , haarCascade_()
    , windowName_(APP_NAME_LONG)
    , cap_()
    , frameWidth_(CaptureWidth)
    , frameHeight_(CaptureHeight)
    , cameraInitialized_(false)
    , yunetInitialized_(false)
    , haarInitialized_(false)
{
    cameraInitialized_ = OpenCamera(0);

    const std::string modelPath = YuNetModelPath;

    try {
        yunet_ = cv::FaceDetectorYN::create(modelPath, "", cv::Size(frameWidth_, frameHeight_),
            0.9f,
            0.3f,
            5000,
            cv::dnn::DNN_BACKEND_DEFAULT,
            cv::dnn::DNN_TARGET_CPU
        );
        if (yunet_) {
            yunet_->setInputSize(cv::Size(frameWidth_, frameHeight_));
            yunetInitialized_ = true;
        }
        else {
            yunetInitialized_ = false;
        }
    }
    catch (const cv::Exception&) {
        MyMessageBoxA(NULL, MB_OK | MB_ICONERROR, APP_NAME_SHORT, "Can not find YuNet model\n%s", modelPath.c_str());
        yunet_.release();
        yunetInitialized_ = false;
    }

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
    if (yunet_) {
        yunet_.release();
    }
}

void FaceDetector::DetectFacesYunet(const cv::Mat& frame, cv::Mat& faces)
{
    if (yunet_ && yunetInitialized_) {
        yunet_->detect(frame, faces);
    }
    else {
        faces.release();
    }
}

void FaceDetector::DrawBoundingBoxesYunet(cv::Mat& frame, const cv::Mat& faces) const
{
    size_t centerFaceIndex = 0;
    int minDistance2 = std::numeric_limits<int>::max();
    if (!faces.empty()) {
        for (int i = 0; i < faces.rows; i++) {
            int x = int(faces.at<float>(i, 0));
            int y = int(faces.at<float>(i, 1));
            int w = int(faces.at<float>(i, 2));
            int h = int(faces.at<float>(i, 3));

            int faceCenterX = x + (w / 2);
            int faceCenterY = y + (h / 2);
            int dx = faceCenterX - GetFrameWidth() / 2;
            int dy = faceCenterY - GetFrameHeight() / 2;

            long long distSquare = (long long)dx * dx + (long long)dy * dy;
            if (distSquare < minDistance2) {
                centerFaceIndex = i;
                minDistance2 = static_cast<int>(distSquare);
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
    if (channel == 3 || channel == 4) {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        gray = frame;
    }

    cv::equalizeHist(gray, gray);

    std::vector<cv::Rect> faces;
    std::vector<int> rejectLevels;
    std::vector<double> levelWeights;

    haarCascade_.detectMultiScale(gray, faces, rejectLevels, levelWeights,
        HaarCascadeScaleFactor,
        HaarCascadeMinNeighbors,
        HaarCascadeFlags,
        cv::Size(HaarCascadeMinFaceWidth, HaarCascadeMinFaceHeight),
        cv::Size(HaarCascadeMaxFaceWidth, HaarCascadeMaxFaceHeight),
        true
    );

    std::vector<cv::Rect> validFaces;
    for (size_t i = 0; i < faces.size(); ++i) {
        double raw = levelWeights[i];
        double scale = 2.0;
        double score = 1.0 / (1.0 + std::exp(-(raw - 1.0) / scale));

        if (score < 0.6) continue;

        cv::putText(frame, cv::format("%.2f", score), faces[i].tl() + cv::Point(2, 14),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(203, 192, 255), 1);

        validFaces.push_back(faces[i]);
    }

    outRects = std::move(validFaces);
}

void FaceDetector::DrawBoundingBoxesHaar(cv::Mat& frame, const std::vector<cv::Rect>& haarRects) const
{
    int centerFaceIndex = 0;
    int minDistance2 = std::numeric_limits<int>::max();

    for (int i = 0; i < haarRects.size(); ++i) {
        int x = haarRects[i].x;
        int y = haarRects[i].y;
        int w = haarRects[i].width;
        int h = haarRects[i].height;
        int faceCenterX = x + (w / 2);
        int faceCenterY = y + (h / 2);
        int dx = faceCenterX - GetFrameWidth() / 2;
        int dy = faceCenterY - GetFrameHeight() / 2;
        if ((dx * dx + dy * dy) < minDistance2) {
            centerFaceIndex = i;
            minDistance2 = dx * dx + dy * dy;
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

void FaceDetector::RecordDetectionLatency(double latencyMs)
{
    m_totalFrames++;
    double currentVal = m_totalLatencyMs.load();
    while (!m_totalLatencyMs.compare_exchange_weak(currentVal, currentVal + latencyMs)) {
    }
}

void FaceDetector::FlushAndResetDetectionData(FaceDetectionMethod currentMethod)
{
    uint64_t frames = m_totalFrames.load();
    if (frames > 0) {
        double totalLoopMs = m_totalLatencyMs.load();
        double avgLatencyMs = totalLoopMs / (double)frames;
        double equivalentFps = 0.0;
        if (avgLatencyMs > 0.0) {
            equivalentFps = 1000.0 / avgLatencyMs;
        }

        std::wstring modeStr = L"Unknown";
        if (currentMethod == FaceDetectionMethod::HaarCascades) {
            modeStr = wFACE_DETECTION_METHOD_HAARCASCADES;
        }
        else if (currentMethod == FaceDetectionMethod::YuNet) {
            modeStr = wFACE_DETECTION_METHOD_YUNET;
        }
        else if (currentMethod == FaceDetectionMethod::Both) {
            modeStr = wFACE_DETECTION_METHOD_BOTH;
        }

        SaveEvaluationLatencyCsv(wFACE_DETECTION_LATENCY_FOLDER_NAME, modeStr, avgLatencyMs, equivalentFps, static_cast<long long>(frames));

        m_totalFrames.store(0);
        m_totalLatencyMs.store(0.0);
    }
}