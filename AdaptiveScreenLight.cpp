#include "pch.h"
#include "AdaptiveScreenLight.h"
#include "FacialAttendance2026Dlg.h"    
#include <thread>

static const wchar_t* kBgClassName = L"AdaptiveScreenLightBg";

/**
 * @brief Constructs the AdaptiveScreenLight instance and registers the background window class.
 */
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
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = AdaptiveScreenLight::WndProc;
    wc.hInstance = AfxGetInstanceHandle();
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = kBgClassName;
    RegisterClassExW(&wc);
}

/**
 * @brief Destroys the AdaptiveScreenLight instance and cleans up underlying window resources.
 */
AdaptiveScreenLight::~AdaptiveScreenLight()
{
    DestroyBackgroundWindow();
}

/**
 * @brief Enables or disables the adaptive screen light functionality.
 * @param enabled Set to true to activate the screen light.
 * @param parentHwnd Handle to the parent window that will receive update notifications.
 */
void AdaptiveScreenLight::SetEnabled(bool enabled, HWND parentHwnd)
{
    enabled_ = enabled;
    hwndNotify_ = parentHwnd;

    if (enabled_) {
        if (!hwndBackground_) CreateBackgroundWindow();
        screenBrightness_ = 0.0f;
        currentColor_ = 0xFFFFFFFF;
        if (hwndBackground_) ShowWindow(hwndBackground_, SW_SHOW);
        if (hwndNotify_)
            SetWindowPos(hwndNotify_, HWND_TOP, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE);
    }
    else {
        if (hwndBackground_) ShowWindow(hwndBackground_, SW_HIDE);
    }
}

/**
 * @brief Adjusts the visibility of the screen light background when the main application minimizes.
 * @param minimized True if the application is being minimized.
 */
