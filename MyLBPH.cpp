#include "pch.h"
#include "MyLBPH.h"

namespace CustomLBPH {

    cv::Mat LBPHCalcImage(const cv::Mat& src)
    {
        cv::Mat dst = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);
        for (int i = 1; i < src.rows - 1; i++) {
            for (int j = 1; j < src.cols - 1; j++) {
                uchar center = src.at<uchar>(i, j);
                unsigned char code = 0;
                code |= (src.at<uchar>(i - 1, j - 1) >= center) << 7;
                code |= (src.at<uchar>(i - 1, j)     >= center) << 6;
                code |= (src.at<uchar>(i - 1, j + 1) >= center) << 5;
                code |= (src.at<uchar>(i, j + 1)     >= center) << 4;
                code |= (src.at<uchar>(i + 1, j + 1) >= center) << 3;
                code |= (src.at<uchar>(i + 1, j)     >= center) << 2;
                code |= (src.at<uchar>(i + 1, j - 1) >= center) << 1;
                code |= (src.at<uchar>(i, j - 1)     >= center) << 0;
                dst.at<uchar>(i, j) = code;
            }
        }
        return dst;
    }

    cv::Mat LBPHCalcSpatialHistogram(const cv::Mat& lbp_image, int grid_x, int grid_y)
    {
        int width = lbp_image.cols / grid_x;
        int height = lbp_image.rows / grid_y;
        cv::Mat hist = cv::Mat::zeros(1, grid_x * grid_y * 256, CV_32FC1);

        int k = 0;
        for (int i = 0; i < grid_y; i++) {
            for (int j = 0; j < grid_x; j++) {
                cv::Mat cell = lbp_image(cv::Rect(j * width, i * height, width, height));
                int hist_cell[256] = { 0 };

                for (int y = 0; y < cell.rows; y++) {
                    for (int x = 0; x < cell.cols; x++) {
                        hist_cell[cell.at<uchar>(y, x)]++;
                    }
                }
                for (int c = 0; c < 256; c++) {
                    hist.at<float>(0, k++) = static_cast<float>(hist_cell[c]);
                }
            }
        }
        return hist;
    }

    double LBPHCalcChiSquareDistance(const cv::Mat& h1, const cv::Mat& h2)
    {
        double dist = 0.0;
        for (int i = 0; i < h1.cols; i++) {
            double a = h1.at<float>(0, i);
            double b = h2.at<float>(0, i);
            if (a + b > 0) {
                dist += (a - b) * (a - b) / (a + b);
            }
        }
        return dist;
    }

} // namespace CustomLBPH