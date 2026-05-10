#include "pch.h"
#include "MyEigenfaces.h"

/**
 * @brief Initializes the Eigenfaces analyzer with the specified number of components.
 * @param numComponents Number of principal components to retain.
 */
MyEigenfaces::MyEigenfaces(int numComponents)
    : numComponents_(numComponents), isTrained_(false)
{
}

/**
 * @brief Destructor for the MyEigenfaces class.
 */
MyEigenfaces::~MyEigenfaces()
{
}

/**
 * @brief Transforms a collection of images into a structured matrix row format suitable for PCA computation.
 * @param images The vector of facial images to be processed.
 * @return A unified float matrix where each row represents an image.
 */
cv::Mat MyEigenfaces::FormatImagesForPCA(const std::vector<cv::Mat>& images) const
{
    if (images.empty()) return cv::Mat();

    int n = static_cast<int>(images.size());
    int dim = static_cast<int>(images[0].total());
    cv::Mat data(n, dim, CV_32FC1);

    for (int i = 0; i < n; i++) {
        cv::Mat row = data.row(i);
        cv::Mat img = images[i].reshape(1, 1);
        img.convertTo(row, CV_32FC1);
    }
    return data;
}

/**
 * @brief Trains the Principal Component Analysis (Eigenfaces) model using a given set of images and labels.
 * @param images A collection of normalized facial images.
 * @param labels Corresponding string identifiers for each image.
 */
void MyEigenfaces::Train(const std::vector<cv::Mat>& images, const std::vector<std::string>& labels)
{
    if (images.empty() || images.size() != labels.size()) {
        isTrained_ = false;
        return;
    }

    cv::Mat data = FormatImagesForPCA(images);

    pca_(data, cv::Mat(), cv::PCA::DATA_AS_ROW, numComponents_);

    projectedModel_ = pca_.project(data);
    labels_ = labels;
    isTrained_ = true;
}

/**
 * @brief Projects a target image into the PCA space and identifies the closest matching representation.
 * @param image The input facial image to be identified.
 * @return A pair containing the matched label string and the calculated Euclidean distance.
 */
std::pair<std::string, double> MyEigenfaces::Predict(const cv::Mat& image) const
{
    if (!isTrained_ || image.empty()) {
        return { "Unknown", DBL_MAX };
    }

    cv::Mat query;
    image.reshape(1, 1).convertTo(query, CV_32FC1);

    cv::Mat projectedQuery = pca_.project(query);

    double minDistance = DBL_MAX;
    int bestIdx = -1;

    for (int i = 0; i < projectedModel_.rows; i++) {
        double dist = cv::norm(projectedModel_.row(i), projectedQuery, cv::NORM_L2);
        if (dist < minDistance) {
            minDistance = dist;
            bestIdx = i;
        }
    }

    if (bestIdx >= 0) {
        return { labels_[bestIdx], minDistance };
    }

    return { "Unknown", DBL_MAX };
}

/**
 * @brief Checks if the PCA data structures have been successfully trained.
 * @return True if the internal model contains valid projected representations.
 */
bool MyEigenfaces::IsTrained() const { return isTrained_; }