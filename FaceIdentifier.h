#pragma once

#include "OpenCV_without_warning.h"
#include <string>
#include <vector>
#include <map>

// 顔認識の手法を表す enum
enum class FaceIdentificationMethod {
    Eigenfaces = 0,
    LBPH = 1,
    SFace = 2
};

// 認識結果を返すための構造体
struct IdentificationResult {
    int label = -1;       // 登録されたID（見つからない場合は -1）
    std::string name;     // 登録された名前（見つからない場合は "Unknown"）
    double distance = 0.0;// 類似度（値、アルゴリズムによって尺度が異なります）
    bool isValid = false; // 閾値でしきった結果の判定
};

class FaceIdentifier
{
public:
    FaceIdentifier();
    ~FaceIdentifier();

    // 初期化や事前学習済みモデルのロード
    bool Initialize(const std::string& modelFolder);

    // 認識（推論）の実行
    IdentificationResult Identify(FaceIdentificationMethod method, const cv::Mat& faceImage, const std::vector<float>& faceData = {});

    // 顔データをメモリ上の辞書に追加（Incremental Enrollment）
    void Enroll(FaceIdentificationMethod method, const std::string& name, const cv::Mat& faceImage, const std::vector<float>& faceData = {});

    // アルゴリズムがロードされているか
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

    // SFace用の特徴量と名前の対応保持辞書
    std::map<std::string, std::vector<cv::Mat>> sfaceFeaturesMap_;

    // 自作LBPH用の特徴量(ヒストグラム)と名前の対応保持辞書
    std::map<std::string, std::vector<cv::Mat>> lbphFeaturesMap_;

    // 各アルゴリズムで固有の認識処理
    IdentificationResult IdentifyEigenfaces(const cv::Mat& faceImage);
    IdentificationResult IdentifyLBPH(const cv::Mat& faceImage, const std::vector<float>& faceData);
    IdentificationResult IdentifySFace(const cv::Mat& faceImage, const std::vector<float>& faceData);

    // 共通の顔切り出し処理
    cv::Mat ExtractFace(const cv::Mat& faceImage, const std::vector<float>& faceData);
public:
    void RecordExtractionLatency(double timeMs);
    void RecordMatchingLatency(double totalTimeMs, int numProfiles);
    void FlushAndResetIdentificationData(FaceIdentificationMethod currentMethod);

};