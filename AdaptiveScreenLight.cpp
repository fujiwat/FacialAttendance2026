#include "pch.h"
#include "AdaptiveScreenLight.h"
#include "FacialAttendance2026Dlg.h"    // WM_UPDATE_SCREEN_LIGHT

static const wchar_t* kBgClassName = L"AdaptiveScreenLightBg";

// ── Constructor / Destructor ──────────────────────────────
AdaptiveScreenLight::AdaptiveScreenLight()
    : hwndBackground_(nullptr)
    , hwndNotify_(nullptr)
    , currentColor_(RGB(0, 0, 0))
    , enabled_(false)
    , screenBrightness_(0.0f)
    , lastSkinBgr_({ 200, 210, 220 })
    , lastAmbient_(0.0f)           
    , lastTarget_(0.0f)           
{
    WNDCLASSEXW wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = AdaptiveScreenLight::WndProc;
    wc.hInstance     = AfxGetInstanceHandle();
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = kBgClassName;
    RegisterClassExW(&wc); // 多重登録は無視
}

AdaptiveScreenLight::~AdaptiveScreenLight()
{
    DestroyBackgroundWindow();
}

// ── Public API ────────────────────────────────────────────
void AdaptiveScreenLight::SetEnabled(bool enabled, HWND parentHwnd)
{
    enabled_    = enabled;
    hwndNotify_ = parentHwnd;

    if (enabled_) {
        if (!hwndBackground_) CreateBackgroundWindow();
        screenBrightness_ = 0.0f;           // 黒からスタート
        currentColor_     = RGB(0, 0, 0);
        if (hwndBackground_) ShowWindow(hwndBackground_, SW_SHOW);
        if (hwndNotify_)
            SetWindowPos(hwndNotify_, HWND_TOP, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE);
    } else {
        if (hwndBackground_) ShowWindow(hwndBackground_, SW_HIDE);
    }
}

