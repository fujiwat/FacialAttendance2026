// FacialAttendance2026Dlg.h : header file
//

#pragma once
#include "FaceDetector.h"
#include "AdaptiveScreenLight.h"
#include <atomic>
#include <mutex>
#include <thread>
#include <afxcontrolbars.h> // ★追加: CMFCMaskedEdit を使用するため

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
    afx_msg void OnBnClickedButtonConfirm();

    DECLARE_MESSAGE_MAP()

private:
    CComboBox    m_comboCamera;
    CSliderCtrl  m_sliderScreenLight;
    CComboBox    m_comboName;
    CEdit        m_editID;
    // ★ CMFCMaskedEdit から CDateTimeCtrl に変更
    CDateTimeCtrl m_editTime;
    CEdit        m_editComment;
    int          m_currentCameraIdx;
    bool         m_shouldChangeCamera;

    FaceDetector m_detector;
    std::mutex   m_frameMutex;
    cv::Mat      m_lastFrame;
    UINT_PTR     m_timerId;
    CFont        m_fontBold;
    CFont        m_fontRegular;
    CFont        m_fontFixed;   // ★ 追加: 固定幅フォント用
    CFont        m_fontFixedList;   // ★ 追加: 固定幅フォント用
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
    void UpdateIdentificationFields(CString name);

    // ★ 追加: CSVへの保存処理を抜き出した関数
    void SaveAttendanceToCsv(const CString& strTime, const CString& strName, const CString& strID, const CString& strComment);

    // Screen Light モードをUIに反映し永続化する
    void    ApplyScreenLightMode(ScreenLightMode mode);
    // スライダー値をUIラベル・色に反映し永続化する
    void    ApplySliderValue(int value);
    // スライダー値からグレースケール COLORREF を求める
    static COLORREF SliderToColor(int value);

    // ★追加: スレッド処理用の変数
    std::thread m_workerThread;
    std::atomic<bool> m_bStopThread{ false };
    std::atomic<bool> m_bShowWarning{ false };
    std::atomic<double> m_currentFps{ 0.0 }; // ← ★これを追加

public:
    afx_msg void OnBnClickedClose();  // 既存のボタン用処理
    virtual void OnCancel();          // ← ★追加！ (×ボタンやESCキー用)
    afx_msg void OnFileExit();
    afx_msg void OnFileShowattendancelist();
};
