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


// ToolDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "ToolDialog.h"
#include "afxdialogex.h"


// CToolDialog dialog

IMPLEMENT_DYNAMIC(CToolDialog, CDialogEx)

CToolDialog::CToolDialog(UINT nIDTemplate, CWnd *pParent)
	: CDialogEx(nIDTemplate, pParent)
{
	m_pParent = pParent;
}


CToolDialog::~CToolDialog()
{
}

void CToolDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CToolDialog, CDialogEx)
	ON_WM_SETFOCUS()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()


// CToolDialog message handlers

void CToolDialog::OnSetFocus(CWnd* pOldWnd)
{
	//CDialogEx::OnSetFocus(pOldWnd);
	
	//CWnd *p=GetParent();

	//if(pOldWnd == m_pParent)
	{
		TRACKMOUSEEVENT tm;
		tm.cbSize=sizeof(tm);
		tm.dwFlags=TME_LEAVE;
		tm.hwndTrack=*this;
		tm.dwHoverTime=HOVER_DEFAULT;
		TrackMouseEvent(&tm);
	}
	
	// TODO: Add your message handler code here
}

void CToolDialog::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default
	
	m_pParent->SetFocus();

	CDialogEx::OnMouseLeave();
}


BOOL CToolDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CString name;
	GetWindowText(name);
	RECT *prc;
	UINT size;
	if(AfxGetApp()->GetProfileBinary(name,_T("pos"),(LPBYTE *)&prc,&size))
	{
		MoveWindow(prc);

		delete [] (LPBYTE)prc;
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CToolDialog::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	CString name;
	GetWindowText(name);
	RECT rc;
	GetWindowRect(&rc);
	AfxGetApp()->WriteProfileBinary(name,_T("pos"),(LPBYTE)&rc,sizeof(rc));



	return CDialogEx::DestroyWindow();
}
