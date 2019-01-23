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



// ChildView.h : interface of the CChildView class
//


#pragma once
#include "C64Interface.h"

// CChildView window

class CChildView : public CWnd, public GraphixMail
{
// Construction
public:
	CChildView();

// Attributes
public:
	C64Interface *m_pbm;
	C64Interface *m_pselection;


// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);


private:
	bool m_goodFormat;
	void SetTitleFileName(LPCTSTR file);

	int m_TimeCount;

	int m_Zoom;
	int m_lastScale;
	CPoint m_ZoomPoint;
	int m_posX;
	int m_posY;
	int m_Grid;
	int m_CellGrid;

	int m_Col1;
	int m_Col2;
	int m_ActivePaintCol;

	bool m_ColorPick;

	COLORREF m_GridColorPixel;
	COLORREF m_GridColorCell;

	CPen m_fLine;
	CPen m_cLine;
	CPen m_sLine[2];

	bool m_CellSnapMarker;
	bool m_AutoMarker;
	bool m_ManualMarker;
	bool m_Marker;
	int m_MarkerCount;
	CPoint m_Mark[2];
	
	void InterpretMarker(int *x, int *y, int *w, int *h);

	bool m_Paste;
	bool m_MaskedPaste;
	BYTE *m_pPasteBuffer;
	CPoint m_PastePoint;
	int	 m_PasteSizeX;
	int	 m_PasteSizeY;
	int m_CutX, m_CutY, m_CutW, m_CutH;

	bool m_Move;
	bool m_Paint;
	bool m_ShowColourInfo;
	CPoint m_LastPaintPix;	//bm coord

	CPoint m_MovePoint;

	bool m_Fill;

	void FloodFill(int x, int y, BYTE col, BYTE replace);

	bool GetPixelFromPoint(CPoint &point, CPoint &pixel, bool select=false);

	void SelectInfo(CellInfo *pset=NULL);

	BYTE SelectionGetPixel(bool pastemode, int x, int y);
	void SelectionSetPixel(bool pastemode, int x, int y, BYTE col);

	static int sign(int x){return x == 0 ? 0 : ( x<0 ? -1 : 1 );}


	void SetViewSpecificMetaData(void);

	CDC *m_pBackBufferDC;
	CImageFast *m_pBackBufferImage;

protected:
	void ZoomCommand(short zDelta, CPoint &pt);

	void SnapToCell(CPoint &pt);

//	afx_msg LRESULT OnUmSelCol1(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnUmSelCol2(WPARAM wParam, LPARAM lParam);

public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnFileLoad();
	afx_msg void OnFileSave();
	void Receive(unsigned short message, UINT_PTR data, unsigned short extra);
protected:
//	afx_msg LRESULT OnUmCellInfoCol(WPARAM wParam, LPARAM lParam);
	bool CheckDirty(void);
	void Load(LPCTSTR file, LPCTSTR ex);

	void OnPaste(bool masked=false);
	void Saveas(C64Interface *i);

public:
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
	afx_msg void OnFileSaveas();
//	afx_msg void OnClose();
//	virtual BOOL DestroyWindow();
//	afx_msg void OnDestroy();

