#include "pch.h"
#include <fstream> 
#include "FaceIdentifier.h"
#include "MyFunctions.h"
#include "MyConst.h"
#include "MyLBPH.h"

FaceIdentifier::FaceIdentifier()
{
}

FaceIdentifier::~FaceIdentifier()
{
}

bool FaceIdentifier::Initialize(const std::string& modelFolder)
{
    try {
        // 自作LBPHの準備 (特にモデルファイルはないため常にTrue)
        lbphLoaded_ = true;

        // SFace の準備
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

IdentificationResult FaceIdentifier::Identify(FaceIdentificationMethod method, const cv::Mat& faceImage, const std::vector<float>& faceData)
{
    IdentificationResult result;
    if (faceImage.empty()) {
        return result;
    }

    switch (method) {
    case FaceIdentificationMethod::Eigenfaces:
        return IdentifyEigenfaces(faceImage);

    case FaceIdentificationMethod::LBPH:
        return IdentifyLBPH(faceImage, faceData);

    case FaceIdentificationMethod::SFace:
        return IdentifySFace(faceImage, faceData);
    }

    return result;
}

IdentificationResult FaceIdentifier::IdentifyEigenfaces(const cv::Mat& faceImage)
{
    IdentificationResult result;
    result.name = "Unknown";
    result.isValid = false;
    return result;
}

cv::Mat FaceIdentifier::ExtractFace(const cv::Mat& faceImage, const std::vector<float>& faceData)
{
    cv::Mat alignedFace;
    bool aligned = false;

    // SFaceがロードされていればSFaceの強力なアライメント(目鼻位置合わせ)を間借りする
    if (sfaceLoaded_ && sface_ && !faceData.empty() && faceData.size() >= 15) {
        cv::Mat faceBox(1, 15, CV_32FC1);
        for (int i = 0; i < 15; i++) faceBox.at<float>(0, i) = faceData[i];
        try {
            sface_->alignCrop(faceImage, faceBox, alignedFace);
            aligned = true;
        }
        catch (...) {}
    }

    // アライメント失敗、または純粋な切り抜き
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

IdentificationResult FaceIdentifier::IdentifyLBPH(const cv::Mat& faceImage, const std::vector<float>& faceData)
{
    IdentificationResult result;
    result.name = "Unknown";
    result.isValid = false;

    if (lbphFeaturesMap_.empty() || faceImage.empty()) {
        return result;
    }

    // 1. 顔の切り出しとグレースケール化
    cv::Mat crop = ExtractFace(faceImage, faceData);
    cv::Mat gray;
    if (crop.channels() == 3) cv::cvtColor(crop, gray, cv::COLOR_BGR2GRAY);
    else gray = crop.clone();

    // 2. 自作LBPH特徴量の抽出(グリッドは8x8)
    cv::TickMeter tmExtraction;  // ★追加
    tmExtraction.start();        // ★追加
    cv::Mat lbpImg = CustomLBPH::LBPHCalcImage(gray);
    cv::Mat testFeature = CustomLBPH::LBPHCalcSpatialHistogram(lbpImg, 8, 8);
    tmExtraction.stop();         // ★追加
    RecordExtractionLatency(tmExtraction.getTimeMilli()); // ★追加

    // 3. 辞書と総当たりで比較 (距離が一番「小さい」ものを探す)
    double minDistance = DBL_MAX;
    std::string bestMatchName = "Unk";
    cv::TickMeter tmMatching;    // ★追加
    int profileCount = 0;        // ★追加

    tmMatching.start();          // ★追加
    for (const auto& pair : lbphFeaturesMap_) {
        const std::string& personName = pair.first;
        for (const cv::Mat& dbFeature : pair.second) {
            profileCount++;      // ★追加
            double dist = CustomLBPH::LBPHCalcChiSquareDistance(testFeature, dbFeature);
            if (dist < minDistance) {
                minDistance = dist;
                bestMatchName = personName;
            }
        }
    }
    tmMatching.stop();           // ★追加
    if (profileCount > 0) {      // ★追加
        RecordMatchingLatency(tmMatching.getTimeMilli(), profileCount); // ★追加
    }

    // カイ二乗距離のしきい値。環境や解像度で異なるため調整が必要(例として9000.0)
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

void FaceIdentifier::Enroll(FaceIdentificationMethod method, const std::string& name, const cv::Mat& faceImage, const std::vector<float>& faceData)
{
    if (faceImage.empty() || name.empty()) return;

    // 顔の切り出し
    cv::Mat alignedFace = ExtractFace(faceImage, faceData);

    if (method == FaceIdentificationMethod::LBPH) {
        cv::Mat gray;
        if (alignedFace.channels() == 3) cv::cvtColor(alignedFace, gray, cv::COLOR_BGR2GRAY);
        else gray = alignedFace.clone();

        // 特徴量化してマップに追加
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
            tmExtraction.start(); // ★計測開始
            sface_->feature(alignedFace, feature);
            tmExtraction.stop();  // ★計測終了

            // ★自身のメンバメソッドを呼んで記録
            RecordExtractionLatency(tmExtraction.getTimeMilli());

            sfaceFeaturesMap_[name].push_back(feature.clone());
        }
        catch (...) {}
        return;
    }
}

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
        tmExtraction.start(); // ★計測開始
        sface_->feature(alignedFace, feature);
        tmExtraction.stop();  // ★計測終了
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

    // ★修正：「一番数字が大きい＝一番似ている」スコアを探す
    double maxSimilarity = -1.0;  // 一番低い値(-1.0)からスタート
    std::string bestMatchName = "Unk";

    cv::TickMeter tmMatching; // ★追加: Matching用タイマー
    int profileCount = 0;     // ★比較した人数（特徴量）のカウント用

    tmMatching.start(); // ★ループ全体の計測開始
    for (const auto& pair : sfaceFeaturesMap_) {
        const std::string& personName = pair.first;
        for (const cv::Mat& dbFeature : pair.second) {
            profileCount++; // ★プロファイル数をカウント
            double score = sface_->match(feature, dbFeature, cv::FaceRecognizerSF::DisType::FR_COSINE);
            // 類似度が今までの最高記録を開新したら上書きする
            if (score > maxSimilarity) {
                maxSimilarity = score;
                bestMatchName = personName;
            }
        }
    }
    tmMatching.stop(); // ★ループ全体の計測終了
    // ★自身のメンバメソッドを呼んで記録 (割る処理はメソッド内でやってくれます)
    if (profileCount > 0) {
        RecordMatchingLatency(tmMatching.getTimeMilli(), profileCount);
    }

    // 「本人であれば 0.70 〜 0.95前後」という高いスコアが出るため、しきい値(類似度)を 0.70 に設定
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

bool FaceIdentifier::IsEigenfacesLoaded() const { return eigenfacesLoaded_; }
bool FaceIdentifier::IsLBPHLoaded() const { return lbphLoaded_; }
bool FaceIdentifier::IsSFaceLoaded() const { return sfaceLoaded_; }

/// 抽出時間の記録
void FaceIdentifier::RecordExtractionLatency(double timeMs) {
    totalExtractionTimeMs_ += timeMs;
    extractionCount_++;
}

// 照合時間（プロファイルあたり）の記録
void FaceIdentifier::RecordMatchingLatency(double totalTimeMs, int numProfiles) {
    if (numProfiles > 0) {
        totalMatchingTimePerProfileMs_ += (totalTimeMs / numProfiles);
        matchingCount_++;
    }
}

// CSVへのエクスポート
void FaceIdentifier::FlushAndResetIdentificationData(FaceIdentificationMethod currentMethod)
{
    if (extractionCount_ == 0 && matchingCount_ == 0) return;

    double avg_extraction = (extractionCount_ > 0) ? (totalExtractionTimeMs_ / extractionCount_) : 0.0;
    double avg_matching = (matchingCount_ > 0) ? (totalMatchingTimePerProfileMs_ / matchingCount_) : 0.0;

    std::wstring modeStr = L"Unknown";
    if (currentMethod == FaceIdentificationMethod::Eigenfaces) modeStr = L"Eigenfaces";
    else if (currentMethod == FaceIdentificationMethod::LBPH) modeStr = L"LBPH";
    else if (currentMethod == FaceIdentificationMethod::SFace) modeStr = L"SFace";

    // ★ 第5引数に extractionCount_ をキャストして渡す
    SaveEvaluationLatencyCsv(L"FaceIdentificationLatency", modeStr, avg_extraction, avg_matching, static_cast<long long>(extractionCount_));

    // 最後に必ずカウンタをリセット
    totalExtractionTimeMs_ = 0.0;
    extractionCount_ = 0;
    totalMatchingTimePerProfileMs_ = 0.0;
    matchingCount_ = 0;
}