void AdaptiveScreenLight::SetMinimized(bool minimized)
{
    if (!hwndBackground_) return;
    if (minimized) {
        ShowWindow(hwndBackground_, SW_HIDE);
    }
    else {
        if (enabled_) ShowWindow(hwndBackground_, SW_SHOW);
        SetWindowPos(hwndBackground_, HWND_BOTTOM, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

/**
 * @brief Evaluates current camera frame and updates the required screen brightness logic.
 * @param frame The captured camera frame matrix.
 * @param faceRect The bounding box of the detected face.
 * @param manualBrightness Explicitly defined brightness level overrides automatic logic.
 */
void AdaptiveScreenLight::Update(const cv::Mat& frame, const cv::Rect& faceRect, float manualBrightness)
{
    if (!enabled_ || frame.empty() || !hwndNotify_) return;

    float measured = MeasureAmbientBrightness(frame);
    bool ambientChanged = std::abs(lastAmbient_ - measured) >= 0.01f;
    lastAmbient_ = measured;

    if (manualBrightness >= 0.0f) {
        screenBrightness_ = manualBrightness;
        lastTarget_ = manualBrightness;
        COLORREF newColor = ComputeColor(screenBrightness_, lastSkinBgr_);

        if (newColor != currentColor_ || ambientChanged) {
            currentColor_ = newColor;
            ::PostMessage(hwndNotify_, WM_UPDATE_SCREEN_LIGHT,
                static_cast<WPARAM>(newColor), 0);
        }
        return;
    }

    float targetBrightness;
    if (measured <= 0.20f) {
        targetBrightness = 1.0f;
    }
    else {
        float norm = measured / 0.90f;
        targetBrightness = std::max(kMinBrightness, 1.0f - norm * norm);
    }
    lastTarget_ = targetBrightness;

    if (screenBrightness_ > targetBrightness + kDeadZone)
        screenBrightness_ = std::max(kMinBrightness, screenBrightness_ - kStepDown);
    else if (screenBrightness_ < targetBrightness - kDeadZone)
        screenBrightness_ = std::min(1.0f, screenBrightness_ + kStepUp);

    COLORREF newColor = ComputeColor(screenBrightness_, lastSkinBgr_);
    if (newColor != currentColor_) {
        currentColor_ = newColor;
        ::PostMessage(hwndNotify_, WM_UPDATE_SCREEN_LIGHT,
            static_cast<WPARAM>(newColor), 0);
    }
}

/**
 * @brief Sets a direct color to the background UI.
 * @param color The explicit RGB value to project.
 */
void AdaptiveScreenLight::ApplyColor(COLORREF color)
{
    currentColor_ = color;
    RepaintBackground();
}

/**
 * @brief Adjusts the current brightness factor independently and processes immediate painting.
 * @param brightness Float percentage defining intended lumen intensity.
 */
void AdaptiveScreenLight::ApplyManualBrightness(float brightness)
{
    screenBrightness_ = brightness;
    COLORREF newColor = ComputeColor(brightness, lastSkinBgr_);
    if (newColor != currentColor_) {
        currentColor_ = newColor;
        RepaintBackground();
    }
}

/**
 * @brief Constructs a borderless full-screen tool-window used as the light emission plane.
 * @return True if system handles invoke successfully.
 */
bool AdaptiveScreenLight::CreateBackgroundWindow()
{
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    hwndBackground_ = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        kBgClassName, L"ScreenLightBg",
        WS_POPUP,
        0, 0, screenW, screenH,
        nullptr,
        nullptr, AfxGetInstanceHandle(), this
    );
    if (!hwndBackground_) return false;

    SetWindowPos(hwndBackground_, HWND_BOTTOM, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    return true;
}

/**
 * @brief Tears down the actively running background layer freeing system GUI elements.
 */
void AdaptiveScreenLight::DestroyBackgroundWindow()
{
    if (hwndBackground_) {
        DestroyWindow(hwndBackground_);
        hwndBackground_ = nullptr;
    }
}

/**
 * @brief Queues forced canvas invalidation forcing background element redraws reliably.
 */
void AdaptiveScreenLight::RepaintBackground()
{
    if (hwndBackground_) {
        InvalidateRect(hwndBackground_, nullptr, FALSE);
    }
}

/**
 * @brief Essential processing block trapping Windows events addressing custom window logic.
 */
LRESULT CALLBACK AdaptiveScreenLight::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_CREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return 0;
    }
    if (msg == WM_MOUSEACTIVATE) {
        return MA_NOACTIVATEANDEAT;
    }
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        auto* self = reinterpret_cast<AdaptiveScreenLight*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        if (self) {
            RECT clientRc;
            GetClientRect(hwnd, &clientRc);

            COLORREF bgColor = (self->currentColor_ != 0xFFFFFFFF) ? self->currentColor_ : RGB(0, 0, 0);

            HBRUSH brush = CreateSolidBrush(bgColor);
            FillRect(hdc, &clientRc, brush);
            DeleteObject(brush);

            char text[128];
            sprintf_s(text, sizeof(text),
                "ambient: %.0f%%  light: %.0f%%",
                self->lastAmbient_ * 100.0f,
                self->screenBrightness_ * 100.0f);

            COLORREF textColor = self->CalculateTextColor(bgColor);
            self->DrawStatusText(hdc, clientRc, text, textColor);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

/**
 * @brief Assesses contrast ensuring text readably projects atop the dynamically shifting screen colors.
 * @param bgColor Calculated underlying canvas background tint.
 * @return Appropriately contrasted RGB text coloring logic.
 */
COLORREF AdaptiveScreenLight::CalculateTextColor(COLORREF bgColor) const
{
    int r = GetRValue(bgColor);
    int g = GetGValue(bgColor);
    int b = GetBValue(bgColor);
    int luminance = (r * 299 + g * 587 + b * 114) / 1000;

    return (luminance > 128) ? RGB(0, 0, 128) : RGB(200, 255, 255);
}

/**
 * @brief Writes debug overlays conveying active illumination values to the rendering canvas.
 * @param hdc Active system device context drawing handle.
 * @param clientRc Bounding dimensional rectangle representing physical screen resolution.
 * @param text Formatted string describing telemetry.
 * @param textColor Validated highly contrasted RGB font selection.
 */
void AdaptiveScreenLight::DrawStatusText(HDC hdc, const RECT& clientRc, const char* text, COLORREF textColor)
{
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, textColor);

    HFONT hFont = CreateFontA(
        96, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    RECT textRect = { 20, clientRc.bottom - 120, clientRc.right, clientRc.bottom };
    DrawTextA(hdc, text, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

/**
 * @brief Senses luminance originating strictly across valid detection matrices extracting averages.
 * @param frame Encoded camera image content source.
 * @param faceRect Positional indicator addressing facial span area elements.
 * @return Normal metric specifying localized brightness factors.
 */
float AdaptiveScreenLight::MeasureFaceBrightness(const cv::Mat& frame, const cv::Rect& faceRect) const
{
    cv::Rect roi;
    if (!faceRect.empty() && faceRect.width > 0) {
        roi = faceRect & cv::Rect(0, 0, frame.cols, frame.rows);
    }
    else {
        int cx = frame.cols / 4, cy = frame.rows / 4;
        roi = cv::Rect(cx, cy, frame.cols / 2, frame.rows / 2);
    }

    if (roi.empty() || roi.width <= 0 || roi.height <= 0)
        return 0.5f;

    cv::Mat gray;
    cv::cvtColor(frame(roi), gray, cv::COLOR_BGR2GRAY);
    return static_cast<float>(cv::mean(gray)[0] / 255.0);
}

/**
 * @brief Gathers general surface textures computing base skin coloration templates dynamically.
 * @param frame Raw camera visualization capture block.
 * @param faceRect Expected facial bounding layout definition.
 * @return Matrix structure encompassing RGB estimates targeting localized features.
 */
cv::Vec3b AdaptiveScreenLight::EstimateSkinTone(const cv::Mat& frame, const cv::Rect& faceRect) const
{
    cv::Rect safe = faceRect & cv::Rect(0, 0, frame.cols, frame.rows);
    int      dx = safe.width / 4, dy = safe.height / 4;
    cv::Rect inner = cv::Rect(safe.x + dx, safe.y + dy, safe.width / 2, safe.height / 2)
        & cv::Rect(0, 0, frame.cols, frame.rows);
    if (inner.empty()) return lastSkinBgr_;

    cv::Mat hsv, mask;
    cv::cvtColor(frame(inner), hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(0, 20, 70), cv::Scalar(25, 255, 255), mask);

    cv::Scalar mean = cv::mean(frame(inner), mask);
    if (mean[0] == 0 && mean[1] == 0 && mean[2] == 0) return lastSkinBgr_;

    return cv::Vec3b(static_cast<uchar>(mean[0]), static_cast<uchar>(mean[1]), static_cast<uchar>(mean[2]));
}

/**
 * @brief Constructs synthetic color compositions merging brightness multipliers against hue standards.
 * @param brightness Required exposure factor limit.
 * @param skinBgr Computed target user composition elements referencing colors natively.
 * @return Finished composite RGB mapping ready to output.
 */
COLORREF AdaptiveScreenLight::ComputeColor(float brightness, const cv::Vec3b& skinBgr) const
{
    const float r = 255.0f;
    const float g = 230.0f;
    const float b = 190.0f;

    return RGB(
        std::min(255, static_cast<int>(r * brightness)),
        std::min(255, static_cast<int>(g * brightness)),
        std::min(255, static_cast<int>(b * brightness))
    );
}

/**
 * @brief Investigates visual perimeters deducing ambient environment illumination indices thoroughly.
 * @param frame Camera derived native image array reference object.
 * @return Normalized fraction demonstrating ambient illumination ratios.
 */
float AdaptiveScreenLight::MeasureAmbientBrightness(const cv::Mat& frame) const
{
    int cw = frame.cols / 5;
    int ch = frame.rows / 5;

    cv::Rect corners[4] = {
        cv::Rect(0,               0,               cw, ch),
        cv::Rect(frame.cols - cw, 0,               cw, ch),
        cv::Rect(0,               frame.rows - ch, cw, ch),
        cv::Rect(frame.cols - cw, frame.rows - ch, cw, ch),
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