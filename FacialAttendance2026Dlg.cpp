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
	DDX_Control(pDX, IDC_SLIDER_SL, m_sliderScreenLight);
	DDX_Control(pDX, IDC_COMBO_NAME, m_comboName);
	DDX_Control(pDX, IDC_EDIT_USER_ID, m_editID);
	DDX_Control(pDX, IDC_EDIT_TIME, m_editTime);
	DDX_Control(pDX, IDC_EDIT_TIME_ORG, m_editTimeOrg);
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
	ON_BN_CLICKED(IDCLOSE, &CFacialAttendance2026Dlg::OnBnClickedClose)
	ON_BN_CLICKED(IDC_BUTTON_PHOTO_OK, &CFacialAttendance2026Dlg::OnBnClickedButtonPhotoOk)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA_ON, &CFacialAttendance2026Dlg::OnBnClickedButtonCameraOn)
	ON_MESSAGE(WM_UPDATE_SCREEN_LIGHT, &CFacialAttendance2026Dlg::OnUpdateScreenLight)
	ON_MESSAGE(WM_APP + 2, &CFacialAttendance2026Dlg::OnInitScreenLight)
	ON_BN_CLICKED(IDC_RADIO_SL_SLIDER, &CFacialAttendance2026Dlg::OnRadioSlSlider)
	ON_BN_CLICKED(IDC_RADIO_SL_AUTO, &CFacialAttendance2026Dlg::OnRadioSlAuto)
	ON_BN_CLICKED(IDC_RADIO_SL_NONE, &CFacialAttendance2026Dlg::OnRadioSlNone)
	ON_BN_CLICKED(IDC_BUTTON_CONFIRM, &CFacialAttendance2026Dlg::OnBnClickedButtonConfirm)
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
	ON_BN_CLICKED(IDC_BUTTON_PHOTO_OK, &CFacialAttendance2026Dlg::OnBnClickedButtonPhotoOk)
	ON_COMMAND(ID_FACEIDENTIFICATION_EIGENFACES, &CFacialAttendance2026Dlg::OnFaceidentificationEigenfaces)
	ON_COMMAND(ID_FACEIDENTIFICATION_LBPH, &CFacialAttendance2026Dlg::OnFaceidentificationLbph)
	ON_COMMAND(ID_FACEIDENTIFICATION_SFACE, &CFacialAttendance2026Dlg::OnFaceidentificationSface)
	ON_UPDATE_COMMAND_UI(ID_FACEIDENTIFICATION_EIGENFACES, &CFacialAttendance2026Dlg::OnUpdateFaceidentificationEigenfaces)
	ON_UPDATE_COMMAND_UI(ID_FACEIDENTIFICATION_LBPH, &CFacialAttendance2026Dlg::OnUpdateFaceidentificationLbph)
	ON_UPDATE_COMMAND_UI(ID_FACEIDENTIFICATION_SFACE, &CFacialAttendance2026Dlg::OnUpdateFaceidentificationSface)
	ON_CBN_SETFOCUS(IDC_COMBO_NAME, &CFacialAttendance2026Dlg::OnCbnSetfocusComboName)
	ON_EN_SETFOCUS(IDC_EDIT_USER_ID, &CFacialAttendance2026Dlg::OnEnSetfocusEditId)
	ON_EN_SETFOCUS(IDC_EDIT_COMMENT, &CFacialAttendance2026Dlg::OnEnSetfocusEditComment)
END_MESSAGE_MAP()


