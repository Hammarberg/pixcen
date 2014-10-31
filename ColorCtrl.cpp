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


// ColorCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "ColorCtrl.h"
#include "afxdialogex.h"
#include "C64Col.h"
#include "C64Interface.h"
#include "C64DropPalette.h"

// CColorCtrl dialog

IMPLEMENT_DYNAMIC(CColorCtrl, CToolDialog)

CColorCtrl::CColorCtrl(CWnd* pParent /*=NULL*/)
	: CToolDialog(CColorCtrl::IDD, pParent)
{
	//col[0]=col[1]=col[2]=col[3]=col[4]=0;
	for(int r=0;r<6;r++)
		m_col[r].m_id = r;

}

CColorCtrl::~CColorCtrl()
{
}

void CColorCtrl::DoDataExchange(CDataExchange* pDX)
{
	CToolDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_0, m_c[0]);
	DDX_Control(pDX, IDC_STATIC_1, m_c[1]);
	DDX_Control(pDX, IDC_STATIC_2, m_c[2]);
	DDX_Control(pDX, IDC_STATIC_3, m_c[3]);
	DDX_Control(pDX, IDC_STATIC_4, m_c[4]);
	DDX_Control(pDX, IDC_STATIC_5, m_c[5]);
	DDX_Control(pDX, IDC_STATIC_C0, m_col[0]);
	DDX_Control(pDX, IDC_STATIC_C1, m_col[1]);
	DDX_Control(pDX, IDC_STATIC_C2, m_col[2]);
	DDX_Control(pDX, IDC_STATIC_C3, m_col[3]);
	DDX_Control(pDX, IDC_STATIC_C4, m_col[4]);
	DDX_Control(pDX, IDC_STATIC_C5, m_col[5]);
}


BEGIN_MESSAGE_MAP(CColorCtrl, CToolDialog)
	ON_WM_PAINT()
	ON_STN_CLICKED(IDC_STATIC_C0, &CColorCtrl::OnStnClickedStaticC0)
	ON_STN_CLICKED(IDC_STATIC_C1, &CColorCtrl::OnStnClickedStaticC1)
	ON_STN_CLICKED(IDC_STATIC_C2, &CColorCtrl::OnStnClickedStaticC2)
	ON_STN_CLICKED(IDC_STATIC_C3, &CColorCtrl::OnStnClickedStaticC3)
	ON_STN_CLICKED(IDC_STATIC_C4, &CColorCtrl::OnStnClickedStaticC4)
	ON_STN_CLICKED(IDC_STATIC_C5, &CColorCtrl::OnStnClickedStaticC5)
	ON_MESSAGE(UM_CELL_RCLICK, &CColorCtrl::OnUmCellRclick)
	ON_MESSAGE(UM_DROP_PAL_SEL, &CColorCtrl::OnUmDropPalSel)
END_MESSAGE_MAP()


// CColorCtrl message handlers


void CColorCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CToolDialog::OnPaint() for painting messages
	PaintCol(dc);

}

void CColorCtrl::PaintCol(CPaintDC &dc)
{
}


//afx_msg LRESULT CColorCtrl::OnUmCellInfo(WPARAM wParam, LPARAM lParam)
//{
//	return 0;
//}


BOOL CColorCtrl::OnInitDialog()
{
	CToolDialog::OnInitDialog();
	CPaintDC dc(this); // device context for painting

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CColorCtrl::OnClickedColStatic(int n)
{
	if(m_col[n].m_Color <= 0x0f)
	{
		Mail(MSG_PAL_SEL_COL, m_col[n].m_Color, 0);
	}
}


void CColorCtrl::OnStnClickedStaticC0()
{
	OnClickedColStatic(0);
}


void CColorCtrl::OnStnClickedStaticC1()
{
	OnClickedColStatic(1);
}


void CColorCtrl::OnStnClickedStaticC2()
{
	OnClickedColStatic(2);
}


void CColorCtrl::OnStnClickedStaticC3()
{
	OnClickedColStatic(3);
}


void CColorCtrl::OnStnClickedStaticC4()
{
	OnClickedColStatic(4);
}


void CColorCtrl::OnStnClickedStaticC5()
{
	OnClickedColStatic(5);
}


afx_msg LRESULT CColorCtrl::OnUmCellRclick(WPARAM wParam, LPARAM lParam)
{
	int c = int(wParam);

	if(m_col[c].m_Color != 0xff)
	{
		CPoint point;
		GetCursorPos(&point);

		C64DropPalette *p=new C64DropPalette();
		p->Create(point.x,point.y,this, (int)wParam);
		p->ShowWindow(SW_SHOW);
	}

	return 0;
}


afx_msg LRESULT CColorCtrl::OnUmDropPalSel(WPARAM wParam, LPARAM lParam)
{
	Mail(MSG_CELL_COL, UINT_PTR(wParam), unsigned short(lParam));
	return 0;
}

void CColorCtrl::Receive(unsigned short message, UINT_PTR data, unsigned short extra)
{
	switch(message)
	{
	case MSG_CELL_COL:
		{
			/*
			int c = extra;
			m_col[c].m_Color = int(data);
			m_col[c].Invalidate(TRUE);
			Invalidate(TRUE);
			*/
		}
		break;


	case MSG_CELL_INFO:
		{
			CellInfo *info = (CellInfo *)data;

			//Invalidate(TRUE);

			for(int r=0;r<6;r++)
			{
				m_col[r].m_Color=info->col[r];

				if(m_col[r].m_Color != 0xff)
				{
					m_c[r].EnableWindow(TRUE);
				}
				else
				{
					m_c[r].EnableWindow(FALSE);
				}

				m_col[r].m_lock = info->lock[r];
				m_col[r].m_crippled = info->crippled[r];

				m_col[r].Invalidate(TRUE);
			}


			delete info;
		}
	default:
		break;
	}
}



