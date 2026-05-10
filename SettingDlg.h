#pragma once
#include "afxdialogex.h"

/**
 * @brief Simple dialog encapsulating primary configuration parameters editing.
 */
class SettingDlg : public CDialogEx
{
    DECLARE_DYNAMIC(SettingDlg)

public:
    /**
     * @brief Constructs setup dialogue managing configuration boundaries.
     * @param pParent Handled pointer to the interface parent.
     */
    SettingDlg(CWnd* pParent = nullptr);
    virtual ~SettingDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DIALOG_SETTINGS };
#endif

protected:
    /**
     * @brief Modifies properties associating UI check states variables accordingly natively.
     */
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    virtual BOOL OnInitDialog();
};