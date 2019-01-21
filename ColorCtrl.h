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
#include "afxwin.h"
#include "col.h"

// CColorCtrl dialog

class CColorCtrl : public CToolDialog
{
	DECLARE_DYNAMIC(CColorCtrl)

public:
	CColorCtrl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CColorCtrl();

// Dialog Data
	enum { IDD = IDD_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	CStatic m_c[6];
	CCol m_col[6];
protected:
	void PaintCol(CPaintDC &dc);

	CRect area[6];

	void OnClickedColStatic(int n);

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnStnClickedStaticC0();
	afx_msg void OnStnClickedStaticC1();
	afx_msg void OnStnClickedStaticC2();
	afx_msg void OnStnClickedStaticC3();
	afx_msg void OnStnClickedStaticC4();
	afx_msg void OnStnClickedStaticC5();
	void Receive(unsigned short message, UINT_PTR data, unsigned short extra);
protected:
	afx_msg LRESULT OnUmCellRclick(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUmDropPalSel(WPARAM wParam, LPARAM lParam);

public:
};
