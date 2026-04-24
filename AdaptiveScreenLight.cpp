#include "pch.h"
#include "AdaptiveScreenLight.h"
#include "FacialAttendance2026Dlg.h"    // WM_UPDATE_SCREEN_LIGHT
#include <thread>

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
        currentColor_     = 0xFFFFFFFF;     // ← 追加: -1を設定して必ず次回の描画を強制する
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

void AdaptiveScreenLight::Update(const cv::Mat& frame, const cv::Rect& faceRect, float manualBrightness)
{
    if (!enabled_ || frame.empty() || !hwndNotify_) return;

    // Step 1: 顔領域の輝度を計測
    float measured = MeasureAmbientBrightness(frame);
    bool ambientChanged = std::abs(lastAmbient_ - measured) >= 0.01f;
    lastAmbient_ = measured;

    // マニュアル(Slider)モード処理
    if (manualBrightness >= 0.0f) {
        screenBrightness_ = manualBrightness;
        lastTarget_ = manualBrightness;
        COLORREF newColor = ComputeColor(screenBrightness_, lastSkinBgr_);
        
        // 色が変わった、または環境光が1%以上変動した場合に再描画通知を送る
        if (newColor != currentColor_ || ambientChanged) {
            currentColor_ = newColor;
            ::PostMessage(hwndNotify_, WM_UPDATE_SCREEN_LIGHT,
                          static_cast<WPARAM>(newColor), 0);
        }
        return;
    }

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

void AdaptiveScreenLight::ApplyManualBrightness(float brightness)
{
    screenBrightness_ = brightness;
    COLORREF newColor = ComputeColor(brightness, lastSkinBgr_);
    if (newColor != currentColor_) {
        currentColor_ = newColor;
        RepaintBackground();
    }
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
    if (hwndBackground_) {
        // 背景の再描画命令をWindowsのメッセージキューに送る
        InvalidateRect(hwndBackground_, nullptr, FALSE);
    }
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
        HDC hdc = BeginPaint(hwnd, &ps);
        auto* self = reinterpret_cast<AdaptiveScreenLight*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        if (self) {
            RECT clientRc;
            GetClientRect(hwnd, &clientRc);
            
            // 現在の背景色
            COLORREF bgColor = (self->currentColor_ != 0xFFFFFFFF) ? self->currentColor_ : RGB(0, 0, 0);

            // 1. 背景の塗りつぶし
            HBRUSH brush = CreateSolidBrush(bgColor);
            FillRect(hdc, &clientRc, brush);
            DeleteObject(brush);

            // 2. 文字列の準備
            char text[128];
            sprintf_s(text, sizeof(text),
                      "ambient: %.0f%%  light: %.0f%%",
                      self->lastAmbient_ * 100.0f,
                      self->screenBrightness_ * 100.0f);

            // 3. テキスト描画（ヘルパー関数呼び出し）
            COLORREF textColor = self->CalculateTextColor(bgColor);
            self->DrawStatusText(hdc, clientRc, text, textColor);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ── 描画ヘルパー関数 ─────────────────────────────────────────

COLORREF AdaptiveScreenLight::CalculateTextColor(COLORREF bgColor) const
{
    // 背景色から輝度(Luminance)を計算 (NTSC加重平均法)
    int r = GetRValue(bgColor);
    int g = GetGValue(bgColor);
    int b = GetBValue(bgColor);
    int luminance = (r * 299 + g * 587 + b * 114) / 1000;
            
    // 背景が明るければ紺色、暗ければ明るい水色を返す
    return (luminance > 128) ? RGB(0, 0, 128) : RGB(200, 255, 255);
}

void AdaptiveScreenLight::DrawStatusText(HDC hdc, const RECT& clientRc, const char* text, COLORREF textColor)
{
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, textColor);

    // フォントの作成と適用
    HFONT hFont = CreateFontA(
        96, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    // 描画領域の計算とテキスト描画
    RECT textRect = { 20, clientRc.bottom - 120, clientRc.right, clientRc.bottom };
    DrawTextA(hdc, text, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // 後始末
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
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
