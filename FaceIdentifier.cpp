#include "pch.h"
#include <fstream> 
#include "FaceIdentifier.h"
#include "MyFunctions.h"
#include "MyConst.h"
#include "MyLBPH.h"

/**
 * @brief Default constructor for FaceIdentifier.
 */
FaceIdentifier::FaceIdentifier()
{
}

/**
 * @brief Destructor for FaceIdentifier. Releases any allocated resources and models.
 */
FaceIdentifier::~FaceIdentifier()
{
    if (sface_) {
        sface_.release();
    }
    sfaceFeaturesMap_.clear();
    lbphFeaturesMap_.clear();
    eigenfacesRawImages_.clear();
}

/**
 * @brief Initializes the necessary components and models for face identification.
 * @param modelFolder The path to the directory containing model files (e.g., ONNX model for SFace).
 * @return True if critical models (such as SFace) load successfully; otherwise false.
 */
bool FaceIdentifier::Initialize(const std::string& modelFolder)
{
    lbphLoaded_ = true;
    eigenfacesLoaded_ = true;
    try {
        std::string sfaceModelPath = "face_recognition_sface_2021dec.onnx";
        sface_ = cv::FaceRecognizerSF::create(sfaceModelPath, "");
        if (sface_) {
            sfaceLoaded_ = true;
        }
        return true;
    }
    catch (const cv::Exception&) {
        CString msg;
        msg.Format(_T("SFace Model Loading Error\nFile not found or invalid format:\nface_recognition_sface_2021dec.onnx"));
        ::MessageBox(NULL, msg, _T("FacialAttendance2026"), MB_OK | MB_ICONERROR);

        sfaceLoaded_ = false;
        return false;
    }
}

/**
 * @brief Routes the incoming image and features to the selected identification method.
 * @param method The facial recognition method to use (Eigenfaces, LBPH, SFace).
 * @param faceImage The source bounding box image of the face.
 * @param faceData Detection metadata including bounding box and landmark points.
 * @return A result object containing the matched name, numeric distance, and validity.
 */
IdentificationResult FaceIdentifier::Identify(FaceIdentificationMethod method, const cv::Mat& faceImage, const std::vector<float>& faceData)
{
    IdentificationResult result;
    if (faceImage.empty()) {
        return result;
    }

    switch (method) {
    case FaceIdentificationMethod::Eigenfaces:
        return IdentifyEigenfaces(faceImage, faceData);

    case FaceIdentificationMethod::LBPH:
        return IdentifyLBPH(faceImage, faceData);

    case FaceIdentificationMethod::SFace:
        return IdentifySFace(faceImage, faceData);
    }

    return result;
}

/**
 * @brief Performs face identification utilizing the Eigenfaces algorithm.
 * @param faceImage The cropped input image containing the face.
 * @param faceData Landmark metadata required for initial extraction.
 * @return The result of the Eigenfaces matching.
 */
IdentificationResult FaceIdentifier::IdentifyEigenfaces(const cv::Mat& faceImage, const std::vector<float>& faceData)
{
    IdentificationResult result;
    result.name = "Unknown";
    result.isValid = false;

    if (!eigenfacesLoaded_ || !myEigenfaces_.IsTrained()) {
        return result;
    }

    cv::TickMeter tmExtraction;
    tmExtraction.start();
    cv::Mat crop = ExtractFace(faceImage, faceData);
    cv::Mat gray;
    if (crop.channels() == 3) cv::cvtColor(crop, gray, cv::COLOR_BGR2GRAY);
    else gray = crop.clone();

    cv::Mat testImg;
    cv::resize(gray, testImg, cv::Size(112, 112));
    tmExtraction.stop();
    RecordExtractionLatency(tmExtraction.getTimeMilli());

    cv::TickMeter tmMatching;
    int profileCount = static_cast<int>(eigenfacesRawImages_.size());

    tmMatching.start();
    auto bestMatch = myEigenfaces_.Predict(testImg);
    tmMatching.stop();

    if (profileCount > 0) {
        RecordMatchingLatency(tmMatching.getTimeMilli(), profileCount);
    }

    double threshold = 25000.0;

    result.distance = bestMatch.second;
    if (result.distance < threshold) {
        result.name = bestMatch.first;
        result.isValid = true;
    }
    else {
        std::string distStr = std::to_string(static_cast<int>(result.distance));
        result.name = "Unk(" + bestMatch.first + "/" + distStr + ")";
        result.isValid = false;
    }

    return result;
}

