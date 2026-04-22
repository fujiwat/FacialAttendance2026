// FacialAttendance2026Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "FacialAttendance2026.h"
#include "FacialAttendance2026Dlg.h"
#include "resource.h"
#include "afxdialogex.h"

#include <Dshow.h>
#pragma comment(lib, "strmiids.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CFacialAttendance2026Dlg dialog


CFacialAttendance2026Dlg::CFacialAttendance2026Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FACIALATTENDANCE2026_DIALOG, pParent)
	, m_currentCameraIdx(0)
	, m_shouldChangeCamera(false)
	, m_timerId(0)
	, m_pvBits(nullptr)
	, m_bmpWidth(0)
	, m_bmpHeight(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CFacialAttendance2026Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CAMERA, m_comboCamera);
}

BEGIN_MESSAGE_MAP(CFacialAttendance2026Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO_CAMERA, &CFacialAttendance2026Dlg::OnCbnSelchangeComboCamera)
	ON_BN_CLICKED(IDCLOSE, &CFacialAttendance2026Dlg::OnBnClickedClose)
END_MESSAGE_MAP()

// --- ★ここから追加（Windowsからカメラ名を取得する関数） ---
std::vector<CString> GetCameraNames()
{
	std::vector<CString> cameraNames;

	// COMライブラリの初期化
	HRESULT hrCo = CoInitialize(NULL);

	ICreateDevEnum* pDevEnum = NULL;
	IEnumMoniker* pEnum = NULL;

	// デバイス列挙子の作成
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
	if (SUCCEEDED(hr)) {
		// ビデオ入力デバイス（カメラ）を指定
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
		if (hr == S_OK) {
			IMoniker* pMoniker = NULL;
			// 順番にカメラを調べる
			while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
				if (pMoniker == nullptr) continue;
				IPropertyBag* pPropBag;
#               pragma warning(suppress : 6387)
				hr = pMoniker->BindToStorage(nullptr, nullptr, IID_PPV_ARGS(&pPropBag));
				if (SUCCEEDED(hr)) {
					VARIANT varName;
					VariantInit(&varName);
					// 「FriendlyName（分かりやすい名前）」を取得
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
					if (SUCCEEDED(hr)) {
						cameraNames.push_back(CString(varName.bstrVal));
						VariantClear(&varName);
					}
					pPropBag->Release();
				}
				pMoniker->Release();
			}
			pEnum->Release();
		}
		pDevEnum->Release();
	}

	if (SUCCEEDED(hrCo)) {
		CoUninitialize();
	}

	return cameraNames;
}
// --- ★ここまで追加 ---


// CFacialAttendance2026Dlg message handlers

