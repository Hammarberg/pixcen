// RemapColourDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "RemapColourDlg.h"
#include "afxdialogex.h"


// RemapColourDlg dialog

IMPLEMENT_DYNAMIC(RemapColourDlg, CDialogEx)

RemapColourDlg::RemapColourDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_COLOURMAPPER, pParent)
{

}

RemapColourDlg::~RemapColourDlg()
{
}

void RemapColourDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COLOUR, m_ColourToMap);
	DDX_Control(pDX, IDC_MAP, m_ColourDest);
}


BEGIN_MESSAGE_MAP(RemapColourDlg, CDialogEx)
	ON_LBN_SELCHANGE(IDC_COLOUR, &RemapColourDlg::OnLbnSelchangeColour)
END_MESSAGE_MAP()


// RemapColourDlg message handlers


void RemapColourDlg::OnLbnSelchangeColour()
{
	// TODO: Add your control notification handler code here
}

void RemapColourDlg::setDataVectors(const std::vector<LPCTSTR>* colour, const std::vector<LPCTSTR>* dest)
{
	m_pColourVector = colour;
	m_pDestVector = dest;
}

int RemapColourDlg::GetSelectedColourIndex()
{
	return m_selectedColourIndex;
}

int RemapColourDlg::GetSelectedDestIndex()
{
	return m_selectedDestIndex;
}


BOOL RemapColourDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	for (auto ci = m_pColourVector->cbegin(); ci != m_pColourVector->cend(); ++ci)
	{
		m_ColourToMap.AddString(*ci);
	}

	for (auto ci = m_pDestVector->cbegin(); ci != m_pDestVector->cend(); ++ci)
	{
		m_ColourDest.AddString(*ci);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void RemapColourDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	m_selectedColourIndex = m_ColourToMap.GetCurSel();
	m_selectedDestIndex = m_ColourDest.GetCurSel();
	CDialogEx::OnOK();
}
