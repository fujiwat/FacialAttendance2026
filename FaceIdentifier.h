#pragma once

#include "OpenCV_without_warning.h"
#include <string>
#include <vector>
#include <map>

// 識別の手法を表す enum
enum class FaceIdentificationMethod {
    Eigenfaces = 0,
    LBPH = 1,
    SFace = 2
};

// 識別結果を返すための構造体
struct IdentificationResult {
    int label = -1;       // 登録されたID（見つからない場合は -1）
    std::string name;     // 登録された名前（見つからない場合は "Unknown"）
    double distance = 0.0;// 類似度距離（アルゴリズムによって尺度が異なります）
    bool isValid = false; // 信頼できるしきい値内かどうか
};

class FaceIdentifier
{
public:
    FaceIdentifier();
    ~FaceIdentifier();

    // 初期化および学習済みモデルのロード
    bool Initialize(const std::string& modelFolder);

    // 評価（推論）の実行
    // method   : 0=Eigenface, 1=LBPH, 2=SFace
    // faceImage: 112x112 のアライメント済み顔画像
    // faceData : YuNetなどで得られた15個の配列 (Rect + 5 landmarks + conf) 
    IdentificationResult Identify(FaceIdentificationMethod method, const cv::Mat& faceImage, const std::vector<float>& faceData = {});

    // ★追加: 顔データをメモリ上の辞書に追加登録する（Incremental Enrollment）
    void Enroll(FaceIdentificationMethod method, const std::string& name, const cv::Mat& faceImage, const std::vector<float>& faceData = {});

    // 学習用イメージの配列
    // bool Train(FaceIdentificationMethod method, const std::vector<cv::Mat>& images, const std::vector<int>& labels);

    // 各アルゴリズムがロードされているか
    bool IsEigenfacesLoaded() const;
    bool IsLBPHLoaded() const;
    bool IsSFaceLoaded() const;

private:
    // 各特徴抽出・認識器のインスタンス
    // cv::Ptr<cv::face::BasicFaceRecognizer> eigenfaces_; // ★コメントアウト
    // cv::Ptr<cv::face::LBPHFaceRecognizer> lbph_;        // ★コメントアウト
    cv::Ptr<cv::FaceRecognizerSF> sface_;

    bool eigenfacesLoaded_ = false;
    bool lbphLoaded_ = false;
    bool sfaceLoaded_ = false;

    // label (ID) と名前 (Name) の対応辞書
    std::map<int, std::string> labelToNameMap_;

    // ★追加: SFace用の特徴量(ベクトル)と名前の対応を保持する辞書
    std::map<std::string, std::vector<cv::Mat>> sfaceFeaturesMap_;

    // 各アルゴリズムで固有の識別処理
    IdentificationResult IdentifyEigenfaces(const cv::Mat& faceImage);
    IdentificationResult IdentifyLBPH(const cv::Mat& faceImage);
    IdentificationResult IdentifySFace(const cv::Mat& faceImage, const std::vector<float>& faceData);
};