#pragma once
#include "afxwin.h"
#include <vector>

// RemapColourDlg dialog

class RemapColourDlg : public CDialogEx
{
	DECLARE_DYNAMIC(RemapColourDlg)

public:
	RemapColourDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~RemapColourDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COLOURMAPPER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	const std::vector<LPCTSTR>* m_pColourVector;
	const std::vector<LPCTSTR>* m_pDestVector;
	int m_selectedColourIndex;
	int m_selectedDestIndex;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLbnSelchangeColour();
	CListBox m_ColourToMap;
	CListBox m_ColourDest;

	void setDataVectors(const std::vector<LPCTSTR>* colour, const std::vector<LPCTSTR>* dest);

	int GetSelectedColourIndex();
	int GetSelectedDestIndex();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