std::vector<CString> GetCameraNames()
{
	std::vector<CString> cameraNames;
	HRESULT hrCo = CoInitialize(NULL);
	ICreateDevEnum* pDevEnum = NULL;
	IEnumMoniker* pEnum = NULL;

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
	if (SUCCEEDED(hr)) {
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
		if (hr == S_OK) {
			IMoniker* pMoniker = NULL;
			while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
				if (pMoniker == nullptr) continue;
				IPropertyBag* pPropBag;
#               pragma warning(suppress : 6387)
				hr = pMoniker->BindToStorage(nullptr, nullptr, IID_PPV_ARGS(&pPropBag));
				if (SUCCEEDED(hr)) {
					VARIANT varName;
					VariantInit(&varName);
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


BOOL CFacialAttendance2026Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	CRect rectWindow;
	GetWindowRect(&rectWindow);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int x = (screenWidth - rectWindow.Width()) / 2;
	SetWindowPos(NULL, x, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	InitializeCameraList();
	InitializeWorkerThread();
	InitializeFontsAndUI();
	InitializeInputFields();
	InitializeListControl();
	InitializeScreenLightSettings();
	InitializeMenuSettings();

	m_identifier.Initialize("");

	{
		std::lock_guard<std::mutex> lock(m_bufferMutex);
		m_faceRingBuffer.clear();
	}

	m_bCapturing = true;
	m_pauseCV.notify_one();

	GetDlgItem(IDC_BUTTON_CAMERA_ON)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_PHOTO_OK)->EnableWindow(TRUE);
	EnableInputFields(FALSE);

	return TRUE;
}


void CFacialAttendance2026Dlg::EnableInputFields(BOOL bEnable)
{
	const int targetIDs[] = {
		IDC_COMBO_NAME,
		IDC_EDIT_USER_ID,
		IDC_EDIT_TIME,
		IDC_EDIT_COMMENT,
		IDC_BUTTON_CONFIRM,
		IDC_STATIC_NAME,
		IDC_STATIC_ID,
		IDC_STATIC_TIME,
		IDC_STATIC_TIME2,
		IDC_STATIC_COMMENT,
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
			{
				std::unique_lock<std::mutex> lock(m_pauseMutex);
				m_pauseCV.wait(lock, [this] { return m_bCapturing.load() || m_bStopThread.load(); });
			}

			if (!::IsWindow(GetSafeHwnd())) break;

			cv::VideoCapture& cap = m_detector.GetCapture();

			try {
				if (cap.isOpened()) {
					bool ret = cap.read(frame);
					if (ret && !frame.empty()) {
						if (m_discardFrames > 0) {
							m_discardFrames--;
							continue;
						}
						cv::flip(frame, frame, 1);

						cv::Mat resizedFrame, displayFrame;
						ProcessCameraFrame(frame, resizedFrame, displayFrame);

						cv::Mat faces;
						std::vector<float> bestFaceData;

						bool faceDetected = PerformFaceDetection(displayFrame, resizedFrame, faces, bestFaceData);
						m_bShowWarning.store(!faceDetected);

						if (m_bCapturing && faceDetected && !bestFaceData.empty()) {
							ProcessAndBufferDetectedFace(displayFrame, resizedFrame, bestFaceData);
						}

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
	m_editTimeOrg.SetFont(&m_fontRegular);
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

	static CFont s_fontDefaultField;
	s_fontDefaultField.DeleteObject();
	lf.lfHeight = UI_FIELD_HEIGHT;
	s_fontDefaultField.CreateFontIndirect(&lf);

	m_comboName.SetFont(&s_fontDefaultField);
	m_editComment.SetFont(&s_fontDefaultField);
	m_comboName.SetItemHeight(-1, UI_FIELD_HEIGHT);

	wcscpy_s(lf.lfFaceName, UI_FONT_NAME_FIXED);
	lf.lfHeight = UI_FIELD_HEIGHT;
	m_fontFixed.DeleteObject();
	m_fontFixed.CreateFontIndirect(&lf);

	m_editID.SetFont(&m_fontFixed);
	m_editTime.SetFont(&m_fontFixed);

	wcscpy_s(lf.lfFaceName, UI_FONT_NAME_FIXED);
	lf.lfHeight = UI_FIELD_HEIGHT_SMALL;
	m_fontFixedSmall.DeleteObject();
	m_fontFixedSmall.CreateFontIndirect(&lf);
	m_editTimeOrg.SetFont(&m_fontFixedSmall);

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
		LOGFONT lf;
		GetFont()->GetLogFont(&lf);
		wcscpy_s(lf.lfFaceName, _T("Tahoma"));
		lf.lfHeight = UI_LIST_FONT_SIZE;

		m_fontFixedList.DeleteObject();
		m_fontFixedList.CreateFontIndirect(&lf);

		pListCtrl->SetFont(&m_fontFixedList);

		pListCtrl->ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
		pListCtrl->SetExtendedStyle(pListCtrl->GetExtendedStyle() | LVS_EX_FULLROWSELECT);

		pListCtrl->InsertColumn(0, _T("*"), LVCFMT_LEFT, 15);
		pListCtrl->InsertColumn(1, _T("TIME"), LVCFMT_LEFT, 45);
		pListCtrl->InsertColumn(2, _T("NAME"), LVCFMT_LEFT, 110);
		pListCtrl->InsertColumn(3, _T("ID"), LVCFMT_LEFT, 90);
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
	m_faceDetectionMode = AfxGetApp()->GetProfileInt(REG_SECTION_SETTINGS, REG_KEY_FACE_DETECTION_MODE, 2);
	m_faceIdentificationMode = AfxGetApp()->GetProfileInt(REG_SECTION_SETTINGS, _T("FaceIdentificationMode"), 2);
}

void CFacialAttendance2026Dlg::UpdateIdentificationFields(CString name)
{
	if (name.CompareNoCase(_T("Unknown")) == 0) {
		m_comboName.SetWindowText(name);
	}
	else {
		m_comboName.SetWindowText(name);
		if (m_comboName.FindStringExact(-1, name) == CB_ERR) {
			m_comboName.AddString(name);
		}
	}

	CTime now = CTime::GetCurrentTime();
	m_editTime.SetTime(&now);

	CString strTime = now.Format(_T("%H:%M"));
	m_editTimeOrg.SetWindowText(strTime);

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

void CFacialAttendance2026Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CFacialAttendance2026Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFacialAttendance2026Dlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1) {
		UpdateFrame();
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
	bmi.bmiHeader.biHeight = -h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pvBits = nullptr;
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

		m_bStopThread = true;
		if (m_workerThread.joinable()) {
			m_workerThread.join();
		}

		m_detector.OpenCamera(m_currentCameraIdx);

		if (m_bCapturing) {
			m_bStopThread = false;
			InitializeWorkerThread();
		}
	}
}

void CFacialAttendance2026Dlg::OnBnClickedClose()
{
	OnCancel();
}

void CFacialAttendance2026Dlg::OnCancel()
{
	m_bStopThread = true;
	m_pauseCV.notify_all();

	if (m_workerThread.joinable()) {
		m_workerThread.join();
	}

	FlushAndResetEvaluationData();

	CDialogEx::OnCancel();
}

void CFacialAttendance2026Dlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialogEx::OnActivate(nState, pWndOther, bMinimized);
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

bool CFacialAttendance2026Dlg::DetectAndGetBestHaarFace(cv::Mat& displayFrame, int cx, int cy, int& minDistance2, std::vector<float>& outBestFaceData)
{
	std::vector<cv::Rect> haarRects;
	bool faceDetected = false;

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
				outBestFaceData.assign(15, 0.0f);
				outBestFaceData[0] = static_cast<float>(rect.x);
				outBestFaceData[1] = static_cast<float>(rect.y);
				outBestFaceData[2] = static_cast<float>(rect.width);
				outBestFaceData[3] = static_cast<float>(rect.height);

				outBestFaceData[4] = rect.x + rect.width * 0.30f;
				outBestFaceData[5] = rect.y + rect.height * 0.45f;
				outBestFaceData[6] = rect.x + rect.width * 0.70f;
				outBestFaceData[7] = rect.y + rect.height * 0.45f;
				outBestFaceData[8] = rect.x + rect.width * 0.50f;
				outBestFaceData[9] = rect.y + rect.height * 0.65f;
				outBestFaceData[10] = rect.x + rect.width * 0.35f;
				outBestFaceData[11] = rect.y + rect.height * 0.85f;
				outBestFaceData[12] = rect.x + rect.width * 0.65f;
				outBestFaceData[13] = rect.y + rect.height * 0.85f;
				outBestFaceData[14] = 1.0f;
			}
		}
	}
	return faceDetected;
}

bool CFacialAttendance2026Dlg::DetectAndGetBestYunetFace(cv::Mat& displayFrame, const cv::Mat& resizedFrame, int cx, int cy, int& minDistance2, cv::Mat& outFaces, std::vector<float>& outBestFaceData)
{
	bool faceDetected = false;
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
				outBestFaceData.assign(15, 0.0f);
				for (int j = 0; j < 15; j++) {
					outBestFaceData[j] = outFaces.at<float>(i, j);
				}
			}
		}
	}
	return faceDetected;
}

