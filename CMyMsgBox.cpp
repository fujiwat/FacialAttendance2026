// CMyMsgBox.cpp : implementation file
//

#include "pch.h"
#include "FacialAttendance2026.h"
#include "afxdialogex.h"
#include "CMyMsgBox.h"


// CMyMsgBox dialog

IMPLEMENT_DYNAMIC(CMyMsgBox, CDialogEx)

CMyMsgBox::CMyMsgBox(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MESSAGEBOX, pParent)
{

}

CMyMsgBox::~CMyMsgBox()
{
}

void CMyMsgBox::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMyMsgBox, CDialogEx)
    ON_BN_CLICKED(IDYES, &CMyMsgBox::OnBnClickedYes)
    ON_BN_CLICKED(IDNO, &CMyMsgBox::OnBnClickedNo)
END_MESSAGE_MAP()


// CMyMsgBox message handlers

BOOL CMyMsgBox::OnInitDialog()
{
	CDialogEx::OnInitDialog();
    if (!m_strTitle.IsEmpty()) {
        SetWindowText(m_strTitle);
    }
    m_strMessage.Replace(_T("\n"), _T("\r\n"));

    m_font.CreatePointFont(200, _T("Segoe UI"));

    CWnd* pEdit = GetDlgItem(IDC_EDIT_MSG);
    if (pEdit) {
        pEdit->SetFont(&m_font);
        pEdit->SetWindowText(m_strMessage);
    }

    HICON hIcon = (HICON)::LoadImage(NULL, IDI_INFORMATION, IMAGE_ICON, 48, 48, LR_SHARED);

    CStatic* pIconStatic = (CStatic*)GetDlgItem(IDC_STATIC_ICON);
    if (pIconStatic) {
        pIconStatic->SetIcon(hIcon);

        // Picture Control 
        pIconStatic->SetWindowPos(NULL, 0, 0, 48, 48, SWP_NOZORDER | SWP_NOMOVE);
    }

    CWnd* pBtn1 = GetDlgItem(IDOK);
    CWnd* pBtn2 = GetDlgItem(IDCANCEL);

    UINT nMsgType = m_nType & MB_TYPEMASK;

    CRect rect1, rect2;
    if (pBtn1 && pBtn2) {
        pBtn1->GetWindowRect(&rect1);
        ScreenToClient(&rect1);
        pBtn2->GetWindowRect(&rect2);
        ScreenToClient(&rect2);
    }

    // change button position and its text
    if (nMsgType == MB_YESNOCANCEL || nMsgType == MB_YESNO) {
        if (pBtn1) {
            pBtn1->SetWindowText(_T("Yes"));
            pBtn1->SetDlgCtrlID(IDYES);
        }
        if (pBtn2) {
            pBtn2->SetWindowText(_T("No"));
            pBtn2->SetDlgCtrlID(IDNO);
            pBtn2->ShowWindow(SW_SHOW);
        }

        // MB_YESNOCANCEL の場合は、右端から逆算して3つのボタンを絶対配置する
        if (nMsgType == MB_YESNOCANCEL && pBtn1 && pBtn2) {
            CRect clientRect;
            GetClientRect(&clientRect); // ダイアログの内部領域サイズを取得

            int bw = rect2.Width();     // 元のボタンの幅
            int bh = rect2.Height();    // 元のボタンの高さ
            int by = rect2.top;         // 元のY座標

            int marginX = clientRect.right - rect2.right; // 右端からの余白
            if (marginX < 10) marginX = 15;
            int padding = 10; // ボタン同士の隙間

            // 第3ボタン(Cancel) を一番右に配置
            CRect r3(clientRect.right - marginX - bw, by, clientRect.right - marginX, by + bh);
            m_btnExtra.Create(_T("Cancel"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, r3, this, IDCANCEL);
            m_btnExtra.SetFont(&m_font);

            // 第2ボタン(No) を第3ボタンの左隣に配置
            CRect r2(r3.left - padding - bw, by, r3.left - padding, by + bh);
            pBtn2->MoveWindow(&r2);

            // 第1ボタン(Yes) を第2ボタンの左隣に配置
            CRect r1(r2.left - padding - bw, by, r2.left - padding, by + bh);
            pBtn1->MoveWindow(&r1);
        }
    }
    else if (nMsgType == MB_OKCANCEL) {
        if (pBtn1) pBtn1->SetWindowText(_T("OK"));
        if (pBtn2) {
            pBtn2->SetWindowText(_T("Cancel"));
            pBtn2->ShowWindow(SW_SHOW);
        }
    }
    else {
        // MB_OK などの場合、Cancelボタンを隠す
        if (pBtn1) pBtn1->SetWindowText(_T("OK"));
        if (pBtn2) pBtn2->ShowWindow(SW_HIDE);
    }

    // change font size
    int btnIDs[] = { IDOK, IDCANCEL, IDYES, IDNO };
    for (int id : btnIDs) {
        CWnd* pBtn = GetDlgItem(id);
        if (pBtn && pBtn->GetSafeHwnd()) {
            pBtn->SetFont(&m_font);
        }
    }

    // ICON size
    UINT iconType = m_nType & MB_ICONMASK;
    if (iconType == MB_ICONWARNING) {
        hIcon = (HICON)::LoadImage(NULL, IDI_WARNING, IMAGE_ICON, 48, 48, LR_SHARED);
        if (pIconStatic) pIconStatic->SetIcon(hIcon);
    }
    else if (iconType == MB_ICONQUESTION) {
        hIcon = (HICON)::LoadImage(NULL, IDI_QUESTION, IMAGE_ICON, 48, 48, LR_SHARED);
        if (pIconStatic) pIconStatic->SetIcon(hIcon);
    }

    // default button

    // get default button from the type
    UINT defButtonFlag = m_nType & MB_DEFMASK;

    // normally: {1st button(OK/Yes), 2nd button(Cancel/No), 3rd button(m_btnExtra=Cancel)}
    CWnd* btnArray[3] = { pBtn1, pBtn2, nullptr };
    if (m_btnExtra.GetSafeHwnd()) {
        btnArray[2] = &m_btnExtra;
    }

    CWnd* pTargetDefBtn = nullptr;

    // determine which button to focus based on the flag
    if (defButtonFlag == MB_DEFBUTTON1) {
        pTargetDefBtn = btnArray[0];
    }
    else if (defButtonFlag == MB_DEFBUTTON2) {
        pTargetDefBtn = btnArray[1];
    }
    else if (defButtonFlag == MB_DEFBUTTON3) {
        pTargetDefBtn = btnArray[2];
    }

    // if the specified default button exists, set focus to it
    if (pTargetDefBtn && pTargetDefBtn->IsWindowVisible()) {
        // set default control for the dialog
        this->SetDefID(pTargetDefBtn->GetDlgCtrlID());
        // set the default button style
        for (int i = 0; i < 3; i++) {
            if (btnArray[i] && btnArray[i]->GetSafeHwnd()) {
                long style = GetWindowLong(btnArray[i]->GetSafeHwnd(), GWL_STYLE);
                if (btnArray[i] == pTargetDefBtn) {
                    style |= BS_DEFPUSHBUTTON; // set default
                }
                else {
                    style &= ~BS_DEFPUSHBUTTON; // unset default
                    style |= BS_PUSHBUTTON;
                }
                ::SendMessage(btnArray[i]->GetSafeHwnd(), BM_SETSTYLE, style, TRUE);
            }
        }

        // prevent the dialog from automatically taking initial focus, then set focus to the button
        pTargetDefBtn->SetFocus();

        // change the return value of the message to inform the system that the focus has been set manually
        return FALSE;
    }

    return TRUE;
 }

void CMyMsgBox::OnBnClickedYes()
{
    EndDialog(IDYES);
}

void CMyMsgBox::OnBnClickedNo()
{
    EndDialog(IDNO);
}

void CMyMsgBox::OnOK()
{
    UINT nMsgType = m_nType & MB_TYPEMASK;

    // YES/NO dialog, when got Enter, return IDYES
    if (nMsgType == MB_YESNOCANCEL || nMsgType == MB_YESNO) {
        EndDialog(IDYES);
    }
    else {
        // Normal Dialog - IDOK
        EndDialog(IDOK);
    }
}

void CMyMsgBox::OnCancel()
{
    UINT nMsgType = m_nType & MB_TYPEMASK;

    // YES/NO/CANCEL dialog, when got Esc, return IDCANCEL or IDNO
    if (nMsgType == MB_YESNOCANCEL) {
        EndDialog(IDCANCEL);
    }
    else if (nMsgType == MB_YESNO) {
        EndDialog(IDNO);    // Yes/No buttons only then Esc=IDNO
    }
    else {
        EndDialog(IDCANCEL);
    }
}
