#pragma once

/**
 * @file OpenCV_without_warning.h
 * @brief Centralized inclusion header bridging OpenCV dependencies safely.
 *        Pushes selective macros ignoring strict static analysis noises inside 3rd party codes locally.
 */

#pragma warning(push)
#pragma warning(disable: 26495 6294 6201 26439 26451 6001 6269)

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/face.hpp>  

#pragma warning(pop)