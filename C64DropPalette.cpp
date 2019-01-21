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


// C64DropPalette.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "C64DropPalette.h"
#include "C64Col.h"
#include "Resource.h"


// C64DropPalette

IMPLEMENT_DYNAMIC(C64DropPalette, CWnd)

C64DropPalette::C64DropPalette()
{

}

C64DropPalette::~C64DropPalette()
{
}

BOOL C64DropPalette::Create(int x, int y, CWnd *pParent, int data)
{
	m_pParent = pParent;
	m_data = data;

	LPCTSTR pClass=AfxRegisterWndClass(CS_DROPSHADOW|CS_HREDRAW,0,0,0);

	const int SIZE = 16;

	BOOL n=  CreateEx(
		0, 
		pClass, _T(""),
		WS_POPUP|WS_BORDER,
		x, y,
		SIZE*2+2, SIZE*12+2,
		pParent->GetSafeHwnd(), (HMENU)0, (LPVOID)NULL);

	m_Image.LoadFromResource(AfxGetApp()->m_hInstance, IDB_BITMAP_DROP_PALETTE);

	return n;
}


BEGIN_MESSAGE_MAP(C64DropPalette, CWnd)
	ON_WM_PAINT()
	ON_WM_KILLFOCUS()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
//	ON_COMMAND(ID_EDIT_REDO, &C64DropPalette::OnEditRedo)
END_MESSAGE_MAP()



// C64DropPalette message handlers




void C64DropPalette::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages
	CRect rc;
	GetClientRect(rc);

	/*
	int x,y;
	int h=rc.bottom/8;
	int w=rc.right/2;

	for(x=0;x<2;x++)
	{
		for(y=0;y<8;y++)
		{
			dc.FillSolidRect(x*w,y*h,w,h,g_Vic2[x*8+y]);

		}
	}
	*/

	//m_Ima

	m_Image.StretchBlt(dc, rc);
}


void C64DropPalette::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	// TODO: Add your message handler code here
	DestroyWindow();
}


void C64DropPalette::OnDestroy()
{
	CWnd::OnDestroy();

	// TODO: Add your message handler code here
	delete this;
}


void C64DropPalette::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rc;
	GetClientRect(rc);

	int h=rc.bottom/12;
	int w=rc.right/2;

	int x=point.x/w;
	int y=point.y/h;

	if(y>=1 && y<=8)
	{
		//Color picked
		m_pParent->PostMessage(UM_DROP_PAL_SEL, x*8+(y-1), m_data);
	}
	else if(y==0)	
	{	
		//Upp
		m_pParent->PostMessage(UM_DROP_PAL_SEL, 100, m_data);
	}
	else if(y==9)	
	{	
		//Cripple
		m_pParent->PostMessage(UM_DROP_PAL_SEL, 101, m_data);
	}
	else if(y==10)	
	{	
		//Lock
		m_pParent->PostMessage(UM_DROP_PAL_SEL, 102, m_data);
	}
	else if(y==11)	
	{	
		//Down
		m_pParent->PostMessage(UM_DROP_PAL_SEL, 103, m_data);
	}

	DestroyWindow();

	//CWnd::OnLButtonDown(nFlags, point);
}


//void C64DropPalette::OnEditRedo()
//{
//	// TODO: Add your command handler code here
//}