/**
 * @brief Normalizes and crops the face region based on facial landmarks, if available.
 *        Falls back to a standard square crop if advanced alignment is not possible.
 * @param faceImage The unaligned localized face image array.
 * @param faceData Bounding box or detailed landmarks for extraction.
 * @return A 112x112 aligned face matrix.
 */
cv::Mat FaceIdentifier::ExtractFace(const cv::Mat& faceImage, const std::vector<float>& faceData)
{
    cv::Mat alignedFace;
    bool aligned = false;

    bool isHaarDummy = (!faceData.empty() && faceData.size() >= 15 && faceData[14] == 1.0f);

    if (sfaceLoaded_ && sface_ && !faceData.empty() && faceData.size() >= 15 && !isHaarDummy) {
        cv::Mat faceBox(1, 15, CV_32FC1);
        for (int i = 0; i < 15; i++) faceBox.at<float>(0, i) = faceData[i];
        try {
            sface_->alignCrop(faceImage, faceBox, alignedFace);
            aligned = true;
        }
        catch (...) {}
    }

    if (!aligned) {
        if (!faceData.empty() && faceData.size() >= 4) {
            int x = std::max(0, static_cast<int>(faceData[0]));
            int y = std::max(0, static_cast<int>(faceData[1]));
            int w = static_cast<int>(faceData[2]);
            int h = static_cast<int>(faceData[3]);
            int side = std::max(w, h);

            x = std::max(0, x - (side - w) / 2);
            y = std::max(0, y - (side - h) / 2);

            w = std::min({ side, faceImage.cols - x });
            h = std::min({ side, faceImage.rows - y });
            side = std::min(w, h);

            if (side > 0) {
                cv::Mat cropFace(faceImage, cv::Rect(x, y, side, side));
                cv::resize(cropFace, alignedFace, cv::Size(112, 112));
            }
            else {
                cv::resize(faceImage, alignedFace, cv::Size(112, 112));
            }
        }
        else {
            cv::resize(faceImage, alignedFace, cv::Size(112, 112));
        }
    }
    return alignedFace;
}

/**
 * @brief Performs face identification utilizing the Custom LBPH algorithm.
 * @param faceImage The source cropped face image matrix.
 * @param faceData Detection metadata used for face alignment.
 * @return The result encompassing identifying details from the LBPH map.
 */
IdentificationResult FaceIdentifier::IdentifyLBPH(const cv::Mat& faceImage, const std::vector<float>& faceData)
{
    IdentificationResult result;
    result.name = "Unknown";
    result.isValid = false;

    if (lbphFeaturesMap_.empty() || faceImage.empty()) {
        return result;
    }

    cv::TickMeter tmExtraction;
    tmExtraction.start();

    cv::Mat crop = ExtractFace(faceImage, faceData);
    cv::Mat gray;
    if (crop.channels() == 3) cv::cvtColor(crop, gray, cv::COLOR_BGR2GRAY);
    else gray = crop.clone();

    cv::Mat lbpImg = CustomLBPH::LBPHCalcImage(gray);
    cv::Mat testFeature = CustomLBPH::LBPHCalcSpatialHistogram(lbpImg, 8, 8);
    tmExtraction.stop();
    RecordExtractionLatency(tmExtraction.getTimeMilli());

    cv::TickMeter tmMatching;
    double minDistance = DBL_MAX;
    std::string bestMatchName = "Unk";
    int profileCount = 0;

    tmMatching.start();
    for (const auto& pair : lbphFeaturesMap_) {
        const std::string& personName = pair.first;
        for (const cv::Mat& dbFeature : pair.second) {
            profileCount++;
            double dist = CustomLBPH::LBPHCalcChiSquareDistance(testFeature, dbFeature);
            if (dist < minDistance) {
                minDistance = dist;
                bestMatchName = personName;
            }
        }
    }
    tmMatching.stop();
    if (profileCount > 0) {
        RecordMatchingLatency(tmMatching.getTimeMilli(), profileCount);
    }

    double threshold = 9000.0;

    if (minDistance < threshold) {
        result.name = bestMatchName;
        result.distance = minDistance;
        result.isValid = true;
    }
    else {
        std::string distStr = std::to_string(static_cast<int>(minDistance));
        result.name = "Unk(" + bestMatchName + "/" + distStr + ")";
        result.distance = minDistance;
        result.isValid = false;
    }

    return result;
}

