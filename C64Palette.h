/*
   Pixcen - A windows platform low level pixel editor for C64
   Copyright (C) 2013  John Hammarberg (crt@nospam.censordesign.com)
   
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

// C64Palette dialog

class C64Palette : public CToolDialog
{
	DECLARE_DYNAMIC(C64Palette)

public:
	C64Palette(CWnd *pParent = NULL);   // standard constructor
	virtual ~C64Palette();

// Dialog Data
	enum { IDD = IDD_C64PAL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	int GetIndex(CPoint &pt);
//	virtual void PreInitDialog();
	virtual BOOL OnInitDialog();

	void Receive(unsigned short message, UINT_PTR data, unsigned short extra);

private:
	CPen m_wpen;
	CPen m_bpen;
	CBrush m_brush;

	int col1;
	int col2;

	void DrawSquare(CPaintDC &dc, int x, int y, int w, int h, int c);
	void DrawEx(CPaintDC &dc, int x, int y, int w, int h, int c);
protected:
//	afx_msg LRESULT OnUmSelCol1(WPARAM wParam, LPARAM lParam);
};
