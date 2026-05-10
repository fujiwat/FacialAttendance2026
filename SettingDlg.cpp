#include "pch.h"
#include "FacialAttendance2026.h"
#include "MyConst.h"
#include "afxdialogex.h"
#include "SettingDlg.h"

IMPLEMENT_DYNAMIC(SettingDlg, CDialogEx)

SettingDlg::SettingDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DIALOG_SETTINGS, pParent)
{
}

SettingDlg::~SettingDlg()
{
}

void SettingDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(SettingDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &SettingDlg::OnBnClickedOk)
END_MESSAGE_MAP()

/**
 * @brief Asserts state validations finalizing choices transcribing values to local app profiling permanently.
 */
void SettingDlg::OnBnClickedOk()
{
    BOOL bRequiresID = (IsDlgButtonChecked(IDC_CHECK_REQUIRES_ID) == BST_CHECKED);
    BOOL bMirrorView = (IsDlgButtonChecked(IDC_CHECK_MIRROR_VIEW) == BST_CHECKED);

    AfxGetApp()->WriteProfileInt(REG_SECTION_SETTINGS, REG_KEY_OPTION_REQUIREDS_ID, bRequiresID);
    AfxGetApp()->WriteProfileInt(REG_SECTION_SETTINGS, REG_KEY_OPTION_MIRROR_VIEW, bMirrorView);

    CDialogEx::OnOK();
}

/**
 * @brief Populates visual component checking based upon previous locally persistent values configuring accurately.
 * @return Standard validation confirming safe allocations processed properly securely.
 */
BOOL SettingDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    int nRequiresID = AfxGetApp()->GetProfileInt(REG_SECTION_SETTINGS, REG_KEY_OPTION_REQUIREDS_ID, DEF_KEY_OPTION_TRUE);
    int nMirrorView = AfxGetApp()->GetProfileInt(REG_SECTION_SETTINGS, REG_KEY_OPTION_MIRROR_VIEW, DEF_KEY_OPTION_TRUE);

    CheckDlgButton(IDC_CHECK_REQUIRES_ID, nRequiresID ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_CHECK_MIRROR_VIEW, nMirrorView ? BST_CHECKED : BST_UNCHECKED);

    return TRUE;
}