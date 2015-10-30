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



// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "Pixcen.h"
#include "ChildView.h"
#include <WinUser.h>
#include "ImportDlg.h"
#include "NewDlg.h"
#include "AddressDlg.h"
#include "Monomap.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define ZOOMVALUE 240

// CChildView

CChildView::CChildView()
{
	m_Zoom = 0;

	m_pbm = new MCBitmap;

	//Show intro pic (Miss Pixcen) on the first load only
	if(AfxGetApp()->GetProfileInt(_T("Intro"),_T("Shown"),0) == 0)
	{
		LoadIntro();
		AfxGetApp()->WriteProfileInt(_T("Intro"),_T("Shown"),1);
	}

	m_pselection = NULL;

	m_Zoom = 340;
	m_ZoomPoint.SetPoint(0,0);
	m_posX = 0;
	m_posY = 0;
	m_lastScale = 1;
	m_Grid = 1;
	m_CellGrid = 1;

	m_Col1 = 1;	//White
	m_Col2 = 0;	//Black

	m_fLine.CreatePen(PS_SOLID,1,COLORREF(0x00101010));
	m_cLine.CreatePen(PS_SOLID,2,COLORREF(0x00404040));
	m_sLine[0].CreatePen(PS_DOT,1,COLORREF(0x00e0e0e0));
	m_sLine[1].CreatePen(PS_DASH,1,COLORREF(0x00202020));

	m_Move = false;
	m_Paint = false;
	m_AutoMarker = false;
	m_ManualMarker = false;
	m_Marker = false;
	m_MarkerCount = 0;
	m_CellSnapMarker = false;

	m_LastPaintPix.x = -1;
	m_LastPaintPix.y = -1;

	m_ActivePaintCol = 0;

	m_TimeCount = 0;

	m_Paste = false;
	m_MaskedPaste = false;
	m_pPasteBuffer = NULL;
	m_PasteSizeX = 0;
	m_PasteSizeY = 0;

	m_ColorPick = false;

	m_pBackBufferDC = NULL;

	m_Fill = false;

	m_goodFormat = false;
}

CChildView::~CChildView()
{
	delete m_pBackBufferDC;
	delete m_pbm;
	delete m_pPasteBuffer;
	if(m_pselection)delete m_pselection;
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_MOUSEWHEEL()
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_FILE_LOAD, &CChildView::OnFileLoad)
	ON_COMMAND(ID_FILE_SAVE, &CChildView::OnFileSave)
ON_COMMAND(ID_EDIT_UNDO, &CChildView::OnEditUndo)
ON_COMMAND(ID_EDIT_REDO, &CChildView::OnEditRedo)
ON_COMMAND(ID_FILE_SAVEAS, &CChildView::OnFileSaveas)
ON_COMMAND(ID_VIEW_GRID, &CChildView::OnViewGrid)
ON_UPDATE_COMMAND_UI(ID_VIEW_GRID, &CChildView::OnUpdateViewGrid)
ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CChildView::OnUpdateFileSave)
ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CChildView::OnUpdateEditUndo)
ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CChildView::OnUpdateEditRedo)
ON_COMMAND(ID_TOOL_OPTIMIZE, &CChildView::OnToolOptimize)
ON_UPDATE_COMMAND_UI(ID_TOOL_OPTIMIZE, &CChildView::OnUpdateToolOptimize)
ON_COMMAND(ID_MODE_BITMAP, &CChildView::OnModeBitmap)
ON_UPDATE_COMMAND_UI(ID_MODE_BITMAP, &CChildView::OnUpdateModeBitmap)
ON_COMMAND(ID_MODE_SPRITE, &CChildView::OnModeSprite)
ON_UPDATE_COMMAND_UI(ID_MODE_SPRITE, &CChildView::OnUpdateModeSprite)
ON_COMMAND(ID_MODE_CHAR, &CChildView::OnModeChar)
ON_UPDATE_COMMAND_UI(ID_MODE_CHAR, &CChildView::OnUpdateModeChar)
ON_COMMAND(ID_MODE_MULTI, &CChildView::OnModeMulti)
ON_UPDATE_COMMAND_UI(ID_MODE_MULTI, &CChildView::OnUpdateModeMulti)
ON_WM_DROPFILES()
ON_WM_CREATE()
ON_WM_DESTROY()
ON_WM_TIMER()
ON_COMMAND(ID_EDIT_CUT, &CChildView::OnEditCut)
ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, &CChildView::OnUpdateEditCut)
ON_COMMAND(ID_EDIT_COPY, &CChildView::OnEditCopy)
ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CChildView::OnUpdateEditCopy)
ON_COMMAND(ID_EDIT_PASTE, &CChildView::OnEditPaste)
ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CChildView::OnUpdateEditPaste)
ON_COMMAND(ID_MODE_UNRESTRICTED, &CChildView::OnModeUnrestricted)
ON_UPDATE_COMMAND_UI(ID_MODE_UNRESTRICTED, &CChildView::OnUpdateModeUnrestricted)
ON_COMMAND(ID_EDIT_SELECTMARKER, &CChildView::OnEditSelectmarker)
ON_COMMAND(ID_EDIT_SELECTCELL, &CChildView::OnEditSelectcell)
ON_COMMAND(ID_EDIT_SELECTALL, &CChildView::OnEditSelectall)
ON_COMMAND(ID_FILE_NEW, &CChildView::OnFileNew)
ON_COMMAND(ID_EDIT_DESELECT, &CChildView::OnEditDeselect)
ON_UPDATE_COMMAND_UI(ID_EDIT_DESELECT, &CChildView::OnUpdateEditDeselect)
ON_COMMAND(ID_COPYROMFONT_UPPERCASE, &CChildView::OnCopyromfontUppercase)
ON_UPDATE_COMMAND_UI(ID_COPYROMFONT_UPPERCASE, &CChildView::OnUpdateCopyromfontUppercase)
ON_COMMAND(ID_COPYROMFONT_LOWERCASE, &CChildView::OnCopyromfontLowercase)
ON_UPDATE_COMMAND_UI(ID_COPYROMFONT_LOWERCASE, &CChildView::OnUpdateCopyromfontLowercase)
ON_UPDATE_COMMAND_UI(ID_SELECTION_FLIPVERTICALLY, &CChildView::OnUpdateSelectionFlipvertically)
ON_COMMAND(ID_SELECTION_FLIPVERTICALLY, &CChildView::OnSelectionFlipvertically)
ON_UPDATE_COMMAND_UI(ID_SELECTION_FLIPHORIZONTALLY, &CChildView::OnUpdateSelectionFliphorizontally)
ON_COMMAND(ID_SELECTION_FLIPHORIZONTALLY, &CChildView::OnSelectionFliphorizontally)
ON_UPDATE_COMMAND_UI(ID_SELECTION_ROTATECW, &CChildView::OnUpdateSelectionRotatecw)
ON_COMMAND(ID_SELECTION_ROTATECW, &CChildView::OnSelectionRotatecw)
ON_UPDATE_COMMAND_UI(ID_SELECTION_ROTATECCW, &CChildView::OnUpdateSelectionRotateccw)
ON_COMMAND(ID_SELECTION_ROTATECCW, &CChildView::OnSelectionRotateccw)
ON_UPDATE_COMMAND_UI(ID_SELECTION_SHIFTLEFT, &CChildView::OnUpdateSelectionShiftleft)
ON_UPDATE_COMMAND_UI(ID_SELECTION_SHIFTRIGHT, &CChildView::OnUpdateSelectionShiftright)
ON_UPDATE_COMMAND_UI(ID_SELECTION_SHIFTUP, &CChildView::OnUpdateSelectionShiftup)
ON_UPDATE_COMMAND_UI(ID_SELECTION_SHIFTDOWN, &CChildView::OnUpdateSelectionShiftdown)
ON_COMMAND(ID_SELECTION_SHIFTLEFT, &CChildView::OnSelectionShiftleft)
ON_COMMAND(ID_SELECTION_SHIFTRIGHT, &CChildView::OnSelectionShiftright)
ON_COMMAND(ID_SELECTION_SHIFTUP, &CChildView::OnSelectionShiftup)
ON_COMMAND(ID_SELECTION_SHIFTDOWN, &CChildView::OnSelectionShiftdown)
ON_UPDATE_COMMAND_UI(ID_EDIT_AUTO_SELECT, &CChildView::OnUpdateEditAutoSelect)
ON_COMMAND(ID_EDIT_AUTO_SELECT, &CChildView::OnEditAutoSelect)
ON_COMMAND(ID_VIEW_NEXTBUFFER, &CChildView::OnViewNextbuffer)
ON_UPDATE_COMMAND_UI(ID_VIEW_NEXTBUFFER, &CChildView::OnUpdateViewNextbuffer)
ON_COMMAND(ID_VIEW_PREVBUFFER, &CChildView::OnViewPrevbuffer)
ON_UPDATE_COMMAND_UI(ID_VIEW_PREVBUFFER, &CChildView::OnUpdateViewPrevbuffer)
ON_COMMAND(ID_EDIT_BACKGROUNDMASKEDPASTE, &CChildView::OnEditBackgroundmaskedpaste)
ON_UPDATE_COMMAND_UI(ID_EDIT_BACKGROUNDMASKEDPASTE, &CChildView::OnUpdateEditBackgroundmaskedpaste)
ON_UPDATE_COMMAND_UI(ID_OVERFLOW_IGNORE, &CChildView::OnUpdateOverflowIgnore)
ON_COMMAND(ID_OVERFLOW_IGNORE, &CChildView::OnOverflowIgnore)
ON_UPDATE_COMMAND_UI(ID_OVERFLOW_CLOSEST, &CChildView::OnUpdateOverflowClosest)
ON_COMMAND(ID_OVERFLOW_CLOSEST, &CChildView::OnOverflowClosest)
ON_UPDATE_COMMAND_UI(ID_OVERFLOW_REPLACE, &CChildView::OnUpdateOverflowReplace)
ON_COMMAND(ID_OVERFLOW_REPLACE, &CChildView::OnOverflowReplace)
ON_COMMAND(ID_COLOROVERFLOW_TOGGLEIGNORE, &CChildView::OnColoroverflowToggleignore)
ON_COMMAND(ID_VIEW_RESTOREMAINVIEW, &CChildView::OnViewRestoremainview)
ON_UPDATE_COMMAND_UI(ID_PREVIEWPIXELASPECTRATIO_PC, &CChildView::OnUpdatePreviewpixelaspectratioPc)
ON_COMMAND(ID_PREVIEWPIXELASPECTRATIO_PC, &CChildView::OnPreviewpixelaspectratioPc)
ON_UPDATE_COMMAND_UI(ID_PREVIEWPIXELASPECTRATIO_PAL, &CChildView::OnUpdatePreviewpixelaspectratioPal)
ON_COMMAND(ID_PREVIEWPIXELASPECTRATIO_PAL, &CChildView::OnPreviewpixelaspectratioPal)
ON_UPDATE_COMMAND_UI(ID_PREVIEWPIXELASPECTRATIO_NTSC, &CChildView::OnUpdatePreviewpixelaspectratioNtsc)
ON_COMMAND(ID_PREVIEWPIXELASPECTRATIO_NTSC, &CChildView::OnPreviewpixelaspectratioNtsc)
ON_UPDATE_COMMAND_UI(ID_FILE_SAVESELECTION, &CChildView::OnUpdateFileSaveselection)
ON_COMMAND(ID_FILE_SAVESELECTION, &CChildView::OnFileSaveselection)
ON_UPDATE_COMMAND_UI(ID_EDIT_SNAPSELECTION, &CChildView::OnUpdateEditSnapselection)
ON_COMMAND(ID_EDIT_SNAPSELECTION, &CChildView::OnEditSnapselection)
ON_UPDATE_COMMAND_UI(ID_TOOL_DELETEUNDOHISTORY, &CChildView::OnUpdateToolDeleteundohistory)
ON_COMMAND(ID_TOOL_DELETEUNDOHISTORY, &CChildView::OnToolDeleteundohistory)
ON_UPDATE_COMMAND_UI(ID_PALETTE_DUMMY, &CChildView::OnUpdateModePalette)
ON_COMMAND_RANGE(ID_MODE_PALETTE_0, ID_MODE_PALETTE_F, &CChildView::OnModePalette)
ON_UPDATE_COMMAND_UI_RANGE(ID_MODE_PALETTE_0, ID_MODE_PALETTE_F, &CChildView::OnUpdateModePaletteRange)
ON_COMMAND(ID_TOOL_FILL, &CChildView::OnToolFill)
ON_COMMAND(ID_VIEW_CELLGRID, &CChildView::OnViewCellgrid)
ON_UPDATE_COMMAND_UI(ID_VIEW_CELLGRID, &CChildView::OnUpdateViewCellgrid)
ON_COMMAND(ID_HELP_LOADINTRO, &CChildView::OnHelpLoadintro)
ON_UPDATE_COMMAND_UI(ID_FONT_1X1, &CChildView::OnUpdateFont1x1)
ON_COMMAND(ID_FONT_1X1, &CChildView::OnFont1x1)
ON_UPDATE_COMMAND_UI(ID_FONT_2X1, &CChildView::OnUpdateFont2x1)
ON_COMMAND(ID_FONT_2X1, &CChildView::OnFont2x1)
ON_UPDATE_COMMAND_UI(ID_FONT_1X2, &CChildView::OnUpdateFont1x2)
ON_COMMAND(ID_FONT_1X2, &CChildView::OnFont1x2)
ON_UPDATE_COMMAND_UI(ID_FONT_2X2, &CChildView::OnUpdateFont2x2)
ON_COMMAND(ID_FONT_2X2, &CChildView::OnFont2x2)
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetTimer(1,100,NULL);

	return 0;
}