/**
 * @brief Registers and encodes a verified face according to a specified mathematical method.
 * @param method The target technique (Eigenfaces, LBPH, SFace).
 * @param name Alphanumeric alias defining the entity.
 * @param faceImage The localized captured face image context.
 * @param faceData Contextual detector info like feature tracking structures.
 */
void FaceIdentifier::Enroll(FaceIdentificationMethod method, const std::string& name, const cv::Mat& faceImage, const std::vector<float>& faceData)
{
    if (faceImage.empty() || name.empty()) return;

    cv::Mat alignedFace = ExtractFace(faceImage, faceData);

    if (method == FaceIdentificationMethod::Eigenfaces) {
        cv::Mat gray;
        if (alignedFace.channels() == 3) cv::cvtColor(alignedFace, gray, cv::COLOR_BGR2GRAY);
        else gray = alignedFace.clone();

        cv::Mat addImg;
        cv::resize(gray, addImg, cv::Size(112, 112));

        eigenfacesRawImages_.push_back(addImg.clone());
        eigenfacesLabels_.push_back(name);

        myEigenfaces_.Train(eigenfacesRawImages_, eigenfacesLabels_);
        return;
    }

    if (method == FaceIdentificationMethod::LBPH) {
        cv::Mat gray;
        if (alignedFace.channels() == 3) cv::cvtColor(alignedFace, gray, cv::COLOR_BGR2GRAY);
        else gray = alignedFace.clone();

        cv::Mat lbpImg = CustomLBPH::LBPHCalcImage(gray);
        cv::Mat histFeature = CustomLBPH::LBPHCalcSpatialHistogram(lbpImg, 8, 8);
        lbphFeaturesMap_[name].push_back(histFeature.clone());
        return;
    }

    if (method == FaceIdentificationMethod::SFace) {
        if (!sfaceLoaded_ || !sface_) return;
        cv::Mat feature;
        cv::TickMeter tmExtraction;
        try {
            tmExtraction.start();
            sface_->feature(alignedFace, feature);
            tmExtraction.stop();

            RecordExtractionLatency(tmExtraction.getTimeMilli());

            sfaceFeaturesMap_[name].push_back(feature.clone());
        }
        catch (...) {}
        return;
    }
}

/**
 * @brief Matches an inputted face against enrolled parameters using SFace networking representations.
 * @param faceImage The provided image matrix bounding the localized face.
 * @param faceData Array enclosing bounding box sizes directly tied to structural positions.
 * @return Validation output referencing closest match probability strings against baseline limits.
 */
