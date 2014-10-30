// BitmapCtrl.h : header file
//
#ifndef __BMP_CTRL__
#define __BMP_CTRL__

#pragma once


#include "GaussSmooth.h"


#define DEG2RAD (float)0.017453292519943295769236907684886	// PI*2/360
#define SQRT2 (float)1.4142135623730950488016887242097		// sqr(2)


#define CAM_BITS   24


// configuration about what to show on the screen
struct DisplaySud
{
	BOOL black;
	BOOL white;
	BOOL rotation;
	BOOL whiteCandidates;
	BOOL whiteArea;
	BOOL lineDetect;
	BOOL grid;
	BOOL OCRresult;
	BOOL tempSolution;
	BOOL finalSolution;
	BOOL smallFont;
};
// 24-bit pixel
typedef union tagPIX32{
	//typedef struct tagPIX32{
	struct {
		BYTE    blue;
		BYTE    green;
		BYTE    red;
		BYTE    grey;
	}; 
	COLORREF colorRef;
	unsigned int INT32;
}
PIX32, *LPPIX32;



typedef  enum BMPtype{bit8=1,bit24} ;

class CBitmapCtrl : public CWnd
{
private:
    DisplaySud	m_display;				// settings about what to display - editable via View menu.
	BYTE*			m_pMono;			// monochromed bitmap
	BYTE*	        m_outImg;

	unsigned int* pIntegral;
	
public:
	void SetBitmap(HBITMAP hBmp);
	void DrawBitmap();
	__int32	scrollbar_width;
	__int32	m_x_offset;
	__int32	m_y_offset;
	__int32	x_off_,y_off_;
		
	CRect				rcClient,rcClient_backup;
	CSize				image_size;
	HBITMAP		hBitmap;
	HWND			hParent;

	// Draw cross lines
	CClientDC		*DragDC;
	CPoint			Pold,Pnow;
	CPoint			Px11,Px12;
	CPoint			Py11,Py12;
	CPoint			Px21,Px22;
	CPoint			Py21,Py22;

	BOOL				m_bBilevel;
	BOOL				m_bGreyScale;
	BOOL				m_ht_circle;
	int					m_Threshold;


	double			m_R_factor;					
	double			 m_G_factor;					
	double 			 m_B_factor;					
	int					 m_Gain;

	BOOL			     m_InProcessing;
   	    
	BITMAPINFOHEADER	bmpInfoHeader,*m_pBIH;
	__int32	m_width,m_height;
	unsigned __int32	*m_pDCbits;
	BOOL					      m_bLoadFromFile;
	
	unsigned char		*m_pBMPbits;
	unsigned __int32	  m_BytesInBMP;
	unsigned __int32	*m_pRawImg;
	BMPtype				  m_BMPtype;
	
	// show live vedio
	__int32	m_un32ImageWidth;
	__int32	m_un32ImageHeight;
	unsigned __int32	m_un32ImageBits;
	BYTE					*m_pImageBytes;	
	BITMAPINFO	*m_pBmp;
	void				ShowImage(BYTE *pImageByte);
	void				Init_Video_MemDC(void);
	BOOL				DetectRect(int theta);

	
private:
	bool				mb_In_Simulation;
	void				MakeBMPHeader(void);
	
	CDOG				*m_gauss;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBitmapCtrl)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
public:
	LRESULT Refresh_BMP(WPARAM Wparm,LPARAM Lparm);
	LRESULT Clear_Cross_Lines(WPARAM Wparm,LPARAM Lparm);
	BOOL SetPixelColor(int row,int col,unsigned __int32 red);
	BOOL GetBitmapSize(int *row,int *col);
	BOOL GetBitmap8(unsigned char **img);
	void SetBitmap8(int row,int col, unsigned char **img);
	BOOL SetBitmap8Pixel(int row, int col, unsigned char value);
	BOOL GetBitmap8Pixel(int row, int col, unsigned char *value, PIX32 *pix32);
	
	BOOL GetBitmap8_Vedio_Pixel(int row, int col, unsigned char *value);

	BOOL SaveGrayBMP(const char *filename,unsigned char **image);
	BOOL DrawLoadFilebyPixel();
	BOOL LoadFromFile(const	char *filename,	unsigned char *image,unsigned int *row,unsigned int	*col);
	BOOL GetTheRowPixels(int row,int x1,int x2,unsigned __int32 *pRowPixels);
	BOOL GetTheColPixels(int col,int *y1,int *y2,unsigned __int32 *pColPixels);
	void InitBitmap32(int row,int col);
	void SetBitmap32(int row,int col,const unsigned __int32 *img);
	void Set_In_Simulation(bool b_In_Simulation);
	void ConvertToGreyScale();
	void SumIntegralImage(unsigned int* pIntegral);

	virtual ~CBitmapCtrl();

	void SetGreyLevelScalePaper(void);
	// Generated message map functions
protected:
	//{{AFX_MSG(CBitmapCtrl)
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	// chia-sung
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CDC memDC;
	CBitmap* pBitmap;
	CBitmap* pOldBitmap;
};

/////////////////////////////////////////////////////////////////////////////


















#endif