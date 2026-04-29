#pragma once

#include "OpenCV_without_warning.h"

namespace CustomLBPH {
    // ©ìLBPH‚ÌŒvZ—pŠÖ”ŒQ
    cv::Mat LBPHCalcImage(const cv::Mat& src);
    cv::Mat LBPHCalcSpatialHistogram(const cv::Mat& lbp_image, int grid_x, int grid_y);
    double LBPHCalcChiSquareDistance(const cv::Mat& h1, const cv::Mat& h2);
}