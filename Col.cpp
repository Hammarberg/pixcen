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


// Col.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "Col.h"
#include "C64Col.h"


// CCol

IMPLEMENT_DYNAMIC(CCol, CStatic)

CCol::CCol(int id)
{
	m_Color = 0;
	m_na.CreateHatchBrush(HS_BDIAGONAL, COLORREF(0));
	m_id=id;
	m_lock=0;
	m_crippled=0;
}

CCol::~CCol()
{
}


BEGIN_MESSAGE_MAP(CCol, CStatic)
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()



// CCol message handlers




void CCol::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rc;
	GetClientRect(rc);

	if(m_Color==0xff)
	{
		if(m_crippled)
		{
			COLORREF colour = GetSysColor(COLOR_3DFACE);
			dc.FillSolidRect(rc,colour);

			CBrush *oldbrush;
			CPen *oldpen,pen(PS_SOLID, 1, g_Vic2[m_Color]);
			oldpen=dc.SelectObject(&pen);
			oldbrush=dc.SelectObject(&m_na);
			dc.Ellipse(rc.left, rc.top, rc.right, rc.bottom);
			dc.SelectObject(oldbrush);
			dc.SelectObject(oldpen);
		}
		else
		{
			dc.FillRect(rc,&m_na);
		}
	}
	else if(m_Color==0xfe)
	{
		CRect r;

		r.top=rc.top;
		r.bottom = rc.CenterPoint().y;
		r.left=rc.left;
		r.right = rc.CenterPoint().x;

		dc.FillSolidRect(r,g_Vic2[2]);

		r.left=rc.CenterPoint().x;
		r.right = rc.right;

		dc.FillSolidRect(r,g_Vic2[3]);

		r.top=rc.CenterPoint().y;
		r.bottom = rc.bottom;
		r.left=rc.left;
		r.right = rc.CenterPoint().x;

		dc.FillSolidRect(r,g_Vic2[4]);

		r.left=rc.CenterPoint().x;
		r.right = rc.right;

		dc.FillSolidRect(r,g_Vic2[5]);
	}
	else
	{
		if(m_crippled)
		{
			COLORREF colour = GetSysColor(COLOR_3DFACE);
			dc.FillSolidRect(rc,colour);

			CPen *oldpen,pen(PS_SOLID, 1, g_Vic2[m_Color]);
			CBrush *oldbrush, solid;
			solid.CreateSolidBrush(g_Vic2[m_Color]);
			oldpen=dc.SelectObject(&pen);
			oldbrush=dc.SelectObject(&solid);
			dc.Ellipse(rc.left, rc.top, rc.right, rc.bottom);
			dc.SelectObject(oldbrush);
			dc.SelectObject(oldpen);
		}
		else
		{
			dc.FillSolidRect(rc,g_Vic2[m_Color]);
		}
	}

	if(m_lock)
	{
		CPen *oldpen,pen(PS_SOLID, 2, m_Color==2?g_Vic2[1]:g_Vic2[2]);
		oldpen=dc.SelectObject(&pen);
		dc.MoveTo(rc.left, rc.top);
		dc.LineTo(rc.right, rc.bottom);
		dc.SelectObject(oldpen);
	}
}


void CCol::OnRButtonDown(UINT nFlags, CPoint point)
{
	GetParent()->PostMessage(UM_CELL_RCLICK, m_id);
	CStatic::OnRButtonDown(nFlags, point);
}