bool CFacialAttendance2026Dlg::PerformFaceDetection(cv::Mat& displayFrame, cv::Mat& resizedFrame, cv::Mat& outFaces, std::vector<float>& outBestFaceData)
{
	int currentMode = m_faceDetectionMode;
	bool faceDetected = false;

	outBestFaceData.clear();
	int minDistance2 = std::numeric_limits<int>::max();
	int cx = displayFrame.cols / 2;
	int cy = displayFrame.rows / 2;

	if (currentMode == 0 || currentMode == 2) {
		if (DetectAndGetBestHaarFace(displayFrame, cx, cy, minDistance2, outBestFaceData)) {
			faceDetected = true;
		}
	}

	if (currentMode == 1 || currentMode == 2) {
		if (DetectAndGetBestYunetFace(displayFrame, resizedFrame, cx, cy, minDistance2, outFaces, outBestFaceData)) {
			faceDetected = true;
		}
	}

	int guideW = displayFrame.cols * 5 / 10;
	int guideH = displayFrame.rows * 8 / 10;
	int guideX = (displayFrame.cols - guideW) / 2;
	int guideY = (displayFrame.rows - guideH) / 2;
	if (guideX >= 0 && guideW > 0 && guideH > 0) {
		cv::rectangle(displayFrame, cv::Rect(guideX, guideY, guideW, guideH), cv::Scalar(0, 180, 200), 2);
	}

	return faceDetected;
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
		m_screenLight.ApplyManualBrightness(m_sliderValue / 100.0f);
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
		m_screenLight.ApplyManualBrightness(value / 100.0f);
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
	CString strName, strID, strTime, strTimeOrg, strComment;

	m_comboName.GetWindowText(strName);
	m_editID.GetWindowText(strID);
	m_editTime.GetWindowText(strTime);
	m_editTimeOrg.GetWindowText(strTimeOrg);
	m_editComment.GetWindowText(strComment);

	strName.Trim();
	strID.Trim();
	strTime.Trim();

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

	strTime.Remove(_T('_'));
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

	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(IDC_ATTENDEES_LIST);
	if (pListCtrl != nullptr) {
		CString timeModifiedMark = (strTime == strTimeOrg) ? _T("") : _T("*");

		int insertIndex = pListCtrl->GetItemCount();
		int rowIndex = pListCtrl->InsertItem(insertIndex, timeModifiedMark);

		pListCtrl->SetItemText(rowIndex, 1, strTime);
		pListCtrl->SetItemText(rowIndex, 2, strName);
		pListCtrl->SetItemText(rowIndex, 3, strID);

		pListCtrl->EnsureVisible(rowIndex, FALSE);
	}

	if (!m_bestItem.rawFrame.empty() && strName.CompareNoCase(_T("Unknown")) != 0) {
		CT2CA pszConvertedAnsiString(strName);
		std::string stdName(pszConvertedAnsiString);

		FaceIdentificationMethod method = static_cast<FaceIdentificationMethod>(m_faceIdentificationMode);
		m_identifier.Enroll(method, stdName, m_bestItem.rawFrame, m_bestItem.faceData);
	}

	SaveAttendanceToCsv(strTime, strName, strID, strComment);

	m_comboName.SetWindowText(_T(""));
	m_editID.SetWindowText(_T("")),
		m_editComment.SetWindowText(_T(""));
	m_editTimeOrg.SetWindowText(_T(""));

	OnBnClickedButtonCameraOn();
}