void CChildView::OnDestroy()
{
	__super::OnDestroy();

	KillTimer(1);
}


void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	m_TimeCount++;

	if(m_MarkerCount > 0 || m_Paste)
		Invalidate();

	if(m_TimeCount%5 == 0)
	{
		CPoint pix;
		if(GetPixelFromPoint(m_MovePoint, pix))
			UpdateStatus(pix);
	}

	__super::OnTimer(nIDEvent);
}


bool CChildView::GetPixelFromPoint(CPoint &point, CPoint &pixel, bool select)
{
	int scale = 1+m_Zoom/240;

	int x,y,mx=m_pbm->GetSizeX(),my=m_pbm->GetSizeY(),pw=m_pbm->GetPixelWidth();
	int bmx = scale * mx * pw;
	int bmy = scale * my;

	x = (point.x - m_posX);
	y = (point.y - m_posY);

	if(select)
	{
		x+=(scale*pw)/2;
		y+=scale/2;

		if(x<0)x=0;
		else if(x>bmx)x=bmx;

		if(y<0)y=0;
		else if(y>bmy)y=bmy;
	}
	else if(x < 0 || x >= bmx || y < 0 || y >= bmy) return false;	//Outside
	
	pixel.x = x / (scale * pw);
	pixel.y = y / (scale);

	return true;
}


void CChildView::OnPaint() 
{
	CPaintDC realdc(this); // device context for painting

	CRect rc;
	GetClientRect(rc);

	if(!m_pBackBufferDC)
	{
		m_pBackBufferDC = new CDC;
		m_pBackBufferDC->CreateCompatibleDC(&realdc);
	}

	C64Interface *pbm = m_pbm;

	CBitmap *bitmap;
	bitmap = m_pBackBufferDC->GetCurrentBitmap();
	BITMAP bm;
	bitmap->GetBitmap(&bm);

	if(bm.bmWidth != rc.right || bm.bmHeight != rc.bottom)
	{
		CBitmap newBm;
		newBm.CreateCompatibleBitmap(&realdc, rc.right, rc.bottom);
		CBitmap *pOld = m_pBackBufferDC->SelectObject(&newBm);
		pOld->DeleteObject();
	}

	CDC &dc = *m_pBackBufferDC;

	int scale = 1+m_Zoom/ZOOMVALUE;

	int x,y,mx=pbm->GetSizeX(),my=pbm->GetSizeY(),pw=pbm->GetPixelWidth();
	int bmx = scale * mx * pw;
	int bmy = scale * my;
	int cw=pbm->GetCellSizeX();
	int ch=pbm->GetCellSizeY();

	COLORREF border = g_Vic2[pbm->GetBorderColor()];

	//Change border color slightly from palette
	BYTE *bd=(BYTE *)&border;
	for(x=3;x>=1;x--)
	{
		if(bd[x]>=0xf0)bd[x]-=0x10;
		else bd[x]+=0x10;
	}
	

	int oldbmx = m_lastScale * mx * pw;
	int oldbmy = m_lastScale * my;

	double divx = (m_ZoomPoint.x-m_posX)/double(oldbmx);
	double divy = (m_ZoomPoint.y-m_posY)/double(oldbmy);

	if(divx>1.0)divx=1.0;
	if(divx<0.0)divx=0.0;
	if(divy>1.0)divy=1.0;
	if(divy<0.0)divy=0.0;

	int startx = int(m_posX + (oldbmx-bmx) * divx);
	int starty = int(m_posY + (oldbmy-bmy) * divy);

	m_posX = startx;
	m_posY = starty;
	m_lastScale = scale;

	int endx = startx + scale * mx * pw, endy = starty + scale * my;

	if(starty > 0)
	{
		//Fill top gap
		dc.FillSolidRect(0,0,rc.right,starty,border);
	}

	if(startx > 0)
	{
		//Fill left gap
		dc.FillSolidRect(0,starty,startx,endy-starty,border);
	}

	if(endx < rc.right)
	{
		//Fill right gap
		dc.FillSolidRect(endx,starty,rc.right-endx,endy-starty,border);
	}

	if(endy < rc.bottom)
	{
		//Fill bottom gap
		dc.FillSolidRect(0,endy,rc.right,rc.bottom-endy,border);
	}

	bool usegrid = scale > 4 && m_Grid ? true : false;
	bool usecellgrid = scale > 4 && m_CellGrid ? true : false;

	CPen *pOld=dc.SelectObject((CPen *)NULL);

	//Give the optimizer some freedom by copying to local
	bool Paste = m_Paste;
	CPoint PastePoint = m_PastePoint;
	bool MaskedPaste = m_MaskedPaste;
	int Col2 = m_Col2;
	int PasteSizeX = m_PasteSizeX;
	int PasteSizeY = m_PasteSizeY;

	int posy,posx;

	for(y=0;y<my;y++)
	{
		if((starty + scale * (y+1)) < 0)continue;

		posy = starty + scale * y;

		if(LONG(posy) >= rc.bottom)break;

		for(x=0;x<mx;x++)
		{
			if((startx + scale * (x+1) * pw) < 0)continue;

			posx = startx + scale * x * pw;

			if(LONG(posx) >= rc.right)break;

			int col;
					
			if(Paste && y>=PastePoint.y && y<PastePoint.y+PasteSizeY && x>=PastePoint.x && x<PastePoint.x+PasteSizeX)
			{
				col = m_pPasteBuffer[(y-PastePoint.y)*PasteSizeX + (x-PastePoint.x)];

				if(MaskedPaste && col == Col2)
				{
					col = pbm->GetPixel(x,y);
				}
			}
			else
			{
				col = pbm->GetPixel(x,y);
			}

			dc.FillSolidRect(posx,posy,(scale*pw),(scale*1),g_Vic2[col]);
		}
	}

	if(usegrid || usecellgrid)
	{
		for(x=0;x<=mx;x++)
		{
			posx = startx + scale * x * pw;

			if(posx < 0)continue;
			if(LONG(posx) >= rc.right)break;

			if(usecellgrid && x%cw==0)
			{
				dc.SelectObject(m_cLine);
				dc.MoveTo(posx,starty);
				dc.LineTo(posx,endy);
			}
			else if(usegrid)
			{
				dc.SelectObject(m_fLine);
				dc.MoveTo(posx,starty);
				dc.LineTo(posx,endy);
			}
		}

		for(y=0;y<my;y++)
		{
			posy = starty + scale * y;

			if(posy < 0)continue;
			if(LONG(posy) >= rc.bottom)break;

			if(usecellgrid && y%ch==0)
			{
				dc.SelectObject(m_cLine);
				dc.MoveTo(startx,posy);
				dc.LineTo(endx,posy);
			}
			else if(usegrid)
			{
				dc.SelectObject(m_fLine);
				dc.MoveTo(startx,posy);
				dc.LineTo(endx,posy);
			}
		}
	}

	if(Paste)
	{
		m_Mark[0] = PastePoint;

		if(m_CellSnapMarker)
				SnapToCell(m_Mark[0]);

		m_Mark[1] = m_Mark[0];

		m_Mark[1].x += m_PasteSizeX;
		m_Mark[1].y += m_PasteSizeY;
	}

	if(m_MarkerCount > 0 || Paste)
	{
		dc.SelectObject(m_sLine[m_TimeCount&1]);

		dc.MoveTo(startx + scale * m_Mark[0].x * pw, starty + scale * m_Mark[0].y);
		dc.LineTo(startx + scale * m_Mark[1].x * pw, starty + scale * m_Mark[0].y);
		dc.LineTo(startx + scale * m_Mark[1].x * pw, starty + scale * m_Mark[1].y);
		dc.LineTo(startx + scale * m_Mark[0].x * pw, starty + scale * m_Mark[1].y);
		dc.LineTo(startx + scale * m_Mark[0].x * pw, starty + scale * m_Mark[0].y);
	}


	dc.SelectObject(pOld);

	realdc.BitBlt(0,0,rc.right,rc.bottom,&dc,0,0,SRCCOPY);
}


