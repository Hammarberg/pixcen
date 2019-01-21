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


// ImportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "ImportDlg.h"
#include "afxdialogex.h"


// CImportDlg dialog

IMPLEMENT_DYNAMIC(CImportDlg, CDialogEx)

CImportDlg::CImportDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CImportDlg::IDD, pParent)
{
	m_nSelect = 0;
}

CImportDlg::~CImportDlg()
{
}

void CImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SELECT, m_Select);
}


BEGIN_MESSAGE_MAP(CImportDlg, CDialogEx)
END_MESSAGE_MAP()


// CImportDlg message handlers


BOOL CImportDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	m_Select.SetCurSel(m_nSelect);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CImportDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	m_nSelect = m_Select.GetCurSel();

	CDialogEx::OnOK();
}
