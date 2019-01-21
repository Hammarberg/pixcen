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


// NewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "NewDlg.h"
#include "afxdialogex.h"


// CNewDlg dialog

IMPLEMENT_DYNAMIC(CNewDlg, CDialogEx)

CNewDlg::CNewDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNewDlg::IDD, pParent)
	, m_select(0)
	, m_multi(TRUE)
	, m_x(160)
	, m_y(200)
	, m_std_x(160)
	, m_std_y(200)
	, m_z(0)
	, m_info(_T(""))
{
	m_last_select = -1;
	m_last_multi = -1;

}

CNewDlg::~CNewDlg()
{
}

void CNewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBO_SELECT, m_select);
	DDX_Check(pDX, IDC_CHECK_MULTI, m_multi);
	DDX_Text(pDX, IDC_EDIT_X, m_x);
	DDX_Text(pDX, IDC_EDIT_Y, m_y);
	DDX_Text(pDX, IDC_EDIT_Z, m_z);
	DDX_Text(pDX, IDC_STATIC_INFO, m_info);
	DDX_Control(pDX, IDOK, m_ok);
}


BEGIN_MESSAGE_MAP(CNewDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_SELECT, &CNewDlg::OnCbnSelchangeComboSelect)
	ON_BN_CLICKED(IDC_CHECK_MULTI, &CNewDlg::OnBnClickedCheckMulti)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_X, &CNewDlg::OnDeltaposSpinX)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_Y, &CNewDlg::OnDeltaposSpinY)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_Z, &CNewDlg::OnDeltaposSpinZ)
	ON_EN_CHANGE(IDC_EDIT_X, &CNewDlg::OnEnChangeEditX)
	ON_EN_CHANGE(IDC_EDIT_Y, &CNewDlg::OnEnChangeEditY)
	ON_EN_CHANGE(IDC_EDIT_Z, &CNewDlg::OnEnChangeEditZ)
	ON_BN_CLICKED(IDC_RESET, &CNewDlg::OnBnClickedReset)
END_MESSAGE_MAP()


// CNewDlg message handlers


void CNewDlg::OnCbnSelchangeComboSelect()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	if(m_x == m_std_x && m_y == m_std_y)
	{
		UpdateInfo();
		OnBnClickedReset();
	}
	else
	{
		UpdateInfo();
	}
}


void CNewDlg::OnBnClickedCheckMulti()
{
	// TODO: Add your control notification handler code here
	OnCbnSelchangeComboSelect();
}


void CNewDlg::OnDeltaposSpinX(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	UpdateData(TRUE);

	m_x -= pNMUpDown->iDelta * m_snap_x;
	m_x = Snap(m_x,m_snap_x);

	UpdateInfo();
}


void CNewDlg::OnDeltaposSpinY(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	UpdateData(TRUE);

	m_y -= pNMUpDown->iDelta * m_snap_y;
	m_y = Snap(m_y,m_snap_y);

	UpdateInfo();
}


void CNewDlg::OnDeltaposSpinZ(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	UpdateData(TRUE);

	m_z -= pNMUpDown->iDelta;

	UpdateInfo();
}


void CNewDlg::OnEnChangeEditX()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	UpdateInfo();
}


void CNewDlg::OnEnChangeEditY()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	UpdateInfo();
}


void CNewDlg::OnEnChangeEditZ()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	UpdateInfo();
}

void CNewDlg::UpdateInfo(void)
{

	if(m_last_select != m_select || m_last_multi != m_multi)
	{
		m_last_select = m_select;
		m_last_multi = m_multi;
		m_z = 1;
		switch(m_select)
		{
		case 0:	//Bitmap
			m_snap_x = m_multi ? 4 : 8;
			m_snap_y = 8;
			m_std_x = m_multi ? 160 : 320;
			m_std_y = 200;
			break;
		case 1:	//Sprite
			m_snap_x = m_multi ? 12 : 24;
			m_snap_y = 21;
			m_std_x = m_multi ? 96 : 192;
			m_std_y = 21 * 8;
			break;
		case 2:	//Char
			m_snap_x = m_multi ? 4 : 8;
			m_snap_y = 8;
			m_std_x = m_multi ? 128: 256;
			m_std_y =  8 * 8;
			break;
		case 3:	//Unrestricted
			m_snap_x = 1;
			m_snap_y = 1;
			m_std_x = m_multi ? 32 : 64;
			m_std_y = 64;
			break;
		};
	}

	if(m_x < 0)
		m_x = 1;

	if(m_y < 0)
		m_y = 1;

	if(m_z < 1)
		m_z = 1;

	//m_x = Snap(m_x,m_snap_x);
	//m_y = Snap(m_y,m_snap_y);
	//m_z = Snap(m_z,1);

	CString sbitmaps;
	{
		int cx = m_x/(m_multi ? 4 : 8);
		int cy = m_y/8;
		sbitmaps.Format(_T("Bitmaps: X:%0.2f Y:%0.2f\n"),m_x/double(m_multi ? 160 : 320),  m_y/double(200));
	}

	CString schars;
	{
		int cx = m_x/(m_multi ? 4 : 8);
		int cy = m_y/8;
		schars.Format(_T("Characters X:%d Y:%d Sum:%d\n"),cx, cy, cx*cy);
	}

	CString ssprites;
	{
		int cx = m_x/(m_multi ? 12 : 24);
		int cy = m_y/21;
		ssprites.Format(_T("Sprites: X:%d Y:%d Sum:%d\n"),cx, cy, cx*cy);
	}


	switch(m_select)
	{
	case 0:
		m_info = sbitmaps + schars;
		break;
	case 1:	//Sprite
		m_info = ssprites;
		break;
	case 2:	//Char
		m_info = schars;
		break;
	case 3:	//Unrestricted
		m_info.Format(_T("%s: X:%0.2f Y:%0.2f\n"),m_multi ? _T("Wide"): _T("Square"), double(m_x),  double(m_y));
		break;
	};

	bool good = true;

	if(m_x != Snap(m_x,m_snap_x))
	{
		CString tmp;
		tmp.Format(_T("Warning: Width must be divisible by %d\n"),m_snap_x);
		m_info+=tmp;
		good = false;
	}

	if(m_y != Snap(m_y,m_snap_y))
	{
		CString tmp;
		tmp.Format(_T("Warning: Height must be divisible by %d\n"),m_snap_y);
		m_info+=tmp;
		good = false;
	}

	m_ok.EnableWindow(good?1:0);

	UpdateData(FALSE);
}

BOOL CNewDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	UpdateData(TRUE);
	UpdateInfo();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CNewDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialogEx::OnOK();
}


void CNewDlg::OnBnClickedReset()
{
	// TODO: Add your control notification handler code here
	m_x = m_std_x;
	m_y = m_std_y;
	UpdateInfo();
}
