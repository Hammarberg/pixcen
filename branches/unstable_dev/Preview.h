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


#pragma once
#include "C64Interface.h"


// CPreview

class CPreview : public CWnd, public GraphixMail
{
	DECLARE_DYNAMIC(CPreview)

public:
	CPreview(double scaleY=1.0);
	virtual ~CPreview();


	C64Interface **m_ppInterface;

protected:
	DECLARE_MESSAGE_MAP()

	int m_lastScale;
	CPoint m_ZoomPoint;
	CPoint m_MovePoint;
	int m_Zoom;
	int m_posX;
	int m_posY;

	double m_scaleY;

public:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void Receive(unsigned short message, UINT_PTR data, unsigned short extra);
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);

	CDC *m_pBackBufferDC;

	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


