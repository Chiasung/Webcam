// 3DBar.h: interface for the C3DBar class.
//
//////////////////////////////////////////////////////////////////////

/*#if !defined(AFX_3DBAR_H__E77033B8_CCDE_11D3_8041_00805FC1DE10__INCLUDED_)
#define AFX_3DBAR_H__E77033B8_CCDE_11D3_8041_00805FC1DE10__INCLUDED_ */

#ifndef C3DBAR_H_INCLUDE_NR1
#define C3DBAR_H_INCLUDE_NR1


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "stdafx.h"

class C3DBar  
{
public:
	void SetBarColour(COLORREF cr);
	void UpdateColours();
	void CreatePens();
	void DrawHorizontal(CDC* pDC, CRect& BarRect);

	C3DBar();
	virtual ~C3DBar();

protected:
	void DeletePens();
	COLORREF DarkenColor(const COLORREF crColor, BYTE byReduceVal);
	COLORREF LightenColor(const COLORREF crColor, BYTE byIncreaseVal);
	COLORREF m_crChannel;
	COLORREF m_crColorLight;
	COLORREF m_crColorLighter;
	COLORREF m_crColorLightest;
	COLORREF m_crColorDark;
	COLORREF m_crColorDarker;
	COLORREF m_crDkShadow;
	COLORREF m_crShadow;
	COLORREF m_crLiteShadow;

	CPen m_penColor;
	CPen m_penColorLight;
	CPen m_penColorLighter;
	CPen m_penColorDark;
	CPen m_penColorDarker;
	CPen m_penDkShadow;
	CPen m_penShadow;
	CPen m_penLiteShadow;
};

#endif // !defined(AFX_3DBAR_H__E77033B8_CCDE_11D3_8041_00805FC1DE10__INCLUDED_)
