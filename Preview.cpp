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


// Preview.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "Preview.h"

#define ZOOMVALUE 240

// CPreview

IMPLEMENT_DYNAMIC(CPreview, CWnd)

CPreview::CPreview(double scaleY)
{
	m_Zoom = 0;
	m_ZoomPoint.SetPoint(0,0);
	m_posX = 0;
	m_posY = 0;
	m_lastScale = 1;
	m_pBackBufferDC = NULL;
	m_scaleY = scaleY;
}

CPreview::~CPreview()
{
	delete m_pBackBufferDC;
}


BEGIN_MESSAGE_MAP(CPreview, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CPreview message handlers




void CPreview::OnPaint()
{
	CPaintDC realdc(this); // device context for painting

	CRect rc;
	GetClientRect(rc);

	if(!m_pBackBufferDC)
	{
		m_pBackBufferDC = new CDC;
		m_pBackBufferDC->CreateCompatibleDC(&realdc);
	}

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

	C64Interface *pInterface = *m_ppInterface;

	int scale = 1+m_Zoom/ZOOMVALUE;

	int x,y,mx=pInterface->GetSizeX(),my=pInterface->GetSizeY(),pw=pInterface->GetPixelWidth();
	int bmx = scale * mx * pw;
	int bmy = scale * my;
	int cw=pInterface->GetCellSizeX();
	int ch=pInterface->GetCellSizeY();

	COLORREF border = g_Vic2[pInterface->GetBorderColor()];

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

	int posy,posx;

	int mys;

	if (starty < 0)
		mys = (-starty) / scale;
	else
		mys = 0;

	if (starty + scale * my > rc.bottom)
	{
		my = ((rc.bottom - starty) / scale) + 1;
	}

	int mxs;

	if (startx < 0)
		mxs = (-startx) / (scale * pw);
	else
		mxs = 0;

	if (startx + scale * mx * pw > rc.right)
	{
		mx = ((rc.right - startx) / (scale * pw)) + 1;
	}

	for(y=mys;y<my;y++)
	{
		posy = starty + scale * y;

		for(x=mxs;x<mx;x++)
		{
			posx = startx + scale * x * pw;

			int col = pInterface->GetPixel(x,y);
			dc.FillSolidRect(posx,posy,(scale*pw),(scale*1),g_Vic2[col]);
		}
	}

	realdc.StretchBlt(0, 0, rc.right, int(rc.bottom*m_scaleY+0.5), &dc, 0, 0, rc.right, rc.bottom, SRCCOPY);
}


BOOL CPreview::OnEraseBkgnd(CDC* pDC)
{
	return 1;
	//return __super::OnEraseBkgnd(pDC);
}

void CPreview::Receive(unsigned short message, UINT_PTR data, unsigned short extra)
{
	switch(message)
	{
	case MSG_PREV_PAR:
		m_scaleY = double(data)/10000000;
	case MSG_REFRESH:
		Invalidate();
		break;

	default:
		break;
	}
}


void CPreview::OnClose()
{
	Mail(MSG_PREV_CLOSE,(UINT_PTR)this);
	__super::OnClose();
}


void CPreview::OnSize(UINT nType, int cx, int cy)
{
	if(nType!=-1)
		__super::OnSize(nType, cx, cy);
}


void CPreview::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	__super::OnWindowPosChanged(lpwndpos);
	Invalidate();
}

BOOL CPreview::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
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

		//CPoint pix;
		//if(GetPixelFromPoint(m_ZoomPoint, pix))
			//UpdateStatus(pix);
	}

	return __super::OnMouseWheel(nFlags, zDelta, pt);
}


void CPreview::OnMouseMove(UINT nFlags, CPoint point)
{
	if(nFlags&(MK_MBUTTON|MK_RBUTTON))
	{
		m_posX+=point.x-m_MovePoint.x;
		m_posY+=point.y-m_MovePoint.y;
		Invalidate();
	}
	else if(nFlags&MK_LBUTTON)
	{
		OnLButtonDown(nFlags, point);
	}

	m_MovePoint = point;

	__super::OnMouseMove(nFlags, point);
}


bool CPreview::GetPixelFromPoint(CPoint &point, CPoint &pixel)
{
	C64Interface *pInterface = *m_ppInterface;

	int scale = 1+m_Zoom/ZOOMVALUE;

	int x,y,mx=pInterface->GetSizeX(),my=pInterface->GetSizeY(),pw=pInterface->GetPixelWidth();
	int bmx = scale * mx * pw;
	int bmy = scale * my;

	x = (point.x - m_posX);
	y = int((point.y - m_posY) / m_scaleY + 0.5);

	if(x < 0 || x >= bmx || y < 0 || y >= bmy) return false;	//Outside
	
	pixel.x = x / (scale * pw);
	pixel.y = y / (scale);

	return true;
}


void CPreview::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint *pix = new CPoint;
	if(GetPixelFromPoint(point, *pix))
	{
		Mail(MSG_PREV_REPOS, UINT_PTR(pix));
	}
	else
	{
		delete pix;
	}

	__super::OnLButtonDown(nFlags, point);
}
