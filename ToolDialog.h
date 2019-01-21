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
#include "ToolDialog.h"


// CToolDialog dialog

class CToolDialog : public CDialogEx, public GraphixMail
{
	DECLARE_DYNAMIC(CToolDialog)

public:
	CToolDialog(UINT nIDTemplate, CWnd *pParent = NULL);

	virtual ~CToolDialog();

// Dialog Data

protected:

	CWnd *m_pParent;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnMouseLeave();

	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
