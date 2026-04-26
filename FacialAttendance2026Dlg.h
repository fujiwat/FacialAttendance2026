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
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

    DECLARE_MESSAGE_MAP()

private:
    int m_faceDetectionMode = 2; // 0:Haar, 1:Yunet, 2:Both
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

    void    UpdateFrame();
    HBITMAP CreateBitmapFromMat(const cv::Mat& mat);
    void UpdateIdentificationFields(CString name);

    // ★ 追加: CSVへの保存処理を抜き出した関数
    void SaveAttendanceToCsv(const CString& strTime, const CString& strName, const CString& strID, const CString& strComment);
    
    // ★追加: 計測データをCSVに吐き出してリセットする関数
    void FlushAndResetEvaluationData();

    // Screen Light モードをUIに反映し永続化する
    void    ApplyScreenLightMode(ScreenLightMode mode);
    void    ApplySliderValue(int value);
    static COLORREF SliderToColor(int value);

    // ★機能ごとに分割した初期化関数
    void InitializeCameraList();
    void InitializeWorkerThread();
    void InitializeFontsAndUI();
    void InitializeInputFields();
    void InitializeListControl();
    void InitializeScreenLightSettings();
    void InitializeMenuSettings();

    std::thread m_workerThread;
    std::atomic<bool> m_bStopThread{ false };
    std::atomic<bool> m_bShowWarning{ false };
    std::atomic<double> m_currentFps{ 0.0 };

    // ★追加: 評価(Evaluation)用データの収集用変数
    std::atomic<uint64_t> m_totalFrames{ 0 };
    std::atomic<double>   m_totalLatencyMs{ 0.0 };

    // === Worker Thread 抽出用ヘルパー関数 ===
    void ProcessCameraFrame(cv::Mat& inOutFrame, cv::Mat& outResizedFrame, cv::Mat& outDisplayFrame);
    bool PerformFaceDetection(cv::Mat& displayFrame, cv::Mat& resizedFrame, cv::Mat& outFaces);
    void UpdateFpsAndLatency(double fps);
    void UpdateScreenLightUsingFaces(const cv::Mat& resizedFrame, const cv::Mat& faces);

    // === UpdateFrame 描画抽出用ヘルパー関数 ===
    void CalculateDrawArea(int srcW, int srcH, int dstW, int dstH, int& drawX, int& drawW, int& drawH) const;
    void DrawFpsText(CDC& memDC, int x, int y);
    void DrawGuideFrame(CDC& memDC, int x, int y, int w, int h);
    void DrawWarningMessage(CDC& memDC, int x, int y, int w);

public:
    afx_msg void OnBnClickedClose();  // 既存のボタン用処理
    virtual void OnCancel();          // ← ★追加！ (×ボタンやESCキー用)
    afx_msg void OnFileExit();
    afx_msg void OnFileShowattendancelist();
    afx_msg void OnFacedetectionHaarcascades();
    afx_msg void OnFacedetectionYunet();
    afx_msg void OnFacedetectionBoth();
    afx_msg void OnUpdateFacedetectionHaarcascades(CCmdUI* pCmdUI);
    afx_msg void OnUpdateFacedetectionYunet(CCmdUI* pCmdUI);
    afx_msg void OnUpdateFacedetectionBoth(CCmdUI* pCmdUI);
    afx_msg void OnFacedetectionShowfolder();
};