void CChildView::ZoomCommand(short zDelta, CPoint &pt)
{
	int oldz=m_Zoom;

	oldz -= zDelta;
	if(oldz < 0) oldz = 0;

	m_ZoomPoint = pt;
	ScreenToClient(&m_ZoomPoint);

	if(oldz != m_Zoom)
	{
		m_Zoom = oldz;
		Invalidate(FALSE);

		CPoint pix;
		if(GetPixelFromPoint(m_ZoomPoint, pix))
			UpdateStatus(pix);
	}
}


BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	ZoomCommand(zDelta, pt);

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}


BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	return 1;

	//return CWnd::OnEraseBkgnd(pDC);
}


void CChildView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CChildView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CChildView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch(nChar)
	{
	case VK_SPACE:
		OnMButtonUp(-1, m_MovePoint);
		break;
	case VK_SHIFT:
		m_ColorPick =  false;
		break;
	}


	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

             
void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch(nChar)
	{
	case VK_LEFT:
		m_posX+=8*m_lastScale;
		Invalidate();
		break;
	case VK_RIGHT:
		m_posX-=8*m_lastScale;
		Invalidate();
		break;
	case VK_UP:
		m_posY+=8*m_lastScale;
		Invalidate();
		break;
	case VK_DOWN:
		m_posY-=8*m_lastScale;
		Invalidate();
		break;
	case VK_SPACE:
		OnMButtonDown(-1, m_MovePoint);
		break;
	case VK_OEM_MINUS:
		{
			CPoint point;
			GetCursorPos(&point);
			ZoomCommand(ZOOMVALUE, point);
		}
		break;
	case VK_OEM_PLUS:
		{
			CPoint point;
			GetCursorPos(&point);
			ZoomCommand(-ZOOMVALUE, point);
		}
		break;
	case 'X':
		Mail(MSG_PAL_SEL_COL, this->m_Col2, 0);
		Mail(MSG_PAL_SEL_COL, this->m_Col1, 1);
		break;
	case VK_ESCAPE:
		m_Paste = false;
		m_Fill = false;
		OnEditDeselect();
		break;
	case VK_DELETE:
	case VK_BACK:
		DeleteArea();
		break;
	case VK_SHIFT:
		m_ColorPick =  true;
	default:
		if(nChar >= '0' && nChar <= '9')
		{
			Mail(MSG_PAL_SEL_COL, nChar - '0', 0);
		}
		else if(nChar >= 'A' && nChar <= 'F')
		{
			Mail(MSG_PAL_SEL_COL, 10 + nChar - 'A', 0);
		}
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CChildView::OnMButtonDown(UINT nFlags, CPoint point)
{
	if(nFlags & MK_MBUTTON || nFlags == -1)
	{
		m_Move=true;
		m_MovePoint=point;

		TRACKMOUSEEVENT tm;
		tm.cbSize=sizeof(tm);
		tm.dwFlags=TME_LEAVE;
		tm.hwndTrack=*this;
		tm.dwHoverTime=HOVER_DEFAULT;
		TrackMouseEvent(&tm);
	}

	if(nFlags!=-1)
		CWnd::OnMButtonDown(nFlags, point);
}


void CChildView::OnMButtonUp(UINT nFlags, CPoint point)
{
	m_Move=false;
	if(nFlags!=-1)
		CWnd::OnMButtonUp(nFlags, point);
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_Move)
	{
		m_posX+=point.x-m_MovePoint.x;
		m_posY+=point.y-m_MovePoint.y;
		Invalidate();
	}

	CPoint pix;

	if(m_Marker)
	{
		if(GetPixelFromPoint(point, pix, true))
		{
			if(m_CellSnapMarker)
				SnapToCell(pix);

			m_Mark[1]=pix;
			Invalidate();
		}
	}
	else if(m_AutoMarker && !m_ManualMarker)
	{
		OnEditSelectcell();
	}

	if(GetPixelFromPoint(point, pix))
	{
		if(m_Paint)
		{
			if(m_LastPaintPix != pix)
			{
				//Bresenham style line between points
				int x = m_LastPaintPix.x;
				int y = m_LastPaintPix.y;
				int dx = pix.x - x;
				int dy = pix.y - y;
				int xo,yo,xr,yr,total,partial,step;

				if(abs(dx) > abs(dy))
				{
					total = abs(dx);
					partial = abs(dy);
					xo=sign(dx);
					xr=yo=0;
					yr=sign(dy);
				}
				else
				{
					total = abs(dy);
					partial = abs(dx);
					yo=sign(dy);
					yr=xo=0;
					xr=sign(dx);
				}

				int count = total+1;
				step = total/2;

				while(count)
				{
					m_pbm->SetPixel(x, y, m_ActivePaintCol);

					x+=xo;
					y+=yo;
					step+=partial;

					if(step>=total)
					{
						x+=xr;
						y+=yr;
						step-=total;
					}

					count--;
				}

				Invalidate(FALSE);
				m_LastPaintPix = pix;
			}
		}
		else if(m_Paste)
		{
			if(m_CellSnapMarker)SnapToCell(pix);
			m_PastePoint = pix;
			Invalidate();
		}

		UpdateStatus(pix);
		

		if(m_MarkerCount != 2)
		{
			CellInfo *info = new CellInfo;
			m_pbm->GetCellInfo(pix.x/m_pbm->GetCellSizeX(), pix.y/m_pbm->GetCellSizeY(), 1, 1, info);
			Mail(MSG_CELL_INFO, UINT_PTR(info));
		}
	}
	else
	{
		if(m_MarkerCount != 2)
		{
			CellInfo *info = new CellInfo;
			m_pbm->GetCellInfo(-1,-1,0,0,info);
			Mail(MSG_CELL_INFO, UINT_PTR(info));
		}
	}

	m_MovePoint = point;

	CWnd::OnMouseMove(nFlags, point);
}


void CChildView::OnMouseLeave()
{
	m_Move=false;
	if(m_Paint)
	{
		m_Paint = false;
		m_pbm->EndHistory();
		Mail(MSG_REFRESH);
	}
	/*
	else if(m_Marker)
	{
		m_Marker = false;
		m_MarkerCount = 0;
		Invalidate();
	}
	*/
	else if(m_Paste)
	{
		//m_Paste = false;
		Invalidate();
	}

	m_LastPaintPix.x = -1;
	m_LastPaintPix.y = -1;
	//Beep(800,30);

	CWnd::OnMouseLeave();
}


void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CPoint pix;
	if(GetPixelFromPoint(point, pix))
	{
		if(m_Paste)
		{
			if(m_CellSnapMarker)SnapToCell(pix);
			m_pbm->BeginHistory();
			int x,y,mx=m_pbm->GetSizeX(),my=m_pbm->GetSizeY();

			//Pre-fill area with background or one solid color
			if(!m_MaskedPaste)
			{
				CellInfo info;
				m_pbm->GetCellInfo(0,0,1,1,&info);
				for(y=0;y<m_PasteSizeY;y++)
				{
					if(y+pix.y>=my)break;
					for(x=0;x<m_PasteSizeX;x++)
					{
						if(x+pix.x>=mx)break;
						m_pbm->SetPixel(pix.x+x,pix.y+y,info.col[0]);
					}
				}
			}

			//Set paste buffer
			for(y=0;y<m_PasteSizeY;y++)
			{
				if(y+pix.y>=my)break;
				for(x=0;x<m_PasteSizeX;x++)
				{
					if(x+pix.x>=mx)break;

					int col = m_pPasteBuffer[x+y*m_PasteSizeX];

					if(!(m_MaskedPaste && col == m_Col2))
						m_pbm->SetPixel(pix.x+x,pix.y+y,col);
				}
			}

			m_pbm->EndHistory();
			m_Paste = false;
			Mail(MSG_REFRESH);
		}
		else if(m_Marker)
		{
			m_MarkerCount = 1;
			if(m_CellSnapMarker)SnapToCell(pix);
			m_Mark[0] = m_Mark[1] = pix;
		}
		else if(m_ColorPick)
		{
			BYTE col = m_pbm->GetPixel(pix.x, pix.y);
			Mail(MSG_PAL_SEL_COL, col, 0);
		}
		else if(m_Fill)
		{
			m_Fill = false;
			FloodFill(pix.x, pix.y, m_Col1, m_pbm->GetPixel(pix.x, pix.y));
			Mail(MSG_REFRESH);
		}
		else
		{
			m_pbm->BeginHistory();
			m_pbm->SetPixel(pix.x, pix.y, m_ActivePaintCol=m_Col1);
			m_LastPaintPix = pix;
			m_Paint = true;
		}

		Invalidate(FALSE);

		TRACKMOUSEEVENT tm;
		tm.cbSize=sizeof(tm);
		tm.dwFlags=TME_LEAVE;
		tm.hwndTrack=*this;
		tm.dwHoverTime=HOVER_DEFAULT;
		TrackMouseEvent(&tm);

		if(m_MarkerCount != 2)
		{
			CellInfo *info = new CellInfo;

			int cx=pix.x/m_pbm->GetCellSizeX();
			int cy=pix.y/m_pbm->GetCellSizeY();
			if(cx>=m_pbm->GetCellCountX())cx=m_pbm->GetCellCountX()-1;
			if(cy>=m_pbm->GetCellCountY())cy=m_pbm->GetCellCountY()-1;

			m_pbm->GetCellInfo(cx, cy, 1, 1, info);
			Mail(MSG_CELL_INFO, UINT_PTR(info));
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}


void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CPoint pix;
	if(GetPixelFromPoint(point, pix))
	{
		if(m_Paste)
		{
			m_Paste = false;
		}
		else if(m_Marker)
		{
			m_MarkerCount = 0;
			m_Marker = false;
		}
		else if(m_ColorPick)
		{
			BYTE col = m_pbm->GetPixel(pix.x, pix.y);
			Mail(MSG_PAL_SEL_COL, col, 1);
		}
		else if(m_Fill)
		{
			m_Fill = false;
			FloodFill(pix.x, pix.y, m_Col2, m_pbm->GetPixel(pix.x, pix.y));
			Invalidate(FALSE);
			Mail(MSG_REFRESH);
		}
		else
		{
			m_pbm->BeginHistory();
			m_pbm->SetPixel(pix.x, pix.y, m_ActivePaintCol=m_Col2);
			m_LastPaintPix = pix;
			m_Paint = true;

			TRACKMOUSEEVENT tm;
			tm.cbSize=sizeof(tm);
			tm.dwFlags=TME_LEAVE;
			tm.hwndTrack=*this;
			tm.dwHoverTime=HOVER_DEFAULT;
			TrackMouseEvent(&tm);

			Invalidate(FALSE);

			if(m_MarkerCount != 2)
			{
				CellInfo *info = new CellInfo;
				m_pbm->GetCellInfo(pix.x/m_pbm->GetCellSizeX(), pix.y/m_pbm->GetCellSizeY(), 1, 1, info);
				Mail(MSG_CELL_INFO, UINT_PTR(info));
			}
		}
	}

	CWnd::OnRButtonDown(nFlags, point);
}


