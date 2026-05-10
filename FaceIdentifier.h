#pragma once

#include "OpenCV_without_warning.h"
#include <string>
#include <vector>
#include <map>
#include "MyEigenfaces.h" 

/**
 * @brief Available methodologies for facial feature extraction and matching.
 */
enum class FaceIdentificationMethod {
    Eigenfaces = 0,
    LBPH = 1,
    SFace = 2
};

/**
 * @brief Represents the result of an identification attempt.
 */
struct IdentificationResult {
    int label = -1;
    std::string name;
    double distance = 0.0;
    bool isValid = false;
};

/**
 * @brief Core module for detecting, enrolling, and matching faces against trained models.
 */
class FaceIdentifier
{
public:
    FaceIdentifier();
    ~FaceIdentifier();

    /**
     * @brief Initializes the necessary components such as loaded models.
     * @param modelFolder The path to the folder containing external model files.
     * @return True if initialized successfully, false otherwise.
     */
    bool Initialize(const std::string& modelFolder);

    /**
     * @brief Evaluates an incoming face and compares it with internal dictionaries.
     * @param method The identification framework to process the image.
     * @param faceImage The source cropped image.
     * @param faceData Extended bounding box or alignment metadata if available.
     * @return Result encompassing matched identities or validation failures.
     */
    IdentificationResult Identify(FaceIdentificationMethod method, const cv::Mat& faceImage, const std::vector<float>& faceData = {});

    /**
     * @brief Records a facial template linked to an identity alias.
     * @param method The algorithm utilized to encode the features.
     * @param name Name string mapping corresponding to the visual identity.
     * @param faceImage Captured individual's normalized picture.
     * @param faceData Supplementary geometrical landmarks info.
     */
    void Enroll(FaceIdentificationMethod method, const std::string& name, const cv::Mat& faceImage, const std::vector<float>& faceData = {});

    bool IsEigenfacesLoaded() const;
    bool IsLBPHLoaded() const;
    bool IsSFaceLoaded() const;

private:
    cv::Ptr<cv::FaceRecognizerSF> sface_;

    bool eigenfacesLoaded_ = false;
    bool lbphLoaded_ = false;
    bool sfaceLoaded_ = false;

    double totalExtractionTimeMs_ = 0.0;
    int extractionCount_ = 0;
    double totalMatchingTimePerProfileMs_ = 0.0;
    int matchingCount_ = 0;

    std::map<std::string, std::vector<cv::Mat>> sfaceFeaturesMap_;
    std::map<std::string, std::vector<cv::Mat>> lbphFeaturesMap_;

    MyEigenfaces myEigenfaces_;
    std::vector<cv::Mat> eigenfacesRawImages_;
    std::vector<std::string> eigenfacesLabels_;

    IdentificationResult IdentifyEigenfaces(const cv::Mat& faceImage, const std::vector<float>& faceData);
    IdentificationResult IdentifyLBPH(const cv::Mat& faceImage, const std::vector<float>& faceData);
    IdentificationResult IdentifySFace(const cv::Mat& faceImage, const std::vector<float>& faceData);

    cv::Mat ExtractFace(const cv::Mat& faceImage, const std::vector<float>& faceData);

public:
    /**
     * @brief Aggregates time metrics spent deriving facial features from inputs.
     * @param timeMs Logged elapsed milliseconds for a single sequence.
     */
    void RecordExtractionLatency(double timeMs);

    /**
     * @brief Accounts time measures consumed scoring templates comparatively.
     * @param totalTimeMs Spent calculation time bridging matches.
     * @param numProfiles Profiles quantity run against input.
     */
    void RecordMatchingLatency(double totalTimeMs, int numProfiles);

    /**
     * @brief Clears accumulated telemetry counters after outputting details physically.
     * @param currentMethod Contextual identification logic measured.
     */
    void FlushAndResetIdentificationData(FaceIdentificationMethod currentMethod);
};