#pragma once

#include "OpenCV_without_warning.h"

/**
 * @brief Exposes manual implementations covering Local Binary Patterns Histograms extraction explicitly.
 */
namespace CustomLBPH {

    /**
     * @brief Translates typical grayscale pixels interpreting local pattern boundaries recursively.
     * @param src Matrix spanning standard intensity values originally.
     * @return Reassembled matrix storing derived texture encodings safely.
     */
    cv::Mat LBPHCalcImage(const cv::Mat& src);

    /**
     * @brief Establishes contextual spatial maps compiling localized histogram structures logically.
     * @param lbp_image Output block resulting previously via pattern encodings.
     * @param grid_x Quantity spanning horizontal slicing blocks.
     * @param grid_y Quantity spanning vertical slicing blocks.
     * @return Extracted flat floating array capturing regional histogram properties directly.
     */
    cv::Mat LBPHCalcSpatialHistogram(const cv::Mat& lbp_image, int grid_x, int grid_y);

    /**
     * @brief Gauges similarity bridging distinct histogram metrics deriving matching likelihood accurately.
     * @param h1 Flat array referencing primary histogram structure safely.
     * @param h2 Flat array referencing opposing histogram structure securely.
     * @return Mathematical result quantifying differences minimizing scaling anomalies.
     */
    double LBPHCalcChiSquareDistance(const cv::Mat& h1, const cv::Mat& h2);
}