IdentificationResult FaceIdentifier::IdentifySFace(const cv::Mat& faceImage, const std::vector<float>& faceData)
{
    IdentificationResult result;
    result.name = "Unknown";
    result.isValid = false;

    if (!sfaceLoaded_ || !sface_) return result;
    if (faceImage.empty()) return result;

    cv::Mat alignedFace = ExtractFace(faceImage, faceData);

    cv::Mat feature;
    cv::TickMeter tmExtraction;
    try {
        tmExtraction.start();
        sface_->feature(alignedFace, feature);
        tmExtraction.stop();
        RecordExtractionLatency(tmExtraction.getTimeMilli());
    }
    catch (...) {
        result.name = "Error: FeatureEx";
        return result;
    }

    if (sfaceFeaturesMap_.empty()) {
        result.name = "Unk: No Data";
        return result;
    }

    double maxSimilarity = -1.0;
    std::string bestMatchName = "Unk";

    cv::TickMeter tmMatching;
    int profileCount = 0;

    tmMatching.start();
    for (const auto& pair : sfaceFeaturesMap_) {
        const std::string& personName = pair.first;
        for (const cv::Mat& dbFeature : pair.second) {
            profileCount++;
            double score = sface_->match(feature, dbFeature, cv::FaceRecognizerSF::DisType::FR_COSINE);
            if (score > maxSimilarity) {
                maxSimilarity = score;
                bestMatchName = personName;
            }
        }
    }
    tmMatching.stop();
    if (profileCount > 0) {
        RecordMatchingLatency(tmMatching.getTimeMilli(), profileCount);
    }

    double testScore = 0.70;

    if (maxSimilarity >= testScore) {
        result.name = bestMatchName;
        result.distance = maxSimilarity;
        result.isValid = true;
    }
    else {
        std::string distStr = std::to_string(maxSimilarity);
        if (distStr.length() > 4) distStr = distStr.substr(0, 4);
        result.name = "Unk(" + distStr + ")";
        result.distance = maxSimilarity;
        result.isValid = false;
    }

    return result;
}

/**
 * @brief Validates if the Eigenfaces engine component has been successfully allocated.
 * @return True if readily accessible.
 */
bool FaceIdentifier::IsEigenfacesLoaded() const { return eigenfacesLoaded_; }

/**
 * @brief Validates if the internal LBPH matching logic variables have been activated.
 * @return True if initialized appropriately.
 */
bool FaceIdentifier::IsLBPHLoaded() const { return lbphLoaded_; }

/**
 * @brief Reports on the capability of applying SFace inferences checking backend readiness strings.
 * @return Boolean signal denoting operation eligibility.
 */
bool FaceIdentifier::IsSFaceLoaded() const { return sfaceLoaded_; }

/**
 * @brief Consolidates feature derivation timeframe variables incrementally storing quantities.
 * @param timeMs Count in milliseconds describing the operation latency.
 */
void FaceIdentifier::RecordExtractionLatency(double timeMs) {
    totalExtractionTimeMs_ += timeMs;
    extractionCount_++;
}

/**
 * @brief Accumulates computational periods evaluated through similarity match profiles.
 * @param totalTimeMs Total sum delay resolving dictionary.
 * @param numProfiles Amount of existing enrollment items matched against.
 */
void FaceIdentifier::RecordMatchingLatency(double totalTimeMs, int numProfiles) {
    if (numProfiles > 0) {
        totalMatchingTimePerProfileMs_ += (totalTimeMs / numProfiles);
        matchingCount_++;
    }
}

/**
 * @brief Saves combined metrics relating latencies processed and zeroes tracking indicators.
 * @param currentMethod Origin algorithm context.
 */
void FaceIdentifier::FlushAndResetIdentificationData(FaceIdentificationMethod currentMethod)
{
    if (extractionCount_ == 0 && matchingCount_ == 0) return;

    double avg_extraction = (extractionCount_ > 0) ? (totalExtractionTimeMs_ / extractionCount_) : 0.0;
    double avg_matching = (matchingCount_ > 0) ? (totalMatchingTimePerProfileMs_ / matchingCount_) : 0.0;

    std::wstring modeStr = L"Unknown";
    if (currentMethod == FaceIdentificationMethod::Eigenfaces) {
        modeStr = wFACE_IDENTIFICATION_METHOD_Eigenfaces;
    }
    else if (currentMethod == FaceIdentificationMethod::LBPH) {
        modeStr = wFACE_IDENTIFICATION_METHOD_LBPH;
    }
    else if (currentMethod == FaceIdentificationMethod::SFace) {
        modeStr = wFACE_IDENTIFICATION_METHOD_SFace;
    }
    SaveEvaluationLatencyCsv(wFACE_IDENTIFICATION_LATENCY_FOLDER_NAME, modeStr, avg_extraction, avg_matching, static_cast<long long>(extractionCount_));

    totalExtractionTimeMs_ = 0.0;
    extractionCount_ = 0;
    totalMatchingTimePerProfileMs_ = 0.0;
    matchingCount_ = 0;
}