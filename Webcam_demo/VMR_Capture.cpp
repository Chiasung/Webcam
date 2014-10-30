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
//  Chiasung Liu modified this source code and export the image raw data from webcam.
//  The image raw data can be used for testing image processing algorithms.
//////////////////////////////////////////////////////////////////////
//
// VMR_Capture.cpp: implementation of the CVMR_Capture class.
//
//////////////////////////////////////////////////////////////////////

///
///   strmiids.lib  //  CSL -- removed from the included lib
///


#include "stdafx.h"
#include "VMR_Capture.h"

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVMR_Capture::CVMR_Capture()
{
	CoInitialize(NULL);
	m_pGraphBuilder			= NULL;
	m_pMediaControl			= NULL;
	m_pMediaEventEx			= NULL;			
	m_pVMRWindowlessControl	= NULL;		
	m_pBaseFilter				= NULL;
	m_pCamOutPin				= NULL;
	//m_pFrame						= NULL;
	//m_nFramelen					= 0;

	m_p32Frame					= NULL;
	m_n32Framelen				= 0;
	m_psCurrent					= Stopped;
	
}

CVMR_Capture::~CVMR_Capture()
{  
	if(m_p32Frame !=NULL){
		delete []m_p32Frame;
		m_p32Frame=NULL;
	}
	CloseInterfaces();
	CoUninitialize ();


}


