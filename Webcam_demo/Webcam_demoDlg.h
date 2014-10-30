
// Webcam_demoDlg.h : header file
//

#pragma once

#include  <math.h>
#include <iostream>
#include <fstream>

using namespace std;

#include "Global_define.h"
#include "BitmapCtrl.h"

#include "StaticCounter.h"
#include "chart.h"

#include "VMR_Capture.h"
#include "afxwin.h"
#include "Showpic.h"
#include "gl3D.h"




// CWebcam_demoDlg dialog
class CWebcam_demoDlg : public CDialogEx
{
// Construction
public:
	CWebcam_demoDlg(CWnd* pParent = NULL);	// standard constructor
	~CWebcam_demoDlg();

	LRESULT ShowScrollPos(WPARAM Wparm,LPARAM Lparm);
	LRESULT FrameGrabe(WPARAM Wparm,LPARAM Lparm);

// Dialog Data
	enum { IDD = IDD_WEBCAM_DEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support




// Implementation
protected:
	HICON m_hIcon;

	//-----------------------------------
	// 2014/10/29 -- CSL
	BOOL mWebcamConnected;
	CBitmapCtrl m_bmpCtrl;

	CRect	 brect_OpenGL_3D_Profile;
	CRect	 brect_2D_Chart;

	CVMR_Capture		*m_pVMRCap;  // setup webcam

	CChart				m_Chart_X;
	GL3D					*gl3D;
	CShowpic              m_ctrlCaptureIMG;
	
	// timer
    int					m_nTimerID, uElapse;

   // display cursor position	
	CStaticCounter m_mouse_x;
	CStaticCounter m_mouse_y;	
	int m_bitmapctr_mouse_x;
	int m_bitmapctr_mouse_y;



	bool InitChart(int series, int points);
	bool InitGL(int width, int height);



	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedBtnConnectWebcam();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedCheckGreyscale();
	afx_msg void OnBnClickedCheckMono();

	CComboBox m_ctrlDevice;
   // init states
	int mThreshold;
	int m_SlideThreshold;
	int m_R_Factor;
	int m_G_Factor;
	int m_B_Factor;
	int m_Gain;
	BOOL m_greyScale;
	bool m_ht_circle;

	
	
	
	
	BOOL m_bGreyScaleChecked;
	BOOL m_bMonoChecked;
};