void CChildView::InterpretMarker(int *x, int *y, int *w, int *h)
{
	if(m_Mark[0].x < m_Mark[1].x)
	{
		*x = m_Mark[0].x;
		*w = m_Mark[1].x - *x;
	}
	else
	{
		*x = m_Mark[1].x;
		*w = m_Mark[0].x - *x;
	}

	if(m_Mark[0].y < m_Mark[1].y)
	{
		*y = m_Mark[0].y;
		*h = m_Mark[1].y - *y;
	}
	else
	{
		*y = m_Mark[1].y;
		*h = m_Mark[0].y - *y;
	}
}


void CChildView::SelectInfo(CellInfo *pset)
{
	if(m_MarkerCount == 2)
	{
		int x, y, w, h;
		InterpretMarker(&x,&y,&w,&h);

		int cx = x / m_pbm->GetCellSizeX();
		int cy = y / m_pbm->GetCellSizeY();

		int cw = ((x+w+m_pbm->GetCellSizeX()-1) / m_pbm->GetCellSizeX()) - cx;
		int ch = ((y+h+m_pbm->GetCellSizeY()-1) / m_pbm->GetCellSizeY()) - cy;

		if(!pset)
		{
			CellInfo *info = new CellInfo;
			m_pbm->GetCellInfo(cx, cy, cw, ch, info);
			Mail(MSG_CELL_INFO, UINT_PTR(info));
		}
		else
		{
			m_pbm->SetCellInfo(cx, cy, cw, ch, pset);
		}
	}
	else
	{
		if(!pset)
		{
			CellInfo *info = new CellInfo;
			m_pbm->GetCellInfo(-1, -1, 0, 0, info);
			Mail(MSG_CELL_INFO, UINT_PTR(info));
		}
		else
		{
			m_pbm->SetCellInfo(-1, -1, 0, 0, pset);
		}
	}
}


void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_Paint)
	{
		m_Paint = false;
		m_pbm->EndHistory();
		Mail(MSG_REFRESH);
	}
	else if(m_Marker)
	{
		CPoint pix;
		if(GetPixelFromPoint(point, pix, true))
		{
			if(m_CellSnapMarker)SnapToCell(pix);
			m_Mark[1]=pix;
			m_MarkerCount = m_Mark[0] != m_Mark[1] ? 2 : 0;

			SelectInfo();
		}

		m_Marker = false;
		m_ManualMarker = true;
	}

	m_LastPaintPix.x = -1;
	m_LastPaintPix.y = -1;

	CWnd::OnLButtonUp(nFlags, point);
}


void CChildView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if(m_Paint)
	{
		m_Paint = false;
		m_pbm->EndHistory();
		Mail(MSG_REFRESH);
	}
	m_LastPaintPix.x = -1;
	m_LastPaintPix.y = -1;

	CWnd::OnRButtonUp(nFlags, point);
}


bool CChildView::CheckDirty(void)
{
	if(m_pbm->IsDirty())
	{
		switch(MessageBox(_T("You have unsaved changes!\nDo you want to save?"),_T("New/Load"),MB_YESNOCANCEL|MB_ICONWARNING))
		{
		case IDCANCEL:
			return false;
		case IDYES:
			OnFileSave();
			break;
		default:
			break;
		}
	}
	return true;
}


void CChildView::Load(LPCTSTR file, LPCTSTR iex)
{
	nstr path=file,tex=_T("gpx");
	nstr ex = iex;
	nstr id;

	int w=0, h=0;

	size_t n;

	while((n=path.find(_T('\"')))!=-1)
	{
		path.cut(n,1);
	}

	if(ex.isempty())
	{

		n=path.rfind(_T('.'));
		if(n!=-1)
		{
			tex=path.mid(n+1);
		}

		ex = tex;
	}


	narray<autoptr<SaveFormat>, int> fmt;
	C64Interface::GetLoadFormats(fmt);

	bool good_format = false;
	//bool unknown_format = true;

	C64Interface::tmode mode = C64Interface::tmode(-1);

	if(ex.cmpi(_T("gpx"))!=0)
		id = C64Interface::IdentifyFile(path);

	if(id.isnotempty())
		ex = id;

	if(ex.isempty())
	{
		ex=_T("bin");
	}
	else
	{
		//Check again
		for(int r=0;r<fmt.count();r++)
		{
			if(fmt[r]->MatchExt(ex))
			{
				mode = C64Interface::tmode(fmt[r]->type);
				good_format = fmt[r]->good;
				w = fmt[r]->width;
				h = fmt[r]->height;
				goto found;
			}
		}

		if(lstrcmpi(_T("bmp"),ex)==0 || lstrcmpi(_T("png"),ex)==0 || lstrcmpi(_T("jpg"),ex)==0 || lstrcmpi(_T("gif"),ex)==0 || lstrcmpi(_T("gpx"),ex)==0)
		{
		}
		else
		{
			ex=_T("bin");
		}
	}

found:
	if(lstrcmpi(_T("bmp"),ex)==0 || lstrcmpi(_T("png"),ex)==0 || lstrcmpi(_T("jpg"),ex)==0 || lstrcmpi(_T("gif"),ex)==0 || lstrcmpi(_T("bin"),ex)==0)
	{
		CImportDlg dlg;

		if(dlg.DoModal() != IDOK)
			return;

		switch(dlg.m_nSelect)
		{
		case 0:
			mode = C64Interface::MC_BITMAP;
			w = 160;
			h = 200;
			break;
		case 1:
			mode = C64Interface::BITMAP;
			w = 320;
			h = 200;
			break;
		case 2:
			mode = C64Interface::MC_CHAR;
			w = 160;
			h = 64;
			break;
		case 3:
			mode = C64Interface::CHAR;
			w = 320;
			h = 64;
			break;
		case 4:
			mode = C64Interface::UNRESTRICTED;
			w = 320;
			h = 200;
			break;
		case 5:
			mode = C64Interface::W_UNRESTRICTED;
			w = 160;
			h = 200;
			break;
		case 6:
			mode = C64Interface::SPRITE;
			w = 192;
			h = 168;
			break;
		case 7:
			mode = C64Interface::MC_SPRITE;
			w = 96;
			h = 168;
			break;
		default:
			return;
			break;
		}
	}

	try
	{
		C64Interface *i=C64Interface::Load(path,ex,mode,w,h);
		//i->InheritHistory(m_pbm);
		delete m_pbm;
		m_pbm = i;

		//View specific meta data
		if(ex.cmpi(_T("gpx"))==0)
		{
			m_AutoMarker = m_pbm->GetMetaInt("autoselect") ? true : false;
			m_CellSnapMarker = m_pbm->GetMetaInt("cellsnap") ? true : false;
		}

		Invalidate();
		Mail(MSG_PREV_PAR,UINT_PTR((1.0/m_pbm->GetPARValue())*10000000));
		Mail(MSG_REFRESH);

		m_pbm->SetFileName(path);
		m_goodFormat = good_format;

		this->SetTitleFileName(path);

		if(!good_format)
		{
			m_pbm->SetDirty();
		}
	}
	catch(LPCTSTR str)
	{
		AfxMessageBox(str);
	}
}


void CChildView::OnFileLoad()
{
	if(!CheckDirty())return;

	narray<autoptr<SaveFormat>, int> fmt;
	C64Interface::GetLoadFormats(fmt);

	int r,t;
	nstr s;
	bool first = true;

	s << _T("Supported Formats|");
	for(r=0;r<fmt.count();r++)
	{
		for(t=0;t<fmt[r]->ext.count();t++)
		{
			if(!first)s << _T(";");

			s << _T("*.") << fmt[r]->ext[t];

			first = false;
		}
	}
	s << _T("|");
	for(r=0;r<fmt.count();r++)
	{
		s << fmt[r]->name << _T("|");

		first = true;

		for(t=0;t<fmt[r]->ext.count();t++)
		{
			if(!first)s << _T(";");

			s << _T("*.") << fmt[r]->ext[t];

			first = false;
		}
			
		s << _T("|");
	}
	s << _T("All Files (*.*)|*.*||");

	CFileDialog dlg(TRUE, _T("gpx"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, s, this, 0);
	if(dlg.DoModal() == IDOK)
	{
		Load(dlg.GetPathName(),dlg.GetFileExt().MakeLower());
	}
}


void CChildView::OnFileSave()
{
	if(m_goodFormat && *m_pbm->GetFileName()!=_T('\0'))
	{
		SetViewSpecificMetaData();
		try
		{
			m_pbm->Save(NULL,NULL);
			m_pbm->ClearDirty();
		}
		catch(LPCTSTR str)
		{
			AfxMessageBox(str);
		}
	}
	else
	{
		OnFileSaveas();
	}
}


void CChildView::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pbm->IsDirty()?1:0);
}


void CChildView::Saveas(C64Interface *i)
{
	SetViewSpecificMetaData();

	narray<autoptr<SaveFormat>, int> fmt;
	i->GetSaveFormats(fmt);

	nstr s;

	for(int r=0;r<fmt.count();r++)
	{
		s << fmt[r]->name << _T("|");

		bool first = true;

		for(int t=0;t<fmt[r]->ext.count();t++)
		{
			if(!first)s << _T(";");

			s << _T("*.") << fmt[r]->ext[t];

			first = false;
		}

		s << _T("|");
	}

	s << _T("|");

	nstr def=i->GetFileName();
	size_t n;
	n=def.rfind(_T('.'));
	if(n!=-1)
		def.limit(n);

	CFileDialog dlg(FALSE, _T("gpx"), def, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, s.cstr(), this, 0);
	if(dlg.DoModal() == IDOK)
	{
		nstr ex = dlg.GetFileExt().MakeLower();

		if(ex==_T("pmap") || ex==_T("pscr") || ex==_T("pcol") || ex==_T("piscr") || ex==_T("pimap"))
		{
			for(;;)
			{
				CAddressDlg addr;
				addr.m_verified_address = m_pbm->GetMetaInt(nstrc(ex));
				if(addr.DoModal()!=IDOK)return;
				if(addr.m_verified_address<0 || addr.m_verified_address>65536)continue;
				m_pbm->SetMetaInt(nstrc(ex),addr.m_verified_address);
				break;
			}
		}

		bool good_format = false;

		for(int r=0;r<fmt.count();r++)
		{
			if(fmt[r]->MatchExt(ex))
			{
				good_format = fmt[r]->good;
				break;
			}
		}

		try
		{
			i->Save(dlg.GetPathName(), ex);

			if((good_format && i->GetBackBufferCount() == 1) || ex.cmpi(_T("gpx"))==0)
			{
				i->ClearDirty();
				SetTitleFileName(dlg.GetPathName());
			}

			i->SetFileName(dlg.GetPathName());
			m_goodFormat = good_format;

		}
		catch(LPCTSTR str)
		{
			AfxMessageBox(str);
		}

		Invalidate();
	}
}


