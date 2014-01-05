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


// Address.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "AddressDlg.h"
#include "afxdialogex.h"


// CAddressDlg dialog

IMPLEMENT_DYNAMIC(CAddressDlg, CDialogEx)

CAddressDlg::CAddressDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAddressDlg::IDD, pParent)
	, m_addr(_T(""))
{
	m_verified_address = 0;
}

CAddressDlg::~CAddressDlg()
{
}

void CAddressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT, m_addr);
}


BEGIN_MESSAGE_MAP(CAddressDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddressDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAddressDlg message handlers


void CAddressDlg::OnBnClickedOk()
{
	UpdateData();
	if(!_stscanf(m_addr, _T("%x"), &m_verified_address))return;
	CDialogEx::OnOK();
}


BOOL CAddressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	TCHAR str[32];
	_stprintf(str, _T("%04X"), m_verified_address);
	m_addr = str;

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