	bool FrameClose(void);
	void UpdateStatus(CPoint &point);
//	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	afx_msg void OnViewGrid();
	afx_msg void OnUpdateViewGrid(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
	afx_msg void OnToolOptimize();
	afx_msg void OnUpdateToolOptimize(CCmdUI *pCmdUI);
	afx_msg void OnModeBitmap();
	afx_msg void OnUpdateModeBitmap(CCmdUI *pCmdUI);
	afx_msg void OnModeSprite();
	afx_msg void OnUpdateModeSprite(CCmdUI *pCmdUI);
	afx_msg void OnModeChar();
	afx_msg void OnUpdateModeChar(CCmdUI *pCmdUI);
//	afx_msg void OnModeCharscreen();
//	afx_msg void OnUpdateModeCharscreen(CCmdUI *pCmdUI);
	afx_msg void OnModeMulti();
	afx_msg void OnUpdateModeMulti(CCmdUI *pCmdUI);

	void ChangeMode(C64Interface::tmode to);

	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
//	virtual BOOL DestroyWindow();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI *pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI *pCmdUI);
	afx_msg void OnModeUnrestricted();
	afx_msg void OnUpdateModeUnrestricted(CCmdUI *pCmdUI);
	afx_msg void OnEditSelectmarker();
	afx_msg void OnEditSelectcell();
	afx_msg void OnEditSelectall();
	afx_msg void OnFileNew();
	afx_msg void OnEditDeselect();
	afx_msg void OnUpdateEditDeselect(CCmdUI *pCmdUI);
	void DeleteArea(bool history=true);
	afx_msg void OnCopyromfontUppercase();
	afx_msg void OnUpdateCopyromfontUppercase(CCmdUI *pCmdUI);
	afx_msg void OnCopyromfontLowercase();
	afx_msg void OnUpdateCopyromfontLowercase(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSelectionFlipvertically(CCmdUI *pCmdUI);
	afx_msg void OnSelectionFlipvertically();
	afx_msg void OnUpdateSelectionFliphorizontally(CCmdUI *pCmdUI);
	afx_msg void OnSelectionFliphorizontally();
	afx_msg void OnUpdateSelectionRotatecw(CCmdUI *pCmdUI);
	afx_msg void OnSelectionRotatecw();
	afx_msg void OnUpdateSelectionRotateccw(CCmdUI *pCmdUI);
	afx_msg void OnSelectionRotateccw();
	afx_msg void OnUpdateSelectionShiftleft(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSelectionShiftright(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSelectionShiftup(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSelectionShiftdown(CCmdUI *pCmdUI);
	afx_msg void OnSelectionShiftleft();
	afx_msg void OnSelectionShiftright();
	afx_msg void OnSelectionShiftup();
	afx_msg void OnSelectionShiftdown();

	afx_msg void OnUpdateEditAutoSelect(CCmdUI *pCmdUI);
	afx_msg void OnEditAutoSelect();
	afx_msg void OnViewNextbuffer();
	afx_msg void OnUpdateViewNextbuffer(CCmdUI *pCmdUI);
	afx_msg void OnViewPrevbuffer();
	afx_msg void OnUpdateViewPrevbuffer(CCmdUI *pCmdUI);
	afx_msg void OnEditBackgroundmaskedpaste();
	afx_msg void OnUpdateEditBackgroundmaskedpaste(CCmdUI *pCmdUI);
	afx_msg void OnUpdateOverflowIgnore(CCmdUI *pCmdUI);
	afx_msg void OnOverflowIgnore();
	afx_msg void OnUpdateOverflowClosest(CCmdUI *pCmdUI);
	afx_msg void OnOverflowClosest();
	afx_msg void OnUpdateOverflowReplace(CCmdUI *pCmdUI);
	afx_msg void OnOverflowReplace();
	afx_msg void OnColoroverflowToggleignore();

	afx_msg void OnViewRestoremainview();
	afx_msg void OnUpdatePreviewpixelaspectratioPc(CCmdUI *pCmdUI);
	afx_msg void OnPreviewpixelaspectratioPc();
	afx_msg void OnUpdatePreviewpixelaspectratioPal(CCmdUI *pCmdUI);
	afx_msg void OnPreviewpixelaspectratioPal();
	afx_msg void OnUpdatePreviewpixelaspectratioNtsc(CCmdUI *pCmdUI);
	afx_msg void OnPreviewpixelaspectratioNtsc();
	afx_msg void OnUpdateFileSaveselection(CCmdUI *pCmdUI);
	afx_msg void OnFileSaveselection();
	afx_msg void OnUpdateEditSnapselection(CCmdUI *pCmdUI);
	afx_msg void OnEditSnapselection();
	afx_msg void OnUpdateToolDeleteundohistory(CCmdUI *pCmdUI);
	afx_msg void OnToolDeleteundohistory();
	afx_msg void OnUpdateModePalette(CCmdUI *pCmdUI);
	afx_msg void OnModePalette(UINT nID);
	afx_msg void OnUpdateModePaletteRange(CCmdUI *pCmdUI);

	afx_msg void OnToolFill();
	afx_msg void OnViewCellgrid();
	afx_msg void OnUpdateViewCellgrid(CCmdUI *pCmdUI);
	afx_msg void OnHelpLoadintro();
	bool LoadIntro(void);
	afx_msg void OnUpdateFont1x1(CCmdUI *pCmdUI);
	afx_msg void OnFont1x1();
	afx_msg void OnUpdateFont2x1(CCmdUI *pCmdUI);
	afx_msg void OnFont2x1();
	afx_msg void OnUpdateFont1x2(CCmdUI *pCmdUI);
	afx_msg void OnFont1x2();
	afx_msg void OnUpdateFont2x2(CCmdUI *pCmdUI);
	afx_msg void OnFont2x2();
	afx_msg void OnViewGridcolors();
	afx_msg void OnColourView();
	afx_msg void OnUpdateColourView(CCmdUI* pCmdUI);
	afx_msg void OnToolRemapcolours();
	afx_msg void OnUpdateToolRemapcolours(CCmdUI *pCmdUI);
	afx_msg void OnToolSwapcellcolours();
	afx_msg void OnUpdateToolSwapcellcolours(CCmdUI *pCmdUI);
};