void CChildView::OnFileSaveas()
{
	Saveas(m_pbm);
}


void CChildView::Receive(unsigned short message, UINT_PTR data, unsigned short extra)
{
	switch(message)
	{
	case MSG_CELL_COL:
		{

			CellInfo info;

			if(data<100)
			{
				info.col[extra]=BYTE(data);
				m_pbm->BeginHistory();
				SelectInfo(&info);
				m_pbm->EndHistory();

			}
			else if(data==101)
			{
				m_pbm->BeginHistory();
				info.crippled[extra]=1;
				SelectInfo(&info);
				m_pbm->EndHistory();
			}
			else if(data==102)
			{
				m_pbm->BeginHistory();
				info.lock[extra]=1;
				SelectInfo(&info);
				m_pbm->EndHistory();
			}

			SelectInfo();

			Mail(MSG_REFRESH);

			Invalidate();
		}
		break;

	case MSG_PAL_SEL_COL:
		if(!extra)
			this->m_Col1=int(data);
		else
			this->m_Col2=int(data);
		break;
	case MSG_LOAD:
		{
			LPTSTR p=(LPTSTR)data;
			Load(p,NULL);
			free(p);
		}
		break;
	case MSG_PREV_REPOS:
		{
			CPoint *pix=(CPoint *)data;

			CRect rc;
			GetClientRect(rc);

			m_posX = rc.right/2 - ((pix->x) * m_lastScale * m_pbm->GetPixelWidth());
			m_posY = rc.bottom/2 - ((pix->y) * m_lastScale);

			Invalidate();

			delete pix;
		}
		break;

	default:
		break;
	}
}


void CChildView::OnEditUndo()
{
	if(m_pbm->CanUndo())
	{
		m_pbm = m_pbm->Undo();
		Invalidate();
		Mail(MSG_REFRESH);
		m_pbm->SetDirty();
	}
}


void CChildView::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pbm->CanUndo()?1:0);
}


void CChildView::OnEditRedo()
{
	if(m_pbm->CanRedo())
	{
		m_pbm = m_pbm->Redo();
		Invalidate();
		Mail(MSG_REFRESH);
		m_pbm->SetDirty();
	}
}


void CChildView::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pbm->CanRedo()?1:0);
}


bool CChildView::FrameClose(void)
{
	if(m_pbm->IsDirty())
	{
		switch(MessageBox(_T("You have unsaved changes!\nDo you want to save before quitting?"),_T("Quit"),MB_YESNOCANCEL|MB_ICONWARNING))
		{
		case IDCANCEL:
			return false;
		case IDYES:
			OnFileSave();
			break;
		default:
			break;
		}
	}

	return true;
}


void CChildView::UpdateStatus(CPoint &pix)
{
	TCHAR *txt=new TCHAR[256];

	if(m_pbm->IsChar())
	{
		_stprintf(txt, _T("Z: %d/%d X:%03d Y:%03d XC:%02d YC:%02d Zoom:%d00%% Chars:%d"), m_pbm->GetBackBuffer()+1, m_pbm->GetBackBufferCount(), pix.x, pix.y, pix.x/m_pbm->GetCellSizeX(), pix.y/m_pbm->GetCellSizeY(), 1+m_Zoom/ZOOMVALUE, m_pbm->GetCharCount());
	}
	else
	{
		_stprintf(txt, _T("Z: %d/%d X:%03d Y:%03d XC:%02d YC:%02d Zoom:%d00%%"), m_pbm->GetBackBuffer()+1, m_pbm->GetBackBufferCount(), pix.x, pix.y, pix.x/m_pbm->GetCellSizeX(), pix.y/m_pbm->GetCellSizeY(), 1+m_Zoom/ZOOMVALUE);
	}

	if(m_MarkerCount > 0)
	{
		int x, y, w, h;
		InterpretMarker(&x,&y,&w,&h);
		TCHAR t[256];
		_stprintf(t, _T(" (Marker: X:%03d Y:%03d W:%03d H:%03d)"), x, y, w, h);
		_tcscat(txt, t);
	}


	Mail(MSG_STATUS_TEXT,UINT_PTR(txt));
}


void CChildView::OnViewGrid()
{
	m_Grid = 1 - m_Grid;
	Invalidate();
}


void CChildView::OnUpdateViewGrid(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_Grid ? 1 : 0);
}


void CChildView::OnViewCellgrid()
{
	m_CellGrid = 1 - m_CellGrid;
	Invalidate();
}


void CChildView::OnUpdateViewCellgrid(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_CellGrid ? 1 : 0);
}


void CChildView::OnToolOptimize()
{
	m_pbm->BeginHistory();
	m_pbm->Optimize();
	m_pbm->EndHistory();
}


void CChildView::OnUpdateToolOptimize(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pbm->CanOptimize()?1:0);
}


void CChildView::OnModeBitmap()
{
	ChangeMode(m_pbm->IsMultiColor()?C64Interface::MC_BITMAP:C64Interface::BITMAP);
}


void CChildView::OnUpdateModeBitmap(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_pbm->IsBitmap()?1:0);
}


void CChildView::OnModeSprite()
{
	ChangeMode(m_pbm->IsMultiColor()?C64Interface::MC_SPRITE:C64Interface::SPRITE);
}


void CChildView::OnUpdateModeSprite(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_pbm->IsSprite()?1:0);
}


void CChildView::OnModeChar()
{
	ChangeMode(m_pbm->IsMultiColor()?C64Interface::MC_CHAR:C64Interface::CHAR);
}


void CChildView::OnUpdateModeChar(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_pbm->IsChar()?1:0);
}


void CChildView::OnModeUnrestricted()
{
	ChangeMode(m_pbm->IsMultiColor()?C64Interface::W_UNRESTRICTED:C64Interface::UNRESTRICTED);
}


void CChildView::OnUpdateModeUnrestricted(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_pbm->IsUnrestricted()?1:0);
}


void CChildView::OnModeMulti()
{
	C64Interface::tmode to;

	switch(m_pbm->GetMode())
	{
		case C64Interface::BITMAP:
			to = C64Interface::MC_BITMAP;
			break;
		case C64Interface::MC_BITMAP:
			to = C64Interface::BITMAP;
			break;
		case C64Interface::UNRESTRICTED:
			to = C64Interface::W_UNRESTRICTED;
			break;
		case C64Interface::W_UNRESTRICTED:
			to = C64Interface::UNRESTRICTED;
			break;
		case C64Interface::CHAR:
			to = C64Interface::MC_CHAR;
			break;
		case C64Interface::MC_CHAR:
			to = C64Interface::CHAR;
			break;
		case C64Interface::SPRITE:
			to = C64Interface::MC_SPRITE;
			break;
		case C64Interface::MC_SPRITE:
			to = C64Interface::SPRITE;
			break;
		default:
			return;
			break;
	};

	ChangeMode(to);
}


void CChildView::OnUpdateModeMulti(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pbm->IsMultiColor()?1:0);
}

void CChildView::ChangeMode(C64Interface::tmode to)
{
	if(to == m_pbm->GetMode()) return;	//Ignore

	if(m_pbm->IsDirty())
	{
		switch(MessageBox(_T("You have unsaved changes!\nDo you want to save before switching mode?"),_T("Mode change"),MB_YESNOCANCEL|MB_ICONWARNING))
		{
		case IDCANCEL:
			return;
		case IDYES:
			OnFileSave();
			break;
		default:
			break;
		}
	}

	try
	{
		int32_t count = m_pbm->GetBackBufferCount();
		int32_t curbuf = m_pbm->GetBackBuffer();

		CImage *img = new CImage[count];

		for(int32_t r=0;r<count;r++)
		{
			m_pbm->SetBackBuffer(r);
			m_pbm->RenderImage(img[r]);
		}

		C64Interface *i=C64Interface::CreateFromImage(img, count, to);

		delete [] img;

		i->InheritHistory(m_pbm);
		m_pbm = i;

		i->SetBackBuffer(curbuf);

		m_goodFormat = false;
		i->SetDirty();
		
		Invalidate();
		Mail(MSG_REFRESH);
	}
	catch(LPCTSTR str)
	{
		AfxMessageBox(str);
	}
}


void CChildView::OnDropFiles(HDROP hdrop)
{
	UINT  uNumFiles;
	TCHAR szNextFile [MAX_PATH];

	// Get the # of files being dropped.
	uNumFiles = DragQueryFile ( hdrop, -1, NULL, 0 );

	for ( UINT uFile = 0; uFile < uNumFiles; uFile++ )
	{
		// Get the next filename from the HDROP info.
		if ( DragQueryFile ( hdrop, uFile, szNextFile, MAX_PATH ) > 0 )
		{
			if(CheckDirty())
			{
				Load(szNextFile, NULL);
			}
		}
	}

	// Free up memory.
	DragFinish ( hdrop );

	__super::OnDropFiles(hdrop);
}


void CChildView::OnEditCut()
{
	OnEditCopy();
	DeleteArea();
}


void CChildView::DeleteArea(bool history)
{
	if(m_MarkerCount == 2)
	{
		BYTE fillcol = history ? m_Col2 : m_pbm->GetBackground();

		int x, y, w, h;
		InterpretMarker(&x,&y,&w,&h);

		m_CutX=x, m_CutY=y, m_CutW=w, m_CutH=h;

		if(history)m_pbm->BeginHistory();

		for(y=m_CutY;y<m_CutY+m_CutH;y++)
		{
			if(y>m_pbm->GetSizeY())break;
			for(x=m_CutX;x<m_CutX+m_CutW;x++)
			{
				if(x>m_pbm->GetSizeX())break;
				m_pbm->SetPixel(x,y, fillcol);
			}
		}

		Mail(MSG_REFRESH);

		if(history)m_pbm->EndHistory();
	}
}


void CChildView::OnUpdateEditCut(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_MarkerCount == 2 ? 1 : 0);
}


