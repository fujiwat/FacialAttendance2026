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
	DDX_Control(pDX, IDC_SLIDER_SL,    m_sliderScreenLight);
	DDX_Control(pDX, IDC_COMBO_NAME, m_comboName);
	DDX_Control(pDX, IDC_EDIT_USER_ID, m_editID);
	DDX_Control(pDX, IDC_EDIT_TIME, m_editTime);
	DDX_Control(pDX, IDC_EDIT_COMMENT, m_editComment);
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
	ON_BN_CLICKED(IDC_BUTTON_PHOTO_OK, &CFacialAttendance2026Dlg::OnBnClickedButtonPhotoOk)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA_ON, &CFacialAttendance2026Dlg::OnBnClickedButtonCameraOn)
	ON_MESSAGE(WM_UPDATE_SCREEN_LIGHT, &CFacialAttendance2026Dlg::OnUpdateScreenLight)
	ON_MESSAGE(WM_APP + 2,             &CFacialAttendance2026Dlg::OnInitScreenLight)
	ON_BN_CLICKED(IDC_RADIO_SL_SLIDER, &CFacialAttendance2026Dlg::OnRadioSlSlider)
	ON_BN_CLICKED(IDC_RADIO_SL_AUTO,   &CFacialAttendance2026Dlg::OnRadioSlAuto)
	ON_BN_CLICKED(IDC_RADIO_SL_NONE,   &CFacialAttendance2026Dlg::OnRadioSlNone)
	ON_BN_CLICKED(IDC_BUTTON_CONFIRM,  &CFacialAttendance2026Dlg::OnBnClickedButtonConfirm)
	ON_COMMAND(ID_FILE_EXIT, &CFacialAttendance2026Dlg::OnFileExit)
	ON_COMMAND(ID_FILE_SHOWATTENDANCELIST, &CFacialAttendance2026Dlg::OnFileShowattendancelist)
	ON_COMMAND(ID_FACEDETECTION_HAARCASCADES, &CFacialAttendance2026Dlg::OnFacedetectionHaarcascades)
	ON_COMMAND(ID_FACEDETECTION_YUNET, &CFacialAttendance2026Dlg::OnFacedetectionYunet)
	ON_COMMAND(ID_FACEDETECTION_BOTH, &CFacialAttendance2026Dlg::OnFacedetectionBoth)
	ON_UPDATE_COMMAND_UI(ID_FACEDETECTION_HAARCASCADES, &CFacialAttendance2026Dlg::OnUpdateFacedetectionHaarcascades)
	ON_UPDATE_COMMAND_UI(ID_FACEDETECTION_YUNET, &CFacialAttendance2026Dlg::OnUpdateFacedetectionYunet)
	ON_UPDATE_COMMAND_UI(ID_FACEDETECTION_BOTH, &CFacialAttendance2026Dlg::OnUpdateFacedetectionBoth)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_FACEDETECTION_SHOWFOLDER, &CFacialAttendance2026Dlg::OnFacedetectionShowfolder)
	ON_BN_CLICKED(IDC_BUTTON_PHOTO_OK,  &CFacialAttendance2026Dlg::OnBnClickedButtonPhotoOk)
	ON_COMMAND(ID_FACEIDENTIFICATION_EIGENFACES, &CFacialAttendance2026Dlg::OnFaceidentificationEigenfaces)
	ON_COMMAND(ID_FACEIDENTIFICATION_LBPH, &CFacialAttendance2026Dlg::OnFaceidentificationLbph)
	ON_COMMAND(ID_FACEIDENTIFICATION_SFACE, &CFacialAttendance2026Dlg::OnFaceidentificationSface)
	ON_UPDATE_COMMAND_UI(ID_FACEIDENTIFICATION_EIGENFACES, &CFacialAttendance2026Dlg::OnUpdateFaceidentificationEigenfaces)
	ON_UPDATE_COMMAND_UI(ID_FACEIDENTIFICATION_LBPH, &CFacialAttendance2026Dlg::OnUpdateFaceidentificationLbph)
	ON_UPDATE_COMMAND_UI(ID_FACEIDENTIFICATION_SFACE, &CFacialAttendance2026Dlg::OnUpdateFaceidentificationSface)

	// ==========================================
	// ★追加: テキスト全選択のためのメッセージマップ
	// ==========================================
	ON_CBN_SETFOCUS(IDC_COMBO_NAME,   &CFacialAttendance2026Dlg::OnCbnSetfocusComboName)
	ON_EN_SETFOCUS(IDC_EDIT_USER_ID,  &CFacialAttendance2026Dlg::OnEnSetfocusEditId)
	ON_EN_SETFOCUS(IDC_EDIT_COMMENT,  &CFacialAttendance2026Dlg::OnEnSetfocusEditComment)

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

	// ウィンドウを画面上部中央に配置
	CRect rectWindow;
	GetWindowRect(&rectWindow);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int x = (screenWidth - rectWindow.Width()) / 2;
	SetWindowPos(NULL, x, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	// --- 初期化関数の呼び出し ---
	InitializeCameraList();
	InitializeWorkerThread();
	InitializeFontsAndUI();
	InitializeInputFields();
	InitializeListControl();
	InitializeScreenLightSettings();
	InitializeMenuSettings();

	// ★追加: Face Identifier の初期化
	// 現時点ではモデルファイルが存在しないため空文字を渡します
	m_identifier.Initialize("");

	// アプリ起動時はキャプチャを有効にし、バッファを空にしておく
	{
		std::lock_guard<std::mutex> lock(m_bufferMutex);
		m_faceRingBuffer.clear();
	}

	m_bCapturing = true;
	// ★ 念のため、スレッドに対して「起きろ」とシグナルも送っておく
	m_pauseCV.notify_one(); 

	// ボタンの初期状態の設定（Camera ON を無効、Photo OK を有効にする）
	GetDlgItem(IDC_BUTTON_CAMERA_ON)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_PHOTO_OK)->EnableWindow(TRUE);
	EnableInputFields(FALSE);		// ★追加: 起動時は入力操作パネルを無効（Disable）にする

	return TRUE;
}


void CFacialAttendance2026Dlg::EnableInputFields(BOOL bEnable)
{
	// 操作したい対象のコントロールIDを配列にまとめる
	const int targetIDs[] = {
		IDC_COMBO_NAME,
		IDC_EDIT_USER_ID,
		IDC_EDIT_TIME,
		IDC_EDIT_COMMENT,
		IDC_BUTTON_CONFIRM
	};

	for (int nID : targetIDs) {
		CWnd* pWnd = GetDlgItem(nID);
		if (pWnd && pWnd->GetSafeHwnd()) {
			pWnd->EnableWindow(bEnable);
		}
	}
}

