
// FacialAttendance2026.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CFacialAttendance2026App:
// See FacialAttendance2026.cpp for the implementation of this class
//

class CFacialAttendance2026App : public CWinApp
{
public:
	CFacialAttendance2026App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CFacialAttendance2026App theApp;