static bool BitmapToClipboard(HBITMAP hBM, HWND hWnd)
{
    if (!::OpenClipboard(hWnd))
        return false;
    ::EmptyClipboard();
    
    BITMAP bm;
    ::GetObject(hBM, sizeof(bm), &bm);
    
    BITMAPINFOHEADER bi;
    ::ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = bm.bmBitsPixel;
    bi.biCompression = BI_RGB;
    if (bi.biBitCount <= 1)	// make sure bits per pixel is valid
        bi.biBitCount = 1;
    else if (bi.biBitCount <= 4)
        bi.biBitCount = 4;
    else if (bi.biBitCount <= 8)
        bi.biBitCount = 8;
    else // if greater than 8-bit, force to 24-bit
        bi.biBitCount = 24;
    
    // Get size of color table.
    SIZE_T dwColTableLen = (bi.biBitCount <= 8) ? SIZE_T(1 << bi.biBitCount) * sizeof(RGBQUAD) : 0;
    
    // Create a device context with palette
    HDC hDC = ::GetDC(NULL);
    HPALETTE hPal = static_cast<HPALETTE>(::GetStockObject(DEFAULT_PALETTE));
    HPALETTE hOldPal = ::SelectPalette(hDC, hPal, FALSE);
    ::RealizePalette(hDC);
    
    // Use GetDIBits to calculate the image size.
    ::GetDIBits(hDC, hBM, 0, static_cast<UINT>(bi.biHeight), NULL,
        reinterpret_cast<LPBITMAPINFO>(&bi), DIB_RGB_COLORS);
    // If the driver did not fill in the biSizeImage field, then compute it.
    // Each scan line of the image is aligned on a DWORD (32bit) boundary.
    if (0 == bi.biSizeImage)
        bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8) * bi.biHeight;
    
    // Allocate memory
    HGLOBAL hDIB = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + dwColTableLen + bi.biSizeImage);
    if (hDIB)
    {
        union tagHdr_u
        {
            LPVOID             p;
            LPBYTE             pByte;
            LPBITMAPINFOHEADER pHdr;
            LPBITMAPINFO       pInfo;
        } Hdr;
        
        Hdr.p = ::GlobalLock(hDIB);
        // Copy the header
        ::CopyMemory(Hdr.p, &bi, sizeof(BITMAPINFOHEADER));
        // Convert/copy the image bits and create the color table
        int nConv = ::GetDIBits(hDC, hBM, 0, static_cast<UINT>(bi.biHeight),
            Hdr.pByte + sizeof(BITMAPINFOHEADER) + dwColTableLen, 
            Hdr.pInfo, DIB_RGB_COLORS);
        ::GlobalUnlock(hDIB);
        if (!nConv)
        {
            ::GlobalFree(hDIB);
            hDIB = NULL;
        }
    }
    if (hDIB)
        ::SetClipboardData(CF_DIB, hDIB);
    ::CloseClipboard();
    ::SelectPalette(hDC, hOldPal, FALSE);
    ::ReleaseDC(NULL, hDC);
    return NULL != hDIB;
}


void CChildView::OnEditCopy()
{
	if(m_MarkerCount == 2)
	{
		CImage img;
		int x, y, w, h;
		InterpretMarker(&x,&y,&w,&h);

		m_pbm->RenderImage(img, x, y, w, h);

		m_CutX=x, m_CutY=y, m_CutW=w, m_CutH=h;

		BitmapToClipboard(img, *this);
	}
}


void CChildView::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_MarkerCount == 2 ? 1 : 0);
}


void CChildView::OnPaste(bool masked)
{
    if (!OpenClipboard())return;
	HBITMAP bm=(HBITMAP)GetClipboardData(CF_BITMAP);
	
	int x,y,w = m_pbm->GetPixelWidth();
	CImage img;
	img.Attach(bm);

	if(m_pPasteBuffer)
	{
		delete [] m_pPasteBuffer;
		m_pPasteBuffer = NULL;
	}

	if(w==2)
		m_PasteSizeX = (img.GetWidth()+1)/2;
	else
		m_PasteSizeX = img.GetWidth();

	m_PasteSizeY = img.GetHeight();

	m_pPasteBuffer =  new BYTE[m_PasteSizeX * m_PasteSizeY];

	if(w==1)
	{
		for(y=0;y<img.GetHeight();y++)
		{
			for(x=0;x<img.GetWidth();x++)
			{
				m_pPasteBuffer[y*m_PasteSizeX + x] = ToC64Index(img.GetPixel(x,y));
			}
		}
	}
	else
	{
		for(y=0;y<img.GetHeight();y++)
		{
			for(x=0;x<img.GetWidth();x+=2)
			{
				if(x+1>=img.GetWidth())
				{
					m_pPasteBuffer[y*m_PasteSizeX + x/2] = ToC64Index(img.GetPixel(x,y));
				}
				else
				{
					m_pPasteBuffer[y*m_PasteSizeX + x/2] = ToC64Index(blend(img.GetPixel(x+0,y),img.GetPixel(x+1,y)));
				}
			}
		}
	}

	img.Detach();
	CloseClipboard();

	CPoint pix;
	if(GetPixelFromPoint(m_MovePoint, pix))
	{
		m_PastePoint = pix;
	}
	else
	{
		m_PastePoint.x = m_PastePoint.y = 0;
	}

	m_Marker = false;
	m_ManualMarker = false;
	m_MarkerCount = 0;
	m_Paste = true;
	m_MaskedPaste = masked;
}


void CChildView::OnEditPaste()
{
	OnPaste(false);
}


void CChildView::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
    if (!OpenClipboard())return;

	pCmdUI->Enable(GetClipboardData(CF_BITMAP) ? 1 : 0);

	CloseClipboard();

}


void CChildView::OnEditBackgroundmaskedpaste()
{
	// TODO: Add your command handler code here
	OnPaste(true);
}


void CChildView::OnUpdateEditBackgroundmaskedpaste(CCmdUI *pCmdUI)
{
	OnUpdateEditPaste(pCmdUI);
}



void CChildView::OnEditSelectmarker()
{
	m_Marker = m_Marker?false:true;
	m_MarkerCount = 0;
	m_Mark[0].x=0;
	m_Mark[0].y=0;
	m_Mark[1]=m_Mark[0];
	Invalidate();
}


void CChildView::OnEditSelectcell()
{
	CPoint pix;
	if(GetPixelFromPoint(m_MovePoint, pix))
	{
		m_Marker = false;
		m_Mark[0].x=pix.x - (pix.x%m_pbm->GetCellSizeX());
		m_Mark[0].y=pix.y - (pix.y%m_pbm->GetCellSizeY());

		m_Mark[1] = m_Mark[0];

		m_Mark[1].x+=m_pbm->GetCellSizeX();
		m_Mark[1].y+=m_pbm->GetCellSizeY();
		m_MarkerCount = 2;
		SelectInfo();
		Invalidate();
	}
}


void CChildView::OnEditSelectall()
{
	m_Marker = false;
	m_Mark[0].x=m_Mark[0].y=0;
	m_Mark[1].x=m_pbm->GetSizeX();
	m_Mark[1].y=m_pbm->GetSizeY();
	m_MarkerCount = 2;
	SelectInfo();
	Invalidate();
}


void CChildView::OnFileNew()
{
	if(CheckDirty())
	{
		CNewDlg dlg;
		if(dlg.DoModal()==IDOK)
		{
			C64Interface *i=NULL;

			switch(dlg.m_select)
			{
			case 0:	//Bitmap
				if(dlg.m_multi)
					i = new MCBitmap(dlg.m_x, dlg.m_y, dlg.m_z);
				else
					i = new Bitmap(dlg.m_x, dlg.m_y, dlg.m_z);
				break;
			case 1:	//Sprite
				if(dlg.m_multi)
					i = new MCSprite(dlg.m_x, dlg.m_y, dlg.m_z);
				else
					i = new Sprite(dlg.m_x, dlg.m_y, dlg.m_z);
				break;
			case 2:	//Char
				if(dlg.m_multi)
					i = new MCFont(dlg.m_x, dlg.m_y, dlg.m_z);
				else
					i = new SFont(dlg.m_x, dlg.m_y, dlg.m_z);
				break;
			case 3:	//Unrestricted
				i = new Unrestricted(dlg.m_x, dlg.m_y, dlg.m_multi?true:false, dlg.m_z);
				break;
			};

			if(i)
			{
				delete m_pbm;
				//i->InheritHistory(m_pbm);
				m_pbm = i;
			}
		}

		Invalidate();
		Mail(MSG_REFRESH);
	}
}


void CChildView::OnEditDeselect()
{
	m_Marker = false;
	m_MarkerCount = 0;
	m_ManualMarker = false;
	Invalidate();
}


void CChildView::OnUpdateEditDeselect(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_MarkerCount || m_Marker ? 1 : 0);
}


void CChildView::OnCopyromfontUppercase()
{
	if(m_pbm->IsChar() && m_pbm->IsSingleColor())
	{
		m_pbm->BeginHistory();
		m_pbm->CustomCommand(0);
		m_pbm->EndHistory();
		Invalidate();
		Mail(MSG_REFRESH);
	}
}


void CChildView::OnUpdateCopyromfontUppercase(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((m_pbm->IsChar() && m_pbm->IsSingleColor()) ? 1 : 0);
}


void CChildView::OnCopyromfontLowercase()
{
	if(m_pbm->IsChar() && m_pbm->IsSingleColor())
	{
		m_pbm->BeginHistory();
		m_pbm->CustomCommand(1);
		m_pbm->EndHistory();
		Invalidate();
		Mail(MSG_REFRESH);
	}
}


void CChildView::OnUpdateCopyromfontLowercase(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((m_pbm->IsChar() && m_pbm->IsSingleColor()) ? 1 : 0);
}


void CChildView::OnUpdateSelectionFlipvertically(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_MarkerCount == 2 || m_Paste ? 1 : 0);
}


void CChildView::OnUpdateSelectionFliphorizontally(CCmdUI *pCmdUI)
{
	OnUpdateSelectionFlipvertically(pCmdUI);
}


void CChildView::OnUpdateSelectionShiftleft(CCmdUI *pCmdUI)
{
	OnUpdateSelectionFlipvertically(pCmdUI);
}


void CChildView::OnUpdateSelectionShiftright(CCmdUI *pCmdUI)
{
	OnUpdateSelectionFlipvertically(pCmdUI);
}


void CChildView::OnUpdateSelectionShiftup(CCmdUI *pCmdUI)
{
	OnUpdateSelectionFlipvertically(pCmdUI);
}


void CChildView::OnUpdateSelectionShiftdown(CCmdUI *pCmdUI)
{
	OnUpdateSelectionFlipvertically(pCmdUI);
}


void CChildView::OnUpdateSelectionRotatecw(CCmdUI *pCmdUI)
{
	if(m_MarkerCount == 2)
	{
		CPoint delta = m_Mark[0] - m_Mark[1];
		pCmdUI->Enable(delta.x == delta.y ? 1 : 0);
	}
	else
	{
		pCmdUI->Enable(m_Paste ? 1 : 0);
	}
}

void CChildView::OnUpdateSelectionRotateccw(CCmdUI *pCmdUI)
{
	OnUpdateSelectionRotatecw(pCmdUI);
}


BYTE CChildView::SelectionGetPixel(bool pastemode, int x, int y)
{
	if(pastemode)
	{
		ASSERT(m_pPasteBuffer);
		return m_pPasteBuffer[x + y * m_PasteSizeX];
	}

	//Kinda expensive but it works
	int mx, my, w, h;
	InterpretMarker(&mx,&my,&w,&h);

	return m_pbm->GetPixel(mx+x, my+y);
}


