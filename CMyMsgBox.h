#pragma once
#include "afxdialogex.h"


// CMyMsgBox dialog

class CMyMsgBox : public CDialogEx
{
	DECLARE_DYNAMIC(CMyMsgBox)

public:
	CMyMsgBox(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMyMsgBox();
    UINT m_nType = MB_OK;
    CButton m_btnExtra; // 3rd button
    CFont m_font;
    CString m_strTitle;
    CString m_strMessage;
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MESSAGEBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg void OnBnClickedYes();
    afx_msg void OnBnClickedNo();
    virtual void OnOK();
    virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};
