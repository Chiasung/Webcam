
#include "stdafx.h"
#include "BitmapCtrl.h"


//////////////////////////////////////////////////////////////////////////////
CBitmapCtrl::~CBitmapCtrl()
{
	if(memDC.GetSafeHdc() != NULL){
		memDC.SelectObject(pOldBitmap);
		if(pBitmap != NULL){
			delete pBitmap;
		}
	}
	if(DragDC){
		delete DragDC;
		DragDC=NULL;
	}
	if(m_pDCbits!=NULL){
		delete [] m_pDCbits;
		m_pDCbits = NULL;
	}

	if(m_pBMPbits!=NULL){
		delete [] m_pBMPbits;
	}

	if(m_pMono){
		delete [] m_pMono;
	}

	if(pIntegral){
		delete [] pIntegral;
	}

	if(m_gauss){
		delete m_gauss;
	}
	
}
//////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CBitmapCtrl, CWnd)
	//{{AFX_MSG_MAP(CBitmapCtrl)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(REFRESH_BMP,CBitmapCtrl::Refresh_BMP)
	ON_MESSAGE(CLEAR_CROSS_LINE,CBitmapCtrl::Clear_Cross_Lines)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////
BOOL CBitmapCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd) 
{
	scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
	DragDC=NULL;
		
	// create window
	BOOL ret;
	hParent=pParentWnd->GetSafeHwnd();
	
	static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	/************************************************************************/
	ret = CWnd::CreateEx(WS_EX_CLIENTEDGE,className,NULL,dwStyle, 
						rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,
						pParentWnd->GetSafeHwnd(),0);
	
	m_width	=rect.right  - rect.left;
	m_height =rect.bottom - rect.top;

	/* When execute the "CreateEx", we input the parentWnd Handle into it!! */
	/*********************************************************/

	// init virtual screen
	GetClientRect(rcClient);
	rcClient_backup=rcClient;
	CClientDC dc(this);
	if(memDC.GetSafeHdc() == NULL)
	{
		memDC.CreateCompatibleDC(&dc);
		if((pBitmap = new CBitmap()) == NULL) return FALSE;
		pBitmap->CreateCompatibleBitmap(&dc,rcClient.Width(),rcClient.Height());
		pOldBitmap = memDC.SelectObject(pBitmap);
		memDC.SelectStockObject(ANSI_VAR_FONT);
		memDC.SetBkMode(TRANSPARENT);
		
	}
	Px11=CPoint(0,0);
	Px12=CPoint(0,0);
	Py11=CPoint(0,0);
	Py12=CPoint(0,0);

	m_x_offset = 0;
	m_y_offset = 0;
	memDC.FillSolidRect(&rcClient,RGB(128,128,128));

	//*********************************************
	
	m_pDCbits			=NULL;
	m_pBMPbits			=NULL;

	m_pMono				=NULL;
	m_outImg				=NULL;  

	m_gauss = NULL;

	pIntegral =NULL;
	
	m_width	=1;
	m_height=1;

	mb_In_Simulation	=false;
	
	m_un32ImageWidth	=1;      //CAM_WIDTH;
	m_un32ImageHeight	=1;  //CAM_HEIGHT;
	m_un32ImageBits	=CAM_BITS;
	m_pImageBytes		=NULL;
	MakeBMPHeader();	

	m_bBilevel			= false;
	m_bGreyScale   = false;
		m_R_factor	= 0.299;
	m_G_factor		= 0.587;	
	m_B_factor		= 0.587;

	m_Threshold		= 128;
	m_Gain				= 1;


	return ret;
}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::OnPaint() 
{
	// Display memDC on the screen !
	CPaintDC dc(this);
	if(memDC.GetSafeHdc() != NULL){
		dc.BitBlt(0,0,rcClient.Width(),rcClient.Height(),&memDC,0,0,SRCCOPY);
	}
}

//////////////////////////////////////////////////////////////////////////////
LRESULT CBitmapCtrl::Clear_Cross_Lines(WPARAM Wparm,LPARAM Lparm)
{

	CPoint point(-m_x_offset,-m_y_offset);

	if(m_pDCbits!=NULL ){
		int result=0;
		result=StretchDIBits( memDC, -m_x_offset, -m_y_offset, 	
								    m_width, m_height,	0, 0,
								    m_pBIH->biWidth, m_pBIH->biHeight,	
								    m_pDCbits,	
								    (BITMAPINFO *) m_pBIH,
								    BI_RGB, SRCCOPY   );

		Px11=CPoint(0,0);
		Px12=CPoint(0,0);
		Py11=CPoint(0,0);
		Py12=CPoint(0,0);
	   //SRCPAINT NOTSRCERASE
	}else{
		//memDC.DrawState(point,image_size,hBitmap,DST_BITMAP|DSS_NORMAL);
	} 

		return true;
}

