
// Webcam_demoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Webcam_demo.h"
#include "Webcam_demoDlg.h"
#include "afxdialogex.h"
#include "BitmapCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define TIMER_ID_CAP_CAMERA			2
#define TIMER_DELAY_CAP_CAMERA		100

CRITICAL_SECTION CriticalSection; 

// CWebcam_demoDlg dialog


CWebcam_demoDlg::CWebcam_demoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWebcam_demoDlg::IDD, pParent)
	, mThreshold(128)
	, m_SlideThreshold(128)
	, m_R_Factor(299)
	, m_G_Factor(587)
	, m_B_Factor(587)
	, m_Gain(1)
	, m_greyScale(FALSE)
	, m_ht_circle(false)
	, m_bGreyScaleChecked(FALSE)
	, m_bMonoChecked(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//Init states
	mWebcamConnected = false;


	m_mouse_x.SetDraw3DBar(false);
	m_mouse_y.SetDraw3DBar(false);

	gl3D = NULL;
	m_pVMRCap = NULL;

}

CWebcam_demoDlg::~CWebcam_demoDlg()
{

	if( gl3D != NULL)
		delete gl3D;

    // Release resources used by the critical section object.
    DeleteCriticalSection(&CriticalSection);
}

void CWebcam_demoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ctrlDevice);
	DDX_Control(pDX, IDC_MOUSE_X, m_mouse_x);
	DDX_Control(pDX, IDC_MOUSE_Y, m_mouse_y);
	DDX_Control(pDX, IDC_STATIC_RAW_IMG, m_ctrlCaptureIMG);
	DDX_Check(pDX, IDC_CHECK_GREYSCALE, m_bGreyScaleChecked);
	DDX_Check(pDX, IDC_CHECK_MONO, m_bMonoChecked);
}

BEGIN_MESSAGE_MAP(CWebcam_demoDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CWebcam_demoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CWebcam_demoDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_CONNECT_WEBCAM, &CWebcam_demoDlg::OnBnClickedBtnConnectWebcam)
	ON_WM_TIMER()

    ON_MESSAGE(SHOW_SCROLL_POS,CWebcam_demoDlg::ShowScrollPos) // CSL

	ON_BN_CLICKED(IDC_CHECK_GREYSCALE, &CWebcam_demoDlg::OnBnClickedCheckGreyscale)
	ON_BN_CLICKED(IDC_CHECK_MONO, &CWebcam_demoDlg::OnBnClickedCheckMono)
END_MESSAGE_MAP()


// CWebcam_demoDlg message handlers

BOOL CWebcam_demoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    // Initialize the critical section one time only.
    if (!InitializeCriticalSectionAndSpinCount(&CriticalSection, 
        0x00000400) ) 
        return false;

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	/**********************************************/
	//create rect to display processed image
	/**********************************************/	
	CRect brect;
	GetDlgItem(IDC_STATIC_MAIN_IMG)->GetWindowRect(brect);
	ScreenToClient(brect);

	m_bmpCtrl.Create(WS_CHILD|WS_VISIBLE,brect,this);