void CFacialAttendance2026Dlg::SaveAttendanceToCsv(const CString& strTime, const CString& strName, const CString& strID, const CString& strComment)
{
	std::wstring csvPath = GetAttendanceCsvPath();

	CString escapedComment = strComment;
	escapedComment.Replace(_T("\""), _T("\"\""));
	escapedComment = _T("\"") + escapedComment + _T("\"");

	bool isNewFile = true;
	FILE* checkFp = nullptr;
	if (_wfopen_s(&checkFp, csvPath.c_str(), L"rb") == 0 && checkFp != nullptr) {
		fseek(checkFp, 0, SEEK_END);
		if (ftell(checkFp) > 0) {
			isNewFile = false;
		}
		fclose(checkFp);
	}

	FILE* fp = nullptr;
	if (_wfopen_s(&fp, csvPath.c_str(), L"ab, ccs=UTF-8") == 0 && fp != nullptr) {
		if (isNewFile) {
			unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
			fwrite(bom, 1, sizeof(bom), fp);
			fwprintf(fp, L"Time,Name,ID,Comment\n");
		}

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
	OnCancel();
}

void CFacialAttendance2026Dlg::OnFileShowattendancelist()
{
}

void CFacialAttendance2026Dlg::OnFacedetectionHaarcascades()
{
	FlushAndResetEvaluationData();
	m_faceDetectionMode = 0;
	AfxGetApp()->WriteProfileInt(REG_SECTION_SETTINGS, REG_KEY_FACE_DETECTION_MODE, m_faceDetectionMode);
}
void CFacialAttendance2026Dlg::OnFacedetectionYunet()
{
	FlushAndResetEvaluationData();
	m_faceDetectionMode = 1;
	AfxGetApp()->WriteProfileInt(REG_SECTION_SETTINGS, REG_KEY_FACE_DETECTION_MODE, m_faceDetectionMode);
}
void CFacialAttendance2026Dlg::OnFacedetectionBoth()
{
	FlushAndResetEvaluationData();
	m_faceDetectionMode = 2;
	AfxGetApp()->WriteProfileInt(REG_SECTION_SETTINGS, REG_KEY_FACE_DETECTION_MODE, m_faceDetectionMode);
}

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
			if ( cmdUI.m_nID == (UINT)-1 || cmdUI.m_nID == 0)
				continue;
			cmdUI.DoUpdate(this, FALSE);
		}
	}
}

