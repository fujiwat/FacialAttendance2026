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
	ON_MESSAGE(WM_UPDATE_SCREEN_LIGHT, &CFacialAttendance2026Dlg::OnUpdateScreenLight)
	ON_MESSAGE(WM_APP + 2,             &CFacialAttendance2026Dlg::OnInitScreenLight)
	ON_BN_CLICKED(IDC_RADIO_SL_SLIDER, &CFacialAttendance2026Dlg::OnRadioSlSlider)
	ON_BN_CLICKED(IDC_RADIO_SL_AUTO,   &CFacialAttendance2026Dlg::OnRadioSlAuto)
	ON_BN_CLICKED(IDC_RADIO_SL_NONE,   &CFacialAttendance2026Dlg::OnRadioSlNone)
	ON_BN_CLICKED(IDC_BUTTON_CONFIRM,  &CFacialAttendance2026Dlg::OnBnClickedButtonConfirm)
	ON_COMMAND(ID_FILE_EXIT, &CFacialAttendance2026Dlg::OnFileExit)
	ON_COMMAND(ID_FILE_SHOWATTENDANCELIST, &CFacialAttendance2026Dlg::OnFileShowattendancelist)
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

							cropRect &= cv::Rect(0, 0, srcW, srcH); // 安全な範囲に収める

							if (cropRect.width > 0 && cropRect.height > 0) {
								cv::Mat croppedFrame = frame(cropRect);
								cv::resize(croppedFrame, resizedFrame, cv::Size(dstW, dstH));
							} else {
								resizedFrame = frame.clone();
							}
						} else {
							resizedFrame = frame.clone();
						}

						// =======================================================
						// 1. 表示用のモノクロキャンバスを作る
						// =======================================================
						cv::Mat displayFrame;
						cv::cvtColor(resizedFrame, displayFrame, cv::COLOR_BGR2GRAY);
						cv::cvtColor(displayFrame, displayFrame, cv::COLOR_GRAY2BGR);

						// =======================================================
						// 2. 顔検出処理 (元のカラー画像で行う)
						// =======================================================
						cv::Mat faces;
						m_detector.DetectFacesYunet(resizedFrame, faces);
						m_detector.DrawBoundingBoxesYunet(displayFrame, faces); // 描画はモノクロの方へ

						std::vector<cv::Rect> haarRects;
						m_detector.DetectFacesHaar(resizedFrame, haarRects);
						m_detector.DrawBoundingBoxesHaar(displayFrame, haarRects); // 描画はモノクロの方へ

						// =======================================================
						// 3. 中央に固定の黄色いガイド枠を描画
						// =======================================================
						int guideW = displayFrame.cols * 5 / 10;
						int guideH = displayFrame.rows * 8 / 10;
						int guideX = (displayFrame.cols - guideW) / 2;
						int guideY = (displayFrame.rows - guideH) / 2;
						
						// 画面内に収まる場合のみ描画
						if (guideX >= 0 && guideW > 0 && guideH > 0) {
							cv::rectangle(displayFrame, cv::Rect(guideX, guideY, guideW, guideH), cv::Scalar(0, 180, 200), 2);
						}

						// =======================================================
						// 4. 顔が検出されなかった場合の警告状態をセット
						// =======================================================
						bool faceDetected = false;
						if (faces.rows > 0 || !haarRects.empty()) {
							faceDetected = true;
						}

						// ここでは画像に文字を書かず、UIスレッドに状態を伝えるだけ
						m_bShowWarning.store(!faceDetected);

						// =======================================================
						// 5. FPS計算・画面更新・ScreenLight処理
						// =======================================================
						double fps = m_fpsCounter.tick();
						// ★ OpenCVでのテキスト描画をやめ、UI側（GDI）へ値を渡す
						m_currentFps.store(fps);

						// 画面表示用の変数には、全てを描画し終わった displayFrame を渡す
						{
							std::lock_guard<std::mutex> lock(m_frameMutex);
							m_lastFrame = displayFrame.clone(); 
						}

						// Adaptive Screen Light の更新(元のカラー映像を使用)
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
	lf.lfWeight = FW_HEAVY; 
	m_fontBold.DeleteObject();
	m_fontBold.CreateFontIndirect(&lf);

	// --- 4. 「標準」フォントの実体化 ---
	lf.lfWeight = FW_NORMAL; // 標準の太さに戻す
	m_fontRegular.DeleteObject();
	m_fontRegular.CreateFontIndirect(&lf);


	// 5. 各ボタンに適用
	// Photo OK ボタン（太字）
	if (GetDlgItem(IDC_BUTTON_Camera_ON)) {
		GetDlgItem(IDC_BUTTON_Camera_ON)->SetFont(&m_fontRegular);
	}
	if (GetDlgItem(IDC_BUTTON_Photo_OK)) {
		GetDlgItem(IDC_BUTTON_Photo_OK)->SetFont(&m_fontBold);
	}

	if (GetDlgItem(IDC_BUTTON_CONFIRM)) {
		GetDlgItem(IDC_BUTTON_CONFIRM)->SetFont(&m_fontBold);
	}

	if (GetDlgItem(IDCLOSE)) {
		GetDlgItem(IDCLOSE)->SetFont(&m_fontRegular);
	}

	// ★ここから追加: 各入力フィールド（コントロール変数）に大きなフォント（m_fontRegular）を適用
	m_comboName.SetFont(&m_fontRegular);
	m_editID.SetFont(&m_fontRegular);
	m_editTime.SetFont(&m_fontRegular);
	m_editComment.SetFont(&m_fontRegular);

	// ★ここから追加: 各ラベル（スタティックテキスト）に大きなフォントを適用
	// ※ IDC_STATIC_NAME などの専用IDがない場合を考慮し、コントロールIDを指定して適用します。
	// resource.h に振られているであろう一般的なラベルID（ここではGetDlgItemで取れるか確認します）
	if (GetDlgItem(IDC_STATIC_NAME))    GetDlgItem(IDC_STATIC_NAME)->SetFont(&m_fontRegular);
	if (GetDlgItem(IDC_STATIC_ID))      GetDlgItem(IDC_STATIC_ID)->SetFont(&m_fontRegular);
	if (GetDlgItem(IDC_STATIC_TIME))    GetDlgItem(IDC_STATIC_TIME)->SetFont(&m_fontRegular);
	if (GetDlgItem(IDC_STATIC_COMMENT)) GetDlgItem(IDC_STATIC_COMMENT)->SetFont(&m_fontRegular);
	if (GetDlgItem(IDC_STATIC_ARROW1)) GetDlgItem(IDC_STATIC_ARROW1)->SetFont(&m_fontBold);
	if (GetDlgItem(IDC_STATIC_ATTENDEES)) GetDlgItem(IDC_STATIC_ATTENDEES)->SetFont(&m_fontRegular);

	// ★ここから追加: 入力フィールドの高さをUI_FIELD_HEIGHTピクセルに設定
	m_comboName.SetItemHeight(-1, UI_FIELD_HEIGHT);
	//int newHeight = UI_FIELD_HEIGHT;
	//CRect rectID, rectTime;
	//m_editID.GetWindowRect(&rectID);
	//m_editID.SetWindowPos(nullptr, 0, 0, rectID.Width(), newHeight, SWP_NOMOVE | SWP_NOSIZE);
	//m_editTime.GetWindowRect(&rectTime);
	//m_editTime.SetWindowPos(nullptr, 0, 0, rectTime.Width(), newHeight, SWP_NOMOVE | SWP_NOSIZE);
	// --- IDフィールドに固定幅フォントを設定 ---
	GetFont()->GetLogFont(&lf); // 現在のダイアログのフォント設定をベースにする
	wcscpy_s(lf.lfFaceName, _T("Tahoma"));
	lf.lfHeight = UI_FIELD_HEIGHT;
	m_fontFixed.CreateFontIndirect(&lf);         // フォントを作成

	// ===== ID / Time / リストコントロール 用の等幅フォント設定 =====

	GetFont()->GetLogFont(&lf); // 現在のダイアログのフォント設定をベースにする
	wcscpy_s(lf.lfFaceName, _T("Tahoma"));

	// --- 修正箇所: CreateFontIndirect の前に DeleteObject() を呼ぶ ---
	
	// リストコントロール用
	m_fontFixedList.DeleteObject();          // <--- これを追加
	m_fontFixedList.CreateFontIndirect(&lf); 

	// ID / Time フィールド用
	lf.lfHeight = UI_FIELD_HEIGHT;
	m_fontFixed.DeleteObject();              // <--- これを追加
	m_fontFixed.CreateFontIndirect(&lf);     

	// フィールドへの適用
	m_editID.SetFont(&m_fontFixed);          
	m_editTime.SetFont(&m_fontFixed);        


	// m_editID (ID)
	m_editID.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
	m_editID.SetWindowPos(NULL, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	// m_editComment (Comment)
	m_editComment.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
	m_editComment.SetWindowPos(NULL, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	// Name コンボボックスのテキスト部分にも念のため枠線スタイルを追加
	m_comboName.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
	m_comboName.SetWindowPos(NULL, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	// ------------

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
	m_editID.SetLimitText(UI_ID_TEXT_MAX_LENGTH);       // IDは8桁
	m_editComment.SetLimitText(UI_COMMENT_TEXT_MAX_LENGTH); // コメントは200文字

	// ★追加: Timeフィールドのロック強制解除と「数字:数字」のマスク設定
	// （※ここにあった m_editTime.SetReadOnly と m_editTime.EnableMask は削除）
	
	// 「時:分」形式 (例: 14:30) に設定する
   m_editTime.SetFormat(_T("HH:mm"));

	// --- IDC_ATTENDEES_LIST の初期化 ---
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(IDC_ATTENDEES_LIST);
	if (pListCtrl != nullptr) {
		// リスト用等幅フォント(デフォルトサイズ)を適用
		pListCtrl->SetFont(&m_fontFixedList); 

		pListCtrl->ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
		pListCtrl->SetExtendedStyle(pListCtrl->GetExtendedStyle() | LVS_EX_FULLROWSELECT);

		// 列幅は画像に合わせて少し小さめに設定（例: 45, 125, 90）
		pListCtrl->InsertColumn(0, _T("Time"), LVCFMT_LEFT, 45);
		pListCtrl->InsertColumn(1, _T("Name"), LVCFMT_LEFT, 125);
		pListCtrl->InsertColumn(2, _T("ID"),   LVCFMT_LEFT, 90);

	}
	// --- 追加ここまで ---

    return TRUE;  // フォーカスをコントロールに設定した場合を除きTRUEを返します。
}

void CFacialAttendance2026Dlg::UpdateIdentificationFields(CString name)
{
	// 1. Nameの設定
	if (name.CompareNoCase(_T("Unknown")) == 0) {
		m_comboName.SetWindowText(_T("")); // Unknownならブランク
	}
	else {
		m_comboName.SetWindowText(name);
		// ドロップダウンリストに未登録なら追加
		if (m_comboName.FindStringExact(-1, name) == CB_ERR) {
			m_comboName.AddString(name);
		}
	}

	// 2. IDは常にブランクで初期化
	m_editID.SetWindowText(_T(""));

	// 3. 認識した時刻を表示 (★秒を消して HH:mm にする)
	CTime now = CTime::GetCurrentTime();
	m_editTime.SetWindowText(now.Format(_T("%H:%M")));

	// 4. Commentはクリア
	m_editComment.SetWindowText(_T(""));
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

			// =======================================================
			// 1. GDIを使った高画質フォントのFPS描画 (一番下)
			// =======================================================
			{
				CString strFps;
				strFps.Format(_T("FPS: %.1f"), m_currentFps.load());
				
				CFont* pOldFont = memDC.SelectObject(&m_fontBold);
				
				memDC.SetTextColor(RGB(255, 255, 0));
				memDC.SetBkMode(TRANSPARENT);
				memDC.TextOut(drawX + 10, drawY + 10, strFps);
				
				memDC.SelectObject(pOldFont);
			}

			// =======================================================
			// 2. GDIを使った黄色いガイド枠の描画 (中間)
			// =======================================================
			{
				int guideW = drawW * 5 / 10;
				int guideH = drawH * 8 / 10;
				int guideX = drawX + (drawW - guideW) / 2;
				int guideY = drawY + (drawH - guideH) / 2;

				CRect guideRect(guideX, guideY, guideX + guideW, guideY + guideH);

				CPen yellowPen(PS_SOLID, 1, RGB(200, 180, 0));
				CPen* pOldPen = memDC.SelectObject(&yellowPen);
				CBrush* pOldBrush = (CBrush*)memDC.SelectStockObject(NULL_BRUSH);

				memDC.Rectangle(&guideRect);

				memDC.SelectObject(pOldPen);
				memDC.SelectObject(pOldBrush);
			}

			// =======================================================
			// 3. 警告メッセージの描画 (最前面に表示するため一番最後に実行)
			// =======================================================
			if (m_bShowWarning.load()) {
				CString warnMsg = _T("Don't turn your face.  Come to center.");

				CFont* pOldFont = memDC.SelectObject(&m_fontBold);
				
				CRect textRect;
				memDC.DrawText(warnMsg, &textRect, DT_CALCRECT);
				
				int tX = drawX + (drawW - textRect.Width()) / 2;
				int tY = drawY + 40;
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
			// =======================================================

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

	// --- 4. CSVファイルへの書き込み (追記モード) ---
	SaveAttendanceToCsv(strTime, strName, strID, strComment);

	// 5. 完了メッセージ（任意で入力欄のクリア等）
	AfxMessageBox(_T("Attendance record saved successfully."));
	
	// 入力欄をクリアして次の人に備える処理を入れる場合はここに追加します
	// m_comboName.SetWindowText(_T(""));
	// m_editID.SetWindowText(_T(""));
}

// --- ★ここから移動: CSVファイルへの保存処理 ---
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
// --- ★ここまで ---

// --- ここから追加 ---

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

// --- ここまで追加 ---