HRESULT CVMR_Capture::Init(int iDeviceID, HWND hWnd, int *iWidth, int *iHeight, HWND hParentWnd)
{
	HRESULT hr;

	m_n32Framelen	= 0;
	m_p32Frame		= NULL;
	
	// Get the interface for DirectShow's GraphBuilder
   hr=CoCreateInstance(CLSID_FilterGraph, 
							  NULL, 
							  CLSCTX_INPROC_SERVER, 
                       IID_IGraphBuilder, 
							  (void **)&m_pGraphBuilder);

   if(SUCCEEDED(hr)){
      //(1) Create the Video Mixing Renderer and add it to the graph
		//*******************************
      //InitializeWindowlessVMR(hWnd);  // copy the function to below      
		//*******************************
   
		IBaseFilter* pVmr = NULL;

		// Create the VMR and add it to the filter graph.
		hr = CoCreateInstance(CLSID_VideoMixingRenderer, 
		 							 NULL,
				                CLSCTX_INPROC, 
									 IID_IBaseFilter, 
									 (void**)&pVmr);

		if (SUCCEEDED(hr)) {
   
			//-- (1-1) -- 
			//=============================
			hr = m_pGraphBuilder->AddFilter(pVmr, L"Video Mixing Renderer");
			//=============================
			if(SUCCEEDED(hr)){
				// Set the rendering mode and number of streams.  
				IVMRFilterConfig* pConfig;

				//-- (1-2) --
				hr = pVmr->QueryInterface(IID_IVMRFilterConfig, (void**)&pConfig);
				if( SUCCEEDED(hr)){
					pConfig->SetRenderingMode(VMRMode_Windowless);
					pConfig->Release();
				}

				//-- (1-3) --
				hr = pVmr->QueryInterface(IID_IVMRWindowlessControl, (void**)&m_pVMRWindowlessControl);
				if( SUCCEEDED(hr)){
					m_pVMRWindowlessControl->SetVideoClippingWindow(hWnd); //<<
				}

				//-- (1-4) --
				/*
				hr = pVmr->QueryInterface(IID_IMediaEventEx, (void **)&m_pMediaEventEx);     

				//==================
				// Have the graph signal event via window callbacks for performance
				if (SUCCEEDED(hr)){
					hr = m_pMediaEventEx->SetNotifyWindow((OAHWND)hParentWnd, WM_FILTER_GRAPHY_NOTIFY, 0);
					if (SUCCEEDED(hr)){
						char eventMsg[500];
						sprintf_s(eventMsg,"set event ok");
						MessageBox (NULL, eventMsg,"init",MB_OK);
						m_pMediaEventEx->Release();
					}
				}
				*/


			}
			pVmr->Release();
		}


		//------------------------------
		// (2) AVI decompressor filter
		//------------------------------
		IBaseFilter* pAVI_Decompressor = NULL;

		// Create the AVI decompressor and add it to the filter graph.
		hr = CoCreateInstance(CLSID_AVIDec, 
		 							 NULL,
				                CLSCTX_INPROC, 
									 IID_IBaseFilter, 
									 (void**)&pAVI_Decompressor);

		if (SUCCEEDED(hr)) {
   		//-- (2-1) -- 
			//=============================
			hr = m_pGraphBuilder->AddFilter(pAVI_Decompressor, L"AVI Decompressor");
			//=============================
			if(SUCCEEDED(hr)){

			}
			pAVI_Decompressor->Release();
		}



		//------------------------------------------------------------------
		// Bind Device Filter.  We know the device because the id was passed in
		//------------------------------------------------------------------
		//if(!BindFilter(iDeviceID, &m_pDF)){
			if(iDeviceID < 0){
				return S_FALSE;
			}

			// enumerate all video capture devices
			CComPtr<ICreateDevEnum> pCreateDevEnum;
			hr = CoCreateInstance(CLSID_SystemDeviceEnum, 
										 NULL, 
										 CLSCTX_INPROC_SERVER,
										 IID_ICreateDevEnum, 
										 (void**)&pCreateDevEnum);
			if(hr != NOERROR){
				return S_FALSE;
			}

			//************************
			CComPtr<IEnumMoniker> pEm;
			//************************
			hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);

			if(hr != NOERROR){
				return S_FALSE;
			}

			pEm->Reset();
			ULONG cFetched;
			IMoniker *pM;
			int index = 0;
			while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK, index <= iDeviceID){

				IPropertyBag *pBag;
				hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);

				if(SUCCEEDED(hr)) {
					VARIANT var;
					var.vt = VT_BSTR;
					hr = pBag->Read(L"FriendlyName", &var, NULL);
					if (hr == NOERROR){
						if (index == iDeviceID){
							//*******************************************************
							pM->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pBaseFilter);
							//*******************************************************
						}
						SysFreeString(var.bstrVal);
					}
					pBag->Release();
				}
				pM->Release();
				index++;
			}
			//return true;
			//return S_FALSE;
		//}
		//------------------------------------------------------------------

		//===========================
		hr=m_pGraphBuilder->AddFilter(m_pBaseFilter, L"Video Capture");
		//===========================
		if (FAILED(hr))
			return hr;

		CComPtr<IEnumPins> pEnum;

		m_pBaseFilter->EnumPins(&pEnum);

		hr = pEnum->Reset();
		hr = pEnum->Next(1, &m_pCamOutPin, NULL); 
		
      // QueryInterface for DirectShow interfaces
		//==================
      hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&m_pMediaControl);
		hr = m_pGraphBuilder->QueryInterface(IID_IMediaEventEx, (void **)&m_pMediaEventEx);     


		//==================

		// Have the graph signal event via window callbacks for performance
		if(SUCCEEDED(hr)){
	      hr = m_pMediaEventEx->SetNotifyWindow((OAHWND)hParentWnd, WM_FILTER_GRAPHY_NOTIFY, 0);
			if(SUCCEEDED(hr)){
				//char eventMsg[500];
				//sprintf_s(eventMsg,"set event ok");
				//MessageBox (NULL, eventMsg,"init",MB_OK);
				//m_pMediaEventEx->Release();
			}
		}
		RECT		rcDest;

		CComPtr<IAMStreamConfig> pConfig;
		IEnumMediaTypes			*pMedia;
		AM_MEDIA_TYPE				*pmt  = NULL;
		AM_MEDIA_TYPE				*pfnt = NULL;

		hr = m_pCamOutPin->EnumMediaTypes( &pMedia );
		if(SUCCEEDED(hr)){

			while(pMedia->Next(1, &pmt, 0) == S_OK){

				if((pmt->formattype == FORMAT_VideoInfo)  &&
					(pmt->majortype  == MEDIATYPE_Video)  ){

					VIDEOINFOHEADER *vih = (VIDEOINFOHEADER *)pmt->pbFormat;

					*iWidth				= vih->bmiHeader.biWidth;
					*iHeight				= vih->bmiHeader.biHeight;

					if( vih->bmiHeader.biWidth == *iWidth && vih->bmiHeader.biHeight == *iHeight ){

					  //************
						 pfnt = pmt;
					  //************
						vih->rcSource.right	= vih->bmiHeader.biWidth;
						vih->rcSource.bottom = vih->bmiHeader.biHeight;
						vih->rcTarget.right	= vih->bmiHeader.biWidth;
						vih->rcTarget.bottom = vih->bmiHeader.biHeight;
						break;
					}
					DeleteMediaType( pmt );
				}
			}
			pMedia->Release();
		}

		///AMDDS_RGB
	   //	AMDDS_RGB

		hr = m_pCamOutPin->QueryInterface( IID_IAMStreamConfig, (void **) &pConfig );
		if(SUCCEEDED(hr)){
			hr = pConfig->GetFormat( &pfnt );
			if(SUCCEEDED(hr)){

				m_nWidth  = ((VIDEOINFOHEADER *)pfnt->pbFormat)->bmiHeader.biWidth;
				m_nHeight = ((VIDEOINFOHEADER *)pfnt->pbFormat)->bmiHeader.biHeight;

				DeleteMediaType( pfnt );
			}
		}
		::GetClientRect (hWnd,&rcDest);

		if(m_p32Frame){
			delete [] m_p32Frame;
		}

		m_n32Framelen	=(*iWidth)*(*iHeight)*4;
		m_p32Frame		=(BYTE*) new BYTE[m_n32Framelen];		

		// Run the graph to play the media file
		
		m_psCurrent=Stopped;
			  //==============     
		hr = m_pGraphBuilder->Render( m_pCamOutPin);
		     //==============
		hr = m_pMediaControl->Run();
		m_psCurrent	= Running;
	}
	
	return hr;
}


