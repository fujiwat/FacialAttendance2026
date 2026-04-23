// FacialAttendance2026Dlg.h : header file
//

#pragma once
#include "FaceDetector.h"
#include "AdaptiveScreenLight.h"
#include <atomic>
#include <mutex>

#define WM_UPDATE_SCREEN_LIGHT (WM_APP + 1)

// Screen Light の動作モード
enum class ScreenLightMode
{
    None   = 0,  // 無効
    Slider = 1,  // スライダー値で固定輝度
    Auto   = 2,  // Adaptive（顔検出連動）
};

// CFacialAttendance2026Dlg dialog
class CFacialAttendance2026Dlg : public CDialogEx
{
// Construction
public:
    CFacialAttendance2026Dlg(CWnd* pParent = nullptr); // standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_FACIALATTENDANCE2026_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnCbnSelchangeComboCamera();
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    afx_msg LRESULT OnUpdateScreenLight(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnInitScreenLight(WPARAM wParam, LPARAM lParam);

    // Screen Light ラジオボタン／スライダー ハンドラー
    afx_msg void OnRadioSlSlider();
    afx_msg void OnRadioSlAuto();
    afx_msg void OnRadioSlNone();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

    DECLARE_MESSAGE_MAP()

private:
    CComboBox    m_comboCamera;
    CSliderCtrl  m_sliderScreenLight;
    int          m_currentCameraIdx;
    bool         m_shouldChangeCamera;

    FaceDetector m_detector;
    std::mutex   m_frameMutex;
    cv::Mat      m_lastFrame;
    UINT_PTR     m_timerId;
    CFont        m_fontBold;
    CFont        m_fontRegular;
    FpsCounter   m_fpsCounter;

    AdaptiveScreenLight            m_screenLight;
    std::atomic<ScreenLightMode>   m_screenLightMode;
    int                            m_sliderValue; // 0-100

    // 描画用
    void*  m_pvBits;
    int    m_bmpWidth;
    int    m_bmpHeight;

    void    UpdateFrame0();
    void    UpdateFrame();
    HBITMAP CreateBitmapFromMat(const cv::Mat& mat);

    // Screen Light モードをUIに反映し永続化する
    void    ApplyScreenLightMode(ScreenLightMode mode);
    // スライダー値をUIラベル・色に反映し永続化する
    void    ApplySliderValue(int value);
    // スライダー値からグレースケール COLORREF を求める
    static COLORREF SliderToColor(int value);

public:
    afx_msg void OnBnClickedClose();
};