void CChildView::SelectionSetPixel(bool pastemode, int x, int y, BYTE col)
{
	if(pastemode)
	{
		ASSERT(m_pPasteBuffer);
		m_pPasteBuffer[x + y * m_PasteSizeX] = col;
		return;
	}

	//Kinda expensive but it works
	int mx, my, w, h;
	InterpretMarker(&mx,&my,&w,&h);

	m_pbm->SetPixel(mx+x, my+y, col);
}


void CChildView::OnSelectionRotatecw()
{
	LPBYTE p;
	int sx, sy;
	int nx, ny;

	if(m_MarkerCount == 2)
	{
		int x, y;
		InterpretMarker(&x,&y,&sx,&sy);
		ASSERT(sx==sy);
	}
	else if(m_Paste)
	{
		sx = m_PasteSizeX;
		sy = m_PasteSizeY;
	}
	else
	{
		ASSERT(false);
		return;
	}

	nx = sy;
	ny = sx;

	p = new BYTE[nx*ny];

	for(int ry=0,wx=nx-1;ry<sy;ry++,wx--)
	{
		for(int rx=0,wy=0;rx<sx;rx++,wy++)
		{
			p[wy * nx + wx]=SelectionGetPixel(m_Paste,rx,ry);
		}
	}

	if(!m_Paste)
	{
		m_pbm->BeginHistory();
		DeleteArea(false);
	}
	else
	{
		m_PasteSizeX = nx;
		m_PasteSizeY = ny;
	}

	for(int y=0;y<ny;y++)
	{
		for(int x=0;x<nx;x++)
		{
			SelectionSetPixel(m_Paste,x,y,p[y * nx + x]);
		}
	}

	if(!m_Paste)
	{
		m_pbm->EndHistory();
	}

	delete [] p;
}


void CChildView::OnSelectionRotateccw()
{
	LPBYTE p;
	int sx, sy;
	int nx, ny;

	if(m_MarkerCount == 2)
	{
		int x, y;
		InterpretMarker(&x,&y,&sx,&sy);
		ASSERT(sx==sy);
	}
	else if(m_Paste)
	{
		sx = m_PasteSizeX;
		sy = m_PasteSizeY;
	}
	else
	{
		ASSERT(false);
		return;
	}

	nx = sy;
	ny = sx;

	p = new BYTE[nx*ny];

	for(int ry=0,wx=0;ry<sy;ry++,wx++)
	{
		for(int rx=0,wy=ny-1;rx<sx;rx++,wy--)
		{
			p[wy * nx + wx]=SelectionGetPixel(m_Paste,rx,ry);
		}
	}

	if(!m_Paste)
	{
		m_pbm->BeginHistory();
		DeleteArea(false);
	}
	else
	{
		m_PasteSizeX = nx;
		m_PasteSizeY = ny;
	}

	for(int y=0;y<ny;y++)
	{
		for(int x=0;x<nx;x++)
		{
			SelectionSetPixel(m_Paste,x,y,p[y * nx + x]);
		}
	}

	if(!m_Paste)
	{
		m_pbm->EndHistory();
	}

	delete [] p;
}


void CChildView::OnSelectionFliphorizontally()
{
	LPBYTE p;
	int sx, sy;

	if(m_MarkerCount == 2)
	{
		int x, y;
		InterpretMarker(&x,&y,&sx,&sy);
	}
	else if(m_Paste)
	{
		sx = m_PasteSizeX;
		sy = m_PasteSizeY;
	}
	else
	{
		ASSERT(false);
		return;
	}

	p = new BYTE[sx*sy];

	for(int ry=0,wy=0;ry<sy;ry++,wy++)
	{
		for(int rx=0,wx=sx-1;rx<sx;rx++,wx--)
		{
			p[wy * sx + wx]=SelectionGetPixel(m_Paste,rx,ry);
		}
	}

	if(!m_Paste)
	{
		m_pbm->BeginHistory();
		DeleteArea(false);
	}

	for(int y=0;y<sy;y++)
	{
		for(int x=0;x<sx;x++)
		{
			SelectionSetPixel(m_Paste,x,y,p[y * sx + x]);
		}
	}

	if(!m_Paste)
	{
		m_pbm->EndHistory();
	}

	delete [] p;
}


void CChildView::OnSelectionFlipvertically()
{
	LPBYTE p;
	int sx, sy;

	if(m_MarkerCount == 2)
	{
		int x, y;
		InterpretMarker(&x,&y,&sx,&sy);
	}
	else if(m_Paste)
	{
		sx = m_PasteSizeX;
		sy = m_PasteSizeY;
	}
	else
	{
		ASSERT(false);
		return;
	}

	p = new BYTE[sx*sy];

	for(int ry=0,wy=sy-1;ry<sy;ry++,wy--)
	{
		for(int rx=0,wx=0;rx<sx;rx++,wx++)
		{
			p[wy * sx + wx]=SelectionGetPixel(m_Paste,rx,ry);
		}
	}

	if(!m_Paste)
	{
		m_pbm->BeginHistory();
		DeleteArea(false);
	}

	for(int y=0;y<sy;y++)
	{
		for(int x=0;x<sx;x++)
		{
			SelectionSetPixel(m_Paste,x,y,p[y * sx + x]);
		}
	}

	if(!m_Paste)
	{
		m_pbm->EndHistory();
	}

	delete [] p;
}


void CChildView::OnSelectionShiftleft()
{
	LPBYTE p;
	int sx, sy;

	if(m_MarkerCount == 2)
	{
		int x, y;
		InterpretMarker(&x,&y,&sx,&sy);
	}
	else if(m_Paste)
	{
		sx = m_PasteSizeX;
		sy = m_PasteSizeY;
	}
	else
	{
		ASSERT(false);
		return;
	}

	p = new BYTE[sx*sy];

	for(int ry=0,wy=0;ry<sy;ry++,wy++)
	{
		for(int rx=0,wx=sx-1;rx<sx;rx++,wx++)
		{
			if(wx == sx)wx = 0;
			p[wy * sx + wx]=SelectionGetPixel(m_Paste,rx,ry);
		}
	}

	if(!m_Paste)
	{
		m_pbm->BeginHistory();
		DeleteArea(false);
	}

	for(int y=0;y<sy;y++)
	{
		for(int x=0;x<sx;x++)
		{
			SelectionSetPixel(m_Paste,x,y,p[y * sx + x]);
		}
	}

	if(!m_Paste)
	{
		m_pbm->EndHistory();
	}

	delete [] p;
}


void CChildView::OnSelectionShiftright()
{
	LPBYTE p;
	int sx, sy;

	if(m_MarkerCount == 2)
	{
		int x, y;
		InterpretMarker(&x,&y,&sx,&sy);
	}
	else if(m_Paste)
	{
		sx = m_PasteSizeX;
		sy = m_PasteSizeY;
	}
	else
	{
		ASSERT(false);
		return;
	}

	p = new BYTE[sx*sy];

	for(int ry=0,wy=0;ry<sy;ry++,wy++)
	{
		for(int rx=0,wx=1;rx<sx;rx++,wx++)
		{
			if(wx == sx)wx = 0;
			p[wy * sx + wx]=SelectionGetPixel(m_Paste,rx,ry);
		}
	}

	if(!m_Paste)
	{
		m_pbm->BeginHistory();
		DeleteArea(false);
	}

	for(int y=0;y<sy;y++)
	{
		for(int x=0;x<sx;x++)
		{
			SelectionSetPixel(m_Paste,x,y,p[y * sx + x]);
		}
	}

	if(!m_Paste)
	{
		m_pbm->EndHistory();
	}

	delete [] p;
}


void CChildView::OnSelectionShiftup()
{
	LPBYTE p;
	int sx, sy;

	if(m_MarkerCount == 2)
	{
		int x, y;
		InterpretMarker(&x,&y,&sx,&sy);
	}
	else if(m_Paste)
	{
		sx = m_PasteSizeX;
		sy = m_PasteSizeY;
	}
	else
	{
		ASSERT(false);
		return;
	}

	p = new BYTE[sx*sy];

	for(int ry=0,wy=sy-1;ry<sy;ry++,wy++)
	{
		if(wy == sy)wy = 0;
		for(int rx=0,wx=0;rx<sx;rx++,wx++)
		{
			p[wy * sx + wx]=SelectionGetPixel(m_Paste,rx,ry);
		}
	}

	if(!m_Paste)
	{
		m_pbm->BeginHistory();
		DeleteArea(false);
	}

	for(int y=0;y<sy;y++)
	{
		for(int x=0;x<sx;x++)
		{
			SelectionSetPixel(m_Paste,x,y,p[y * sx + x]);
		}
	}

	if(!m_Paste)
	{
		m_pbm->EndHistory();
	}

	delete [] p;
}


void CChildView::OnSelectionShiftdown()
{
	LPBYTE p;
	int sx, sy;

	if(m_MarkerCount == 2)
	{
		int x, y;
		InterpretMarker(&x,&y,&sx,&sy);
	}
	else if(m_Paste)
	{
		sx = m_PasteSizeX;
		sy = m_PasteSizeY;
	}
	else
	{
		ASSERT(false);
		return;
	}

	p = new BYTE[sx*sy];

	for(int ry=0,wy=1;ry<sy;ry++,wy++)
	{
		if(wy == sy)wy = 0;
		for(int rx=0,wx=0;rx<sx;rx++,wx++)
		{
			p[wy * sx + wx]=SelectionGetPixel(m_Paste,rx,ry);
		}
	}

	if(!m_Paste)
	{
		m_pbm->BeginHistory();
		DeleteArea(false);
	}

	for(int y=0;y<sy;y++)
	{
		for(int x=0;x<sx;x++)
		{
			SelectionSetPixel(m_Paste,x,y,p[y * sx + x]);
		}
	}

	if(!m_Paste)
	{
		m_pbm->EndHistory();
	}

	delete [] p;
}


void CChildView::OnUpdateEditAutoSelect(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_AutoMarker ? 1 : 0);
}


void CChildView::OnEditAutoSelect()
{
	if((m_AutoMarker = m_AutoMarker ? false : true) == false)
		OnEditDeselect();
}


void CChildView::OnViewNextbuffer()
{
	int n = m_pbm->GetBackBuffer()+1;
	if(n == m_pbm->GetBackBufferCount()) n = 0;
	m_pbm->SetBackBuffer(n);
	Mail(MSG_REFRESH);
	Invalidate();
}


void CChildView::OnUpdateViewNextbuffer(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pbm->GetBackBufferCount()>1?1:0);
}


void CChildView::OnViewPrevbuffer()
{
	int n = m_pbm->GetBackBuffer() - 1;
	if(n == -1) n = m_pbm->GetBackBufferCount()-1;
	m_pbm->SetBackBuffer(n);
	Mail(MSG_REFRESH);
	Invalidate();
}


void CChildView::OnUpdateViewPrevbuffer(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pbm->GetBackBufferCount()>1?1:0);
}


