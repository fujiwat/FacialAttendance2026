#pragma once

#include "OpenCV_without_warning.h"
#include <string>
#include <atomic> // ★追加
#include "MyFunctions.h"

// ★追加: 顔検出の処理モードを表す列挙型
enum class FaceDetectionMethod {
	HaarCascades = 0,
	YuNet = 1,
	Both = 2
};

class FaceDetector
{
public:
	// Constructor: initializes camera and the YuNet face detector (uses camera frame size).
	FaceDetector();
	~FaceDetector();

	// 既存のメソッド...
	bool OpenCamera(int cameraIdx = 0);
	bool OpenCamera_ok(int cameraIdx = 0);

	void DetectFacesYunet(const cv::Mat& frame, cv::Mat& faces);
	void DrawBoundingBoxesYunet(cv::Mat& frame, const cv::Mat& faces) const;

	void DetectFacesHaar(const cv::Mat& frame, std::vector<cv::Rect>& outRects);
	void DrawBoundingBoxesHaar(cv::Mat& frame, const std::vector<cv::Rect>& haarRects) const;

	const char* GetWindowName() const;
	void SetupWindow();

	bool IsCameraInitialized() const;
	bool IsYunetInitialized() const;
	bool IsHaarInitialized() const;
	bool IsDetectorInitialized() const;

	cv::VideoCapture& GetCapture();
	int GetFrameWidth() const;
	int GetFrameHeight() const;

	// ★追加: レイテンシ計測とCSV出力をFaceDetectorで管理
	void RecordDetectionLatency(double latencyMs);
	void FlushAndResetDetectionData(FaceDetectionMethod currentMethod);

private:
	cv::Ptr<cv::FaceDetectorYN> yunet_;
	cv::CascadeClassifier haarCascade_;
	std::string windowName_;
	cv::VideoCapture cap_;
	int frameWidth_;
	int frameHeight_;
	bool cameraInitialized_;
	bool yunetInitialized_;
	bool haarInitialized_;

	// ★追加: 評価(Evaluation)用データの収集用変数
	std::atomic<uint64_t> m_totalFrames{ 0 };
	std::atomic<double>   m_totalLatencyMs{ 0.0 };
};