//////////////////////////////////////////////////////////////////////////////
LRESULT CBitmapCtrl::Refresh_BMP(WPARAM Wparm,LPARAM Lparm)
{
	CPoint point(-m_x_offset, -m_y_offset);

	if(m_pDCbits!=NULL && !m_bLoadFromFile){

		Invalidate();	

		int aaa=StretchDIBits( memDC, -m_x_offset, -m_y_offset,
								m_width, m_height,
								0, 0,
								m_pBIH->biWidth, m_pBIH->biHeight,
								m_pDCbits,
								(BITMAPINFO *) m_pBIH,
								BI_RGB, SRCCOPY  /*SRCPAINT NOTSRCERASE*/ );

		memDC.SetROP2(R2_NOTXORPEN);
		memDC.MoveTo(Px11);
		memDC.LineTo(Px12);
		memDC.MoveTo(Py11);
		memDC.LineTo(Py12);   

		Px21=CPoint((int)Wparm,0);
		Px22=CPoint((int)Wparm,m_height);
		Py21=CPoint(0	,(int)Lparm);
		Py22=CPoint(m_width	,(int)Lparm);



		Pold = (Wparm, Lparm);//point;
		Px11=Px21;
		Px12=Px22;
		Py11=Py21;
		Py12=Py22; 

		::SendMessage(hParent,SHOW_SCROLL_POS,Wparm,Lparm); 
	}else{
			//memDC.DrawState(point,image_size,hBitmap,DST_BITMAP|DSS_NORMAL);
	}

	Invalidate();			
	return 1;
}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::OnMouseMove( UINT nFlags, CPoint point )
{
	if(!mb_In_Simulation){
		if((point.x < (m_width))  && (point.x >0) &&
		   (point.y < (m_height)) && (point.y >0)){


			memDC.SetROP2(R2_NOTXORPEN);
			memDC.MoveTo(Px11);
			memDC.LineTo(Px12);
			memDC.MoveTo(Py11);
			memDC.LineTo(Py12);


			Px21=CPoint(point.x	,0);
			Px22=CPoint(point.x	,m_height);
			Py21=CPoint(0			,point.y);
			Py22=CPoint(m_width	,point.y);


			memDC.MoveTo(Px21);
			memDC.LineTo(Px22);
			memDC.MoveTo(Py21);
			memDC.LineTo(Py22);
		
			Pold = point;
			Px11=Px21;
			Px12=Px22;
			Py11=Py21;
			Py12=Py22;

			Invalidate();	

		}else{
			Clear_Cross_Lines(0,0);
			Invalidate();		
	
		}
		::SendMessage(hParent, SHOW_SCROLL_POS, point.x + m_x_offset, point.y + m_y_offset); 
	}

}
//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::OnLButtonUp( UINT nFlags, CPoint point )
{

}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::OnLButtonDown( UINT nFlags, CPoint point )
{


}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::SetBitmap(HBITMAP hBmp)
{
	m_bLoadFromFile=TRUE;
	//reset offsets
	m_x_offset = 0;
	m_y_offset = 0;

	//reset scrollbar
	SCROLLINFO		si;
	si.fMask		= SIF_PAGE | SIF_RANGE;
	si.nMin		= 0;
	si.nMax		= 0;
	si.nPage		= 0;
	SetScrollInfo(SB_HORZ, &si, TRUE);
	SetScrollInfo(SB_VERT, &si, TRUE);
	
	//redraw background
	GetClientRect(&rcClient);
	memDC.FillSolidRect(&rcClient,RGB(128,128,128));
		
	//get bitmap handle
	hBitmap = hBmp;

	if (hBitmap == 0){
		memDC.TextOut(0,0,"File not found.");
		Invalidate();
		return;
	}
	//get image size
	CPoint point(m_x_offset, m_y_offset);
   //------------------------------------------------
	BITMAP bmpInfo;
	CBitmap::FromHandle(hBitmap)->GetBitmap(&bmpInfo);
	//------------------------------------------------
	image_size.cx  = bmpInfo.bmWidth;
	image_size.cy = bmpInfo.bmHeight;
	
	//check image size
	BOOL x_fit;
	BOOL y_fit;
	x_fit = (bmpInfo.bmWidth <= rcClient.Width());
	if (!x_fit){
		rcClient.bottom -= scrollbar_width;
	}
	y_fit = (bmpInfo.bmHeight <= rcClient.Height());
	
	if (!y_fit){
		rcClient.right -= scrollbar_width;
		x_fit = (bmpInfo.bmWidth <= rcClient.Width());
	}
	
	if (!x_fit){
		ShowScrollBar(SB_HORZ);
		// update scrollbar
		SCROLLINFO si;
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nMin  = 0;
		si.nMax  = bmpInfo.bmWidth-1;
		si.nPage = rcClient.Width();
		SetScrollInfo(SB_HORZ, &si, TRUE);
	}
	
	if (!y_fit){
		// show scrollbar
		ShowScrollBar(SB_VERT);
		// update scrollbar
		SCROLLINFO si;
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nMin  = 0;
		si.nMax  = bmpInfo.bmHeight-1;
		si.nPage = rcClient.Height();
		SetScrollInfo(SB_VERT, &si, TRUE);
	}
	//	DrawBitmap();
	/*************************************************/
	if(m_pBMPbits!=NULL) 
		delete [] m_pBMPbits;
	
	m_pBMPbits	=new unsigned char [bmpInfo.bmWidthBytes*bmpInfo.bmHeight];
	m_width		    =bmpInfo.bmWidth;
	m_height		=bmpInfo.bmHeight;
	//------------------------------------------------
	memcpy(m_pBMPbits,bmpInfo.bmBits, bmpInfo.bmWidthBytes*bmpInfo.bmHeight);
	//------------------------------------------------

	if(bmpInfo.bmWidthBytes/bmpInfo.bmWidth >1){
		m_BMPtype=bit24;
	}else{
		m_BMPtype=bit8;
	}
}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::DrawBitmap()
{

	CPoint point(-m_x_offset, -m_y_offset);

	if(m_pDCbits!=NULL && !m_bLoadFromFile){
		      StretchDIBits( memDC, 
										-m_x_offset, -m_y_offset,
										m_width, m_height,
										0, 0,
										m_pBIH->biWidth, m_pBIH->biHeight,
										m_pDCbits,
										(BITMAPINFO *) m_pBIH,
										BI_RGB, SRCCOPY   );
		//SRCPAINT NOTSRCERASE
	}else{
		//memDC.DrawState(point,image_size,hBitmap,DST_BITMAP|DSS_NORMAL);
	} 
}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int nSmall = 1;
	int nLarge = 20;

	if(pScrollBar == GetScrollBarCtrl(SB_HORZ))
	{
		if(nSBCode == SB_LEFT || nSBCode == SB_LINELEFT)
			m_x_offset -= nSmall;
		if(nSBCode == SB_PAGELEFT)
			m_x_offset -= nLarge;
		if(nSBCode == SB_RIGHT || nSBCode == SB_LINERIGHT)
			m_x_offset += nSmall;
		if(nSBCode == SB_PAGERIGHT)
			m_x_offset += nLarge;
		if(nSBCode == SB_THUMBPOSITION)
			m_x_offset = (int)nPos;
		if(nSBCode == SB_THUMBTRACK)
			m_x_offset = (int)nPos;
		if (m_x_offset<0)
			m_x_offset = 0;
		if (m_x_offset>image_size.cx - rcClient.Width())
			m_x_offset = image_size.cx - rcClient.Width();
		SetScrollPos(SB_HORZ, m_x_offset, TRUE);
		DrawBitmap();
	}
	memDC.SetROP2(R2_NOTXORPEN);
	memDC.MoveTo(Px11);
	memDC.LineTo(Px12);
	memDC.MoveTo(Py11);
	memDC.LineTo(Py12);
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int nSmall = 1;
	int nLarge = 20;

	if(pScrollBar == GetScrollBarCtrl(SB_VERT))
	{
		if(nSBCode == SB_TOP || nSBCode == SB_LINEUP)
			m_y_offset -= nSmall;
		if(nSBCode == SB_PAGEUP)
			m_y_offset -= nLarge;
		if(nSBCode == SB_BOTTOM || nSBCode == SB_LINEDOWN)
			m_y_offset += nSmall;
		if(nSBCode == SB_PAGEDOWN)
			m_y_offset += nLarge;
		if(nSBCode == SB_THUMBPOSITION)
			m_y_offset = (int)nPos;
		if(nSBCode == SB_THUMBTRACK)
			m_y_offset = (int)nPos;
		if (m_y_offset<0)
			m_y_offset = 0;
		if (m_y_offset>image_size.cy - rcClient.Height())
			m_y_offset = image_size.cy - rcClient.Height();
		SetScrollPos(SB_VERT, m_y_offset, TRUE);
		DrawBitmap();
	}
	memDC.SetROP2(R2_NOTXORPEN);
	memDC.MoveTo(Px11);
	memDC.LineTo(Px12);
	memDC.MoveTo(Py11);
	memDC.LineTo(Py12);

	Invalidate();
}


