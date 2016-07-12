#pragma once
#include "afxcolorbutton.h"


// CGridColorDlg dialog

class CGridColorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGridColorDlg)

public:
	CGridColorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGridColorDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GRID };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CMFCColorButton m_CellColor;
	CMFCColorButton m_PixelColor;
};
