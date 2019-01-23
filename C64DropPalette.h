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
#include <atlimage.h>


// C64DropPalette

class C64DropPalette : public CWnd
{
	DECLARE_DYNAMIC(C64DropPalette)

public:
	C64DropPalette();
	virtual ~C64DropPalette();

	BOOL Create(int x, int y, CWnd *pParent, int data);

protected:
	CWnd *m_pParent;
	int m_data;
	DECLARE_MESSAGE_MAP()

	CImage m_Image;

public:
	afx_msg void OnPaint();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
//	afx_msg void OnEditRedo();
};


