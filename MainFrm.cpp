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



// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Pixcen.h"

#include "MainFrm.h"

#include <afxinet.h>

#include "res\version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	ON_MESSAGE(UM_GRAPHIX, &CMainFrame::OnUmGraphix)
	ON_WM_CLOSE()
	ON_COMMAND(ID_VIEW_PREVIEW, &CMainFrame::OnViewPreview)
	ON_COMMAND(ID_HELP_CHECKFORUPDATES, &CMainFrame::OnHelpCheckforupdates)
	ON_UPDATE_COMMAND_UI(ID_HELP_CHECKFORUPDATES, &CMainFrame::OnUpdateHelpCheckforupdates)
	ON_MESSAGE(UM_UPDATE_VERSION, &CMainFrame::OnUmUpdateVersion)
	ON_COMMAND(ID_HELP_HELP, &CMainFrame::OnHelpHelp)
	ON_COMMAND(ID_HELP_DOWNLOADSOURCECODE, &CMainFrame::OnHelpDownloadsourcecode)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
	m_pC64Pal = NULL;
	m_pCColCtrl = NULL;
	m_pUpdateThread = NULL;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;
	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW/*|WS_HSCROLL|WS_VSCROLL*/, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	m_wndView.DragAcceptFiles();

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	// Allow user-defined toolbars operations:
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Enable toolbar and docking window menu replacement
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == NULL)
	{
		// load user-defined toolbar images
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}

	// enable menu personalization (most-recently used commands)
	// TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.
	/*
	CList<UINT, UINT> lstBasicCommands;

	lstBasicCommands.AddTail(ID_APP_EXIT);
	lstBasicCommands.AddTail(ID_EDIT_CUT);
	lstBasicCommands.AddTail(ID_EDIT_PASTE);
	lstBasicCommands.AddTail(ID_EDIT_UNDO);
	lstBasicCommands.AddTail(ID_EDIT_REDO);
	lstBasicCommands.AddTail(ID_APP_ABOUT);
	lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2003);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_VS_2005);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLUE);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_SILVER);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLACK);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_AQUA);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_WINDOWS_7);

	CMFCToolBar::SetBasicCommands(lstBasicCommands);
	*/

	m_pC64Pal = new C64Palette(&m_wndView);
	m_pC64Pal->Create(IDD_C64PAL, &m_wndView);
	m_pC64Pal->ShowWindow(SW_SHOW);


	m_pCColCtrl = new CColorCtrl(&m_wndView);
	m_pCColCtrl->Create(IDD_CONTROL, &m_wndView);
	m_pCColCtrl->ShowWindow(SW_SHOW);


	OnHelpCheckforupdates();

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}


	// enable customization button for all user toolbars
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
}



BOOL CMainFrame::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	m_pC64Pal->DestroyWindow();
	delete m_pC64Pal;

	m_pCColCtrl->DestroyWindow();
	delete m_pCColCtrl;

	for(int r=0;r<m_pPreview.count();r++)
	{
		m_pPreview[r]->DestroyWindow();
		delete m_pPreview[r];
	}

	return CFrameWndEx::DestroyWindow();
}


afx_msg LRESULT CMainFrame::OnUmGraphix(WPARAM wParam, LPARAM lParam)
{
	unsigned short message = lParam&0xffff, extra = lParam >> 16;
	UINT_PTR data = UINT_PTR(wParam);

	Receive(message,data,extra);
	m_pC64Pal->Receive(message,data,extra);
	m_pCColCtrl->Receive(message,data,extra);
	m_wndView.Receive(message,data,extra);

	for(int r=0;r<m_pPreview.count();r++)
	{
		m_pPreview[r]->Receive(message,data,extra);
	}

	return 0;
}

void CMainFrame::Receive(unsigned short message, UINT_PTR data, unsigned short extra)
{
	switch(message)
	{
	case MSG_STATUS_TEXT:
		{
			TCHAR *p=(TCHAR *)data;
			m_wndStatusBar.SetPaneText(0,p);
			delete [] p;
		}
		break;

	case MSG_PREV_CLOSE:
		{
			CPreview *p=(CPreview *)data;
			p->DestroyWindow();
			delete p;
			int n=m_pPreview.find(p);
			if(n!=-1)
			{
				m_pPreview.remove(n);
			}
		}
		break;
	default:
		break;
	}
}


void CMainFrame::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	if(this->m_wndView.FrameClose())
	{
		__super::OnClose();
	}
}


void CMainFrame::OnViewPreview()
{
	// TODO: Add your command handler code here
	CPreview *p=new CPreview(1.0/m_wndView.m_pbm->GetPARValue());

	LPCTSTR pClass=AfxRegisterWndClass(CS_DROPSHADOW|CS_HREDRAW,0,0,0);
	p->CreateEx(0,pClass,_T("Preview"),WS_OVERLAPPEDWINDOW|WS_VISIBLE,100,100,320,200,*this,(HMENU)0);

	p->m_ppInterface = &m_wndView.m_pbm;

	m_pPreview.add(p);
}

UINT CMainFrame::UpdateThread( LPVOID pParam )
{
	try
	{
		CMainFrame *pWnd = (CMainFrame *)pParam;
		CInternetSession session; 
		CHttpFile* file = NULL; 
		file = (CHttpFile *)session.OpenURL(_T("http://censordesign.com/pixcen/ver"), 1UL, INTERNET_FLAG_TRANSFER_ASCII|INTERNET_FLAG_RELOAD); 

		if (NULL != file)
		{
			char txt[17];

			UINT n=file->Read(txt,16);

			file->Close();

			if(n>0)
			{
				txt[n]=0;
				::PostMessage(*pWnd, UM_UPDATE_VERSION, WPARAM(atoi(txt)), NULL);
			}
			else
			{
				::PostMessage(*pWnd, UM_UPDATE_VERSION, WPARAM(0), NULL);
			}

			//Do something here with the web request
			//Clean up the file here to avoid a memory leak!!!!!!!


			delete file; 

		}

		session.Close(); 
	}
	catch(CInternetException *ex)
	{
		ex->Delete();
	}

	return 0;
}

afx_msg LRESULT CMainFrame::OnUmUpdateVersion(WPARAM wParam, LPARAM lParam)
{
	int ver = int(wParam);

	if(ver > VERSION_NUM)
	{
		if(MessageBox(_T("There is an updated version of Pixcen.\nWould you like to download?"),_T("New version"),MB_YESNO|MB_ICONQUESTION) == IDYES)
		{
			ShellExecute(NULL,L"open",L"http://censordesign.com/pixcen/pixcen.zip",NULL,NULL,SW_SHOWNORMAL);
		}
	}

	//m_pUpdateThread->Delete();

	m_pUpdateThread = NULL;
	return 0;
}


void CMainFrame::OnHelpCheckforupdates()
{
	// TODO: Add your command handler code here
	if(m_pUpdateThread)return;
	m_pUpdateThread = AfxBeginThread(UpdateThread, this);
}


void CMainFrame::OnUpdateHelpCheckforupdates(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_pUpdateThread ? 0 : 1);
}



void CMainFrame::OnHelpHelp()
{
	// TODO: Add your command handler code here
	ShellExecute(NULL,L"open",L"http://censordesign.com/pixcen/manual.txt",NULL,NULL,SW_SHOWNORMAL);
}


void CMainFrame::OnHelpDownloadsourcecode()
{
	// TODO: Add your command handler code here
	ShellExecute(NULL,L"open",L"http://censordesign.com/pixcen/pixcen_src.zip",NULL,NULL,SW_SHOWNORMAL);
}
