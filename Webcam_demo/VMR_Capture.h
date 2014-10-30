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

// VMR_Capture.h: interface for the CVMR_Capture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VMR_CAPTURE_H__186091F3_30FA_4FAA_AC8B_EF25E8463B9A__INCLUDED_)
#define      AFX_VMR_CAPTURE_H__186091F3_30FA_4FAA_AC8B_EF25E8463B9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <atlbase.h>

//#include <uuids.h>


// strmiids.lib

//#define WM_GRAPHNOTIFY  WM_USER+13
enum PLAYSTATE {Stopped, Paused, Running, Init};


class CVMR_Capture  
{
//friend CBitmapCtrlDemoDlg;

public:

	CVMR_Capture();
	virtual ~CVMR_Capture();	
	
	int		EnumDevices(HWND hList);	
	HRESULT Init(int iDeviceID, HWND hWnd, int *iWidth, int *iHeight, HWND hParentWnd);
	//DWORD	GetFrame(BYTE ** pFrame);
	DWORD	GetFrame32(BYTE **p32Frame);
	BOOL	Pause();
	DWORD	ImageCapture(LPCTSTR szFile);
	DWORD	GrabFrame();

	int   GetWidth(){return m_nWidth;};
	int   GetHeight(){return m_nHeight;};
	PLAYSTATE	GetCurrState(){ return m_psCurrent;};

	
	IMediaControl*	GetMediaControl(){ return	m_pMediaControl;};


protected:
	
	IGraphBuilder			*m_pGraphBuilder;
	IMediaControl			*m_pMediaControl;
	IMediaEventEx			*m_pMediaEventEx;
	
	IVMRWindowlessControl9	*m_pVMRWindowlessControl;
	IPin						    *m_pCamOutPin;
	IBaseFilter				*m_pBaseFilter;

	PLAYSTATE			m_psCurrent;

	int						    m_nWidth;
	int						    m_nHeight;

	BYTE						*m_p32Frame;
	long						m_n32Framelen;

	bool	BindFilter(int deviceId, IBaseFilter **pFilter);
	HRESULT InitializeWindowlessVMR(HWND hWnd);
	HRESULT InitVideoWindow(HWND hWnd,int width, int height);
	void	StopCapture();
	void	CloseInterfaces(void);
	
	void	DeleteMediaType(AM_MEDIA_TYPE *pmt);
	bool	Convert24Image(BYTE *p32Img,BYTE *p24Img,DWORD dwSize32);
	

private:
	
};

#endif // !defined(AFX_VMR_CAPTURE_H__186091F3_30FA_4FAA_AC8B_EF25E8463B9A__INCLUDED_)