void CFacialAttendance2026Dlg::OnFacedetectionShowfolder()
{
	FlushAndResetEvaluationData();
	OpenEvaluationFolder(wFACE_DETECTION_LATENCY_FOLDER_NAME);
}

void CFacialAttendance2026Dlg::FlushAndResetEvaluationData()
{
	uint64_t frames = m_totalFrames.load();
	if (frames > 0) {

		double totalLoopMs = m_totalLatencyMs.load();

		double avgLatencyMs = totalLoopMs / (double)frames;
		double equivalentFps = 0.0;
		if (avgLatencyMs > 0.0) {
			equivalentFps = 1000.0 / avgLatencyMs;
		}

		SaveEvaluationLatencyCsv(m_faceDetectionMode, avgLatencyMs, equivalentFps);

		m_totalFrames.store(0);
		m_totalLatencyMs.store(0.0);
	}
}

double CFacialAttendance2026Dlg::CalculateSharpness(const cv::Mat& img)
{
	if (img.empty()) return 0.0;

	cv::Mat gray, laplacian;
	if (img.channels() == 3) {
		cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
	}
	else {
		gray = img;
	}

	cv::Laplacian(gray, laplacian, CV_64F);

	cv::Scalar mu, sigma;
	cv::meanStdDev(laplacian, mu, sigma);

	return sigma.val[0] * sigma.val[0];
}

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
	return sigma.val[0];
}

