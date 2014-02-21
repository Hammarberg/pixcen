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


// Lock.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "Lock.h"


// CLock

IMPLEMENT_DYNAMIC(CLock, CStatic)

CLock::CLock()
{
	m_locked = 0;
	m_img_lock[0].LoadFromResource(AfxGetInstanceHandle(),IDB_BITMAP_UNLOCK);
	m_img_lock[1].LoadFromResource(AfxGetInstanceHandle(),IDB_BITMAP_LOCK);
}

CLock::~CLock()
{
}


BEGIN_MESSAGE_MAP(CLock, CStatic)
//	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CLock message handlers




//void CLock::OnLButtonDown(UINT nFlags, CPoint point)
//{
//	// TODO: Add your message handler code here and/or call default
//
//	CStatic::OnLButtonDown(nFlags, point);
//}


void CLock::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CStatic::OnPaint() for painting messages
	CRect rc;
	GetClientRect(rc);

	m_img_lock[m_locked].BitBlt(dc,rc,CPoint(0,0));
}


BOOL CLock::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return CStatic::OnEraseBkgnd(pDC);
}
