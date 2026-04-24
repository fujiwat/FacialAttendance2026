// FacialAttendance2026Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "FacialAttendance2026.h"
#include "FacialAttendance2026Dlg.h"
#include "resource.h"
#include "afxdialogex.h"
#include "MyConst.h"

#include <Dshow.h>
#pragma comment(lib, "strmiids.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ---------------------------------------------------------------------------
// CAboutDlg
// ---------------------------------------------------------------------------
class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();
#ifdef AFX_DESIGN_TIME
                enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// ---------------------------------------------------------------------------
// CFacialAttendance2026Dlg
// ---------------------------------------------------------------------------
CFacialAttendance2026Dlg::CFacialAttendance2026Dlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_FACIALATTENDANCE2026_DIALOG, pParent)
    , m_currentCameraIdx(0)
    , m_shouldChangeCamera(false)
    , m_timerId(0)
    , m_pvBits(nullptr)
    , m_bmpWidth(0)
    , m_bmpHeight(0)
    , m_screenLightMode(ScreenLightMode::Auto)
    , m_sliderValue(100)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CFacialAttendance2026Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CAMERA, m_comboCamera);
	DDX_Control(pDX, IDC_SLIDER_SL,    m_sliderScreenLight);  // ← 追加
}

BEGIN_MESSAGE_MAP(CFacialAttendance2026Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
	ON_WM_HSCROLL()
	ON_CBN_SELCHANGE(IDC_COMBO_CAMERA, &CFacialAttendance2026Dlg::OnCbnSelchangeComboCamera)
	ON_BN_CLICKED(IDCLOSE,             &CFacialAttendance2026Dlg::OnBnClickedClose)
	ON_MESSAGE(WM_UPDATE_SCREEN_LIGHT, &CFacialAttendance2026Dlg::OnUpdateScreenLight)
	ON_MESSAGE(WM_APP + 2,             &CFacialAttendance2026Dlg::OnInitScreenLight)
	ON_BN_CLICKED(IDC_RADIO_SL_SLIDER, &CFacialAttendance2026Dlg::OnRadioSlSlider)
	ON_BN_CLICKED(IDC_RADIO_SL_AUTO,   &CFacialAttendance2026Dlg::OnRadioSlAuto)
	ON_BN_CLICKED(IDC_RADIO_SL_NONE,   &CFacialAttendance2026Dlg::OnRadioSlNone)
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

	// 4. ワーカースレッドの開始
	std::thread t([this]() {
		cv::Mat frame;
		while (!m_bStopThread) {
			if (!::IsWindow(GetSafeHwnd())) break;

			cv::VideoCapture& cap = m_detector.GetCapture();

			try {
				// ★ドロップダウンから切り替えた時の処理がこれだけで済む！
				if (m_shouldChangeCamera) {
					{
						std::lock_guard<std::mutex> lock(m_frameMutex);
						m_lastFrame = cv::Mat(); // 切り替え時に一旦クリア
					}
					// FaceDetector内の安全なオープナーを呼ぶ
					m_detector.OpenCamera(m_currentCameraIdx);

					m_shouldChangeCamera = false;
				}

				// カメラから画像が読み込めるかチェック
				if (cap.isOpened()) {
					bool ret = cap.read(frame);
					
					if (ret && !frame.empty()) {
						cv::flip(frame, frame, 1);	// mirror mode

						// --- 安全なクロップ＆リサイズ処理 ---
						int srcW = frame.cols;
						int srcH = frame.rows;
						int dstW = m_detector.GetFrameWidth();
						int dstH = m_detector.GetFrameHeight();
						
						cv::Mat resizedFrame;

						// ゼロ除算防止と安全確認
						if (dstW > 0 && dstH > 0 && srcW > 0 && srcH > 0) {
							double srcAspect = (double)srcW / srcH;
							double dstAspect = (double)dstW / dstH;

							cv::Rect cropRect;
							if (srcAspect > dstAspect) {
								int newW = (int)(srcH * dstAspect);
								cropRect = cv::Rect((srcW - newW) / 2, 0, newW, srcH);
							}
							else {
								int newH = (int)(srcW / dstAspect);
								cropRect = cv::Rect(0, (srcH - newH) / 2, srcW, newH);
							}

							// 万が一計算結果枠がはみ出しても、安全な範囲に強制的に収める
							cropRect &= cv::Rect(0, 0, srcW, srcH);

							// クロップしてリサイズ
							if (cropRect.width > 0 && cropRect.height > 0) {
								cv::Mat croppedFrame = frame(cropRect);
								cv::resize(croppedFrame, resizedFrame, cv::Size(dstW, dstH));
							} else {
								resizedFrame = frame.clone();
							}
						} else {
							resizedFrame = frame.clone();
						}

						// YuNet Facial Detection
						cv::Mat faces;
						m_detector.DetectFacesYunet(resizedFrame, faces);
						m_detector.DrawBoundingBoxesYunet(resizedFrame, faces);

						// Haar Cascade Facial Detection
						std::vector<cv::Rect> haarRects;
						m_detector.DetectFacesHaar(resizedFrame, haarRects);
						m_detector.DrawBoundingBoxesHaar(resizedFrame, haarRects);

						// FPS calculation
						double fps = m_fpsCounter.tick();
						cv::putText(resizedFrame, cv::format("FPS: %.1f", fps), cv::Point(8, 24),
							cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);

						// copy to shared variable
						{
							std::lock_guard<std::mutex> lock(m_frameMutex);
							m_lastFrame = resizedFrame.clone(); 
						}

						// Adaptive Screen Light の更新
						auto mode = m_screenLightMode.load();
						if (mode != ScreenLightMode::None) {
							try {
								cv::Rect centerFaceRect;
								if (faces.rows > 0) {
									centerFaceRect = cv::Rect(
										static_cast<int>(faces.at<float>(0, 0)),
										static_cast<int>(faces.at<float>(0, 1)),
										static_cast<int>(faces.at<float>(0, 2)),
										static_cast<int>(faces.at<float>(0, 3))
									);
								}
								float manualB = (mode == ScreenLightMode::Slider) ? (m_sliderValue / 100.0f) : -1.0f;
								m_screenLight.Update(resizedFrame, centerFaceRect, manualB);
							}
							catch (...) {}
						}

						// 画面に描画する
						PostMessage(WM_TIMER, 1, 0);
						if (fps > 40.0) {
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
			}  // try ブロックの終わり
			catch (...) {
				m_shouldChangeCamera = false;
				if (cap.isOpened()) cap.release();
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		} // while (!m_bStopThread) の終わり
	}); // スレッド定義の終わり

	// 警告 C26444 を回避する
	m_workerThread = std::move(t);

	// 5. 描画更新用のタイマー
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
//	lf.lfWeight = FW_HEAVY; // 極太
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

	// Adaptive Screen Light を有効化（カメラ起動と同時にスタート）
//    //m_screenLight.SetEnabled(true, GetSafeHwnd());
    PostMessage(WM_APP + 2, 0, 0); // ダイアログ表示後に遅延実行

	// ── Screen Light 永続化された設定を復元 ──────────────────
	// 初期値: AUTO(2)、スライダー 100
	int savedMode   = AfxGetApp()->GetProfileInt(_T("ScreenLight"), _T("Mode"),   2);
	int savedSlider = AfxGetApp()->GetProfileInt(_T("ScreenLight"), _T("Slider"), 100);
	if (savedSlider < 0)   savedSlider = 0;
	if (savedSlider > 100) savedSlider = 100;

	m_sliderScreenLight.SetRange(0, 100, FALSE);
	m_sliderScreenLight.SetPos(savedSlider);
	m_sliderValue = savedSlider;

	CString slLabel;
	slLabel.Format(_T("%d%%"), savedSlider);
	SetDlgItemText(IDC_STATIC_SL_VALUE, slLabel);

	ScreenLightMode savedModeEnum = ScreenLightMode::Auto;
	switch (savedMode)
	{
	case 0: savedModeEnum = ScreenLightMode::None;   break;
	case 1: savedModeEnum = ScreenLightMode::Slider; break;
	default: savedModeEnum = ScreenLightMode::Auto;  break;
	}
	int radioId = IDC_RADIO_SL_AUTO;
	switch (savedModeEnum)
	{
	case ScreenLightMode::Slider: radioId = IDC_RADIO_SL_SLIDER; break;
	case ScreenLightMode::None:   radioId = IDC_RADIO_SL_NONE;   break;
	default:                      radioId = IDC_RADIO_SL_AUTO;   break;
	}
	CheckRadioButton(IDC_RADIO_SL_SLIDER, IDC_RADIO_SL_NONE, radioId);
	m_screenLightMode.store(savedModeEnum);
	// ────────────────────────────────────────────────────────
    return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}
void CFacialAttendance2026Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    UINT cmd = nID & 0xFFF0;
    if (cmd == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else if (cmd == SC_MINIMIZE) {
        m_screenLight.SetMinimized(true);
        CDialogEx::OnSysCommand(nID, lParam);
    }
    else if (cmd == SC_RESTORE) {
        CDialogEx::OnSysCommand(nID, lParam);
        m_screenLight.SetMinimized(false);
        SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    else {
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
		// 画面を黒い長方形ではなく、ダイアログの背景色で塗りつぶす
		memDC.FillSolidRect(&rect, GetSysColor(COLOR_3DFACE));
	}
	else {
		HBITMAP hBmp = CreateBitmapFromMat(m_lastFrame);
		if (hBmp) {
			CDC imageDC;
			imageDC.CreateCompatibleDC(&memDC);
			HBITMAP hOld = (HBITMAP)imageDC.SelectObject(hBmp);

			// ★ここを追加：画像を貼り付ける前に、背景をダイアログ色で塗っておく
			memDC.FillSolidRect(&rect, GetSysColor(COLOR_3DFACE));

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
		// 画面を黒い長方形ではなく、ダイアログの背景色で塗りつぶす
		pDC->FillSolidRect(&rect, GetSysColor(COLOR_3DFACE));
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
    // CLOSEボタンが押されたときも、×ボタンやESCと同じ終了ルートをたどらせる
    OnCancel();
}

void CFacialAttendance2026Dlg::OnCancel()
{
    // スレッドに終了を通知
    m_bStopThread = true;
    
    // スレッドが安全に終わるまで待機
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    // 本来のダイアログ終了処理（ここでウィンドウが閉じます）
    CDialogEx::OnCancel();
}

void CFacialAttendance2026Dlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    CDialogEx::OnActivate(nState, pWndOther, bMinimized);
    // Alt-Tab で戻ったときにメインダイアログを前面に維持する
    if (nState != WA_INACTIVE)
        SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

LRESULT CFacialAttendance2026Dlg::OnUpdateScreenLight(WPARAM wParam, LPARAM lParam)
{
    m_screenLight.ApplyColor(static_cast<COLORREF>(wParam));
    return 0;
}

LRESULT CFacialAttendance2026Dlg::OnInitScreenLight(WPARAM, LPARAM)
{
	// ダイアログ表示後に保存済みモードを実際に適用
	ApplyScreenLightMode(m_screenLightMode.load());
	return 0;
}

// ---------------------------------------------------------------------------
// Screen Light ヘルパー
// ---------------------------------------------------------------------------
/*static*/ COLORREF CFacialAttendance2026Dlg::SliderToColor(int value)
{
	BYTE v = static_cast<BYTE>(value * 255 / 100);
	return RGB(v, v, v);
}

void CFacialAttendance2026Dlg::ApplyScreenLightMode(ScreenLightMode mode)
{
	m_screenLightMode.store(mode);
	switch (mode)
	{
	case ScreenLightMode::Slider:
		m_screenLight.SetEnabled(true, GetSafeHwnd());
		m_screenLight.ApplyManualBrightness(m_sliderValue / 100.0f); // ← 変更
		break;
	case ScreenLightMode::Auto:
		m_screenLight.SetEnabled(true, GetSafeHwnd());
		break;
	case ScreenLightMode::None:
	default:
		m_screenLight.SetEnabled(false);
		break;
	}
	AfxGetApp()->WriteProfileInt(_T("ScreenLight"), _T("Mode"), static_cast<int>(mode));
}

void CFacialAttendance2026Dlg::ApplySliderValue(int value)
{
	m_sliderValue = value;
	CString label;
	label.Format(_T("%d%%"), value);
	SetDlgItemText(IDC_STATIC_SL_VALUE, label);
	if (m_screenLightMode.load() == ScreenLightMode::Slider)
		m_screenLight.ApplyManualBrightness(value / 100.0f); // ← 変更
	AfxGetApp()->WriteProfileInt(_T("ScreenLight"), _T("Slider"), value);
}

void CFacialAttendance2026Dlg::OnRadioSlSlider()
{
	ApplyScreenLightMode(ScreenLightMode::Slider);
}

void CFacialAttendance2026Dlg::OnRadioSlAuto()
{
	ApplyScreenLightMode(ScreenLightMode::Auto);
}

void CFacialAttendance2026Dlg::OnRadioSlNone()
{
	ApplyScreenLightMode(ScreenLightMode::None);
}

void CFacialAttendance2026Dlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar && pScrollBar->GetSafeHwnd() == m_sliderScreenLight.GetSafeHwnd())
	{
		// 自動的にSlider Valueモードへ切り替え
		if (m_screenLightMode.load() != ScreenLightMode::Slider)
		{
			CheckRadioButton(IDC_RADIO_SL_SLIDER, IDC_RADIO_SL_NONE, IDC_RADIO_SL_SLIDER);
			ApplyScreenLightMode(ScreenLightMode::Slider);
		}
		
		ApplySliderValue(m_sliderScreenLight.GetPos());
		return;
	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
