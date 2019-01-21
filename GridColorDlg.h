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

#pragma once
#include "afxcolorbutton.h"


// CGridColorDlg dialog

class CGridColorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGridColorDlg)

public:
	CGridColorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGridColorDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GRID };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CMFCColorButton m_CellColor;
	CMFCColorButton m_PixelColor;
};
