#pragma once

#include "OpenCV_without_warning.h"
#include <string>
#include <atomic> 
#include "MyFunctions.h"

/**
 * @brief Enumeration representing the facial detection method.
 */
enum class FaceDetectionMethod {
    HaarCascades = 0,
    YuNet = 1,
    Both = 2
};

/**
 * @brief Handles face detection using different algorithms such as YuNet and Haar Cascades.
 * Also manages the camera operations, frame reading, and detection latency records.
 */
class FaceDetector
{
public:
    /**
     * @brief Constructor that initializes default values.
     */
    FaceDetector();

    /**
     * @brief Destructor that releases allocated resources.
     */
    ~FaceDetector();

    /**
     * @brief Opens the camera for capturing video frames.
     * @param cameraIdx Index of the camera device. Defaults to 0.
     * @return True if the camera opened successfully, false otherwise.
     */
    bool OpenCamera(int cameraIdx = 0);

    /**
     * @brief Fully initializes the camera and validates video capture availability.
     * @param cameraIdx Index of the camera device. Defaults to 0.
     * @return True if camera opens and provides stable frames, false otherwise.
     */
    bool OpenCamera_ok(int cameraIdx = 0);

    /**
     * @brief Detects faces in a given frame using the YuNet algorithm.
     * @param frame The input image frame.
     * @param faces Output matrix storing the coordinates of detected faces and landmarks.
     */
    void DetectFacesYunet(const cv::Mat& frame, cv::Mat& faces);

    /**
     * @brief Overlays detected bounding boxes and landmarks from YuNet onto the frame.
     * @param frame The input image frame.
     * @param faces Detected faces returned by DetectFacesYunet.
     */
    void DrawBoundingBoxesYunet(cv::Mat& frame, const cv::Mat& faces) const;

    /**
     * @brief Detects faces in a given frame using the Haar Cascades classifier.
     * @param frame The input image frame.
     * @param outRects Output vector storing rectangular bounds for detected faces.
     */
    void DetectFacesHaar(const cv::Mat& frame, std::vector<cv::Rect>& outRects);

    /**
     * @brief Overlays detected rectangular bounding boxes from Haar Cascades onto the frame.
     * @param frame The input image frame.
     * @param haarRects Detected face rectangles returned by DetectFacesHaar.
     */
    void DrawBoundingBoxesHaar(cv::Mat& frame, const std::vector<cv::Rect>& haarRects) const;

    /**
     * @brief Retrieves the window name setup for output display.
     * @return A C-style string containing the window's name.
     */
    const char* GetWindowName() const;

    /**
     * @brief Prepares and formats the output window context.
     */
    void SetupWindow();

    /**
     * @brief Checks if the active camera configuration is fully initialized.
     * @return True if the camera is functional, false otherwise.
     */
    bool IsCameraInitialized() const;

    /**
     * @brief Checks whether the YuNet deep learning detector is loaded and initialized.
     * @return True if YuNet is available, false otherwise.
     */
    bool IsYunetInitialized() const;

    /**
     * @brief Checks whether the Haar Cascades classifier is loaded and initialized.
     * @return True if Haar Cascades is available, false otherwise.
     */
    bool IsHaarInitialized() const;

    /**
     * @brief Validates if at least one detector (YuNet or Haar) is ready.
     * @return True if detection operations can be issued, false otherwise.
     */
    bool IsDetectorInitialized() const;

    /**
     * @brief Retrieves the underlying OpenCV video capture element.
     * @return Reference to the VideoCapture instance.
     */
    cv::VideoCapture& GetCapture();

    /**
     * @brief Retrieves the current frame width dimension.
     * @return The frame width in pixels.
     */
    int GetFrameWidth() const;

    /**
     * @brief Retrieves the current frame height dimension.
     * @return The frame height in pixels.
     */
    int GetFrameHeight() const;

    /**
     * @brief Records the time consumed for processing detection.
     * @param latencyMs Measured latency in milliseconds.
     */
    void RecordDetectionLatency(double latencyMs);

    /**
     * @brief Processes cumulative evaluation metrics into CSV files and resets statistics.
     * @param currentMethod The detection algorithm currently being evaluated.
     */
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

    std::atomic<uint64_t> m_totalFrames{ 0 };
    std::atomic<double>   m_totalLatencyMs{ 0.0 };
};