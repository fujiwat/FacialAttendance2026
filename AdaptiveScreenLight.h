#pragma once
#include <windows.h>
#include "OpenCV_without_warning.h"

/**
 * @brief Manages dynamic overlay brightness adjusting system lighting matching ambient or face inputs limits.
 */
class AdaptiveScreenLight
{
public:
    AdaptiveScreenLight();
    ~AdaptiveScreenLight();

    /**
     * @brief Engages visual overlays processing attaching hooks onto targeted parents securely.
     * @param enabled Set true matching active webcam operation.
     * @param parentHwnd Main dialogue target receiving feedback natively.
     */
    void SetEnabled(bool enabled, HWND parentHwnd = nullptr);
    bool IsEnabled() const { return enabled_; }

    /**
     * @brief Triggers visibility linking minimizing behaviors against the main interface forms natively.
     * @param minimized Window visibility flag indicator determining logic.
     */
    void SetMinimized(bool minimized);

    /**
     * @brief Computes logic calculating frames adjusting intensities dynamically based on ambient metrics.
     * @param frame Processed visualization matrix evaluated.
     * @param faceRect Bounding structure defining main prioritized lighting targets natively.
     * @param manualBrightness Manually assigned intensity thresholds.
     */
    void Update(const cv::Mat& frame, const cv::Rect& faceRect, float manualBrightness = -1.0f);

    /**
     * @brief Implements user-dictated numeric settings bypassing automatic light balancing mechanisms.
     * @param brightness Scale defining active percentage threshold targeting UI limits.
     */
    void ApplyManualBrightness(float brightness);

    /**
     * @brief Maps active coloration signals updating UI components properly via message queue overrides.
     * @param color Evaluated RGB structures.
     */
    void ApplyColor(COLORREF color);

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    bool CreateBackgroundWindow();
    void DestroyBackgroundWindow();
    void RepaintBackground();

    COLORREF CalculateTextColor(COLORREF bgColor) const;
    void DrawStatusText(HDC hdc, const RECT& clientRc, const char* text, COLORREF textColor);

    float     MeasureFaceBrightness(const cv::Mat& frame, const cv::Rect& faceRect) const;
    float     MeasureAmbientBrightness(const cv::Mat& frame) const;
    cv::Vec3b EstimateSkinTone(const cv::Mat& frame, const cv::Rect& faceRect) const;
    COLORREF  ComputeColor(float brightness, const cv::Vec3b& skinBgr) const;

    HWND      hwndBackground_;
    HWND      hwndNotify_;
    COLORREF  currentColor_;
    bool      enabled_;
    float     screenBrightness_;
    cv::Vec3b lastSkinBgr_;
    float     lastAmbient_;
    float     lastTarget_;

    static constexpr float kMinBrightness = 0.50f;
    static constexpr float kStepUp = 0.02f;
    static constexpr float kStepDown = 0.01f;
    static constexpr float kDeadZone = 0.02f;
};