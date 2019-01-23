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


// CAddressDlg dialog

class CAddressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddressDlg)

public:
	CAddressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddressDlg();

// Dialog Data
	enum { IDD = IDD_ADDRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	CString m_addr;
public:
	int m_verified_address;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