//-------------------------------------------------------------------------------
HRESULT CVMR_Capture::InitializeWindowlessVMR(HWND hWnd)
{
   IBaseFilter* pVmr = NULL;

   // Create the VMR and add it to the filter graph.
   HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer, 
											NULL,
                                 CLSCTX_INPROC, 
											IID_IBaseFilter, 
											(void**)&pVmr);
   if (SUCCEEDED(hr)) {
    
      hr = m_pGraphBuilder->AddFilter(pVmr, L"Video Mixing Renderer");
      if(SUCCEEDED(hr)){
         // Set the rendering mode and number of streams.  
         IVMRFilterConfig* pConfig;

         hr = pVmr->QueryInterface(IID_IVMRFilterConfig, (void**)&pConfig);
         if( SUCCEEDED(hr)){
            pConfig->SetRenderingMode(VMRMode_Windowless);
            pConfig->Release();
         }

         hr = pVmr->QueryInterface(IID_IVMRWindowlessControl, (void**)&m_pVMRWindowlessControl);
         if( SUCCEEDED(hr)){
            m_pVMRWindowlessControl->SetVideoClippingWindow(hWnd);
         }
      }
      pVmr->Release();
   }
   return hr;
}

bool CVMR_Capture::BindFilter(int deviceId, IBaseFilter **pFilter)
{
	if(deviceId < 0){
		return false;
	}
	
   // enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
   HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, 
											NULL, 
											CLSCTX_INPROC_SERVER,
											IID_ICreateDevEnum, 
											(void**)&pCreateDevEnum);
   if(hr != NOERROR){
		return false;
	}

   CComPtr<IEnumMoniker> pEm;
   hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
    
   if(hr != NOERROR){
		return false;
   }

   pEm->Reset();
   ULONG cFetched;
   IMoniker *pM;
	int index = 0;
   while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK, index <= deviceId){
    
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		
		if(SUCCEEDED(hr)) {
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR){
				if (index == deviceId){
					pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
				}
				SysFreeString(var.bstrVal);
			}
			pBag->Release();
		}
		pM->Release();
		index++;
   }
	return true;
}

