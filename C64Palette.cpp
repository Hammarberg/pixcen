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


// C64Palette.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "C64Palette.h"
#include "afxdialogex.h"
#include "C64Col.h"

// C64Palette dialog

IMPLEMENT_DYNAMIC(C64Palette, CToolDialog)

C64Palette::C64Palette(CWnd* pParent /*=NULL*/)
	: CToolDialog(C64Palette::IDD, pParent)
{
	col1=1;
	col2=0;
}

C64Palette::~C64Palette()
{
}

void C64Palette::DoDataExchange(CDataExchange* pDX)
{
	CToolDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(C64Palette, CToolDialog)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
//	ON_MESSAGE(UM_SEL_COL1, &C64Palette::OnUmSelCol1)
END_MESSAGE_MAP()


// C64Palette message handlers




void C64Palette::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CToolDialog::OnPaint() for painting messages

	CRect rc;
	GetClientRect(rc);

	int x,y;
	int h=rc.bottom/8;
	int w=rc.right/2;

	for(x=0;x<2;x++)
	{
		for(y=0;y<8;y++)
		{
			dc.FillSolidRect(x*w,y*h,w,h,g_Vic2[x*8+y]);

			if(x*8+y == col1)
			{
				DrawSquare(dc,x*w,y*h,w,h,col1);
			}
			if(x*8+y == col2)
			{
				DrawEx(dc,x*w,y*h,w,h,col2);
			}
		}
	}
}

void C64Palette::DrawSquare(CPaintDC &dc, int x, int y, int w, int h, int c)
{
	CPen *old=dc.SelectObject(c==0?&(m_wpen):(&m_bpen));

	dc.MoveTo(x+3,y+3);
	dc.LineTo(x+w-3,y+3);
	dc.LineTo(x+w-3,y+h-3);
	dc.LineTo(x+3,y+h-3);
	dc.LineTo(x+3,y+2);

	dc.SelectObject(old);
}

void C64Palette::DrawEx(CPaintDC &dc, int x, int y, int w, int h, int c)
{
	CPen *old=dc.SelectObject(c==0?&(m_wpen):(&m_bpen));

	dc.MoveTo(x+8,y+8);
	dc.LineTo(x+w-8,y+h-8);
	dc.MoveTo(x+w-8,y+8);
	dc.LineTo(x+8,y+h-8);

	dc.SelectObject(old);
}

void C64Palette::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	Mail(MSG_PAL_SEL_COL,GetIndex(point),0);
	
	CToolDialog::OnLButtonDown(nFlags, point);
}


void C64Palette::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	Mail(MSG_PAL_SEL_COL,GetIndex(point),1);

	CToolDialog::OnRButtonDown(nFlags, point);
}


int C64Palette::GetIndex(CPoint &point)
{
	CRect rc;
	GetClientRect(rc);

	int h=rc.bottom/8;
	int w=rc.right/2;

	int x=point.x/w;
	int y=point.y/h;

	return x*8+y;
}



//void C64Palette::PreInitDialog()
//{
//	// TODO: Add your specialized code here and/or call the base class
//
//	CToolDialog::PreInitDialog();
//}


BOOL C64Palette::OnInitDialog()
{
	CToolDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_wpen.CreatePen(PS_SOLID,2,COLORREF(0x00ffffff));
	m_bpen.CreatePen(PS_SOLID,2,COLORREF(0x00000000));


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//Incomming message to force set col1
//afx_msg LRESULT C64Palette::OnUmSelCol1(WPARAM wParam, LPARAM lParam)
//{
//	col1 = int(wParam);
//	m_pParent->PostMessage(UM_SEL_COL1,WPARAM(col1),NULL);
//	Invalidate();
//	return 0;
//}

void C64Palette::Receive(unsigned short message, UINT_PTR data, unsigned short extra)
{
	switch(message)
	{

	case MSG_PAL_SEL_COL:
		if(!extra)
			col1 = int(data);
		else
			col2 = int(data);
		Invalidate();
		break;

	default:
		break;
	}
}
