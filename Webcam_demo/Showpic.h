//////////////////////////////////////////////////////////////////////
//
//  This class is designed to provide simple interface for 
//  simultaneous Video Capture & Preview using DirectShow
//
//////////////////////////////////////////////////////////////////////
//
//	References: MS DirectShow Samples
//
//		
//////////////////////////////////////////////////////////////////////
//
//	This class was written by Sagar K.R . 
//  Use of this class is not restricted in any
//	way whatsoever.Please report the bugs to krssagar@firsteccom.co.kr
//
//	Special thanks to all the members at The Code Project! 
//	(www.codeproject.com)
//
//////////////////////////////////////////////////////////////////////
#if !defined(AFX_SHOWPIC_H__C4AB306E_DBF3_4E18_8EC9_9AB0DAF4BCC0__INCLUDED_)
#define AFX_SHOWPIC_H__C4AB306E_DBF3_4E18_8EC9_9AB0DAF4BCC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Showpic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CShowpic window

class CShowpic : public CStatic
{
// Construction
public:
	CShowpic();

	int		m_un32ImageBits;
	BYTE		*m_pImageBytes;
// Attributes
public:

	
// Operations
public:

	BITMAPINFO* m_pBmp;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShowpic)
	//}}AFX_VIRTUAL

// Implementation
public:
	void	ShowImage(BYTE *pImageByte, int width, int height);
	void	MakeBMPHeader();
	virtual ~CShowpic();

	// Generated message map functions
protected:
	//{{AFX_MSG(CShowpic)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHOWPIC_H__C4AB306E_DBF3_4E18_8EC9_9AB0DAF4BCC0__INCLUDED_)