HRESULT CVMR_Capture::InitVideoWindow(HWND hWnd,int width, int height)
{

	// Set the grabbing size
	// First we iterate through the available media types and 
	// store the first one that fits the requested size.
	// If we have found one, we set it.
	// In any case we query the size of the current media type
	// to have this information for clients of this class.
	//     Gerhard Reitmayr <reitmayr@i ...............>

	HRESULT	hr;
	RECT		rcDest;

	CComPtr<IAMStreamConfig> pConfig;
	IEnumMediaTypes			*pMedia;
	AM_MEDIA_TYPE				*pmt  = NULL;
	AM_MEDIA_TYPE				*pfnt = NULL;

	hr = m_pCamOutPin->EnumMediaTypes( &pMedia );
	if(SUCCEEDED(hr)){

		while(pMedia->Next(1, &pmt, 0) == S_OK){

			if( pmt->formattype == FORMAT_VideoInfo ){

				/*--------------------------
				typedef struct tagVIDEOINFOHEADER {
					RECT            rcSource;          // The bit we really want to use
					RECT            rcTarget;          // Where the video should go
					DWORD           dwBitRate;         // Approximate bit data rate
					DWORD           dwBitErrorRate;    // Bit error rate for this stream
					REFERENCE_TIME  AvgTimePerFrame;   // Average time per frame (100ns units)

				   BITMAPINFOHEADER bmiHeader;

				} VIDEOINFOHEADER;
				-----------------------------*/

				VIDEOINFOHEADER *vih = (VIDEOINFOHEADER *)pmt->pbFormat;

				// printf("Size %i  %i\n", vih->bmiHeader.biWidth, vih->bmiHeader.biHeight );
				if( vih->bmiHeader.biWidth == width && vih->bmiHeader.biHeight == height ){

				  //************
					 pfnt = pmt;
				  //************

					char test[500];

					vih->rcSource.right	= vih->bmiHeader.biWidth;
					vih->rcSource.bottom = vih->bmiHeader.biHeight;
					vih->rcTarget.right	= vih->bmiHeader.biWidth;
					vih->rcTarget.bottom = vih->bmiHeader.biHeight;


					
					sprintf_s(test,"bmp Width=%d \n  bmp Height=%d \n Source RECT(%d,%d,%d,%d) \n Target RECT(%d,%d,%d,%d) \n bitRate: %d \n  %d time(0.1us)/frame \n biSize=%d, biBitCount=%d, \n biCompression=%d, biSizeImage=%d",
						             //biSize=%d, biBitCount=%d, biCompression=%d, biSizeImage=%d",
											vih->bmiHeader.biWidth, vih->bmiHeader.biHeight,
											vih->rcSource.left, vih->rcSource.top,	vih->rcSource.right,	vih->rcSource.bottom,
											vih->rcTarget.left, vih->rcTarget.top, vih->rcTarget.right, vih->rcTarget.bottom,
											vih->dwBitRate,
											vih->AvgTimePerFrame,
											vih->bmiHeader.biSize,
											vih->bmiHeader.biBitCount,
											vih->bmiHeader.biCompression,
											vih->bmiHeader.biSizeImage
								);
               
					MessageBox (NULL, test,"test",MB_OK);

					break;
				}
				DeleteMediaType( pmt );
			}                        
		}
		pMedia->Release();
	}


   ///AMDDS_RGB
   //	AMDDS_RGB

	hr = m_pCamOutPin->QueryInterface( IID_IAMStreamConfig, (void **) &pConfig );
	if(SUCCEEDED(hr)){

		if( pfnt != NULL ){
			// 2012/07/13 -- CSL -----------------
			pfnt->majortype = MEDIATYPE_Video;
			pfnt->subtype   = MEDIASUBTYPE_RGB24;
			//------------------------------------
			hr=pConfig->SetFormat( pfnt );

			if(SUCCEEDED(hr)){        
				MessageBox(NULL, "SetFormat OK","test",MB_OK);
			}else{
				MessageBox(NULL, "SetFormat Fail","test",MB_OK);
			}

			DeleteMediaType( pfnt );
		}
		hr = pConfig->GetFormat( &pfnt );
		if(SUCCEEDED(hr)){

			m_nWidth  = ((VIDEOINFOHEADER *)pfnt->pbFormat)->bmiHeader.biWidth;
			m_nHeight = ((VIDEOINFOHEADER *)pfnt->pbFormat)->bmiHeader.biHeight;

			DeleteMediaType( pfnt );
		}
	}
	::GetClientRect (hWnd,&rcDest);
	//**************************************************
	// hr = m_pWC->SetVideoPosition(NULL, &rcDest);
	//**************************************************    
	return hr;
}

void CVMR_Capture::StopCapture()
{
   HRESULT hr;

	if((m_psCurrent == Paused) || (m_psCurrent == Running)){
      LONGLONG pos = 0;
      hr				= m_pMediaControl->Stop();
      m_psCurrent	= Stopped;		
      // Display the first frame to indicate the reset condition
		//***************************************************************        
      // hr = m_pMC->Pause();
		//***************************************************************
   }
}

