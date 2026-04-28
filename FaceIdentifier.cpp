#include "pch.h"
#include "FaceIdentifier.h"
#include "MyFunctions.h"
#include "MyConst.h"

FaceIdentifier::FaceIdentifier()
{
}

FaceIdentifier::~FaceIdentifier()
{
}

bool FaceIdentifier::Initialize(const std::string& modelFolder)
{
    try {
        // 1. Eigenfaces の準備
        // eigenfaces_ = cv::face::EigenFaceRecognizer::create();

        // 2. LBPH の準備
        // lbph_ = cv::face::LBPHFaceRecognizer::create();

        // 3. SFace の準備
        std::string sfaceModelPath = "face_recognition_sface_2021dec.onnx"; 
        sface_ = cv::FaceRecognizerSF::create(sfaceModelPath, "");
        if (sface_) {
            sfaceLoaded_ = true;
        }

        return true;
    }
    catch (const cv::Exception& e) {
        // 初期化エラー（ファイルがない等）を分かりやすくポップアップで知らせる
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
        return IdentifyLBPH(faceImage);

    case FaceIdentificationMethod::SFace:
        return IdentifySFace(faceImage, faceData);
    }

    return result;
}

IdentificationResult FaceIdentifier::IdentifyEigenfaces(const cv::Mat& faceImage)
{
    IdentificationResult result;
    result.name = "Unknown";
    // if (!eigenfaces_ || eigenfaces_->empty()) return result; // ★コメントアウト

    cv::Mat gray;
    if (faceImage.channels() == 3) {
        cv::cvtColor(faceImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = faceImage.clone();
    }

    // eigenfaces_->predict(gray, result.label, result.distance);
    // result.isValid = (result.distance < 5000.0); // 適当なしきい値

    // 仮実装
    result.isValid = false; 

    return result;
}

IdentificationResult FaceIdentifier::IdentifyLBPH(const cv::Mat& faceImage)
{
    IdentificationResult result;
    result.name = "Unknown";
    // if (!lbph_ || lbph_->empty()) return result; // ★コメントアウト

    cv::Mat gray;
    if (faceImage.channels() == 3) {
        cv::cvtColor(faceImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = faceImage.clone();
    }

    // lbph_->predict(gray, result.label, result.distance);
    // result.isValid = (result.distance < 100.0); // LBPHの一般的なしきい値

    // 仮実装
    result.isValid = false; 

    return result;
}

// ★追加: SFace等の特徴量を抽出してメモリ上の辞書に追加し、次回の識別に使えるようにする
void FaceIdentifier::Enroll(FaceIdentificationMethod method, const std::string& name, const cv::Mat& faceImage, const std::vector<float>& faceData)
{
    if (!sfaceLoaded_ || !sface_ || faceImage.empty() || name.empty()) {
        return;
    }

    cv::Mat alignedFace;
    bool aligned = false;
    if (!faceData.empty() && faceData.size() >= 15) {
        // 安全に 1x15 の CV_32FC1 行列を作成する
        cv::Mat faceBox(1, 15, CV_32FC1);
        for (int i = 0; i < 15; i++) faceBox.at<float>(0, i) = faceData[i];

        try {
            sface_->alignCrop(faceImage, faceBox, alignedFace);
            aligned = true;
        }
        catch (...) {}
    }

    // 万が一アライメントに失敗した場合の安全策（顔の四角形だけを自力で切り抜く！）
    if (!aligned) {
        if (!faceData.empty() && faceData.size() >= 4) {
            int x = std::max(0, static_cast<int>(faceData[0]));
            int y = std::max(0, static_cast<int>(faceData[1]));
            int w = static_cast<int>(faceData[2]);
            int h = static_cast<int>(faceData[3]);
            int side = std::max(w, h);
            
            x = std::max(0, x - (side - w) / 2);
            y = std::max(0, y - (side - h) / 2); // ★ 修正(cy -> y)
            
            w = std::min({side, faceImage.cols - x});
            h = std::min({side, faceImage.rows - y});
            side = std::min(w, h);

            if (side > 0) {
                cv::Mat cropFace(faceImage, cv::Rect(x, y, side, side));
                cv::resize(cropFace, alignedFace, cv::Size(112, 112));
            } else {
                cv::resize(faceImage, alignedFace, cv::Size(112, 112));
            }
        } else {
            cv::resize(faceImage, alignedFace, cv::Size(112, 112));
        }
    }

    cv::Mat feature;
    try {
        sface_->feature(alignedFace, feature); // 128次元の特徴量を抽出
        sfaceFeaturesMap_[name].push_back(feature.clone());
    } catch (...) { }
}

IdentificationResult FaceIdentifier::IdentifySFace(const cv::Mat& faceImage, const std::vector<float>& faceData)
{
    IdentificationResult result;
    result.name = "Unknown";
    result.isValid = false;

    if (!sfaceLoaded_ || !sface_) return result;
    if (faceImage.empty()) return result;

    cv::Mat alignedFace;
    bool aligned = false;
    if (!faceData.empty() && faceData.size() >= 15) {
        // 安全に 1x15 の CV_32FC1 行列を作成する
        cv::Mat faceBox(1, 15, CV_32FC1);
        for (int i = 0; i < 15; i++) faceBox.at<float>(0, i) = faceData[i];

        try {
            sface_->alignCrop(faceImage, faceBox, alignedFace);
            aligned = true;
        } catch (...) { }
    }

    // 万が一アライメントに失敗した場合の安全策（顔の四角形だけを自力で切り抜く！）
    if (!aligned) {
        if (!faceData.empty() && faceData.size() >= 4) {
            int x = std::max(0, static_cast<int>(faceData[0]));
            int y = std::max(0, static_cast<int>(faceData[1]));
            int w = static_cast<int>(faceData[2]);
            int h = static_cast<int>(faceData[3]);
            int side = std::max(w, h);
            
            x = std::max(0, x - (side - w) / 2);
            y = std::max(0, y - (side - h) / 2);
            
            w = std::min({side, faceImage.cols - x});
            h = std::min({side, faceImage.rows - y});
            side = std::min(w, h);

            if (side > 0) {
                cv::Mat cropFace(faceImage, cv::Rect(x, y, side, side));
                cv::resize(cropFace, alignedFace, cv::Size(112, 112));
            } else {
                cv::resize(faceImage, alignedFace, cv::Size(112, 112));
            }
        } else {
            cv::resize(faceImage, alignedFace, cv::Size(112, 112));
        }
    }

    cv::Mat feature;
    try {
        sface_->feature(alignedFace, feature);
    } catch (...) {
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

    for (const auto& pair : sfaceFeaturesMap_) {
        const std::string& personName = pair.first;
        for (const cv::Mat& dbFeature : pair.second) {
            double score = sface_->match(feature, dbFeature, cv::FaceRecognizerSF::DisType::FR_COSINE);
            // 類似度が今までの最高記録を開新したら上書きする
            if (score > maxSimilarity) {
                maxSimilarity = score;
                bestMatchName = personName;
            }
        }
    }

    // 「本人であれば 0.70 〜 0.95前後」という高いスコアが出るため、しきい値(類似度)を 0.70 に設定
    double testScore = 0.70; 

    if (maxSimilarity >= testScore) { 
        result.name = bestMatchName;
        result.distance = maxSimilarity;
        result.isValid = true;
    } else {
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