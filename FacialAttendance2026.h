#pragma once

#ifndef __AFXWIN_H__
#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

/**
 * @brief Application class managing main instance initialization and metadata setups.
 */
class CFacialAttendance2026App : public CWinApp
{
public:
    CFacialAttendance2026App();
    virtual BOOL InitInstance();
    void InitializeApplicationName();

    DECLARE_MESSAGE_MAP()
};

extern CFacialAttendance2026App theApp;