void CVMR_Capture::CloseInterfaces(void)
{
    HRESULT hr;

    
	// Stop media playback
    if(m_pMediaControl)
        hr = m_pMediaControl->Stop();	    
    m_psCurrent = Stopped;  

	// Disable event callbacks
    if (m_pMediaEventEx)
        hr = m_pMediaEventEx->SetNotifyWindow((OAHWND)NULL, 0, 0);

	SAFE_RELEASE(m_pMediaEventEx);	



    // Release and zero DirectShow interfaces
	if(m_pCamOutPin)
		m_pCamOutPin->Disconnect ();


	SAFE_RELEASE(m_pCamOutPin);        
   SAFE_RELEASE(m_pMediaControl);    
   SAFE_RELEASE(m_pGraphBuilder);    
   SAFE_RELEASE(m_pVMRWindowlessControl);	
	SAFE_RELEASE(m_pBaseFilter);
	
	
//delete allocated memory 
	if(m_p32Frame!=NULL)
		delete []m_p32Frame;

}

//Capture RAW IMAGE BITS 24bits/pixel
DWORD CVMR_Capture::ImageCapture(LPCTSTR szFile)
{
	BYTE *pImage;
	DWORD dwSize,dwWritten;
	dwSize=this->GrabFrame ();
	this->GetFrame32 (&pImage);
				
	HANDLE hFile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_READ, NULL,
					  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE)
	   return FALSE;

	WriteFile(hFile, (LPCVOID)pImage , /*m_nFramelen*/ m_n32Framelen, &dwWritten, 0);
	// Close the file
	CloseHandle(hFile);

	return dwWritten;
}

void CVMR_Capture::DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
    // allow NULL pointers for coding simplicity

    if (pmt == NULL) {
        return;
    }

    if (pmt->cbFormat != 0) {
        CoTaskMemFree((PVOID)pmt->pbFormat);

        // Strictly unnecessary but tidier
        pmt->cbFormat = 0;
        pmt->pbFormat = NULL;
    }
    if (pmt->pUnk != NULL) {
        pmt->pUnk->Release();
        pmt->pUnk = NULL;
    }

    CoTaskMemFree((PVOID)pmt);
}


DWORD CVMR_Capture::GrabFrame()
{
	long lOut=-1;

	if(m_pVMRWindowlessControl ){
		BYTE* lpCurrImage = NULL;

		// Read the current video frame into a byte buffer.  The information
		// will be returned in a packed Windows DIB and will be allocated
		// by the VMR.
		if(m_pVMRWindowlessControl->GetCurrentImage(&lpCurrImage) == S_OK){

			LPBITMAPINFOHEADER  pdib = (LPBITMAPINFOHEADER) lpCurrImage;

			//if(/*m_pFrame==NULL ||*/ (pdib->biHeight * pdib->biWidth * 3) !=m_n32Framelen ){
				/*
				if(m_pFrame!=NULL){
					delete []m_pFrame;
				}
				*/
				//if(m_p32Frame !=NULL){
				//	delete []m_p32Frame;
				//}

				/*
				//-----------------------------------------------
				// Frame 24 bit
				m_nFramelen	=pdib->biHeight * pdib->biWidth * 3;
				m_pFrame		=new BYTE [m_nFramelen] ;
				//-----------------------------------------------
				*/

				//-----------------------------------------------
				// Frame 32 bit
				//m_n32Framelen	=pdib->biHeight * pdib->biWidth * 4;
				//m_p32Frame		=new BYTE [m_n32Framelen] ;
				//-----------------------------------------------

				//m_nHeight = pdib->biHeight;
				//m_nWidth  = pdib->biWidth ;

			//}			

			if(pdib->biBitCount ==32){
				//-------------
				// 32 bit DIB
				//-------------
				//DWORD  dwSize=0, dwWritten=0;			

				BYTE *p32BitFrameInDIB;
				p32BitFrameInDIB=lpCurrImage + sizeof(BITMAPINFOHEADER);

				memcpy(m_p32Frame, p32BitFrameInDIB, pdib->biSizeImage);

				/*
				//change from 32 to 24 bit /pixel
				this->Convert24Image(p32BitFrameInDIB, m_pFrame, pdib->biSizeImage);
				*/

			}

//			lpCurrImage = NULL;

			CoTaskMemFree(lpCurrImage);	//free the image 
		}else{
			return lOut;
		}

	}else{
		return lOut;
	}

	return lOut=m_n32Framelen;

}

