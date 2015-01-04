// PButton.cpp : implementation file
//

#include "stdafx.h"
#include "Pixcen.h"
#include "PButton.h"


// CPButton

IMPLEMENT_DYNAMIC(CPButton, CButton)

CPButton::CPButton()
{

}

CPButton::~CPButton()
{
}


BEGIN_MESSAGE_MAP(CPButton, CButton)
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CPButton message handlers

void CPButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	GraphixMail::Mail(MSG_CELL_REMAP_CLICK);
	CButton::OnLButtonUp(nFlags, point);
}

