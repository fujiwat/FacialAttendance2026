#pragma once

#include "OpenCV_without_warning.h"
#include <string>
#include "MyFunctions.h"

class FaceDetector
{
public:
	// Constructor: initializes camera and the YuNet face detector (uses camera frame size).
	FaceDetector();
	~FaceDetector();

	// YuNet detector parameter setters and getters
	void DetectFacesYunet(const cv::Mat& frame, cv::Mat& faces);
	void DrawBoundingBoxesYunet(cv::Mat& frame, const cv::Mat& faces);

	// HaarCascade parameters (not used in this implementation, but can be added if needed)
	void DetectFacesHaar(const cv::Mat& frame, std::vector<cv::Rect>& outRects);
	void DrawBoundingBoxesHaar(cv::Mat& frame, const std::vector<cv::Rect>& faces);

	// Window name to use for display
	const char* GetWindowName() const;
	void SetupWindow();

	// Camera and frame-size accessors
	bool IsCameraInitialized() const;
	bool IsYunetInitialized() const;
	bool IsHaarInitialized() const;
	bool IsDetectorInitialized() const;

	cv::VideoCapture& GetCapture();
	int GetFrameWidth() const;
	int GetFrameHeight() const;

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
};