//	DisplaySud mdisplay;
//	mdisplay.grid = true;
//	m_bmpCtrl.SetDisplayMode(mdisplay);

	/**********************************************/
	//create 2D-chart control
	/**********************************************/	
	
	GetDlgItem(IDC_STATIC_PROFILE_CHART)->GetWindowRect(brect_2D_Chart);
	ScreenToClient(brect_2D_Chart);
	
	
	/**********************************************/
	//create OpenGL control
	/**********************************************/	
	
	GetDlgItem(IDC_STATIC_3D_PROFILE)->GetWindowRect(brect_OpenGL_3D_Profile);
	ScreenToClient(brect_OpenGL_3D_Profile);

	
	/**********************************************/
	// Display Grey Level on the Canvas
	/**********************************************/	

	int row=480,col=640;
	unsigned char **img;
	img=new unsigned char *[row];

	for(int i=0;i<row;i++){
		img[i]=new unsigned char [col];
	}

	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){
			img[i][j]=  (int)(255.0 *i/row);
		}
	}
	m_bmpCtrl.SetBitmap8(row,col,img);
	for(int  i=0;i<row;i++){
		delete [] img[i];
	}
	delete [] img;	
	
	if(m_pVMRCap == NULL){

		m_pVMRCap = new CVMR_Capture();  // setup webcam
		int iSel=-1;
		iSel=m_ctrlDevice.GetCurSel();
		
		HWND thisHWND = m_ctrlCaptureIMG.GetSafeHwnd();
		HWND thisParentHWND = this->GetSafeHwnd();
		m_pVMRCap->Init( iSel, /*NULL*/ thisHWND, &m_bmpCtrl.m_un32ImageWidth, &m_bmpCtrl.m_un32ImageHeight, thisParentHWND);
		/**********************************************/
		// Select Camera in the combo box
		/**********************************************/	
		this->GetDlgItem(IDC_BTN_CONNECT_WEBCAM) ->EnableWindow(FALSE);
	
		if(m_pVMRCap->EnumDevices( m_ctrlDevice.m_hWnd )>0){
			m_ctrlDevice.SetCurSel (0);
			this->GetDlgItem (IDC_BTN_CONNECT_WEBCAM)->EnableWindow(TRUE);
		}	
	}


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWebcam_demoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWebcam_demoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CWebcam_demoDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CWebcam_demoDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CWebcam_demoDlg::OnBnClickedBtnConnectWebcam()
{
	// TODO: Add your control notification handler code here
	mWebcamConnected = !mWebcamConnected;

	CWnd *pB = GetDlgItem(IDC_BTN_CONNECT_WEBCAM);

	if(mWebcamConnected){
		CString sCaption = "Disconnect";
	    //CString str(szTest[i]) ;
		pB->SetWindowText(sCaption);
		
		// Request ownership of the critical section.
		EnterCriticalSection(&CriticalSection); 

		this->KillTimer(TIMER_ID_CAP_CAMERA);
		//-------------------------
		int iSel=-1;
		iSel=m_ctrlDevice.GetCurSel();
	
		HWND thisHWND = m_ctrlCaptureIMG.GetSafeHwnd();
		HWND thisParentHWND = this->GetSafeHwnd();
	
		if(m_pVMRCap != NULL){
			delete m_pVMRCap;
		}
		m_pVMRCap = new CVMR_Capture();  // setup webcam
		m_pVMRCap->Init( iSel, /*NULL*/ thisHWND, &m_bmpCtrl.m_un32ImageWidth, &m_bmpCtrl.m_un32ImageHeight, thisParentHWND);
	    
		InitChart(3, m_bmpCtrl.m_un32ImageWidth);
		InitGL(m_bmpCtrl.m_un32ImageWidth, m_bmpCtrl.m_un32ImageHeight);
		
	
		m_bmpCtrl.InitBitmap32(m_bmpCtrl.m_un32ImageHeight, m_bmpCtrl.m_un32ImageWidth);
	
		m_bmpCtrl.Init_Video_MemDC();
		/////////////////////////////
	

		
		/////////////////////////////
			m_Chart_X.Corrdinate(2,2);
			m_Chart_X.SetChartTitle("Horizontal intensity profile");
			m_Chart_X.SetChartLabel("","Y");
			m_Chart_X.SetGridXYNumber(8,2);
			m_Chart_X.SetRange(0, m_bmpCtrl.m_un32ImageWidth, 0,255);
			m_Chart_X.SetAxisStyle(0);
			m_Chart_X.mpSerie[0].m_plotColor = RGB(255,0,0);
			m_Chart_X.mpSerie[1].m_plotColor = RGB(0,255,0);
			m_Chart_X.mpSerie[2].m_plotColor = RGB(0,0,255);
			m_Chart_X.m_BGColor = RGB(255,255,255);
			m_Chart_X.SetGridXYNumber(20,15);
			m_Chart_X.UpdateWindow() ;// .Create(WS_CHILD|WS_VISIBLE,brect_2D_Chart,this,IDC_CHART_X);		
		
			/*
			unsigned char **img;
			img=new unsigned char *[m_bmpCtrl.m_un32ImageHeight];
		
			for(int i=0;i<m_bmpCtrl.m_un32ImageHeight;i++)
				img[i]=new unsigned char [m_bmpCtrl.m_un32ImageWidth];
		
			for(int i=0;i<m_bmpCtrl.m_un32ImageHeight;i++){
				for(int j=0;j<m_bmpCtrl.m_un32ImageWidth;j++){
					img[i][j]=  200;
				}
			}
			m_bmpCtrl.SetBitmap8(m_bmpCtrl.m_un32ImageHeight, m_bmpCtrl.m_un32ImageWidth, img);
			for(int  i=0;i<m_bmpCtrl.m_un32ImageHeight;i++)
				delete [] img[i];
			delete [] img;
			*/
		/////////////////////////////
		this->SetTimer (TIMER_ID_CAP_CAMERA,TIMER_DELAY_CAP_CAMERA,NULL);	
	    
		// Release ownership of the critical section.
		LeaveCriticalSection(&CriticalSection);

		//this->GetDlgItem(IDC_STARTCAP)->EnableWindow(TRUE);
		//this->GetDlgItem(IDC_BTN_CAP_WEBCAM)->EnableWindow(TRUE);
		//this->GetDlgItem(IDC_INITCAM)->EnableWindow(FALSE);	
	


	}else{
		CString sCaption = "Connect";
	    //CString str(szTest[i]) ;
		pB->SetWindowText(sCaption);

		if(m_pVMRCap){
			delete m_pVMRCap;
			m_pVMRCap = NULL;
//			m_pVMRCap = new CVMR_Capture();  // setup webcam
			int iSel=-1;
			iSel=m_ctrlDevice.GetCurSel();
			
//			HWND thisHWND = m_ctrlCaptureIMG.GetSafeHwnd();
//			HWND thisParentHWND = this->GetSafeHwnd();
//			m_pVMRCap->Init( iSel, /*NULL*/ thisHWND, &m_bmpCtrl.m_un32ImageWidth, &m_bmpCtrl.m_un32ImageHeight, thisParentHWND);

		}

		this->KillTimer(TIMER_ID_CAP_CAMERA);

		/**********************************************/
		// Select Camera in the combo box
		/**********************************************/	
//		this->GetDlgItem(IDC_BTN_CONNECT_WEBCAM) ->EnableWindow(FALSE);
	
//		if(m_pVMRCap->EnumDevices( m_ctrlDevice.m_hWnd )>0){
//			m_ctrlDevice.SetCurSel (0);
//			this->GetDlgItem (IDC_BTN_CONNECT_WEBCAM)->EnableWindow(TRUE);
//		}	
	

	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWebcam_demoDlg::InitChart(int series, int points)
{
	//Allocate space for series .
	if ( !m_Chart_X.AllocSerie(points) ) {
		AfxMessageBox("Error allocating chart serie") ;
        //return (false);
	}

	// Customize chart 
	m_Chart_X.Corrdinate(2,2);
	m_Chart_X.SetChartTitle("X Section Profile");
	m_Chart_X.SetChartLabel("X","Y");
	m_Chart_X.SetGridXYNumber(8,2);
	m_Chart_X.SetRange(0, points, 0, 255);
	m_Chart_X.SetAxisStyle(0); 
	m_Chart_X.mpSerie[0].m_plotColor = RGB(255,0,0);
	m_Chart_X.mpSerie[1].m_plotColor = RGB(0,255,0);
	m_Chart_X.mpSerie[2].m_plotColor = RGB(0,0,255);
	m_Chart_X.m_BGColor = RGB(255,255,255);
	m_Chart_X.SetGridXYNumber(20,15);
	//m_Chart2d.bLogScale = TRUE ;
	if(!m_Chart_X.bCreated){
		m_Chart_X.Create(WS_CHILD|WS_VISIBLE,brect_2D_Chart,this,IDC_STATIC_PROFILE_CHART);
	}


	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWebcam_demoDlg::InitGL(int width, int height)
{
	  
		if(gl3D != NULL){
			delete  gl3D;
		}
		
	  gl3D = new GL3D(width, height);
	  gl3D->Create( WS_CHILD|WS_VISIBLE, brect_OpenGL_3D_Profile,  this, IDC_STATIC_3D_PROFILE);		
				  
      gl3D->m_bAutoScaleColorMap	= true;
      gl3D->m_bAutoScaleZ	= true;
      gl3D->m_slicex				= -1;
      gl3D->m_slicey				= -1;
      gl3D->rotx						= 120;
      gl3D->roty						= 0;
      gl3D->rotz						= 45;

		for(int i=0;i<10000;i++){
			gl3D->data[i] = 0;//i%255;
		}
      
		gl3D->SetRotation(gl3D->rotx, gl3D->roty, gl3D->rotz);
		// SetColorIndexColor();
      GLColor color[] = {{0.0, 0.0, 0.0}, {1.0, 0.0, 1.0}, {0.0, 0.0, 1.0},
								 {0.0, 1.0, 1.0}, {0.0, 1.0, 0.0}, {1.0, 1.0, 0.0},
								 {1.0, 0.0, 0.0}, {1.0, 1.0, 1.0}};
      vector<GLColor> v;
		for(int i=0; i<sizeof(color)/sizeof(GLColor); i++){
			v.push_back(color[i]);
		}

      gl3D->SetColorIndex(v);        
      gl3D->setlevel				   = 150;        
      gl3D->m_bLevelPlane  = false;	
      gl3D->setlevel				   = 150;        
      gl3D->SetViewportSize(180 ,180 );
      gl3D->SetLevelPlane(gl3D->setlevel);
                        
		for(int i=0;i< m_bmpCtrl.m_un32ImageHeight;i++){
			for(int j=0;j< m_bmpCtrl.m_un32ImageWidth;j++){
				gl3D->data[i*m_bmpCtrl.m_un32ImageWidth+j] = 255.*j/m_bmpCtrl.m_un32ImageWidth;
			}
		}

	   gl3D->DrawData(gl3D->data, 
							m_bmpCtrl.m_un32ImageWidth, 
							m_bmpCtrl.m_un32ImageHeight, 
							gl3D->m_slicex, 
							gl3D->m_slicey,
                     GL_FILL, 
                     (unsigned short *)NULL);                         

	   gl3D->Invalidate(1);

	   gl3D->DrawData(gl3D->data, 
							m_bmpCtrl.m_un32ImageWidth, 
							m_bmpCtrl.m_un32ImageHeight, 
							gl3D->m_slicex, 
							gl3D->m_slicey,
                            GL_FILL, 
                            (unsigned short *)NULL);                         

		gl3D->Invalidate(1);
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebcam_demoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CString		m_tmp_g_code;
	CString		strNo;
	int			frameWidth, frameHeight;
	int         currMouseX, currMouseY;

	currMouseX = m_bitmapctr_mouse_x;
	currMouseY = m_bitmapctr_mouse_y;

		
	if( nIDEvent == TIMER_ID_CAP_CAMERA && !this->m_bmpCtrl.m_InProcessing && (mWebcamConnected)){
		DWORD dwSize;
       this->m_bmpCtrl.m_InProcessing  = true;

	   if(m_pVMRCap){
			dwSize=this->m_pVMRCap->GrabFrame();
	   }

		
	
		if(dwSize>0){
			BYTE *pImage;
			BYTE *p32Image;

			frameWidth  = this->m_pVMRCap->GetWidth();
			frameHeight = this->m_pVMRCap->GetHeight();

			if(m_pVMRCap){
				this->m_pVMRCap->GetFrame32(&p32Image);
			}

			this->m_ctrlCaptureIMG.ShowImage(p32Image, frameWidth, frameHeight);
			this->m_bmpCtrl.m_InProcessing  = true;
			this->m_bmpCtrl.SetBitmap32( frameHeight, frameWidth,(unsigned __int32 *)p32Image);
			this->m_bmpCtrl.Invalidate();
			//------------------------------------------------------------------------
			
			unsigned char m_tmp_pixel;
			PIX32 mpix;

			
			for(int i=0;i<  frameHeight;i++){
				for(int j=0;j<  frameWidth;j++){
					m_bmpCtrl.GetBitmap8Pixel(i,j, &m_tmp_pixel, &mpix);
					gl3D->data[i* frameWidth+j] =   mpix.grey;

					if(i == currMouseY){
						m_Chart_X.SetXYValue( (double)j,(double) mpix.red	, j, 0);
						m_Chart_X.SetXYValue( (double)j,(double) mpix.green, j, 1);
						m_Chart_X.SetXYValue( (double)j,(double) mpix.blue , j, 2);
					}
				}
			}


	      gl3D->DrawData(gl3D->data, 
							   frameWidth, 
								frameHeight, 
								gl3D->m_slicex, 
								gl3D->m_slicey,
                        GL_FILL, 
                        (unsigned short *)NULL); 			

			gl3D->Invalidate();
			m_Chart_X.SetVLineIndex(currMouseX);
			m_Chart_X.Invalidate();	
			
		}
		this->m_bmpCtrl.m_InProcessing  = false;
	}	

	CDialogEx::OnTimer(nIDEvent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CWebcam_demoDlg::ShowScrollPos(WPARAM Wparm,LPARAM Lparm)
{
		//unsigned char m_tmp_pixel;
		//PIX32 mpix;

		CString aa,m_x,m_y;
		aa.Format("pos x=%d y=%d",Wparm,Lparm);
		m_x.Format("%4d",Wparm);
		m_y.Format("%4d",Lparm);

		m_bitmapctr_mouse_x = (int) Wparm;
		m_bitmapctr_mouse_y = (int) Lparm;

		m_mouse_x.Display(m_x);
		m_mouse_y.Display(m_y);

		UpdateData(FALSE);

                        
		return 1;
}

void CWebcam_demoDlg::OnBnClickedCheckGreyscale()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if(m_bGreyScaleChecked){
		 m_bMonoChecked = FALSE;
		  UpdateData(FALSE);
		  m_bmpCtrl.m_bGreyScale = true;
		  m_bmpCtrl.m_bBilevel      = false;

	}else{
		m_bmpCtrl.m_bGreyScale = false;
	}

	Invalidate();
}


void CWebcam_demoDlg::OnBnClickedCheckMono()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if(m_bMonoChecked){
		  m_bGreyScaleChecked = FALSE;
		  UpdateData(FALSE);
		  m_bmpCtrl.m_bGreyScale = false;
		  m_bmpCtrl.m_bBilevel      = true;
	}else{
		m_bmpCtrl.m_bBilevel      = false;
	}
	Invalidate();
}
