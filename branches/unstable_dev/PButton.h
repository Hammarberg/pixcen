#pragma once


// CPButton

class CPButton : public CButton
{
	DECLARE_DYNAMIC(CPButton)

public:
	CPButton();
	virtual ~CPButton();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


