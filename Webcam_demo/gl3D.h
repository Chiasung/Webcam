#ifndef _GL3DH_
#define _GL3DH_
#include "glut.h"
#include "math.h"
#include "font.h"               //display font

using namespace std;
#include <vector>
#include "afxwin.h"
//---------------------------------------------------------------------------
struct GLColor                
{
        float R, G, B;          
};
enum ProfileType {ptMesh=1, ptOutline, ptPeak};
enum ColorTranslation {ctIndex=1, ctMap};
//---------------------------------------------------------------------------
class GL3D : public CDialog
{
public:
        GL3D(int width, int height);    
        GL3D(HDC hdc, int width, int height);   
        ~GL3D();
        
        BOOL    Create(DWORD dwStyle, CRect &rect, CWnd *pParent, UINT id);        
        BOOL    SetupRC(HDC hdc);
        void    SetViewportSize(int width, int height);
        void    SetClippingVolume(double sizex, double sizey, double sizez);
        void    SetColorIndex(GLColor *color_index);
        void    SetColorIndex(vector<GLColor> &color_index);
        void    SetRotation(float rotx, float roty, float rotz);
        void    SetTranslation(float transx, float transy, float transz);
        void    SetLevelPlane(double &val);
			 

        void    DrawData(unsigned short *pSrc, int sizex, int sizey, 
						 int slicex=-1, int slicey=-1,
						  GLenum mode=GL_FILL, unsigned short *pMap=NULL);

        bool    m_bAutoScaleZ;
        bool    m_bVisibleWall;
        bool    m_bAutoScaleColorMap;
        bool    m_bLevelPlane;
        
		//----------------------------------------
		void		init(void);
		void		redraw(void);
		void		resize(void);
		void		setupPixelFormat(HDC hDC);
		void		setupPalette(HDC hDC);

		LPCWSTR		className		;
		LPCWSTR		windowName;
		int			winX			;//= 0;
		int			winY			;//= 0;
		int			winWidth		;//= 300;
		int			winHeight		;//= 300;
		HDC			hDC;
		HGLRC		hGLRC;
		HPALETTE	hPalette;
		
        int rotx, roty, rotz;
        int m_slicex, m_slicey;
	    unsigned short *data;
        double	setlevel;
		int m_GLPixelIndex;
		
		bool bLMouseDown;
		int LMouseDownX, LMouseDownY;
		//----------------------------------------
		
protected:
		CDC		memDC;	
		CRect		rcClient,rcClient_backup;	
		CBitmap* pBitmap;
		CBitmap* pOldBitmap;	
		BOOL		SetWindowPixelFormat(HDC hDC);
private:

		CRect	m_ctlRect ;
	

        HGLRC   m_glhRC;
        HDC     m_glhDC;
        int     m_PixelFormat;
        GLColor m_ColorIndex[256];
        GLFONT  *m_Font;
        double  m_LevelPlaneValue;

        int     m_ViewportWidth, m_ViewportHeight;
        int     m_ClippingVolumeX, m_ClippingVolumeY, m_ClippingVolumeZ;
        int     m_DataSizeX, m_DataSizeY;
        double  m_DataMax, m_DataMin;
        float   m_RotX, m_RotY, m_RotZ, m_TransX, m_TransY, m_TransZ;

        void    Initial(int width, int height);
        void    ReleaseResource();
        BOOL    bSetupPixelFormat(HDC hdc);
        void    DrawCoordinate(int sizex,   int sizey, double sizez, 
							   int slicex=4,int slicey=4, int slicez=4);
        void    DrawColorBar(double max, double min, int text_slice);
        void    DrawLevelPlane(int sizex, int sizey, double val);

        template <typename TSrc, typename TMap>
        void    DrawMesh(TSrc *pSrc, int sizex, int sizey, 
						 int slicex=-1, int slicey=-1,
						 GLenum mode=GL_FILL, TMap *pMap=NULL);

        template <typename TSrc>
        void    SearchMaxMin(TSrc *pSrc, double &max, double &min, int len);

		DECLARE_MESSAGE_MAP()
		afx_msg void OnPaint();
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
public:
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
};
#endif