//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::InitBitmap32(int row,int col)
{
	if( (row >0)&& (col >0)){

		m_bLoadFromFile	= FALSE;
		rcClient				= rcClient_backup;

		if((m_width	!= col) ||(m_height	 != row)){

			if(m_pDCbits!=NULL){
				delete [] m_pDCbits;
				m_pDCbits = NULL;
			}

		}

		m_width				= col;
		m_height				= row;

		// Fill the bitmap info structure
		bmpInfoHeader.biSize				= sizeof(BITMAPINFOHEADER);
		bmpInfoHeader.biPlanes			= 1;
		bmpInfoHeader.biBitCount		= 32;            
		bmpInfoHeader.biCompression	= BI_RGB;
		bmpInfoHeader.biXPelsPerMeter	= 0;
		bmpInfoHeader.biYPelsPerMeter	= 0;
		bmpInfoHeader.biClrUsed			= 1;
		bmpInfoHeader.biClrImportant	= 0;
		bmpInfoHeader.biWidth			= m_width;
		bmpInfoHeader.biHeight			= m_height;
		bmpInfoHeader.biSizeImage		= m_width * m_height;

		if(m_pMono!=NULL){
			delete [] m_pMono;
		}

		if(m_outImg!=NULL){
			delete [] m_outImg;
		}

		if(pIntegral!=NULL){
			delete [] pIntegral;
		}

		m_pMono = new BYTE[m_width*m_height];			
		m_outImg  = new BYTE[m_width*m_height];			
		m_pDCbits=new unsigned __int32 [m_width*m_height];
        pIntegral    =new unsigned int [m_width*m_height];
		m_pBIH	    = &bmpInfoHeader;
	}
}
//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::SetBitmap32(int row, int col, const unsigned __int32 *img)
{
	if((row >0) && (col >0) && (img !=NULL)){

		if( (row != m_height) || (col != m_width) ){
			InitBitmap32(row, col);
		}

		
		memcpy(m_pDCbits, img, m_width*m_height*4);
		int yOffset;

		for(int i=0; i< m_height; i++){
			yOffset = i*m_width;
			for(int j=0; j< m_width; j++){
				*(m_pDCbits+ yOffset+m_width - j) =  *(img+ yOffset+ j) ;
			}
		}
		


		
		LPPIX32 buf = (LPPIX32)m_pDCbits;
		int intensityI;
		for(int i=0; i< m_width*m_height; i++){
			intensityI = (int)((buf[i].red     *0.299 +
							  		buf[i].green *0.587 +
									buf[i].blue   *0.587 )) ;

			if(intensityI > 254){
				buf[i].grey = 254;
				m_pMono[i] = 254; 
			}else{
				buf[i].grey = (unsigned char )intensityI ;
				m_pMono[i] = (unsigned char )intensityI ; 
			}
		}

		//=============================
		if(m_bBilevel && !m_bGreyScale){
			AdaptiveThreshold();  //CSL
		}else	if(!m_bBilevel && m_bGreyScale  ){
				for(int i=0; i< m_width*m_height; i++){
					buf[i].red		= buf[i].grey;
					buf[i].green	= buf[i].grey;
					buf[i].blue		= buf[i].grey;
				}
		}
		//=============================



		//SetBitmap(hBitmap);
		//reset offsets
		m_x_offset = 0;
		m_y_offset = 0;

		//reset scrollbar
		SCROLLINFO si;
		si.fMask		= SIF_PAGE | SIF_RANGE;
		si.nMin		= 0;
		si.nMax		= 0;
		si.nPage		= 0;
		SetScrollInfo(SB_HORZ, &si, TRUE);
		SetScrollInfo(SB_VERT, &si, TRUE);


		//get image size

		CPoint point(m_x_offset, m_y_offset);
		/*BITMAP bmpInfo;
		CBitmap::FromHandle(hBitmap)->GetBitmap(&bmpInfo);*/
		image_size.cx = m_width;//bmpInfo.bmWidth;
		image_size.cy = m_height;//bmpInfo.bmHeight;

		//check image size
		BOOL x_fit;
		BOOL y_fit;
		
		x_fit = (m_width <= rcClient.Width());
		if (!x_fit){
			rcClient.bottom -= scrollbar_width;
		}else{
			memDC.FillSolidRect(rcClient,0x0);
		}
		
		y_fit = (m_height <= rcClient.Height());
		if (!y_fit){
			rcClient.right -= scrollbar_width;
			x_fit = (m_width <= rcClient.Width());
		}else{
			memDC.FillSolidRect(rcClient,0x0);
		}

		if (!x_fit){
			ShowScrollBar(SB_HORZ);
			// update scrollbar
			SCROLLINFO si;
			si.fMask		= SIF_PAGE | SIF_RANGE;
			si.nMin		= 0;
			si.nMax		= m_width-1;
			si.nPage		= rcClient.Width();
			SetScrollInfo(SB_HORZ, &si, TRUE);
		}
		if (!y_fit){
			// show scrollbar
			ShowScrollBar(SB_VERT);
			// update scrollbar
			SCROLLINFO si;
			si.fMask		= SIF_PAGE | SIF_RANGE;
			si.nMin		= 0;
			si.nMax		= m_height-1;
			si.nPage		= rcClient.Height();
			SetScrollInfo(SB_VERT, &si, TRUE);
		}

		DrawBitmap();

		memDC.SetROP2(R2_NOTXORPEN);
		memDC.MoveTo(Px11);
		memDC.LineTo(Px12);
		memDC.MoveTo(Py11);
		memDC.LineTo(Py12);
	}
}