BOOL CFacialAttendance2026Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// ★ウィンドウを画面上部中央に配置
	CRect rectWindow, rectScreen;
	GetWindowRect(&rectWindow);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int windowWidth = rectWindow.Width();
	int windowHeight = rectWindow.Height();
	
	int x = (screenWidth - windowWidth) / 2;  // 左右中央
	int y = 0;  // 上端
	
	SetWindowPos(NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	// --- 追加の初期化処理（ここから） ---

	// 1. 利用可能なカメラの名前を取得してコンボボックスに追加
	std::vector<CString> camNames = GetCameraNames();
	for (int i = 0; i < static_cast<int>(camNames.size()); i++) {
		CString itemText;
		itemText.Format(_T("%d. %s"), i + 1, (LPCTSTR)camNames[i]);
		m_comboCamera.AddString(itemText);
	}

	// 2. カメラの初期選択状態を設定
	if (m_comboCamera.GetCount() > 0) {
		m_comboCamera.SetCurSel(0);
		m_currentCameraIdx = 0;
		// ★初回にカメラを開く
		m_shouldChangeCamera = true;
	}
	else {
		m_currentCameraIdx = -1; // カメラが見つからない場合
		m_shouldChangeCamera = false;
	}

	// 4. ワーカースレッドの開始（裏でカメラ映像を処理し続ける）
	// 4. ワーカースレッドの開始
	std::thread([this]() {
		cv::Mat frame;
		while (true) {
			if (!::IsWindow(GetSafeHwnd())) break;

			cv::VideoCapture& cap = m_detector.GetCapture();

			try {
				if (m_shouldChangeCamera) {
					{
						std::lock_guard<std::mutex> lock(m_frameMutex);
						m_lastFrame = cv::Mat(); // 切り替え時に一旦クリア
					}
					cap.release();
					cap.open(m_currentCameraIdx, cv::CAP_DSHOW);
					
					// ★デバッグ: カメラが開けたか確認
					if (cap.isOpened()) {
						cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
						cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
					} else {
//						OutputDebugString(_T("Failed to open camera\n"));
					}
					
					m_shouldChangeCamera = false;
				}

				// カメラから画像が読み込めるかチェック
				if (cap.isOpened()) {
					bool ret = cap.read(frame);
					if (!ret) {
//						OutputDebugString(_T("Failed to read frame\n"));
					}
					if (frame.empty()) {
//						OutputDebugString(_T("Frame is empty\n"));
					}
					
					if (ret && !frame.empty()) {
						cv::flip(frame, frame, 1);	// mirror mode

						// YuNet Facial Detection
						cv::Mat faces;
						m_detector.DetectFacesYunet(frame, faces);
						m_detector.DrawBoundingBoxesYunet(frame, faces);

						// 2. Haar Cascade Facial Detection
						std::vector<cv::Rect> haarRects;
						m_detector.DetectFacesHaar(frame, haarRects);
						m_detector.DrawBoundingBoxesHaar(frame, haarRects);

						// 3. FPS calculation and drawing on the frame
						double fps = m_fpsCounter.tick();
						cv::putText(frame, cv::format("FPS: %.1f", fps), cv::Point(8, 24),
							cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);

						// copy to shared variable
						{
							std::lock_guard<std::mutex> lock(m_frameMutex);
							m_lastFrame = frame.clone();
						}
						// すぐに描画する。
						PostMessage(WM_TIMER, 1, 0); // 画面更新の指示
						if (fps > 40.0) {
							// 40FPSを超えている時は、5ミリ秒だけ休憩してペースを落とす
							std::this_thread::sleep_for(std::chrono::milliseconds(5));
						}
					}
					else {
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}  // ★ try ブロックの終わり
			catch (...) {
				m_shouldChangeCamera = false;
				if (cap.isOpened()) cap.release();
				// ★ここで休憩！空回りでPCがフリーズ（CPU100%）するのを防ぐ
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}
	}).detach();

	// 5. 描画更新用のタイマー開始 (33ms間隔 ＝ 約30FPS)
	// m_timerId = SetTimer(1, 33, nullptr);

	//m_largeFont.CreatePointFont(140, _T("MS Shell Dlg"));
	//GetDlgItem(IDC_BUTTON_Photo_OK)->SetFont(&m_largeFont);
	//GetDlgItem(IDCLOSE)->SetFont(&m_largeFont);

	LOGFONT lf;
	GetFont()->GetLogFont(&lf);

	// 2. サイズを「18」に設定（共通）
	HDC hdc = ::GetDC(NULL);
	lf.lfHeight = -MulDiv(18, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	::ReleaseDC(NULL, hdc);
	_tcscpy_s(lf.lfFaceName, _T("Segoe UI"));

	// --- 3. 「太字」フォントの実体化 ---
	lf.lfWeight = FW_HEAVY; // 極太
	m_fontBold.DeleteObject();
	m_fontBold.CreateFontIndirect(&lf);

	// --- 4. 「標準」フォントの実体化 ---
	lf.lfWeight = FW_NORMAL; // 標準の太さに戻す
	m_fontRegular.DeleteObject();
	m_fontRegular.CreateFontIndirect(&lf);

	// 5. 各ボタンに適用
	// Photo OK ボタン（太字）
	if (GetDlgItem(IDC_BUTTON_Photo_OK)) {
		GetDlgItem(IDC_BUTTON_Photo_OK)->SetFont(&m_fontBold);
	}

	if (GetDlgItem(IDCLOSE)) {
		GetDlgItem(IDCLOSE)->SetFont(&m_fontRegular);
	}
	// --- 追加の初期化処理（ここまで） ---


	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}
void CFacialAttendance2026Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFacialAttendance2026Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFacialAttendance2026Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// --- ここから下をファイルの末尾に追加 ---

void CFacialAttendance2026Dlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1) {
		UpdateFrame(); // 画面更新
	}
	CDialogEx::OnTimer(nIDEvent);
}
void CFacialAttendance2026Dlg::UpdateFrame()
{
	std::lock_guard<std::mutex> lock(m_frameMutex);

	CWnd* pPreview = GetDlgItem(IDC_STATIC_PREVIEW);
	if (!pPreview) return;

	CClientDC dc(pPreview);  // ★GetDC() の代わりに CClientDC を使用
	CRect rect;
	pPreview->GetClientRect(&rect);

	// ★ダブルバッファリング用のメモリDCとビットマップを作成
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap backBuffer;
	backBuffer.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&backBuffer);

	if (m_lastFrame.empty()) {
		// 画面を黒い長方形で塗りつぶす
		memDC.FillSolidRect(&rect, RGB(0, 0, 0));
	}
	else {
		HBITMAP hBmp = CreateBitmapFromMat(m_lastFrame);
		if (hBmp) {
			CDC imageDC;
			imageDC.CreateCompatibleDC(&memDC);
			HBITMAP hOld = (HBITMAP)imageDC.SelectObject(hBmp);

			// 背景を塗りつぶさない（コメントアウト）
			// memDC.FillSolidRect(&rect, RGB(0, 0, 0));

			int srcW = m_lastFrame.cols;
			int srcH = m_lastFrame.rows;
			int dstW = rect.Width();
			int dstH = rect.Height();

			// 画像と枠の比率を計算
			double srcAspect = (double)srcW / srcH;
			double dstAspect = (double)dstW / dstH;

			int drawW = dstW;
			int drawH = dstH;
			int drawX = 0;
			int drawY = 0;  // 上部固定

			// 枠に合わせて最適なサイズと中央の描画位置を決定
			if (srcAspect > dstAspect) {
				// 画像の方が横長（上下に余白）
				drawH = (int)(dstW / srcAspect);
			}
			else {
				// 画像の方が縦長（左右に余白）
				drawW = (int)(dstH * srcAspect);
				drawX = (dstW - drawW) / 2;  // 左右は中央
			}

			// 縮小・拡大時の画質を少し良くする設定
			memDC.SetStretchBltMode(COLORONCOLOR);

			// ★メモリDCに描画（画面には直接描画しない）
			memDC.StretchBlt(drawX, drawY, drawW, drawH,
				&imageDC, 0, 0, srcW, srcH, SRCCOPY);

			imageDC.SelectObject(hOld);
			DeleteObject(hBmp);
		}
	}

	// ★完成したバックバッファを一気に画面に転送（これでちらつき解消！）
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBitmap);
	// ★ReleaseDC は不要（CClientDC がデストラクタで自動的に解放）
}