void AdaptiveScreenLight::SetMinimized(bool minimized)
{
    if (!hwndBackground_) return;
    if (minimized) {
        ShowWindow(hwndBackground_, SW_HIDE);
    } else {
        if (enabled_) ShowWindow(hwndBackground_, SW_SHOW);
        SetWindowPos(hwndBackground_, HWND_BOTTOM, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

void AdaptiveScreenLight::Update(const cv::Mat& frame, const cv::Rect& faceRect)
{
    if (!enabled_ || frame.empty() || !hwndNotify_) return;

    // Step 1: 顔領域の輝度を計測
    float measured = MeasureAmbientBrightness(frame);
    lastAmbient_ = measured;        // ← 追加：計測値を保存

    // Step 2: ambient=20%以下は常に100%、それ以上は二乗曲線＋50%フロア
    float targetBrightness;
    if (measured <= 0.20f) {
        targetBrightness = 1.0f;                                        // 暗い → 常に100%
    } else {
        float norm     = measured / 0.90f;
        targetBrightness = std::max(kMinBrightness, 1.0f - norm * norm); // 明るい → 曲線＋50%フロア
    }
    lastTarget_ = targetBrightness; // ← 保存

    // Step 3: screenBrightness_ を目標値に向けて徐々に調整
    if (screenBrightness_ > targetBrightness + kDeadZone)
        screenBrightness_ = std::max(kMinBrightness, screenBrightness_ - kStepDown); // ← 0.0f → kMinBrightness
    else if (screenBrightness_ < targetBrightness - kDeadZone)
        screenBrightness_ = std::min(1.0f, screenBrightness_ + kStepUp);

    // Step 4: 色を計算してUIスレッドへ通知
    COLORREF newColor = ComputeColor(screenBrightness_, lastSkinBgr_);
    if (newColor != currentColor_) {
        currentColor_ = newColor;
        ::PostMessage(hwndNotify_, WM_UPDATE_SCREEN_LIGHT,
                      static_cast<WPARAM>(newColor), 0);
    }
}

void AdaptiveScreenLight::ApplyColor(COLORREF color)
{
    currentColor_ = color;
    RepaintBackground();
}

// ── Win32 ─────────────────────────────────────────────────
bool AdaptiveScreenLight::CreateBackgroundWindow()
{
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    hwndBackground_ = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        kBgClassName, L"ScreenLightBg",
        WS_POPUP,
        0, 0, screenW, screenH,
        nullptr,                    // オーナーなし（Z順序逆転防止）
        nullptr, AfxGetInstanceHandle(), this
    );
    if (!hwndBackground_) return false;

    // 常に最背面
    SetWindowPos(hwndBackground_, HWND_BOTTOM, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    return true;
}

void AdaptiveScreenLight::DestroyBackgroundWindow()
{
    if (hwndBackground_) {
        DestroyWindow(hwndBackground_);
        hwndBackground_ = nullptr;
    }
}

void AdaptiveScreenLight::RepaintBackground()
{
    if (!hwndBackground_) return;

    RECT rc;
    GetClientRect(hwndBackground_, &rc);
    HDC hdc = GetDC(hwndBackground_);
    if (!hdc) return;

    HBRUSH brush = CreateSolidBrush(currentColor_);
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);

    // 3つの値を表示
    char text[128];
    sprintf_s(text, sizeof(text),
              "ambient: %.0f%%  light: %.0f%%",
              lastAmbient_ * 100.0f,
              screenBrightness_ * 100.0f);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 128));  // 紺色（ネイビー）

    HFONT hFont = CreateFontA(
        96, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,  // ← FW_BOLD → FW_HEAVY
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    RECT textRect = { 20, rc.bottom - 120, rc.right, rc.bottom };
    DrawTextA(hdc, text, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    ReleaseDC(hwndBackground_, hdc);
}

LRESULT CALLBACK AdaptiveScreenLight::WndProc(HWND hwnd, UINT msg,
                                               WPARAM wp, LPARAM lp)
{
    if (msg == WM_CREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return 0;
    }
    // クリックされても前面に出ないようにする
    if (msg == WM_MOUSEACTIVATE) {
        return MA_NOACTIVATEANDEAT; // アクティブ化もクリックも無視
    }
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        auto* self = reinterpret_cast<AdaptiveScreenLight*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (self) {
            HBRUSH brush = CreateSolidBrush(self->currentColor_);
            FillRect(ps.hdc, &ps.rcPaint, brush);
            DeleteObject(brush);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ── 輝度・色計算 ──────────────────────────────────────────
float AdaptiveScreenLight::MeasureFaceBrightness(const cv::Mat& frame,
                                                  const cv::Rect& faceRect) const
{
    cv::Rect roi;
    if (!faceRect.empty() && faceRect.width > 0) {
        roi = faceRect & cv::Rect(0, 0, frame.cols, frame.rows);
    } else {
        int cx = frame.cols / 4, cy = frame.rows / 4;
        roi = cv::Rect(cx, cy, frame.cols / 2, frame.rows / 2);
    }

    // roiが無効な場合は中間値を返す（例外防止）
    if (roi.empty() || roi.width <= 0 || roi.height <= 0)
        return 0.5f;    // ← kTargetBrightness → 0.5f

    cv::Mat gray;
    cv::cvtColor(frame(roi), gray, cv::COLOR_BGR2GRAY);
    return static_cast<float>(cv::mean(gray)[0] / 255.0);
}

cv::Vec3b AdaptiveScreenLight::EstimateSkinTone(const cv::Mat& frame,
                                                const cv::Rect& faceRect) const
{
    cv::Rect safe  = faceRect & cv::Rect(0, 0, frame.cols, frame.rows);
    int      dx    = safe.width / 4, dy = safe.height / 4;
    cv::Rect inner = cv::Rect(safe.x + dx, safe.y + dy,
                              safe.width / 2, safe.height / 2)
                     & cv::Rect(0, 0, frame.cols, frame.rows);
    if (inner.empty()) return lastSkinBgr_;

    cv::Mat hsv, mask;
    cv::cvtColor(frame(inner), hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(0, 20, 70), cv::Scalar(25, 255, 255), mask);

    cv::Scalar mean = cv::mean(frame(inner), mask);
    if (mean[0] == 0 && mean[1] == 0 && mean[2] == 0) return lastSkinBgr_;

    return cv::Vec3b(static_cast<uchar>(mean[0]),
                     static_cast<uchar>(mean[1]),
                     static_cast<uchar>(mean[2]));
}

COLORREF AdaptiveScreenLight::ComputeColor(float brightness,
                                            const cv::Vec3b& skinBgr) const
{
    // 全肌色に公平な暖色白（電球色: 色温度約3000K相当）
    // 黒人→screenBrightness_が高くなる（フィードバックで自動補正）
    // 白人→screenBrightness_が低くなる（フィードバックで自動補正）
    // 肌色は光の色には使わない（バイアス防止）
    const float r = 255.0f;
    const float g = 230.0f;
    const float b = 190.0f;

    return RGB(
        std::min(255, static_cast<int>(r * brightness)),
        std::min(255, static_cast<int>(g * brightness)),
        std::min(255, static_cast<int>(b * brightness))
    );
}

float AdaptiveScreenLight::MeasureAmbientBrightness(const cv::Mat& frame) const
{
    // フレーム四隅（各1/5サイズ）を計測
    // → 顔は中央にあり四隅には映らない
    // → スクリーンライトは手前の顔を照らすので四隅の背景には届きにくい
    int cw = frame.cols / 5;
    int ch = frame.rows / 5;

    cv::Rect corners[4] = {
        cv::Rect(0,               0,               cw, ch), // 左上
        cv::Rect(frame.cols - cw, 0,               cw, ch), // 右上
        cv::Rect(0,               frame.rows - ch, cw, ch), // 左下
        cv::Rect(frame.cols - cw, frame.rows - ch, cw, ch), // 右下
    };

    float total = 0.0f;
    int   count = 0;
    for (const auto& corner : corners) {
        cv::Rect safe = corner & cv::Rect(0, 0, frame.cols, frame.rows);
        if (safe.empty()) continue;
        cv::Mat gray;
        cv::cvtColor(frame(safe), gray, cv::COLOR_BGR2GRAY);
        total += static_cast<float>(cv::mean(gray)[0] / 255.0);
        count++;
    }
    return (count > 0) ? (total / count) : 0.5f;
}