//////////////////////////////////////////////////////////////////////////////
BOOL CBitmapCtrl::GetTheColPixels(int col, int *y1, int *y2, unsigned __int32 *pColPixels)
{
	int start_pixel=*y1, stop_pixel=(*y2+1);
	// Now, Only support the 8 bits monochrome image !
	if(m_pDCbits!=NULL && !m_bLoadFromFile){
		for(int i=start_pixel;i<stop_pixel;i++){
			*(pColPixels+i)=*(m_pDCbits+i*m_width+col);
		}
		return TRUE;
	}
	if(m_bLoadFromFile){
		COLORREF colour;
		for(int i=start_pixel;i<stop_pixel;i++){
			colour=memDC.GetPixel(col,i);
			*(pColPixels+i)=(unsigned int)colour;
		}
		return TRUE;
	}else{

		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////////////
BOOL CBitmapCtrl::GetTheRowPixels(int row, int x1, int x2, unsigned __int32 *pRowPixels)
{
	int start_pixel=x1;
	int stop_pixel =(x2+1);


	if(x1 == -1){
		start_pixel = 0;
	}
	if(x2 == -1){
		stop_pixel =  m_pBIH->biWidth;
	}

	// Now, Only support the 8 bits monochrome image !
	if(m_pDCbits!=NULL && !m_bLoadFromFile){
		
		for(int i=start_pixel; i<stop_pixel; i++){
			*(pRowPixels+i) = *(m_pDCbits+m_width*(m_pBIH->biHeight-row)+i);
		}
		return TRUE;
	}


	if(m_bLoadFromFile){
		COLORREF colour;
		for(int i=start_pixel; i<stop_pixel; i++){
			colour=memDC.GetPixel(i,row);

			*(pRowPixels+i)=(unsigned __int32)colour;
		}
		return TRUE;
	}else{
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////////////
BOOL CBitmapCtrl::LoadFromFile(const char    *filename,
										 unsigned char *image,
										 unsigned int	*row,
										 unsigned int	*col)
{
	return (TRUE);
}

//////////////////////////////////////////////////////////////////////////////
BOOL CBitmapCtrl::DrawLoadFilebyPixel()
{
	if(m_pBMPbits!=NULL){
		int extrabytes,bytesize;
		int row	=	m_height;
		int col	=	m_width;
		
		unsigned __int32 *img;
		img	=new unsigned __int32 [row*col];
		
	   extrabytes	= (4 - (m_width * 3) % 4) % 4;
		bytesize		= (m_width * 3 + extrabytes) * m_height;

		switch(m_BMPtype){
			case bit24:{
				for(int i=0;i<row;i++){
					for(int j=0;j<col;j++){
						img[i*m_width+j]=(*(m_pBMPbits+(m_width*3+extrabytes)*i+j*3) |*(m_pBMPbits+(m_width*3+extrabytes)*i+j*3+1)<<8 | *(m_pBMPbits+(m_width*3+extrabytes)*i+j*3+2)<<16)&0x00FFFFFF; //((i)%255 | (i)%255 <<8 | (i)%255 <<16)&0x00FFFFFF;
					}
				}
				break;
			}
			case bit8:{
				for(int i=0;i<row;i++){
					for(int j=0;j<col;j++){
						img[i*m_width+j]=(*(m_pBMPbits+(m_width+extrabytes)*i+j) | *(m_pBMPbits+(m_width+extrabytes)*i+j) <<8 | *(m_pBMPbits+(m_width+extrabytes)*i+j) <<16)&0x00FFFFFF;
					}
				}
				break;
			}
		}

		unsigned char *tmpImg;
		tmpImg=new unsigned char [row*col];
		for(int i=0;i<row;i++){
			for(int j=0;j<col;j++){
				tmpImg[i*col+j]=(unsigned char )((img[i*col+j] & 0x000000FF)*0.299+((img[i*col+j] >> 8) & 0x000000FF)*0.587+((img[i*col+j] >> 16) & 0x000000FF)*0.114);
			}
		}
				
		InitBitmap32(row,col);

		SetBitmap32(row,col,img);
		delete [] tmpImg;
		delete [] img;
		return (TRUE);
	}
	else
		return (FALSE);
}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::Set_In_Simulation(bool b_In_Simulation)
{
	mb_In_Simulation = b_In_Simulation;

}

//////////////////////////////////////////////////////////////////////////////
BOOL CBitmapCtrl::SaveGrayBMP(const char *filename,unsigned char **image)
{

	BITMAPINFOHEADER       bmpInfoHeader;
	BITMAPFILEHEADER       bmpFileHeader;
	FILE                   *filep=NULL;
	unsigned __int32    col,row,width,height,i,numPaletteEntries = 256;
	unsigned __int32    extrabytes, bytesize;
	unsigned char          *paddedImage = NULL;
	RGBQUAD				palette[256];

	width	=m_width;
	height=m_height;

   // Create the palette - each pixel is an index into the palette
	for(i = 0; i < numPaletteEntries; i++){
		palette[i].rgbRed			= i;
		palette[i].rgbGreen		= i;
		palette[i].rgbBlue		= i;
		palette[i].rgbReserved	= 0;
	}
	/* The .bmp format requires that the image data is aligned on a 4 byte boundary.  For 8 - bit bitmaps,
	   this means that the width of the bitmap must be a multiple of 4. This code determines
	   the extra padding needed to meet this requirement. */
        extrabytes = (4 - width % 4) % 4;   

	// This is the size of the padded bitmap
	bytesize = (width + extrabytes) * height; 

	// Fill the bitmap file header structure
	bmpFileHeader.bfType            = 'MB';   // Bitmap header
	bmpFileHeader.bfSize            = 0;      // This can be 0 for BI_RGB bitmaps
	bmpFileHeader.bfReserved1       = 0;
	bmpFileHeader.bfReserved2       = 0;
	bmpFileHeader.bfOffBits         = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * numPaletteEntries;

	// Fill the bitmap info structure
	bmpInfoHeader.biSize            = sizeof(BITMAPINFOHEADER);
	bmpInfoHeader.biWidth           = width;
	bmpInfoHeader.biHeight          = height;
	bmpInfoHeader.biPlanes          = 1;
	bmpInfoHeader.biBitCount        = 8;            // 8 - bit bitmap
	bmpInfoHeader.biCompression     = BI_RGB;
	bmpInfoHeader.biSizeImage       = bytesize;     // includes padding for 4 byte alignment
	bmpInfoHeader.biXPelsPerMeter   = 0;
	bmpInfoHeader.biYPelsPerMeter   = 0;
	bmpInfoHeader.biClrUsed         = numPaletteEntries;
	bmpInfoHeader.biClrImportant    = 0;

	// Open file
	fopen_s(&filep,filename,"wb");
	//if ((filep = fopen(filename, "wb")) == NULL) {
	if (filep  == NULL) {
		printf("Error opening file %s\n", filename);
		return FALSE;
	}

	// Write bmp file header
	if (fwrite(&bmpFileHeader, 1, sizeof(BITMAPFILEHEADER), filep) < sizeof(BITMAPFILEHEADER)) {
		printf("Error writing bitmap file header\n");
		fclose(filep);
		return FALSE;
	}


	// Write bmp info header
	if (fwrite(&bmpInfoHeader, 1, sizeof(BITMAPINFOHEADER), filep) < sizeof(BITMAPINFOHEADER)) {
		printf("Error writing bitmap info header\n");
		fclose(filep);
		return FALSE;
	}


	// Write bmp palette
	if (fwrite(palette, 1, numPaletteEntries * sizeof(RGBQUAD), filep) < numPaletteEntries * sizeof(RGBQUAD)) {
		printf("Error writing bitmap palette\n");
		fclose(filep);
		return FALSE;
	}


	// Allocate memory for some temporary storage
	paddedImage = (unsigned char *)calloc(sizeof(unsigned char), bytesize);
	if (paddedImage == NULL) {
		printf("Error allocating memory \n");
		fclose(filep);
		return FALSE;
	}


	/* Flip image - bmp format is upside down.  Also pad the paddedImage array so that the number
	   of pixels is aligned on a 4 byte boundary. */
	for (row = 0; row < height; row++){
		for(col=0;col<width;col++){
			*(paddedImage+(row * (width + extrabytes))+col)=image[row][col];
		}
	}


	//Write bmp data
	if (fwrite(paddedImage, 1, bytesize, filep) < bytesize) {
		printf("Error writing bitmap data\n");
		free(paddedImage);
		fclose(filep);
		return FALSE;
	}
	// Close file
	fclose(filep);
	free(paddedImage);
	return (TRUE);
}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::SetBitmap8(int row, int col, unsigned char **img)
{
		unsigned int *img32;
		img32		=new unsigned int [row*col];
		m_width		=col;
		m_height		=row;

		for(int i=0;i<row;i++)
			for(int j=0;j<col;j++){
				*(img32+i*col+j)= RGB(img[i][j],img[i][j],img[i][j]);
			}
		
		InitBitmap32(row,col);
		SetBitmap32(row,col,img32);
		Invalidate();
		delete [] img32;
		memDC.Ellipse(col-10,row-10,col+10, row+10);
}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::SetGreyLevelScalePaper(void)
{
		unsigned int tmpPixel;
		for(int i=0;i<this->m_height;i++){
			tmpPixel = (unsigned int)(255.0 *i/m_height);
			for(int j=0;j< this->m_width;j++){
				*(m_pDCbits+i*m_width+j)= RGB(tmpPixel, tmpPixel, tmpPixel);
			}
		}
}

//////////////////////////////////////////////////////////////////////////////
BOOL CBitmapCtrl::GetBitmap8(unsigned char **img)
{
	if(m_pDCbits!=NULL && img!=NULL){
		for(int i=0;i<m_height;i++){
			for(int j=0;j<m_width;j++){
				img[i][j]=(*(m_pDCbits+m_width*i+j)& 0x000000FF);
			}
		}
		return (TRUE);
	}else
		return (FALSE);
}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::AdaptiveThreshold()
{
	int totalPixels;

	if(m_pDCbits != NULL){
		//-------------------
		int x, y, yOffset;
		unsigned int sum;
		FillMemory(pIntegral, m_height * m_width, 0);
		BYTE mean, byte;
		static const BYTE C=1;		// it is better 1 in some cases
		static const int RECT=3;	// TODO dynamically adapt value from 3 to 8 based on image size
		static const unsigned int count = (RECT*2+1)*(RECT*2+1);
		//-------------------
		//Bin m_strongestLine;
		//-------------------
		LPPIX32 buf = (LPPIX32)m_pDCbits;
		
		totalPixels = m_width * m_height;

		//-------------------
		// fill integral image. First step is to copy orig.image -> integral image
		for (x=totalPixels-1; x>=0; x--){
			pIntegral[x] = (((buf[x].red   * 77)   + 
							 (buf[x].green * 150) + 
							 (buf[x].blue  * 28)) >> 8) & 255;		// 100% Intesity = 30% Red + 59% Green + 11% Blue
		}	
		SumIntegralImage(pIntegral);
	
		//-------------------
		// "Mean Adaptive Thresholding" method
		for (y=RECT+1; y<m_height-RECT; y++){
			yOffset = y * m_width;
			for (x=RECT+1; x<m_width-RECT; x++){
				// sum the surounding pixels RECTxRECT around x,y
				sum  = pIntegral[(y+RECT)*m_width+x+RECT] - pIntegral[(y-RECT-1)*m_width+x+RECT] - pIntegral[(y+RECT)*m_width+x-RECT-1] + pIntegral[(y-RECT-1)*m_width+x-RECT-1];
				mean = (BYTE)(sum/count);		// calculate mean value of RECTxRECT neighbours pixels
				
				byte = (BYTE)((((buf[yOffset + x].red     *  77)  + 
									         (buf[yOffset + x].green * 150) + 
									         (buf[yOffset + x].blue   *   28)) >> 8) & 255);		// 100% Intesity = 30% Red + 59% Green + 11% Blue

				if (byte < (mean-C)){		// original pixel intensity above or below the local threshold?
					m_pMono[yOffset + x] = 0;		// black
				}else{
					m_pMono[yOffset + x] = 255;	// white
				}
			}
		}

		for (y=0; y<m_height; y++){
			yOffset = y * m_width;
			for (x=0; x<m_width; x++){
				if (m_pMono[yOffset + x] == 0){
					buf[yOffset +x].INT32   = 0;
				}else{
					buf[yOffset +x].red		= 255;
					buf[yOffset +x].green	= 255;
					buf[yOffset +x].blue	= 255;
				}	
			}	
		}	
	}
}

//////////////////////////////////////////////////////////////////////////////
BOOL CBitmapCtrl::SetBitmap8Pixel(int row, int col, unsigned char value)
{
	if(m_pDCbits!=NULL && (row >-1)&& (col >-1)&& (row <m_pBIH->biHeight)&& (col <m_pBIH->biWidth)){
		*(m_pDCbits+m_width*(m_pBIH->biHeight-row)+col)=value& 0x000000FF;
		return (TRUE);
	}else
		return (FALSE);
}

//////////////////////////////////////////////////////////////////////////////
BOOL CBitmapCtrl::GetBitmap8Pixel(int row, int col, unsigned char *value, PIX32 *pix32)
{
	if(m_pDCbits!=NULL && (row >-1)&& (col >-1)&& (row <m_pBIH->biHeight)&& (col <m_pBIH->biWidth)){
		
		PIX32 pix;
		LPPIX32 buf = (LPPIX32)m_pDCbits;

		pix = buf[m_width*(m_pBIH->biHeight-row)+col];
		*pix32 = pix;
		*value = pix.grey;
		return (TRUE);
	}else
		*value =0;
		return (FALSE);
}

//////////////////////////////////////////////////////////////////////////////
BOOL CBitmapCtrl::GetBitmap8_Vedio_Pixel(int row, int col, unsigned char *value)
{
	if(m_pImageBytes!=NULL && (row >-1)&& (col >-1)&& (row <m_height)&& (col <m_width)){
		*value =*(m_pImageBytes+m_width*(m_height-row)+col)& 0x000000FF;

		return (TRUE);
	}else
		*value =0;
		return (FALSE);
}

//////////////////////////////////////////////////////////////////////////////
BOOL CBitmapCtrl::GetBitmapSize(int *row, int *col)
{
	if(m_pBIH != NULL && m_pBIH->biWidth >0 && m_pBIH->biHeight>0 ){
		*row=m_pBIH->biHeight;
		*col=m_pBIH->biWidth;
		return (TRUE);
	}else{
		return (FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////////
BOOL CBitmapCtrl::SetPixelColor(int row, int col, unsigned int color)
{
	// Only Draw the color on the specified DC pixel
	if((row>0 && col>0) && (row<=m_height && col <=m_width)){
		
		if(memDC.GetSafeHdc() != NULL){
			/*memDC.SetPixel(col,m_height-(row+1),color );
			CPaintDC dc(this);
			dc.BitBlt(0,0,rcClient.Width(),rcClient.Height(),&memDC,0,0,SRCCOPY);*/
			CPaintDC dc(this);
			dc.SetPixel(col,m_height-(row+1),color );
		}
		Invalidate();
		return (TRUE);
	}else{
		return (FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::MakeBMPHeader(void)
{
	DWORD  dwBitmapInfoSize;
	
	dwBitmapInfoSize = sizeof(BITMAPINFO);
	m_pBmp			 = (BITMAPINFO *)new BYTE [dwBitmapInfoSize];

	m_pBmp->bmiHeader.biSize				= sizeof(BITMAPINFOHEADER);
    m_pBmp->bmiHeader.biWidth			= m_un32ImageWidth;
    m_pBmp->bmiHeader.biHeight			= m_un32ImageHeight*-1;	
    m_pBmp->bmiHeader.biPlanes			= 1;
    m_pBmp->bmiHeader.biBitCount			    =(unsigned short) m_un32ImageBits;
    m_pBmp->bmiHeader.biCompression	    = BI_RGB;
    m_pBmp->bmiHeader.biSizeImage		    = 0;
    m_pBmp->bmiHeader.biXPelsPerMeter	= 0;
    m_pBmp->bmiHeader.biYPelsPerMeter	= 0;
    m_pBmp->bmiHeader.biClrUsed			    = 0;
    m_pBmp->bmiHeader.biClrImportant	    = 0;
}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::Init_Video_MemDC(void)
{
	//reset offsets
	m_x_offset = 0;
	m_y_offset = 0;

	//reset scrollbar
	SCROLLINFO		si;
	si.fMask			= SIF_PAGE | SIF_RANGE;
	si.nMin			= 0;
	si.nMax			= 0;
	si.nPage			= 0;
	SetScrollInfo(SB_HORZ, &si, TRUE);
	SetScrollInfo(SB_VERT, &si, TRUE);
	
	//redraw background
	GetClientRect(&rcClient);
	memDC.FillSolidRect(&rcClient,RGB(128,128,128));

	if (hBitmap == 0){
		memDC.TextOut(0,0,"File not found.");
		Invalidate();
		return;
	}
	//get image size
	CPoint point(m_x_offset, m_y_offset);

	image_size.cx = m_un32ImageWidth;
	image_size.cy = m_un32ImageHeight;
	
	//check image size
	BOOL x_fit;
	BOOL y_fit;
	x_fit = (m_un32ImageWidth <= rcClient.Width());
	if (!x_fit)
		rcClient.bottom -= scrollbar_width;
	y_fit = (m_un32ImageHeight <= rcClient.Height());
	
	if (!y_fit){
		rcClient.right -= scrollbar_width;
		x_fit = (m_un32ImageWidth <= rcClient.Width());
	}
	
	if (!x_fit){
		ShowScrollBar(SB_HORZ);
		// update scrollbar
		SCROLLINFO si;
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nMin  = 0;
		si.nMax  = m_un32ImageWidth-1;
		si.nPage = rcClient.Width();
		SetScrollInfo(SB_HORZ, &si, TRUE);
	}
	
	if (!y_fit){
		// show scrollbar
		ShowScrollBar(SB_VERT);
		// update scrollbar
		SCROLLINFO si;
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nMin  = 0;
		si.nMax  = m_un32ImageHeight-1;
		si.nPage = rcClient.Height();
		SetScrollInfo(SB_VERT, &si, TRUE);
	}
	
	m_width	  =m_un32ImageWidth;
	m_height =m_un32ImageHeight;

	m_pBmp->bmiHeader.biWidth		= m_width;
	m_pBmp->bmiHeader.biHeight		= m_height -1;	


	//-------------------------
	if(m_pMono != NULL){
		delete [] m_pMono;
	}
	m_pMono = new BYTE[m_height * m_width];
	FillMemory(m_pMono, m_height * m_width, 255);

	if(m_gauss){
		delete m_gauss;
	}


	///////////////////////////////////////////////////////////////////
	float sigma;
	int   low,high;
	sigma		= 1;
	low		= 20;
	high		= 30;	
	m_gauss = new CDOG(m_height, m_width, sigma, low, high);
	///////////////////////////////////////////////////////////////////

	//-------------------------
	if(pIntegral != NULL){
		delete [] pIntegral;
	}
	pIntegral = new unsigned int[m_height * m_width]; 	// integral image (sums of the original image)
	FillMemory(pIntegral, m_height * m_width, 0);
	//-------------------------

	//ZeroMemory(  m_grid, sizeof(POINT)*10*10);

	m_InProcessing = false;
	m_display.white					= false;
	m_display.black					= false;
	m_display.grid						= true;		
	m_display.rotation				= false;		
	m_display.finalSolution		= true;
	m_display.lineDetect			= false;
	m_display.OCRresult			= false;
	m_display.smallFont			= false;
	m_display.tempSolution		= false;
	m_display.whiteArea			= false;
	m_display.whiteCandidates	= false;

	//-------------------------

//	memcpy(m_pBMPbits,bmpInfo.bmBits, bmpInfo.bmWidthBytes*bmpInfo.bmHeight);
}

//////////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::ShowImage(BYTE *pImageByte)
{
	if(pImageByte!=NULL){
		m_pImageBytes=pImageByte;
		if(m_pImageBytes!=NULL){
			
		
			m_width  = m_pBIH->biWidth  = m_un32ImageWidth;
			m_height = m_pBIH->biHeight = m_un32ImageHeight;
						      
		
		
			int aaa=StretchDIBits(	memDC,
											-m_x_offset, 
											-m_y_offset,
											m_width, 
											m_height,
											0, 
											0,
											m_pBIH->biWidth, 
											m_pBIH->biHeight,
											m_pImageBytes,
											m_pBmp,
											BI_RGB, 
											SRCCOPY   );		

			//	LocalFree((HLOCAL)m_pImageBytes);
	
			this->Invalidate (FALSE);
		}
	}
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Function:	SumIntegralImage()
// Purpose:		Builds the integral image.
// Parameters:	pIntegral - pointer to the integral image to build. Initially must be filled with the copy of original image. 
/////////////////////////////////////////////////////////////////////////
void CBitmapCtrl::SumIntegralImage(unsigned int* pIntegral)
{
	int x, y;
	unsigned int *pIntegralA, *pIntegralB, *pIntegralC, *pIntegralD;	// use pointers for speed

	// sum the first column
	pIntegralA = pIntegral;
	pIntegralB = pIntegral + m_width;
	for (y=1; y<m_height; y++){
		*pIntegralB += *pIntegralA;
		pIntegralA += m_width;
		pIntegralB += m_width;
	}

	// sum the first row
	pIntegralA = pIntegral;
	pIntegralB = pIntegral + 1;
	for (x=1; x<m_width; x++){
		*pIntegralB += *pIntegralA;
		pIntegralA++;
		pIntegralB++;
	}

	// sum all other pixels
	for (y=1; y<m_height; y++){
		pIntegralA = pIntegral + y*m_width + 1;
		pIntegralB = pIntegral + (y-1)*m_width + 1;
		pIntegralC = pIntegral + y*m_width;
		pIntegralD = pIntegral + (y-1)*m_width;
		for (x=1; x<m_width; x++){
			*pIntegralA += *pIntegralB + *pIntegralC - *pIntegralD;
			pIntegralA++;
			pIntegralB++;
			pIntegralC++;
			pIntegralD++;
		}	
	}
}
