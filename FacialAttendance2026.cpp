#include "pch.h"
#include "framework.h"
#include "FacialAttendance2026.h"
#include "FacialAttendance2026Dlg.h"
#include "MyConst.h" 
#include "MyFunctions.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CFacialAttendance2026App, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

/**
 * @brief Determines and initializes the application's global string metadata.
 * It accesses the executable's path to figure out the required short and long names.
 */
void CFacialAttendance2026App::InitializeApplicationName()
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    std::wstring wPath(exePath);
    size_t lastSlash = wPath.find_last_of(L"\\/");
    std::wstring fileName = (lastSlash == std::wstring::npos) ? wPath : wPath.substr(lastSlash + 1);

    size_t lastDot = fileName.find_last_of(L".");
    if (lastDot != std::wstring::npos) {
        fileName = fileName.substr(0, lastDot);
    }

    g_wAppNameShort = fileName;

    int size = WideCharToMultiByte(CP_UTF8, 0, &fileName[0], (int)fileName.size(), NULL, 0, NULL, NULL);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &fileName[0], (int)fileName.size(), &result[0], size, NULL, NULL);
    g_appNameShort = result;

    g_wAppNameLong = g_wAppNameShort + L" - YuNet/HaarCascade Face Detection";
    g_appNameLong = g_appNameShort + " - YuNet/HaarCascade Face Detection";
}

/**
 * @brief Constructs the application object and configures restart manager features.
 */
CFacialAttendance2026App::CFacialAttendance2026App()
{
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CFacialAttendance2026App theApp;

/**
 * @brief Performs core application initialization, starts the visual manager, and launches the main dialog.
 * @return Returns FALSE to end the application process immediately after the dialog is closed.
 */
BOOL CFacialAttendance2026App::InitInstance()
{
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    AfxEnableControlContainer();

    CShellManager* pShellManager = new CShellManager;
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    InitializeApplicationName();

    CFacialAttendance2026Dlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
    }
    else if (nResponse == IDCANCEL)
    {
    }
    else if (nResponse == -1)
    {
        TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
        TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
    }

    if (pShellManager != nullptr)
    {
        delete pShellManager;
    }

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
    ControlBarCleanUp();
#endif

    return FALSE;
}