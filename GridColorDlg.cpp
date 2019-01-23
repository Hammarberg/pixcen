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

// GridColorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "GridColorDlg.h"
#include "afxdialogex.h"


// CGridColorDlg dialog

IMPLEMENT_DYNAMIC(CGridColorDlg, CDialogEx)

CGridColorDlg::CGridColorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_GRID, pParent)
{

}

CGridColorDlg::~CGridColorDlg()
{
}

void CGridColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCCOLORBUTTON_CELL, m_CellColor);
	DDX_Control(pDX, IDC_MFCCOLORBUTTON_PIXEL, m_PixelColor);
}


BEGIN_MESSAGE_MAP(CGridColorDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CGridColorDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CGridColorDlg message handlers


void CGridColorDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}
