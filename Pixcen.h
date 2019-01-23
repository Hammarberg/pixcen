/*
Pixcen - A windows platform low level pixel editor for C64
Copyright (C) 2013  John Hammarberg (crt@nospam.binarybone.com)

This file is part of Pixcen.

Pixcen is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Pixcen is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Pixcen.  If not, see <http://www.gnu.org/licenses/>.
*/

// Pixcen.h : main header file for the Pixcen application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CGraphixApp:
// See Pixcen.cpp for the implementation of this class
//

class CGraphixApp : public CWinAppEx
{
public:
	CGraphixApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CGraphixApp theApp;
