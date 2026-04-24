#pragma once
#include <windows.h>
#include "OpenCV_without_warning.h"

class AdaptiveScreenLight
{
public:
    AdaptiveScreenLight();
    ~AdaptiveScreenLight();

    // enabled=true でカメラON時に呼ぶ。parentHwnd はメインダイアログのHWND
    void SetEnabled(bool enabled, HWND parentHwnd = nullptr);
    bool IsEnabled() const { return enabled_; }

    // 最小化・復元の連動（OnSysCommandから呼ぶ）
    void SetMinimized(bool minimized);

    // 毎フレーム呼び出す（ワーカースレッドからOK）
    void Update(const cv::Mat& frame, const cv::Rect& faceRect, float manualBrightness = -1.0f);

    // 手動で明るさを設定してUIに即反映させる（UIスレッド用）
    void ApplyManualBrightness(float brightness);

    // UIスレッドから呼ぶ（PostMessage経由）
    void ApplyColor(COLORREF color);

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    bool CreateBackgroundWindow();
    void DestroyBackgroundWindow();
    void RepaintBackground();

    float     MeasureFaceBrightness(const cv::Mat& frame,
                                    const cv::Rect& faceRect) const;
    float     MeasureAmbientBrightness(const cv::Mat& frame) const;
    cv::Vec3b EstimateSkinTone(const cv::Mat& frame,
                               const cv::Rect& faceRect) const;
    COLORREF  ComputeColor(float brightness, const cv::Vec3b& skinBgr) const;

    HWND      hwndBackground_;
    HWND      hwndNotify_;
    COLORREF  currentColor_;
    bool      enabled_;
    float     screenBrightness_;
    cv::Vec3b lastSkinBgr_;
    float     lastAmbient_;
    float     lastTarget_;

    static constexpr float kMinBrightness = 0.50f;  // 常に最低50%は保つ
    static constexpr float kStepUp           = 0.02f;
    static constexpr float kStepDown         = 0.01f;
    static constexpr float kDeadZone         = 0.02f;  // 目標値±2%は変化しない
};