void CFacialAttendance2026Dlg::InitializeCameraList()
{
	std::vector<CString> camNames = GetCameraNames();
	for (int i = 0; i < static_cast<int>(camNames.size()); i++) {
		CString itemText;
		itemText.Format(_T("%d. %s"), i + 1, (LPCTSTR)camNames[i]);
		m_comboCamera.AddString(itemText);
	}

	if (m_comboCamera.GetCount() > 0) {
		m_comboCamera.SetCurSel(0);
		m_currentCameraIdx = 0;
		// ★ スレッドではなく、起動時にここでカメラを初期化する
		m_detector.OpenCamera(m_currentCameraIdx); 
	}
	else {
		m_currentCameraIdx = -1;
	}
}

void CFacialAttendance2026Dlg::InitializeWorkerThread()
{
	std::thread t([this]() {
		cv::Mat frame;
		while (!m_bStopThread) {
			
			// --- ★ ここから追加：セマフォ(条件変数)による完全な待機 ---
			{
				std::unique_lock<std::mutex> lock(m_pauseMutex);
				// m_bCapturing が true になるか、終了フラグが立つまで「深い眠り」につく
				m_pauseCV.wait(lock, [this] { return m_bCapturing.load() || m_bStopThread.load(); });
			}
			// --- ★ ここまで追加 ---

			if (!::IsWindow(GetSafeHwnd())) break;

			cv::VideoCapture& cap = m_detector.GetCapture();

			try {

				if (cap.isOpened()) {
					bool ret = cap.read(frame);
					if (ret && !frame.empty()) {
						// ★追加: 残留バッファのフラッシュ（顔検出・描画・バッファ登録をすべてスキップ）
						if (m_discardFrames > 0) {
							m_discardFrames--;
							continue;
						}
						cv::flip(frame, frame, 1);  // mirror effect

						cv::Mat resizedFrame, displayFrame;
						ProcessCameraFrame(frame, resizedFrame, displayFrame);

						cv::Mat faces;
						std::vector<float> bestFaceData; // ★変更：配列を受け取る

						// ★変更
						bool faceDetected = PerformFaceDetection(displayFrame, resizedFrame, faces, bestFaceData);
						m_bShowWarning.store(!faceDetected);

						// キャプチャ動作中で、かつ顔が一つでも検出された場合
						if (m_bCapturing && faceDetected && !bestFaceData.empty()) {
							// ★スッキリ！処理を別関数に切り離し
							ProcessAndBufferDetectedFace(displayFrame, resizedFrame, bestFaceData);
						}
						// --- Ring Buffer への顔画像保存 ここまで ---

						double fps = m_fpsCounter.tick();
						m_currentFps.store(fps);
						UpdateFpsAndLatency(fps);

						{
							std::lock_guard<std::mutex> lock(m_frameMutex);
							m_lastFrame = displayFrame.clone();
						}

						UpdateScreenLightUsingFaces(resizedFrame, faces);

						PostMessage(WM_TIMER, 1, 0);
						if (fps > 40.0) std::this_thread::sleep_for(std::chrono::milliseconds(5));
					}
					else std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
				else std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			catch (...) {
				m_shouldChangeCamera = false;
				if (cap.isOpened()) cap.release();
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}
		});

	m_workerThread = std::move(t);
}

void CFacialAttendance2026Dlg::InitializeFontsAndUI()
{
	LOGFONT lf;
	GetFont()->GetLogFont(&lf);

	HDC hdc = ::GetDC(NULL);
	lf.lfHeight = -MulDiv(18, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	::ReleaseDC(NULL, hdc);
	_tcscpy_s(lf.lfFaceName, UI_FONT_NAME);

	lf.lfWeight = FW_HEAVY;
	m_fontBold.DeleteObject();
	m_fontBold.CreateFontIndirect(&lf);

	lf.lfWeight = FW_NORMAL;
	m_fontRegular.DeleteObject();
	m_fontRegular.CreateFontIndirect(&lf);

	if (GetDlgItem(IDC_BUTTON_CAMERA_ON)) GetDlgItem(IDC_BUTTON_CAMERA_ON)->SetFont(&m_fontRegular);
	if (GetDlgItem(IDC_BUTTON_PHOTO_OK))  GetDlgItem(IDC_BUTTON_PHOTO_OK)->SetFont(&m_fontBold);
	if (GetDlgItem(IDC_BUTTON_CONFIRM))   GetDlgItem(IDC_BUTTON_CONFIRM)->SetFont(&m_fontBold);
	if (GetDlgItem(IDCLOSE))              GetDlgItem(IDCLOSE)->SetFont(&m_fontRegular);

	m_comboName.SetFont(&m_fontRegular);
	m_editID.SetFont(&m_fontRegular);
	m_editTime.SetFont(&m_fontRegular);
	m_editComment.SetFont(&m_fontRegular);

	if (GetDlgItem(IDC_STATIC_NAME))      GetDlgItem(IDC_STATIC_NAME)->SetFont(&m_fontRegular);
	if (GetDlgItem(IDC_STATIC_ID))        GetDlgItem(IDC_STATIC_ID)->SetFont(&m_fontRegular);
	if (GetDlgItem(IDC_STATIC_TIME))      GetDlgItem(IDC_STATIC_TIME)->SetFont(&m_fontRegular);
	if (GetDlgItem(IDC_STATIC_COMMENT))   GetDlgItem(IDC_STATIC_COMMENT)->SetFont(&m_fontRegular);
	if (GetDlgItem(IDC_STATIC_ARROW1))    GetDlgItem(IDC_STATIC_ARROW1)->SetFont(&m_fontBold);
	if (GetDlgItem(IDC_STATIC_ATTENDEES)) GetDlgItem(IDC_STATIC_ATTENDEES)->SetFont(&m_fontRegular);

	SetDlgItemText(IDC_STATIC_FACE_SCORE, _T(""));
	SetDlgItemText(IDC_STATIC_FACE_WORST_SCORE, _T(""));
}

void CFacialAttendance2026Dlg::InitializeInputFields()
{
	LOGFONT lf;
	GetFont()->GetLogFont(&lf);

	// 1. IDC_COMBO_NAME と IDC_EDIT_COMMENT 用 (デフォルトフォント)
	static CFont s_fontDefaultField;
	s_fontDefaultField.DeleteObject();
	lf.lfHeight = UI_FIELD_HEIGHT;
	s_fontDefaultField.CreateFontIndirect(&lf);

	m_comboName.SetFont(&s_fontDefaultField);
	m_editComment.SetFont(&s_fontDefaultField);
	m_comboName.SetItemHeight(-1, UI_FIELD_HEIGHT);

	// 2. IDC_EDIT_USER_ID と IDC_EDIT_TIME 用 (Consolas)
	wcscpy_s(lf.lfFaceName, UI_FONT_NAME_FIXED);
	lf.lfHeight = UI_FIELD_HEIGHT;
	m_fontFixed.DeleteObject();
	m_fontFixed.CreateFontIndirect(&lf);

	m_editID.SetFont(&m_fontFixed);
	m_editTime.SetFont(&m_fontFixed);

	// スタイルとその他の設定
	m_editID.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
	m_editID.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	m_editComment.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
	m_editComment.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	m_comboName.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
	m_comboName.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	m_editID.SetLimitText(UI_ID_TEXT_MAX_LENGTH);
	m_editComment.SetLimitText(UI_COMMENT_TEXT_MAX_LENGTH);

	m_editTime.SetFormat(_T("HH:mm"));
}

void CFacialAttendance2026Dlg::InitializeListControl()
{
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(IDC_ATTENDEES_LIST);
	if (pListCtrl != nullptr) {
		// リスト用フォントの作成 (Tahoma, UI_LIST_FONT_SIZE)
		LOGFONT lf;
		GetFont()->GetLogFont(&lf);
		wcscpy_s(lf.lfFaceName, _T("Tahoma"));
		lf.lfHeight = UI_LIST_FONT_SIZE; // ※MyConst.h で定義を追加してください

		m_fontFixedList.DeleteObject();
		m_fontFixedList.CreateFontIndirect(&lf);

		pListCtrl->SetFont(&m_fontFixedList);

		// リストのスタイル設定
		pListCtrl->ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
		pListCtrl->SetExtendedStyle(pListCtrl->GetExtendedStyle() | LVS_EX_FULLROWSELECT);

		pListCtrl->InsertColumn(0, _T("TIME"), LVCFMT_LEFT, 45);
		pListCtrl->InsertColumn(1, _T("NAME"), LVCFMT_LEFT, 125);
		pListCtrl->InsertColumn(2, _T("ID"), LVCFMT_LEFT, 90);
	}
}
void CFacialAttendance2026Dlg::InitializeScreenLightSettings()
{
	PostMessage(WM_APP + 2, 0, 0);

	int savedMode = AfxGetApp()->GetProfileInt(_T("ScreenLight"), _T("Mode"), 2);
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
}

void CFacialAttendance2026Dlg::InitializeMenuSettings()
{
	m_faceDetectionMode = AfxGetApp()->GetProfileInt(REG_SECTION_SETTINGS, REG_KEY_FACE_DETECTION_MODE, 2);	// 0:Haar, 1:Yunet, 2:Both
	m_faceIdentificationMode = AfxGetApp()->GetProfileInt(REG_SECTION_SETTINGS, _T("FaceIdentificationMode"), 2);
}

void CFacialAttendance2026Dlg::UpdateIdentificationFields(CString name)
{
	// 1. Nameの設定
	if (name.CompareNoCase(_T("Unknown")) == 0) {
//		m_comboName.SetWindowText(_T("")); // Unknownならブランク
		m_comboName.SetWindowText(name);
	}
	else {
		m_comboName.SetWindowText(name);
		// ドロップダウンリストに未登録なら追加
		if (m_comboName.FindStringExact(-1, name) == CB_ERR) {
			m_comboName.AddString(name);
		}
	}

	// 2. 認識した時刻を表示
	CTime now = CTime::GetCurrentTime();
	
	// ★修正ここから: 文字列ではなく CTime を直接セットする
	m_editTime.SetTime(&now); 
	// ★修正ここまで

	// ★追加: テキストを更新したあと、強制的に画面の再描画をかける
	m_editTime.Invalidate(FALSE);
	m_editTime.UpdateWindow();

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

	CClientDC dc(pPreview);
	CRect rect;
	pPreview->GetClientRect(&rect);

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap backBuffer;
	backBuffer.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&backBuffer);

	if (m_lastFrame.empty()) {
		memDC.FillSolidRect(&rect, GetSysColor(COLOR_3DFACE));
	}
	else {
		HBITMAP hBmp = CreateBitmapFromMat(m_lastFrame);
		if (hBmp) {
			CDC imageDC;
			imageDC.CreateCompatibleDC(&memDC);
			HBITMAP hOld = (HBITMAP)imageDC.SelectObject(hBmp);

			memDC.FillSolidRect(&rect, GetSysColor(COLOR_3DFACE));

			int drawX = 0, drawW = rect.Width(), drawH = rect.Height();
			CalculateDrawArea(m_lastFrame.cols, m_lastFrame.rows, rect.Width(), rect.Height(), drawX, drawW, drawH);

			memDC.SetStretchBltMode(COLORONCOLOR);
			memDC.StretchBlt(drawX, 0, drawW, drawH, &imageDC, 0, 0, m_lastFrame.cols, m_lastFrame.rows, SRCCOPY);

			DrawFpsText(memDC, drawX, 0);
			DrawGuideFrame(memDC, drawX, 0, drawW, drawH);
			if (m_bShowWarning.load()) {
				DrawWarningMessage(memDC, drawX, 0, drawW);
			}

			imageDC.SelectObject(hOld);
			DeleteObject(hBmp);
		}
	}

	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBitmap);
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

void CFacialAttendance2026Dlg::OnCbnSelchangeComboCamera()
{
	int sel = m_comboCamera.GetCurSel();
	if (sel != LB_ERR) {
		m_currentCameraIdx = sel;
		
		// ★ スレッドを一旦完全に止める
		m_bStopThread = true;
		if (m_workerThread.joinable()) {
			m_workerThread.join();
		}
		
		// ★ メインスレッドでカメラを開き直す
		m_detector.OpenCamera(m_currentCameraIdx);
		
		// ★ 再びスレッドをスタートさせる (Photo OK状態でない場合のみ)
		if (m_bCapturing) {
			m_bStopThread = false;
			InitializeWorkerThread();
		}
	}
}
void CFacialAttendance2026Dlg::OnBnClickedClose()
{
    // CLOSEボタンが押されたときも、×ボタンやESCと同じ終了ルートをたどらせる
    OnCancel();
}

void CFacialAttendance2026Dlg::OnCancel()
{
    m_bStopThread = true;
    m_pauseCV.notify_all(); // ★ 寝ているスレッドを起こして終了させる
    
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    // ★終了時にここで１回保存する
    FlushAndResetEvaluationData();
    
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
// ヘルパー
// ---------------------------------------------------------------------------

void CFacialAttendance2026Dlg::ProcessCameraFrame(cv::Mat& inOutFrame, cv::Mat& outResizedFrame, cv::Mat& outDisplayFrame)
{
	int srcW = inOutFrame.cols;
	int srcH = inOutFrame.rows;
	int dstW = m_detector.GetFrameWidth();
	int dstH = m_detector.GetFrameHeight();

	if (dstW > 0 && dstH > 0 && srcW > 0 && srcH > 0) {
		double srcAspect = (double)srcW / (double)srcH;
		double dstAspect = (double)dstW / (double)dstH;
		cv::Rect cropRect;

		if (srcAspect > dstAspect) {
			int newW = (int)(srcH * dstAspect);
			cropRect = cv::Rect((srcW - newW) / 2, 0, newW, srcH);
		}
		else {
			int newH = (int)(srcW / dstAspect);
			cropRect = cv::Rect(0, (srcH - newH) / 2, srcW, newH);
		}

		cropRect &= cv::Rect(0, 0, srcW, srcH);
		if (cropRect.width > 0 && cropRect.height > 0) {
			cv::Mat croppedFrame = inOutFrame(cropRect);
			cv::resize(croppedFrame, outResizedFrame, cv::Size(dstW, dstH));
		}
		else {
			outResizedFrame = inOutFrame.clone();
		}
	}
	else {
		outResizedFrame = inOutFrame.clone();
	}

	cv::cvtColor(outResizedFrame, outDisplayFrame, cv::COLOR_BGR2GRAY);
	cv::cvtColor(outDisplayFrame, outDisplayFrame, cv::COLOR_GRAY2BGR);
}

bool CFacialAttendance2026Dlg::PerformFaceDetection(cv::Mat& displayFrame, cv::Mat& resizedFrame, cv::Mat& outFaces, std::vector<float>& outBestFaceData)
{
	int currentMode = m_faceDetectionMode;
	std::vector<cv::Rect> haarRects;
	bool faceDetected = false;

	outBestFaceData.clear(); // 初期化
	int minDistance2 = std::numeric_limits<int>::max();
	int cx = displayFrame.cols / 2;
	int cy = displayFrame.rows / 2;

	// Haar (モード 0 または 2)
	if (currentMode == 0 || currentMode == 2) {
		m_detector.DetectFacesHaar(displayFrame, haarRects);
		m_detector.DrawBoundingBoxesHaar(displayFrame, haarRects);
		if (!haarRects.empty()) {
			faceDetected = true;
			for (const auto& rect : haarRects) {
				int dx = (rect.x + rect.width / 2) - cx;
				int dy = (rect.y + rect.height / 2) - cy;
				int dist = dx * dx + dy * dy;
				if (dist < minDistance2) {
					minDistance2 = dist;
					
					// ★追加: 15要素の擬似ランドマークと枠を作成して代入
					outBestFaceData.assign(15, 0.0f);
					outBestFaceData[0] = static_cast<float>(rect.x);
					outBestFaceData[1] = static_cast<float>(rect.y);
					outBestFaceData[2] = static_cast<float>(rect.width);
					outBestFaceData[3] = static_cast<float>(rect.height);
					// 右目
					outBestFaceData[4] = rect.x + rect.width * 0.30f;
					outBestFaceData[5] = rect.y + rect.height * 0.45f;
					// 左目
					outBestFaceData[6] = rect.x + rect.width * 0.70f;
					outBestFaceData[7] = rect.y + rect.height * 0.45f;
					// 鼻先
					outBestFaceData[8] = rect.x + rect.width * 0.50f;
					outBestFaceData[9] = rect.y + rect.height * 0.65f;
					// 右口角
					outBestFaceData[10] = rect.x + rect.width * 0.35f;
					outBestFaceData[11] = rect.y + rect.height * 0.85f;
					// 左口角
					outBestFaceData[12] = rect.x + rect.width * 0.65f;
					outBestFaceData[13] = rect.y + rect.height * 0.85f;
					// 確度 (Haarはスコアがないので1.0固定)
					outBestFaceData[14] = 1.0f; 
				}
			}
		}
	}

	// YuNet (モード 1 または 2)
if (currentMode == 1 || currentMode == 2) {
		m_detector.DetectFacesYunet(resizedFrame, outFaces);
		m_detector.DrawBoundingBoxesYunet(displayFrame, outFaces);
		if (!outFaces.empty() && outFaces.rows > 0) {
			faceDetected = true;
			for (int i = 0; i < outFaces.rows; i++) {
				int x = int(outFaces.at<float>(i, 0));
				int y = int(outFaces.at<float>(i, 1));
				int w = int(outFaces.at<float>(i, 2));
				int h = int(outFaces.at<float>(i, 3));

				int dx = (x + w / 2) - cx;
				int dy = (y + h / 2) - cy;
				int dist = dx * dx + dy * dy;
				if (dist < minDistance2) {
					minDistance2 = dist;
					
					// ★追加: YuNetの出力をそのまま15要素のvectorへコピー
					outBestFaceData.assign(15, 0.0f);
					for (int j = 0; j < 15; j++) {
						outBestFaceData[j] = outFaces.at<float>(i, j);
					}
				}
			}
		}
	}

	// ガイド枠の描画
	int guideW = displayFrame.cols * 5 / 10;
	int guideH = displayFrame.rows * 8 / 10;
	int guideX = (displayFrame.cols - guideW) / 2;
	int guideY = (displayFrame.rows - guideH) / 2;
	if (guideX >= 0 && guideW > 0 && guideH > 0) {
		cv::rectangle(displayFrame, cv::Rect(guideX, guideY, guideW, guideH), cv::Scalar(0, 180, 200), 2);
	}

	return faceDetected;
}

void CFacialAttendance2026Dlg::ProcessAndBufferDetectedFace(const cv::Mat& displayFrame, const cv::Mat& resizedFrame, const std::vector<float>& bestFaceData)
{
    // ★追加: 15要素の配列から、これまで使っていた変数（RectとConfidence）を復元する
    cv::Rect targetFaceRect(
        static_cast<int>(bestFaceData[0]),
        static_cast<int>(bestFaceData[1]),
        static_cast<int>(bestFaceData[2]),
        static_cast<int>(bestFaceData[3])
    );
    double faceConfidence = static_cast<double>(bestFaceData[14]);

    if (targetFaceRect.width > 0 && targetFaceRect.height > 0) {
        // ---- これ以降は今まで通りの切り出し処理 ----
        int cx = targetFaceRect.x + targetFaceRect.width / 2;
        int cy = targetFaceRect.y + targetFaceRect.height / 2;
        int sideLength = std::max(targetFaceRect.width, targetFaceRect.height);

        // 枠が画面外にはみ出さないよう安全に計算
        int x = std::max(0, cx - sideLength / 2);
        int y = std::max(0, cy - sideLength / 2);
        int w = std::min(displayFrame.cols - x, sideLength);
        int h = std::min(displayFrame.rows - y, sideLength);

        // ここでもし画面端で正方形にならなかった場合、もう一度短い方に合わせて完全な正方形にする
        int finalSide = std::min(w, h);

        if (finalSide > 0) {
            // 表示とスコア計算用 (112x112) の単純切り抜き
            cv::Mat cropFace(resizedFrame, cv::Rect(x, y, finalSide, finalSide));
            cv::Mat simpleFace112;
            cv::resize(cropFace, simpleFace112, cv::Size(FACE_NORM_SIZE, FACE_NORM_SIZE));
            
            double sharpness = CalculateSharpness(simpleFace112);
            double contrast = CalculateContrast(simpleFace112);
            double totalScore = CalculateBestFaceScore(sharpness, faceConfidence, contrast);

            // ★ SFace回転補正用の "のりしろ" 付き軽量画像を生成してメモリ節約 ★
            int padSide = static_cast<int>(finalSide * 1.5); // 顔の1.5倍の領域をのりしろとして確保
            int padX = std::max(0, cx - padSide / 2);
            int padY = std::max(0, cy - padSide / 2);
            int padW = std::min(resizedFrame.cols - padX, padSide);
            int padH = std::min(resizedFrame.rows - padY, padSide);
            
            cv::Mat paddedCrop = resizedFrame(cv::Rect(padX, padY, padW, padH)).clone();

            // ランドマーク(目鼻口の座標)を、切り抜いた paddedCrop 基準にシフト(マイナス)する
            std::vector<float> shiftedFaceData = bestFaceData;
            shiftedFaceData[0] -= padX; // x
            shiftedFaceData[1] -= padY; // y
            for (int i = 4; i < 14; i += 2) {
                shiftedFaceData[i] -= padX;     // landmarks_x
                shiftedFaceData[i + 1] -= padY; // landmarks_y
            }

            FaceBufferItem item;
            item.face112 = simpleFace112;            // 画面表示・リストアイコン用の112画像
            item.rawFrame = paddedCrop;              // 画面全体ではなく「のりしろ付き顔領域」だけを保存
            item.faceData = shiftedFaceData;         // シフト補正済みの座標
            item.sharpness = sharpness;
            item.confidence = faceConfidence;
            item.contrast = contrast;
            item.totalScore = totalScore;
            // ロックして配列の末尾に追加する処理
            {
                std::lock_guard<std::mutex> lock(m_bufferMutex);
                m_faceRingBuffer.push_back(item);

                // 300個(MAX_FACE_RING_BUFFER)を超えたら一番古いものを消す
                if (m_faceRingBuffer.size() > MAX_FACE_RING_BUFFER) {
                    m_faceRingBuffer.pop_front();
                }
            }
        }
    }
}

void CFacialAttendance2026Dlg::UpdateFpsAndLatency(double fps)
{
	if (fps > 0.0) {
		m_totalFrames++;
		double frameMs = 1000.0 / fps;
		double expectedMs = m_totalLatencyMs.load();
		while (!m_totalLatencyMs.compare_exchange_weak(expectedMs, expectedMs + frameMs)) {
			// リトライ
		}
	}
}

void CFacialAttendance2026Dlg::UpdateScreenLightUsingFaces(const cv::Mat& resizedFrame, const cv::Mat& faces)
{
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
}

void CFacialAttendance2026Dlg::CalculateDrawArea(int srcW, int srcH, int dstW, int dstH, int& drawX, int& drawW, int& drawH) const
{
	double srcAspect = (double)srcW / (double)srcH;
	double dstAspect = (double)dstW / (double)dstH;
	if (srcAspect > dstAspect) {
		drawH = (int)(dstW / srcAspect);
	}
	else {
		drawW = (int)(dstH * srcAspect);
		drawX = (dstW - drawW) / 2;
	}
}

void CFacialAttendance2026Dlg::DrawFpsText(CDC& memDC, int x, int y)
{
	CString strFps;
	strFps.Format(_T("FPS: %.1f"), m_currentFps.load());
	CFont* pOldFont = memDC.SelectObject(&m_fontBold);
	memDC.SetTextColor(RGB(255, 255, 0));
	memDC.SetBkMode(TRANSPARENT);
	memDC.TextOut(x + 10, y + 10, strFps);
	memDC.SelectObject(pOldFont);
}

void CFacialAttendance2026Dlg::DrawGuideFrame(CDC& memDC, int x, int y, int w, int h)
{
	int guideW = w * 5 / 10;
	int guideH = h * 8 / 10;
	int guideX = x + (w - guideW) / 2;
	int guideY = y + (h - guideH) / 2;

	CRect guideRect(guideX, guideY, guideX + guideW, guideY + guideH);
	CPen yellowPen(PS_SOLID, 1, RGB(200, 180, 0));
	CPen* pOldPen = memDC.SelectObject(&yellowPen);
	CBrush* pOldBrush = (CBrush*)memDC.SelectStockObject(NULL_BRUSH);

	memDC.Rectangle(&guideRect);
	memDC.SelectObject(pOldPen);
	memDC.SelectObject(pOldBrush);
}

void CFacialAttendance2026Dlg::DrawWarningMessage(CDC& memDC, int x, int y, int w)
{
	CString warnMsg = _T("Don't turn your face.  Come to center.");
	CFont* pOldFont = memDC.SelectObject(&m_fontBold);

	CRect textRect;
	memDC.DrawText(warnMsg, &textRect, DT_CALCRECT);

	int tX = x + (w - textRect.Width()) / 2;
	int tY = y + 40;
	textRect.MoveToXY(tX, tY);

	CRect bgRect = textRect;
	bgRect.InflateRect(15, 10);
	memDC.FillSolidRect(&bgRect, RGB(0, 0, 0));

	CPen redPen(PS_SOLID, 2, RGB(255, 0, 0));
	CPen* pOldPen = memDC.SelectObject(&redPen);
	CBrush* pOldBrush = (CBrush*)memDC.SelectStockObject(NULL_BRUSH);

	memDC.Rectangle(&bgRect);
	memDC.SetTextColor(RGB(255, 0, 0));
	memDC.SetBkMode(TRANSPARENT);
	memDC.DrawText(warnMsg, &textRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	memDC.SelectObject(pOldPen);
	memDC.SelectObject(pOldBrush);
	memDC.SelectObject(pOldFont);
}

// ---------------------------------------------------------------------------
// Screen Light ヘルパー
// ---------------------------------------------------------------------------
COLORREF CFacialAttendance2026Dlg::SliderToColor(int value)
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

void CFacialAttendance2026Dlg::OnBnClickedButtonConfirm()
{
	CString strName, strID, strTime, strComment;

	// 1. 各フィールドからテキストを取得
	m_comboName.GetWindowText(strName);
	m_editID.GetWindowText(strID);
	m_editTime.GetWindowText(strTime); // ★追加
	m_editComment.GetWindowText(strComment); // コメントも取得

	// 左右の空白を除去
	strName.Trim();
	strID.Trim();
	strTime.Trim();

	// 2. 必須入力チェック
	if (strName.IsEmpty())
	{
		AfxMessageBox(_T("Name is required."));
		m_comboName.SetFocus();
		return;
	}

	if (strID.IsEmpty())
	{
		AfxMessageBox(_T("ID is required."));
		m_editID.SetFocus();
		return;
	}

	// 24時間形式 (HH:mm) のバリデーション
	strTime.Remove(_T('_')); // 入力途中のアンダースコアを取り除く
	if (strTime.GetLength() != 5 || strTime[2] != _T(':')) {
		AfxMessageBox(_T("Time must be in exactly HH:mm format."));
		m_editTime.SetFocus();
		return;
	}

	int h = _ttoi(strTime.Left(2));
	int m = _ttoi(strTime.Right(2));
	if (h < 0 || h >= 24 || m < 0 || m >= 60) {
		AfxMessageBox(_T("Invalid time. Valid range is 00:00 to 23:59."));
		m_editTime.SetFocus();
		return;
	}

	// --- 3. リストコントロール (IDC_ATTENDEES_LIST) への追加 ---
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(IDC_ATTENDEES_LIST);
	if (pListCtrl != nullptr) {
		// 先頭 (インデックス0) に新しい行を挿入する
		int rowIndex = pListCtrl->InsertItem(0, strTime); // 0列目: Time
		pListCtrl->SetItemText(rowIndex, 1, strName);     // 1列目: Name
		pListCtrl->SetItemText(rowIndex, 2, strID);       // 2列目: ID
	}

    // ★追加: ここでメモリ上の辞書(SFace識別器)に顔を追加登録する (Incremental Enrollment)
    // ただし、もし名前が空だったり "Unknown" 指定のままConfirmした場合は登録しない
    if (!m_bestItem.rawFrame.empty() && strName.CompareNoCase(_T("Unknown")) != 0) {
        // CString から std::string への変換 (Trim適用済のstrNameを使用)
        CT2CA pszConvertedAnsiString(strName);
        std::string stdName(pszConvertedAnsiString);
        
        FaceIdentificationMethod method = static_cast<FaceIdentificationMethod>(m_faceIdentificationMode);
        // ★ 登録(Enroll)時も、パディング付き画像と座標を渡して完璧な正面顔を作らせてから登録する
        m_identifier.Enroll(method, stdName, m_bestItem.rawFrame, m_bestItem.faceData);
    }

	// --- 4. CSVファイルへの書き込み (追記モード) ---
	SaveAttendanceToCsv(strTime, strName, strID, strComment);

	// 入力欄をクリアして次の人に備える
	m_comboName.SetWindowText(_T(""));
	m_editID.SetWindowText(_T("")),
	m_editComment.SetWindowText(_T(""));


    // ★追加: 確認完了したら自動的に次の人のキャプチャを再起動する（Camera ONボタンを押したのと同じ挙動）
    OnBnClickedButtonCameraOn();
}

void CFacialAttendance2026Dlg::SaveAttendanceToCsv(const CString& strTime, const CString& strName, const CString& strID, const CString& strComment)
{
	std::wstring csvPath = GetAttendanceCsvPath(); // MyFunctions.cpp で作成した関数
	
	// コメントのエスケープ処理
	CString escapedComment = strComment;
	escapedComment.Replace(_T("\""), _T("\"\""));
	escapedComment = _T("\"") + escapedComment + _T("\"");

	// ファイルが存在しない、もしくはサイズが0かどうかをチェックする
	bool isNewFile = true;
	FILE* checkFp = nullptr;
	if (_wfopen_s(&checkFp, csvPath.c_str(), L"rb") == 0 && checkFp != nullptr) {
		fseek(checkFp, 0, SEEK_END);
		if (ftell(checkFp) > 0) {
			isNewFile = false; // 既に中身があるなら新規ではない
		}
		fclose(checkFp);
	}

	// 追記モードのバイナリ + UTF-8 エンコーディング指定
	FILE* fp = nullptr;
	if (_wfopen_s(&fp, csvPath.c_str(), L"ab, ccs=UTF-8") == 0 && fp != nullptr) {
		
		// まったくの新規作成時の場合だけ、UTF-8のBOM (EF BB BF) を先頭に書き込む
		if (isNewFile) {
			unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
			fwrite(bom, 1, sizeof(bom), fp);

			// ヘッダー（見出し行）を出力
			fwprintf(fp, L"Time,Name,ID,Comment\n");
		}

		// CSV形式: Time, Name, ID, Comment
		fwprintf(fp, L"%ls,%ls,%ls,%ls\n", 
			(LPCTSTR)strTime, 
			(LPCTSTR)strName, 
			(LPCTSTR)strID, 
			(LPCTSTR)escapedComment);
			
		fclose(fp);
	}
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


void CFacialAttendance2026Dlg::OnFileExit()
{
	// 親クラスのものではなく、自クラス内でスレッド停止などのオーバーライド処理が書かれているOnCancel()を呼ぶ
	OnCancel(); 
}


void CFacialAttendance2026Dlg::OnFileShowattendancelist()
{
	// TODO: Add your command handler code here
}

// --- メニューのクリックイベント 兼 設定の保存 ---
void CFacialAttendance2026Dlg::OnFacedetectionHaarcascades()
{
	FlushAndResetEvaluationData(); // ★追加: 切り替える前に現在のデータを保存してリセット
	m_faceDetectionMode = 0;
	AfxGetApp()->WriteProfileInt(REG_SECTION_SETTINGS, REG_KEY_FACE_DETECTION_MODE, m_faceDetectionMode);
}
void CFacialAttendance2026Dlg::OnFacedetectionYunet()
{
	FlushAndResetEvaluationData(); // ★追加
	m_faceDetectionMode = 1;
	AfxGetApp()->WriteProfileInt(REG_SECTION_SETTINGS, REG_KEY_FACE_DETECTION_MODE, m_faceDetectionMode);
}
void CFacialAttendance2026Dlg::OnFacedetectionBoth()
{
	FlushAndResetEvaluationData(); // ★追加
	m_faceDetectionMode = 2;
	AfxGetApp()->WriteProfileInt(REG_SECTION_SETTINGS, REG_KEY_FACE_DETECTION_MODE, m_faceDetectionMode);
}

// --- メニューのチェックマークの更新 ---
void CFacialAttendance2026Dlg::OnUpdateFacedetectionHaarcascades(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_faceDetectionMode == 0);
}
void CFacialAttendance2026Dlg::OnUpdateFacedetectionYunet(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_faceDetectionMode == 1);
}
void CFacialAttendance2026Dlg::OnUpdateFacedetectionBoth(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_faceDetectionMode == 2);
}

// --- ダイアログでメニューのUPDATE_COMMAND_UIを動作させるためのおまじない ---
void CFacialAttendance2026Dlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialogEx::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	if (!bSysMenu && pPopupMenu != nullptr)
	{
		CCmdUI cmdUI;
		cmdUI.m_pMenu = pPopupMenu;
		cmdUI.m_nIndexMax = pPopupMenu->GetMenuItemCount();

		for (cmdUI.m_nIndex = 0; cmdUI.m_nIndex < cmdUI.m_nIndexMax; ++cmdUI.m_nIndex)
		{
			cmdUI.m_nID = pPopupMenu->GetMenuItemID(cmdUI.m_nIndex);
			// セパレーター（区切り線）等はスキップ
			if (cmdUI.m_nID == (UINT)-1 || cmdUI.m_nID == 0)
				continue;

			// 各メニュー項目に対して OnUpdate... 処理を強制的に呼び出す
			cmdUI.DoUpdate(this, FALSE);
		}
	}
}

// --- "Show Folder" メニューがクリックされた時の処理 ---
void CFacialAttendance2026Dlg::OnFacedetectionShowfolder()
{
	// 現在の計測値をCSVに書き込んでからリセットする
	FlushAndResetEvaluationData();

	// ★修正: Face Detection のレイテンシ用フォルダ名 (MyConst.h の定数) を渡す
	OpenEvaluationFolder(wFACE_DETECTION_LATENCY_FOLDER_NAME);
}

// ★追加: 溜まった計測データをCSVに書き込み、累計を0にリセットする
void CFacialAttendance2026Dlg::FlushAndResetEvaluationData()
{
    // スレッドが裏で動いている最中なので、一応アトミック変数の値をスナップショットで取得する
    uint64_t frames = m_totalFrames.load();
    if (frames > 0) {
        
        // ★修正: アルゴリズム単体の速度ではなく、全体の実際のループにかかった時間(FPSの逆数)を使う場合
        // （画面上で表示されているFPSの平均値に合わせる）        
        double totalLoopMs = m_totalLatencyMs.load(); // いったんそのまま
        
        // 平均を求める
        double avgLatencyMs = totalLoopMs / (double)frames;
        double equivalentFps = 0.0;
        if (avgLatencyMs > 0.0) {
            equivalentFps = 1000.0 / avgLatencyMs; // これまでの理論FPS
        }

        // CSVファイルへ保存：純粋な推論時間(avgLatencyMs)と、現実のFPSの平均を出す場合
        // もし現実のFPSを出したい場合は、fpsCounterの値を足し込むようにコードの計測位置をループの先頭に出す必要があります。
        
        SaveEvaluationLatencyCsv(m_faceDetectionMode, avgLatencyMs, equivalentFps);
        
        // ★ カウンターをリセットして、再起動したのと同じようにする
        m_totalFrames.store(0);
        m_totalLatencyMs.store(0.0);
    }
}

// --- ★ここから追加（リングバッファと顔評価・表示の処理） ---

// ブレ具合（鮮明度）を計算する関数 (Laplacian Varianceを利用)
double CFacialAttendance2026Dlg::CalculateSharpness(const cv::Mat& img)
{
    if (img.empty()) return 0.0;

    cv::Mat gray, laplacian;
    if (img.channels() == 3) {
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = img;
    }

    // ラプラシアンフィルタで輪郭（エッジ）を抽出
    cv::Laplacian(gray, laplacian, CV_64F);

    // 平均と標準偏差を計算
    cv::Scalar mu, sigma;
    cv::meanStdDev(laplacian, mu, sigma);

    // 分散（標準偏差の2乗）がSharpnessスコアになる（高いほどクッキリ）
    return sigma.val[0] * sigma.val[0];
}

// 濃淡（コントラスト）の計算
double CFacialAttendance2026Dlg::CalculateContrast(const cv::Mat& img)
{
	if (img.empty()) return 0.0;
	cv::Mat gray;
	if (img.channels() == 3) {
		cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
	}
	else {
		gray = img;
	}
	cv::Scalar mu, sigma;
	cv::meanStdDev(gray, mu, sigma);
	return sigma.val[0]; // 標準偏差をコントラストスコアとする
}

// 総合スコアの計算 (重み付け)
double CFacialAttendance2026Dlg::CalculateBestFaceScore(double sharpness, double faceConfidence, double contrast)
{
	// システムの環境に合わせて最大値の目安（分母）を調整してください
	double normSharpness = std::min(sharpness / 2000.0, 1.0);
	double normContrast = std::min(contrast / 80.0, 1.0);

	// HaarCascadeの場合 faceConfidence が1.0固定になるため、常に満点になります。
	return (faceConfidence * FACE_WEIGHT_CONFIDENCE) 
		+ (normSharpness * FACE_WEIGHT_SHARPNESS)
		+ (normContrast * FACE_WEIGHT_CONTRAST);
}

// 任意のStatic Controlに cv::Mat を描画する共通関数
void CFacialAttendance2026Dlg::DrawMatToStatic(int nID, const cv::Mat& mat)
{
	CWnd* pWnd = GetDlgItem(nID);
	if (!pWnd) return;

	CClientDC dc(pWnd);
	CRect rect;
	pWnd->GetClientRect(&rect);

	// ★先に背景色で塗りつぶす（これによって、古い写真が完全に消えます）
	dc.FillSolidRect(&rect, GetSysColor(COLOR_3DFACE));

	// 画像が空の場合は背景を塗りつぶしただけで終了
	if (mat.empty()) return;

	HBITMAP hBmp = CreateBitmapFromMat(mat);
	if (hBmp) {
		CDC memDC;
		memDC.CreateCompatibleDC(&dc);
		HBITMAP hOld = (HBITMAP)memDC.SelectObject(hBmp);

		// ★アスペクト比を維持して中央に配置するための計算
		int srcW = mat.cols;
		int srcH = mat.rows;
		int dstW = rect.Width();
		int dstH = rect.Height();

		int drawW = dstW;
		int drawH = dstH;
		int drawX = 0;
		int drawY = 0;

		double srcAspect = (double)srcW / (double)srcH;
		double dstAspect = (double)dstW / (double)dstH;

		if (srcAspect > dstAspect) {
			// 横長の画像の場合、上下に余白を作る
			drawH = (int)(dstW / srcAspect);
			drawY = (dstH - drawH) / 2;
		}
		else {
			// 縦長の画像の場合、左右に余白を作る
			drawW = (int)(dstH * srcAspect);
			drawX = (dstW - drawW) / 2;
		}

		// キレイに縮小・拡大して中央に描画
		dc.SetStretchBltMode(COLORONCOLOR);
		dc.StretchBlt(drawX, drawY, drawW, drawH,
			&memDC, 0, 0, srcW, srcH, SRCCOPY);

		memDC.SelectObject(hOld);
		DeleteObject(hBmp);
	}
}

void CFacialAttendance2026Dlg::OnBnClickedButtonPhotoOk()
{
    m_bCapturing = false; // キャプチャ停止

    GetDlgItem(IDC_BUTTON_CAMERA_ON)->EnableWindow(TRUE); 
	EnableInputFields(TRUE);

	SetDefID(IDC_BUTTON_CONFIRM);
	GetDlgItem(IDC_BUTTON_PHOTO_OK)->EnableWindow(FALSE); // GotoDlgCtrlはここから移動しました

    std::lock_guard<std::mutex> lock(m_bufferMutex);
    
    // 表示のクリア
    cv::Mat emptyMat;
    DrawMatToStatic(IDC_STATIC_FACE, emptyMat);
    DrawMatToStatic(IDC_STATIC_FACE_WORST, emptyMat);
    SetDlgItemText(IDC_STATIC_FACE_SCORE, _T(""));
	SetDlgItemText(IDC_STATIC_FACE_WORST_SCORE, _T(""));

    if (m_faceRingBuffer.empty()) {
        MyMessageBoxW(GetSafeHwnd(), MB_OK | MB_ICONINFORMATION, g_wAppNameLong, L"顔が検出されていません。");
        return;
    }

	double maxScore = -1.0;
	double minScore = 9999999.0;
	cv::Mat bestFace;
	cv::Mat worstFace;
    
	for (const auto& item : m_faceRingBuffer) {
		if (item.totalScore > maxScore) {
			maxScore = item.totalScore;
			bestFace = item.face112;
            m_bestItem = item; // 評価がベストのものを保持
		}
		if (item.totalScore < minScore) {
			minScore = item.totalScore;
			worstFace = item.face112;
		}
	}

	// ベストショットを描画してスコアを表示
	if (!bestFace.empty()) {
		DrawMatToStatic(IDC_STATIC_FACE, bestFace);
		CString strBestScore;
		strBestScore.Format(_T("%.2f"), maxScore * 100.0);
		SetDlgItemText(IDC_STATIC_FACE_SCORE, strBestScore);

        FaceIdentificationMethod method = static_cast<FaceIdentificationMethod>(m_faceIdentificationMode);
        
        // 識別器に画像の登録情報を渡して識別
        IdentificationResult result = m_identifier.Identify(method, m_bestItem.rawFrame, m_bestItem.faceData);

        CString strName(result.name.c_str());
        UpdateIdentificationFields(strName);
	}

	// ワーストショットを描画してスコアを表示
	if (!worstFace.empty()) {
		DrawMatToStatic(IDC_STATIC_FACE_WORST, worstFace);
		CString strWorstScore;
		strWorstScore.Format(_T("%.2f"), minScore * 100.0);
		SetDlgItemText(IDC_STATIC_FACE_WORST_SCORE, strWorstScore);
	}

    // ★修正: すべて終わった、一番最後のタイミングで名前欄にフォーカス！
    GotoDlgCtrl(GetDlgItem(IDC_COMBO_NAME));
}

// --- Camera ON ボタンの処理 ---
void CFacialAttendance2026Dlg::OnBnClickedButtonCameraOn()
{
	// 1. Ring buffer をクリア
	{
		std::lock_guard<std::mutex> lock(m_bufferMutex);
		m_faceRingBuffer.clear();
	}
	EnableInputFields(FALSE);

	// ① まず Photo OK ボタンを「有効」にする
	GetDlgItem(IDC_BUTTON_PHOTO_OK)->EnableWindow(TRUE);

	// ② 強力にデフォルトボタンを指定（MFCのSetDefIDだけでなく、Windows APIレベルで確実に設定）
	SendMessage(DM_SETDEFID, IDC_BUTTON_PHOTO_OK, 0);

	// ③ 有効になった Photo OK ボタンへ確実にフォーカスをジャンプさせる
	GetDlgItem(IDC_BUTTON_PHOTO_OK)->SetFocus();

	// ④ その後で不要になった Camera ON ボタンを「無効」にする
	GetDlgItem(IDC_BUTTON_CAMERA_ON)->EnableWindow(FALSE);


    // 2. 右側（ベスト／ワースト写真）の表示をグレーに戻したい場合はここで消去
    cv::Mat emptyMat;
    DrawMatToStatic(IDC_STATIC_FACE, emptyMat);
    DrawMatToStatic(IDC_STATIC_FACE_WORST, emptyMat);

    // ★ テキストのスコアも一緒に消去してリセットする
	SetDlgItemText(IDC_STATIC_FACE_SCORE, _T(""));
	SetDlgItemText(IDC_STATIC_FACE_WORST_SCORE, _T("[未認識]"));

    // 3. フラグを true に戻し、眠っているスレッドに「起きろ」とシグナルを送信する
    m_bCapturing = true;
    m_pauseCV.notify_one(); 
} // --- 関数終了 ---

// --- Face Identification モード切り替え処理 ---
void CFacialAttendance2026Dlg::OnFaceidentificationEigenfaces()
{
	m_faceIdentificationMode = 0;
	AfxGetApp()->WriteProfileInt(REG_SECTION_SETTINGS, _T("FaceIdentificationMode"), m_faceIdentificationMode);
}

void CFacialAttendance2026Dlg::OnFaceidentificationLbph()
{
	m_faceIdentificationMode = 1;
	AfxGetApp()->WriteProfileInt(REG_SECTION_SETTINGS, _T("FaceIdentificationMode"), m_faceIdentificationMode);
}

void CFacialAttendance2026Dlg::OnFaceidentificationSface()
{
	m_faceIdentificationMode = 2;
	AfxGetApp()->WriteProfileInt(REG_SECTION_SETTINGS, _T("FaceIdentificationMode"), m_faceIdentificationMode);
}

// --- Face Identification チェックマーク更新処理 ---
void CFacialAttendance2026Dlg::OnUpdateFaceidentificationEigenfaces(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_faceIdentificationMode == 0); // ラジオボタンスタイルであれば SetRadio を推奨
}

void CFacialAttendance2026Dlg::OnUpdateFaceidentificationLbph(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_faceIdentificationMode == 1);
}

void CFacialAttendance2026Dlg::OnUpdateFaceidentificationSface(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_faceIdentificationMode == 2);
}

// --- Face Identification のメニュー更新処理 終わり ---

// ===========================================================================
// ★追加: フォーカスが当たったときに「全選択」状態（ハイライト）にする今風のUI処理
// ===========================================================================

void CFacialAttendance2026Dlg::OnCbnSetfocusComboName()
{
    // Name欄のテキストをすべて選択状態にする
    m_comboName.SetEditSel(0, -1);
}

void CFacialAttendance2026Dlg::OnEnSetfocusEditId()
{
    // ID欄のテキストをすべて選択状態にする
    m_editID.SetSel(0, -1, TRUE);
}

void CFacialAttendance2026Dlg::OnEnSetfocusEditComment()
{
    // コメント欄のテキストをすべて選択状態にする
    m_editComment.SetSel(0, -1, TRUE);
}
