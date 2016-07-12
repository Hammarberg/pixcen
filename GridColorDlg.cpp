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