double CFacialAttendance2026Dlg::CalculateBestFaceScore(double sharpness, double faceConfidence, double contrast)
{
	double normSharpness = std::min(sharpness / 2000.0, 1.0);
	double normContrast = std::min(contrast / 80.0, 1.0);

	return (faceConfidence * FACE_WEIGHT_CONFIDENCE)
		+ (normSharpness * FACE_WEIGHT_SHARPNESS)
		+ (normContrast * FACE_WEIGHT_CONTRAST);
}

void CFacialAttendance2026Dlg::DrawMatToStatic(int nID, const cv::Mat& mat)
{
	CWnd* pWnd = GetDlgItem(nID);
	if (!pWnd) return;

	CClientDC dc(pWnd);
	CRect rect;
	pWnd->GetClientRect(&rect);

	dc.FillSolidRect(&rect, GetSysColor(COLOR_3DFACE));

	if (mat.empty()) return;

	HBITMAP hBmp = CreateBitmapFromMat(mat);
	if (hBmp) {
		CDC memDC;
		memDC.CreateCompatibleDC(&dc);
		HBITMAP hOld = (HBITMAP)memDC.SelectObject(hBmp);

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
			drawH = (int)(dstW / srcAspect);
			drawY = (dstH - drawH) / 2;
		}
		else {
			drawW = (int)(dstH * srcAspect);
			drawX = (dstW - drawW) / 2;
		}

		dc.SetStretchBltMode(COLORONCOLOR);
		dc.StretchBlt(drawX, drawY, drawW, drawH,
			&memDC, 0, 0, srcW, srcH, SRCCOPY);

		memDC.SelectObject(hOld);
		DeleteObject(hBmp);
	}
}

void CFacialAttendance2026Dlg::OnBnClickedButtonPhotoOk()
{
	m_bCapturing = false;

	GetDlgItem(IDC_BUTTON_CAMERA_ON)->EnableWindow(TRUE);
	EnableInputFields(TRUE);

	SetDefID(IDC_BUTTON_CONFIRM);
	GetDlgItem(IDC_BUTTON_PHOTO_OK)->EnableWindow(FALSE);

	std::lock_guard<std::mutex> lock(m_bufferMutex);

	ClearFaceUI();

	if (m_faceRingBuffer.empty()) {
		MyMessageBoxW(GetSafeHwnd(), MB_OK | MB_ICONINFORMATION, g_wAppNameLong, L"顔が検出されていません。");
		return;
	}

	double maxScore = -1.0;
	double minScore = 9999999.0;
	cv::Mat bestFace;
	cv::Mat worstFace;

	FindBestAndWorstFaces(bestFace, worstFace, maxScore, minScore);
	ApplyFaceIdentificationResults(bestFace, worstFace, maxScore, minScore);

	GotoDlgCtrl(GetDlgItem(IDC_COMBO_NAME));
}

// --- 新しく追加・分離するヘルパー関数 ---

void CFacialAttendance2026Dlg::ClearFaceUI()
{
	cv::Mat emptyMat;
	DrawMatToStatic(IDC_STATIC_FACE, emptyMat);
	DrawMatToStatic(IDC_STATIC_FACE_WORST, emptyMat);
	SetDlgItemText(IDC_STATIC_FACE_SCORE, _T(""));
	SetDlgItemText(IDC_STATIC_FACE_WORST_SCORE, _T(""));
}

void CFacialAttendance2026Dlg::FindBestAndWorstFaces(cv::Mat& outBestFace, cv::Mat& outWorstFace, double& outMaxScore, double& outMinScore)
{
	outMaxScore = -1.0;
	outMinScore = 9999999.0;

	for (const auto& item : m_faceRingBuffer) {
		if (item.totalScore > outMaxScore) {
			outMaxScore = item.totalScore;
			outBestFace = item.face112;
			m_bestItem = item;
		}
		if (item.totalScore < outMinScore) {
			outMinScore = item.totalScore;
			outWorstFace = item.face112;
		}
	}
}