void CFacialAttendance2026Dlg::UpdateFrame0()
{
	std::lock_guard<std::mutex> lock(m_frameMutex);

	CWnd* pPreview = GetDlgItem(IDC_STATIC_PREVIEW);
	if (!pPreview) return;

	CDC* pDC = pPreview->GetDC();
	if (!pDC) return;

	CRect rect;
	pPreview->GetClientRect(&rect);

	if (m_lastFrame.empty()) {
		// 画面を黒い長方形で塗りつぶす
		pDC->FillSolidRect(&rect, RGB(0, 0, 0));
	}
	else {
		HBITMAP hBmp = CreateBitmapFromMat(m_lastFrame);
		if (hBmp) {
			CDC memDC;
			memDC.CreateCompatibleDC(pDC);
			HBITMAP hOld = (HBITMAP)memDC.SelectObject(hBmp);

			// --- ★ここから：縦横比を計算して正しく描画する処理 ---

			// 背景を一旦黒でクリア（黒帯になる部分）
			pDC->FillSolidRect(&rect, RGB(0, 0, 0));

			int srcW = m_lastFrame.cols;
			int srcH = m_lastFrame.rows;
			int dstW = rect.Width();
			int dstH = rect.Height();

			// 画像と枠の比率を計算
			double srcAspect = (double)srcW / srcH;
			double dstAspect = (double)dstW / dstH;

			int drawW = dstW;
			int drawH = dstH;
			int drawX = 0;
			int drawY = 0;

			// 枠に合わせて最適なサイズと中央の描画位置を決定
			if (srcAspect > dstAspect) {
				// 画像の方が横長（上下に黒帯）
				drawH = (int)(dstW / srcAspect);
				drawY = (dstH - drawH) / 2;
			}
			else {
				// 画像の方が縦長（左右に黒帯）
				drawW = (int)(dstH * srcAspect);
				drawX = (dstW - drawW) / 2;
			}

			// 縮小・拡大時の画質を少し良くする設定
			pDC->SetStretchBltMode(COLORONCOLOR);

			// 計算した位置・サイズで描画（比率が完璧に維持されます）
			pDC->StretchBlt(drawX, drawY, drawW, drawH,
				&memDC, 0, 0, srcW, srcH, SRCCOPY);

			// --- ★ここまで ---

			memDC.SelectObject(hOld);
			DeleteObject(hBmp);
		}
	}
	pPreview->ReleaseDC(pDC);
}

HBITMAP CFacialAttendance2026Dlg::CreateBitmapFromMat(const cv::Mat& mat)
{
	if (mat.empty()) return NULL;

	cv::Mat rgba;
	if (mat.channels() == 3) {
		cv::cvtColor(mat, rgba, cv::COLOR_BGR2BGRA);
	}
	else if (mat.channels() == 4) {
		rgba = mat;
	}
	else {
		cv::cvtColor(mat, rgba, cv::COLOR_GRAY2BGRA);
	}

	int w = rgba.cols;
	int h = rgba.rows;
	size_t bytes = static_cast<size_t>(w) * static_cast<size_t>(h) * 4;

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = -h; // トップダウン
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pvBits = nullptr; // メンバ変数ではなく、その都度ローカルで作る
	HDC hdc = ::GetDC(NULL);
	HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
	::ReleaseDC(NULL, hdc);

	if (hBmp && pvBits) {
		std::memcpy(pvBits, rgba.data, bytes);
	}

	return hBmp;
}

// ★この関数がまるごと存在するか確認！
void CFacialAttendance2026Dlg::OnCbnSelchangeComboCamera()
{
	int sel = m_comboCamera.GetCurSel();
	if (sel != LB_ERR) {
		m_currentCameraIdx = sel;
		m_shouldChangeCamera = true; // スレッド側に「切り替えて！」と合図を送る
	}
}
void CFacialAttendance2026Dlg::OnBnClickedClose()
{
	OnCancel();
}
