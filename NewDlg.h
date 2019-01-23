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


#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// CNewDlg dialog

class CNewDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNewDlg)

public:
	CNewDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewDlg();

// Dialog Data
	enum { IDD = IDD_NEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboSelect();
	afx_msg void OnBnClickedCheckMulti();
	afx_msg void OnDeltaposSpinX(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinY(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinZ(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditX();
	afx_msg void OnEnChangeEditY();
	afx_msg void OnEnChangeEditZ();
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	void UpdateInfo(void);

	int m_snap_x;
	int m_snap_y;

	static int Snap(int n, int snap)
	{
		n+=snap/2;
		return n - (n%snap);
	}

	int m_last_select;
	int m_select;

	BOOL m_last_multi;
	BOOL m_multi;

	int m_x;
	int m_y;
	int m_z;

	int m_std_x;
	int m_std_y;

	CString m_info;
	afx_msg void OnBnClickedReset();
	CButton m_ok;
};