void CChildView::OnUpdateOverflowIgnore(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pbm->GetOverflow() == C64Interface::NOTHING ? 1 : 0);
}


void CChildView::OnOverflowIgnore()
{
	m_pbm->SetOverflow(C64Interface::NOTHING);
}


void CChildView::OnUpdateOverflowClosest(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pbm->GetOverflow() == C64Interface::CLOSEST ? 1 : 0);
}


void CChildView::OnOverflowClosest()
{
	m_pbm->SetOverflow(C64Interface::CLOSEST);
}


void CChildView::OnUpdateOverflowReplace(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pbm->GetOverflow() == C64Interface::REPLACE ? 1 : 0);
}


void CChildView::OnOverflowReplace()
{
	m_pbm->SetOverflow(C64Interface::REPLACE);
}


void CChildView::OnColoroverflowToggleignore()
{
	m_pbm->SetOverflow( m_pbm->GetOverflow() == C64Interface::NOTHING ? C64Interface::REPLACE : C64Interface::NOTHING );
}


void CChildView::OnViewRestoremainview()
{
	m_Zoom = 0;
	m_ZoomPoint.SetPoint(0,0);
	m_posX = 0;
	m_posY = 0;
	m_lastScale = 1;
	m_Grid = 1;
	Invalidate();
}


void CChildView::OnUpdatePreviewpixelaspectratioPc(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pbm->GetPAR()==0?1:0);
}


void CChildView::OnPreviewpixelaspectratioPc()
{
	m_pbm->SetPAR(0);
	Mail(MSG_PREV_PAR,UINT_PTR((1.0/m_pbm->GetPARValue())*10000000));
}


void CChildView::OnUpdatePreviewpixelaspectratioPal(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pbm->GetPAR()==1?1:0);
}


void CChildView::OnPreviewpixelaspectratioPal()
{
	m_pbm->SetPAR(1);
	Mail(MSG_PREV_PAR,UINT_PTR((1.0/m_pbm->GetPARValue())*10000000));
}


void CChildView::OnUpdatePreviewpixelaspectratioNtsc(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_pbm->GetPAR()==2?1:0);
}


void CChildView::OnPreviewpixelaspectratioNtsc()
{
	m_pbm->SetPAR(2);
	Mail(MSG_PREV_PAR,UINT_PTR((1.0/m_pbm->GetPARValue())*10000000));
}


void CChildView::OnUpdateFileSaveselection(CCmdUI *pCmdUI)
{
	if(m_pselection)
	{
		delete m_pselection;
		m_pselection = NULL;
	}

	if(m_MarkerCount == 2)
	{
		CImage img;
		int x, y, w, h;
		InterpretMarker(&x,&y,&w,&h);

		m_pselection = m_pbm->CreateFromSelection(x, y, w, h);
	}

	pCmdUI->Enable(m_pselection ? 1 : 0);
}


void CChildView::OnFileSaveselection()
{
	if(m_pselection)
		Saveas(m_pselection);
}


void CChildView::OnUpdateEditSnapselection(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_CellSnapMarker ? 1 : 0);
}


void CChildView::OnEditSnapselection()
{
	m_CellSnapMarker = m_CellSnapMarker ? false : true;
}


void CChildView::SnapToCell(CPoint &pt)
{
	pt.x += m_pbm->GetCellSizeX()/2;
	pt.x -= pt.x%m_pbm->GetCellSizeX();
	pt.y += m_pbm->GetCellSizeY()/2;
	pt.y -= pt.y%m_pbm->GetCellSizeY();
}


void CChildView::OnUpdateToolDeleteundohistory(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pbm->CanUndo()||m_pbm->CanRedo());
}


void CChildView::OnToolDeleteundohistory()
{
	if(AfxMessageBox(_T("Are you sure?"), MB_YESNO) == IDYES)
	{
		m_pbm->DeleteHistory();
	}
}


void CChildView::OnUpdateModePalette(CCmdUI *pCmdUI)
{
	CMenu *sub = pCmdUI->m_pSubMenu;

	if(sub->GetMenuItemCount()==1)
	{
		LPCTSTR str;
		for(int r=0;;r++)
		{
			str=GetPaletteName(r);
			if(!*str)break;
			sub->AppendMenu(MF_STRING, ID_MODE_PALETTE_0+r, str);
		}

		sub->DeleteMenu(ID_PALETTE_DUMMY, MF_BYCOMMAND);
	}
}


void CChildView::OnUpdateModePaletteRange(CCmdUI *pCmdUI)
{
	int check = GetPalette();
	pCmdUI->SetCheck(pCmdUI->m_nID-ID_MODE_PALETTE_0 == check ? 1 : 0);

}


void CChildView::OnModePalette(UINT nID)
{
	int n = int(nID-ID_MODE_PALETTE_0);

	SetPalette(n);
	m_pbm->SetMetaInt("palette", n);

	Invalidate();
	Mail(MSG_REFRESH);
}


void CChildView::OnToolFill()
{
	m_Fill = true;
}


static void workFill(int x, int y, BYTE replace, Monomap *mono, C64Interface *pbm)
{
	std::vector<std::pair<int, int> > queue;

	if(x >= 0 && x < pbm->GetSizeX() && y >= 0 && y < pbm->GetSizeY() && mono->GetPixel(x,y) == 0 && pbm->GetPixel(x,y) == replace)
	{
		queue.push_back(std::pair<int, int>(x,y));

		while(queue.size())
		{
			x = queue[queue.size()-1].first;
			y = queue[queue.size()-1].second;
			queue.pop_back();

			mono->SetPixel(x,y);
		
			if(x+1 < pbm->GetSizeX() && mono->GetPixel(x+1,y) == 0 && pbm->GetPixel(x+1,y) == replace)
				queue.push_back(std::pair<int, int>(x+1,y));

			if(x-1 >= 0 && mono->GetPixel(x-1,y) == 0 && pbm->GetPixel(x-1,y) == replace)
				queue.push_back(std::pair<int, int>(x-1,y));

			if(y+1 < pbm->GetSizeY() && mono->GetPixel(x,y+1) == 0 && pbm->GetPixel(x,y+1) == replace)
				queue.push_back(std::pair<int, int>(x,y+1));

			if(y-1 >= 0 && mono->GetPixel(x,y-1) == 0 && pbm->GetPixel(x,y-1) == replace)
				queue.push_back(std::pair<int, int>(x,y-1));
		}
	}
}


void CChildView::FloodFill(int x, int y, BYTE col, BYTE replace)
{
	m_pbm->BeginHistory();

	Monomap mono(m_pbm->GetSizeX(), m_pbm->GetSizeY());
	workFill(x, y, replace, &mono, m_pbm);

	for(y=0;y<m_pbm->GetSizeY();y++)
	{
		for(x=0;x<m_pbm->GetSizeX();x++)
		{
			if(mono.GetPixel(x,y))
			{
				m_pbm->SetPixel(x,y,col);
			}
		}
	}

	m_pbm->EndHistory();
}


void CChildView::SetViewSpecificMetaData(void)
{
	m_pbm->SetMetaInt("autoselect", m_AutoMarker?1:0);
	m_pbm->SetMetaInt("cellsnap",m_CellSnapMarker?1:0);
}

void CChildView::SetTitleFileName(LPCTSTR file)
{
	nstr tmp,path = file;
	size_t n = path.rfindany(_T("\\/"));
	if(n==-1)
		tmp = path;
	else
		tmp = path.mid(n+1);

	TCHAR *p=new TCHAR[tmp.size()+1];
	lstrcpy(p, tmp);
	Mail(MSG_FILE_TITLE, (UINT_PTR)p);
}


void CChildView::OnHelpLoadintro()
{
	if(LoadIntro())
	{
		Invalidate();
		Mail(MSG_REFRESH);
	}
}


bool CChildView::LoadIntro(void)
{
	HRSRC hIntro = FindResource(NULL, MAKEINTRESOURCE(IDR_INTRO), _T("bin"));
	HGLOBAL hResource = LoadResource(NULL, hIntro);
	if(hResource)
	{
		C64Interface *i = new MCBitmap;
		i->InheritHistory(m_pbm);
		m_pbm = i;

		nmemfile file(LockResource(hResource), SizeofResource(NULL, hIntro));
		m_pbm->Load(file, _T("kla"), 0);

		UnlockResource(hResource);
		m_Zoom = 340;
		return true;
	}

	return false;
}


void CChildView::OnUpdateFont1x1(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pbm->IsChar() ? 1 : 0);
	pCmdUI->SetCheck(m_pbm->GetFontDisplay() == CommonFont::FONT_1X1 ? 1 : 0);
}


void CChildView::OnFont1x1()
{
	m_pbm->SetFontDisplay(CommonFont::FONT_1X1);
	Invalidate();
	Mail(MSG_REFRESH);
}


void CChildView::OnUpdateFont2x1(CCmdUI *pCmdUI)
{
	int cellcount = m_pbm->GetCellCountX() * m_pbm->GetCellCountY();
	pCmdUI->Enable(m_pbm->IsChar() && (m_pbm->GetCellCountX() & 1) == 0 && (cellcount == 256 || cellcount == 128) ? 1 : 0);
	pCmdUI->SetCheck(m_pbm->GetFontDisplay() == CommonFont::FONT_2X1 ? 1 : 0);
}


void CChildView::OnFont2x1()
{
	m_pbm->SetFontDisplay(CommonFont::FONT_2X1);
	Invalidate();
	Mail(MSG_REFRESH);
}


void CChildView::OnUpdateFont1x2(CCmdUI *pCmdUI)
{
	int cellcount = m_pbm->GetCellCountX() * m_pbm->GetCellCountY();
	pCmdUI->Enable(m_pbm->IsChar() && (m_pbm->GetCellCountY() & 1) == 0 && (cellcount == 256 || cellcount == 128) ? 1 : 0);
	pCmdUI->SetCheck(m_pbm->GetFontDisplay() == CommonFont::FONT_1X2 ? 1 : 0);
}


void CChildView::OnFont1x2()
{
	m_pbm->SetFontDisplay(CommonFont::FONT_1X2);
	Invalidate();
	Mail(MSG_REFRESH);
}


void CChildView::OnUpdateFont2x2(CCmdUI *pCmdUI)
{
	int cellcount = m_pbm->GetCellCountX() * m_pbm->GetCellCountY();
	pCmdUI->Enable(m_pbm->IsChar() && (m_pbm->GetCellCountX() & 1) == 0 && (m_pbm->GetCellCountY() & 1) == 0 && cellcount == 256 ? 1 : 0);
	pCmdUI->SetCheck(m_pbm->GetFontDisplay() == CommonFont::FONT_2X2 ? 1 : 0);
}


void CChildView::OnFont2x2()
{
	m_pbm->SetFontDisplay(CommonFont::FONT_2X2);
	Invalidate();
	Mail(MSG_REFRESH);
}