void CFacialAttendance2026Dlg::ApplyFaceIdentificationResults(const cv::Mat& bestFace, const cv::Mat& worstFace, double maxScore, double minScore)
{
	if (!bestFace.empty()) {
		DrawMatToStatic(IDC_STATIC_FACE, bestFace);
		CString strBestScore;
		strBestScore.Format(_T("%.2f"), maxScore * 100.0);
		SetDlgItemText(IDC_STATIC_FACE_SCORE, strBestScore);

		FaceIdentificationMethod method = static_cast<FaceIdentificationMethod>(m_faceIdentificationMode);

		IdentificationResult result = m_identifier.Identify(method, m_bestItem.rawFrame, m_bestItem.faceData);

		CString strName(result.name.c_str());
		UpdateIdentificationFields(strName);
	}

	if (!worstFace.empty()) {
		DrawMatToStatic(IDC_STATIC_FACE_WORST, worstFace);
		CString strWorstScore;
		strWorstScore.Format(_T("%.2f"), minScore * 100.0);
		SetDlgItemText(IDC_STATIC_FACE_WORST_SCORE, strWorstScore);
	}
}
void CFacialAttendance2026Dlg::OnBnClickedButtonCameraOn()
{
	{
		std::lock_guard<std::mutex> lock(m_bufferMutex);
		m_faceRingBuffer.clear();
	}
	EnableInputFields(FALSE);

	GetDlgItem(IDC_BUTTON_PHOTO_OK)->EnableWindow(TRUE);

	SendMessage(DM_SETDEFID, IDC_BUTTON_PHOTO_OK, 0);

	GetDlgItem(IDC_BUTTON_PHOTO_OK)->SetFocus();

	GetDlgItem(IDC_BUTTON_CAMERA_ON)->EnableWindow(FALSE);


	cv::Mat emptyMat;
	DrawMatToStatic(IDC_STATIC_FACE, emptyMat);
	DrawMatToStatic(IDC_STATIC_FACE_WORST, emptyMat);

	SetDlgItemText(IDC_STATIC_FACE_SCORE, _T(""));
	SetDlgItemText(IDC_STATIC_FACE_WORST_SCORE, _T(""));

	m_discardFrames.store(5);
	m_bCapturing = true;
	m_pauseCV.notify_one();
}

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

void CFacialAttendance2026Dlg::OnUpdateFaceidentificationEigenfaces(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_faceIdentificationMode == 0);
}

void CFacialAttendance2026Dlg::OnUpdateFaceidentificationLbph(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_faceIdentificationMode == 1);
}

void CFacialAttendance2026Dlg::OnUpdateFaceidentificationSface(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_faceIdentificationMode == 2);
}

void CFacialAttendance2026Dlg::OnCbnSetfocusComboName()
{
	m_comboName.SetEditSel(0, -1);
}

void CFacialAttendance2026Dlg::OnEnSetfocusEditId()
{
	m_editID.SetSel(0, -1, TRUE);
}

void CFacialAttendance2026Dlg::OnEnSetfocusEditComment()
{
	m_editComment.SetSel(0, -1, TRUE);
}

// --- ヘルパー関数の実装 ---

bool CFacialAttendance2026Dlg::CheckAndResetTargetJump(int cx, int cy, int faceWidth)
{
	std::lock_guard<std::mutex> lock(m_bufferMutex);
	bool jumped = false;

	if (!m_faceRingBuffer.empty() && m_lastTargetCenter.x >= 0) {
		double dist = std::sqrt(std::pow(cx - m_lastTargetCenter.x, 2) + std::pow(cy - m_lastTargetCenter.y, 2));

		if (dist > faceWidth * FACE_TARGET_JUMP_RATIO) {
			m_faceRingBuffer.clear();
			jumped = true;
		}
	}
	m_lastTargetCenter = cv::Point(cx, cy);

	return jumped;
}

cv::Rect CFacialAttendance2026Dlg::GetSafeSquareRect(int cx, int cy, int sideLength, int maxWidth, int maxHeight)
{
	int x = std::max(0, cx - sideLength / 2);
	int y = std::max(0, cy - sideLength / 2);
	int w = std::min(maxWidth - x, sideLength);
	int h = std::min(maxHeight - y, sideLength);

	int finalSide = std::min(w, h);

	return cv::Rect(x, y, finalSide, finalSide);
}

