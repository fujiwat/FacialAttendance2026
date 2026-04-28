#pragma once

// Suppress analysis noise coming from third-party headers (adjust numbers if needed)
#pragma warning(push)
#pragma warning(disable: 26495 6294 6201 26439 26451 6001 6269)
// Include OpenCV (single umbrella header or specific modules as needed)
#include <opencv2/opencv.hpp>
//#include <opencv2/face.hpp>            // ★コメントアウト: 環境に無いモジュールEigenface, LBPH に必要
#include <opencv2/objdetect/face.hpp>  // ★SFace 用。これは標準機能なので残す

// Restore warning state
#pragma warning(pop)