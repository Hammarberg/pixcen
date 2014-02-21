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



// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "ChildView.h"
#include "C64Palette.h"
#include "ColorCtrl.h"
#include "Preview.h"
#include "narray.h"

class CMainFrame : public CFrameWndEx, public GraphixMail
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;
	CChildView    m_wndView;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()


	C64Palette *m_pC64Pal;
	CColorCtrl *m_pCColCtrl;

	narray<CPreview *,int> m_pPreview;

public:
	virtual BOOL DestroyWindow();
protected:
//	afx_msg LRESULT OnUmStatusText(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnUmCellInfo(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnUmCellInfoCol(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUmGraphix(WPARAM wParam, LPARAM lParam);
	void Receive(unsigned short message, UINT_PTR data, unsigned short extra);

	static UINT __cdecl UpdateThread( LPVOID pParam );
	CWinThread *m_pUpdateThread;

public:
	afx_msg void OnClose();
	afx_msg void OnViewPreview();
	afx_msg void OnHelpCheckforupdates();
	afx_msg void OnUpdateHelpCheckforupdates(CCmdUI *pCmdUI);
protected:
	afx_msg LRESULT OnUmUpdateVersion(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnHelpHelp();
	afx_msg void OnHelpDownloadsourcecode();
};


