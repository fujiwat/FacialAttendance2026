
// FacialAttendance2026Dlg.h : header file
//

#pragma once
#include "FaceDetector.h"
#include <mutex>


// CFacialAttendance2026Dlg dialog
class CFacialAttendance2026Dlg : public CDialogEx
{
// Construction
public:
	CFacialAttendance2026Dlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FACIALATTENDANCE2026_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


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
	DECLARE_MESSAGE_MAP()

private:
	CComboBox m_comboCamera;    // コンボボックス操作用
	int m_currentCameraIdx;     // 現在使用中のカメラ番号
	bool m_shouldChangeCamera;  // カメラ切り替えフラグ

	FaceDetector m_detector;      // 顔検出・カメラ操作
	std::mutex m_frameMutex;      // スレッド間のデータ保護
	cv::Mat m_lastFrame;          // 最新の画像データ
	UINT_PTR m_timerId;           // タイマーID
	CFont m_fontBold;
	CFont m_fontRegular;
	FpsCounter m_fpsCounter;

	// 描画用の変数
	void* m_pvBits;
	int m_bmpWidth;
	int m_bmpHeight;

	// 描画用の補助関数
	void UpdateFrame0();// debug
	void UpdateFrame();
	HBITMAP CreateBitmapFromMat(const cv::Mat& mat);
public:
	afx_msg void OnBnClickedClose();
};
