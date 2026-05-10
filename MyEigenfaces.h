#pragma once

#include "OpenCV_without_warning.h"
#include <vector>
#include <string>

/**
 * @brief Provides Principal Component Analysis (Eigenfaces) based facial recognition methodologies.
 */
class MyEigenfaces {
public:
    /**
     * @brief Constructs an Eigenfaces analyzer configuring internal dimensional boundaries.
     * @param numComponents Number of principal components retained. Zero indicates maximum retention.
     */
    MyEigenfaces(int numComponents = 0);

    /**
     * @brief Destructor releasing allocated resources natively.
     */
    ~MyEigenfaces();

    /**
     * @brief Submits collections of processed facial images training the underlying structural PCA space model.
     * @param images Vector containing formatted matrices portraying user faces.
     * @param labels Ordered identifying mapped representations string.
     */
    void Train(const std::vector<cv::Mat>& images, const std::vector<std::string>& labels);

    /**
     * @brief Matches an unspecified picture validating dimensional Euclidean distances natively.
     * @param image Extracted dimensional snapshot evaluated securely.
     * @return Pair combining the mapped profile string reference and its measured distance metric securely.
     */
    std::pair<std::string, double> Predict(const cv::Mat& image) const;

    /**
     * @brief Indicates whether underlying matrices established dimensional profiles properly.
     * @return True if initialized with faces natively.
     */
    bool IsTrained() const;

private:
    int numComponents_;
    bool isTrained_;
    cv::PCA pca_;
    cv::Mat projectedModel_;
    std::vector<std::string> labels_;

    /**
     * @brief Normalizes images mapping structural boundaries into flat arrays ensuring mathematical compatibility.
     * @param images Source collection referencing normalized pictures.
     * @return Fully reshaped unified float data matrix arrays.
     */
    cv::Mat FormatImagesForPCA(const std::vector<cv::Mat>& images) const;
};