bool CVMR_Capture::Convert24Image(BYTE *p32Img, BYTE *p24Img,DWORD dwSize32)
{

	if(p32Img != NULL && p24Img != NULL && dwSize32>0){

		typedef struct{
			unsigned char r;
			unsigned char g;
			unsigned char b;
		} st_24Img;
		
		st_24Img *stp_24Img;

		typedef struct{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} st_32Img;

		st_32Img *stp_32Img;

		DWORD dwSize24;
		dwSize24=(dwSize32 * 3)/4;

		BYTE *pTemp,*ptr;
		//pTemp=p32Img + sizeof(BITMAPINFOHEADER); ;
		pTemp=p32Img;

		stp_32Img = (st_32Img *)p32Img;
		stp_24Img = (st_24Img *)p24Img;

		ptr=p24Img + dwSize24-1 ;

		int ival=0;
		int m_inv_row, m_inv_col;

		for(int nRow=0;nRow <m_nHeight; nRow++){
			for(int nCol=0; nCol <m_nWidth; nCol++){
				m_inv_row = m_nHeight -nRow-1;
				m_inv_col = m_nWidth - nCol-1;
				/*stp_24Img[nRow*m_nWidth+nCol].r= stp_32Img[nRow*m_nWidth+m_inv_col].r;
				stp_24Img[nRow*m_nWidth+nCol].g= stp_32Img[nRow*m_nWidth+m_inv_col].g;
				stp_24Img[nRow*m_nWidth+nCol].b= stp_32Img[nRow*m_nWidth+m_inv_col].b; */

				memcpy(stp_24Img+(nRow*m_nWidth+nCol), stp_32Img+(m_inv_row*m_nWidth+m_inv_col),sizeof(st_24Img));

				/*stp_24Img[nRow*m_nWidth+nCol].r= stp_32Img[nRow*m_nWidth+nCol].r;
				stp_24Img[nRow*m_nWidth+nCol].g= stp_32Img[nRow*m_nWidth+nCol].g;
				stp_24Img[nRow*m_nWidth+nCol].b= stp_32Img[nRow*m_nWidth+nCol].b;*/
			}
		}



		/*for (DWORD index = 0; index < dwSize32/4 ; index++){									
		
			unsigned char r = *(pTemp++);
			unsigned char g = *(pTemp++);
			unsigned char b = *(pTemp++);
			(pTemp++);//skip alpha
						
			*(ptr--) = b;
			*(ptr--) = g;
			*(ptr--) = r;			
		}*/	
	}else{
		return false;
	}

return true;
}

BOOL CVMR_Capture::Pause()
{	
    if (!m_pMediaControl)
        return FALSE;
  

    if(((m_psCurrent == Paused) || (m_psCurrent == Stopped)) ){
		this->StopCapture();
        if (SUCCEEDED(m_pMediaControl->Run()))
            m_psCurrent = Running;
		
    }else{
        if (SUCCEEDED(m_pMediaControl->Pause()))
            m_psCurrent = Paused;
    }
	return TRUE;
}



/*
DWORD CVMR_Capture::GetFrame(BYTE **pFrame)
{
	if(m_pFrame && m_nFramelen){
		*pFrame=m_pFrame;
	}

	return m_nFramelen;
}
*/

DWORD CVMR_Capture::GetFrame32(BYTE **p32Frame)
{
	if(m_p32Frame && m_n32Framelen){
		*p32Frame=m_p32Frame;
	}

	return m_n32Framelen;
}

int  CVMR_Capture::EnumDevices(HWND hList)
{
	if(!hList){
		return  -1;
	}

	int id = 0;
	
   // enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
    
   // ICreateDevEnum *pCreateDevEnum;
   HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, 
											NULL, 
											CLSCTX_INPROC_SERVER,
											IID_ICreateDevEnum, 
											(void**)&pCreateDevEnum);
    
    if (hr != NOERROR){
		return -1;
	}

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
    if (hr != NOERROR){
		return -1 ;
    }

    pEm->Reset();

    ULONG		cFetched;
    IMoniker	*pM;

    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK){
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);

		if(SUCCEEDED(hr)){
			VARIANT var;
			var.vt = VT_BSTR;
			hr	    = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR){
				TCHAR str[2048];		
				id++;
				WideCharToMultiByte( CP_ACP,0,var.bstrVal, -1, str, 2048, NULL, NULL);

				(long)SendMessage(hList, CB_ADDSTRING, 0,(LPARAM)str); // Send the Friendly device name to the combobox

				SysFreeString(var.bstrVal);
			}
			pBag->Release();
		}
		pM->Release();
    }
	return id;
}