cv::Rect CFacialAttendance2026Dlg::GetPaddedRectAndShiftLandmarks(int cx, int cy, int padSide, int maxWidth, int maxHeight, const std::vector<float>& originalData, std::vector<float>& outShiftedData)
{
	int padX = std::max(0, cx - padSide / 2);
	int padY = std::max(0, cy - padSide / 2);
	int padW = std::min(maxWidth - padX, padSide);
	int padH = std::min(maxHeight - padY, padSide);

	outShiftedData = originalData;
	outShiftedData[0] -= padX; // x
	outShiftedData[1] -= padY; // y
	for (int i = 4; i < 14; i += 2) {
		outShiftedData[i] -= padX;     // landmarks_x
		outShiftedData[i + 1] -= padY; // landmarks_y
	}

	return cv::Rect(padX, padY, padW, padH);
}

void CFacialAttendance2026Dlg::AddItemToRingBuffer(const FaceBufferItem& item)
{
	std::lock_guard<std::mutex> lock(m_bufferMutex);
	m_faceRingBuffer.push_back(item);

	ULONGLONG currentTick = GetTickCount64();

	while (!m_faceRingBuffer.empty()) {
		if ((currentTick - m_faceRingBuffer.front().timestamp) > FACE_RING_BUFFER_TIMEOUT_MS) {
			m_faceRingBuffer.pop_front();
		}
		else if (m_faceRingBuffer.size() > MAX_FACE_RING_BUFFER) {
			m_faceRingBuffer.pop_front();
		}
		else {
			break;
		}
	}
}

void CFacialAttendance2026Dlg::ProcessAndBufferDetectedFace(const cv::Mat& displayFrame, const cv::Mat& resizedFrame, const std::vector<float>& bestFaceData)
{
	cv::Rect targetFaceRect(
		static_cast<int>(bestFaceData[0]),
		static_cast<int>(bestFaceData[1]),
		static_cast<int>(bestFaceData[2]),
		static_cast<int>(bestFaceData[3])
	);
	double faceConfidence = static_cast<double>(bestFaceData[14]);

	if (targetFaceRect.width <= 0 || targetFaceRect.height <= 0) return;

	int cx = targetFaceRect.x + targetFaceRect.width / 2;
	int cy = targetFaceRect.y + targetFaceRect.height / 2;

	CheckAndResetTargetJump(cx, cy, targetFaceRect.width);

	int sideLength = std::max(targetFaceRect.width, targetFaceRect.height);
	cv::Rect safeRect = GetSafeSquareRect(cx, cy, sideLength, displayFrame.cols, displayFrame.rows);

	if (safeRect.width > 0 && safeRect.height > 0) {
		cv::Mat cropFace(resizedFrame, safeRect);
		cv::Mat simpleFace112;
		cv::resize(cropFace, simpleFace112, cv::Size(FACE_NORM_SIZE, FACE_NORM_SIZE));

		double sharpness = CalculateSharpness(simpleFace112);
		double contrast = CalculateContrast(simpleFace112);
		double totalScore = CalculateBestFaceScore(sharpness, faceConfidence, contrast);

		int padSide = static_cast<int>(safeRect.width * 1.5);
		std::vector<float> shiftedFaceData;
		cv::Rect paddedRect = GetPaddedRectAndShiftLandmarks(cx, cy, padSide, resizedFrame.cols, resizedFrame.rows, bestFaceData, shiftedFaceData);

		FaceBufferItem item;
		item.face112 = simpleFace112;
		item.rawFrame = resizedFrame(paddedRect).clone();
		item.faceData = shiftedFaceData;
		item.sharpness = sharpness;
		item.confidence = faceConfidence;
		item.contrast = contrast;
		item.totalScore = totalScore;
		item.timestamp = GetTickCount64();

		AddItemToRingBuffer(item);
	}
}