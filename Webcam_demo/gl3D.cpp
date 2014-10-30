//---------------------------------------------------- 
 
# include   "stdafx.h"        
# pragma hdrstop 
 
#include"gl3D.h" 
 
 
//#pragma package(smart_init) 
BEGIN_MESSAGE_MAP(GL3D,CDialog) 
	ON_WM_PAINT() 
	ON_WM_LBUTTONDOWN() 
	ON_WM_MOUSEMOVE() 
	ON_WM_LBUTTONUP() 
	ON_WM_NCMOUSEMOVE() 
END_MESSAGE_MAP() 
 
 
inline void GLVertex3f(float x,float y,float z){glVertex3f(x,y,-z);} 
inline void GLRasterPos3i(int x,int y,int z){glRasterPos3i(x,y,-z);} 
inline void GLRasterPos3f(float x,float y,float z){glRasterPos3f(x,y,-z);} 
inline void GLTranslatef(float x,float y,float z){glTranslatef(x,y,-z);} 
//--------------------------------------------------------------------------- 
 
//--------------------------------------------------------------------------- 
GL3D::GL3D(int width, int height) 
{ 
	//Initial(); 
	Initial(width, height); 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
 
 
BOOL GL3D::Create(DWORD dwStyle,CRect &rect,CWnd *pParent,UINT id) 
{ 
	Initial(winWidth, winHeight); 
	//staticCStringclassName=AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW); 
	BOOL			result	=false; 
	HWND		mh_subwnd=NULL; 
	HDC			hdc		=NULL; 
	//CREATESTRUCT	m_create_st; 
 
	result=CWnd::CreateEx(WS_EX_CLIENTEDGE|WS_EX_OVERLAPPEDWINDOW, 
	NULL,NULL,dwStyle, 
	rect.left,rect.top, 
	rect.right-rect.left,rect.bottom-rect.top, 
	pParent->GetSafeHwnd(),(HMENU)id); 
 
	mh_subwnd=this->GetSafeHwnd(); 
 
	if(mh_subwnd!=NULL){ 
		result=true; 
	} 
 
 
	GetClientRect(rcClient); 
	rcClient_backup=rcClient; 
	CClientDC dc(this); 
 
	if(memDC.GetSafeHdc()==NULL) 
	{ 
		memDC.CreateCompatibleDC(&dc); 
		if((pBitmap=new CBitmap())==NULL) return FALSE; 
		pBitmap->CreateCompatibleBitmap(&dc,rcClient.Width(),rcClient.Height()); 
		pOldBitmap=memDC.SelectObject(pBitmap); 
		memDC.SelectStockObject(ANSI_VAR_FONT); 
		memDC.SetBkMode(TRANSPARENT); 
 
	} 
	memDC.FillSolidRect(&rcClient,RGB(200,1,1)); 
 
 
 
	if(!result) 
		AfxMessageBox("Errorcreatingwindow"); 
 
	if(m_glhDC!=NULL && hdc!=NULL){ 
		return true; 
	}else{ 
		ReleaseResource(); 
	} 

	hdc=::GetDC(pParent->GetDlgItem(id)->GetSafeHwnd()); 
 
	m_glhDC=hdc; 
	if(!bSetupPixelFormat(m_glhDC)){ 
		return false; 
	} 
	m_glhRC =NULL; 
	m_glhRC =wglCreateContext(m_glhDC); 
	result  =wglMakeCurrent(m_glhDC,m_glhRC); 
	m_Font  =FontCreate(m_glhDC,"TimesNewRoman",10,0,1); 

	return result; 
} 
 
void GL3D::OnPaint() 
{ 
	SwapBuffers(hDC); 
	CPaintDC dc(this);//devicecontextforpainting 
 
} 
 
GL3D::GL3D(HDC hdc, int width, int height) 
{ 
	data = NULL;
	Initial(width, height); 
	SetupRC(hdc); 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
GL3D::~GL3D() 
{ 
	if(data){
		delete [] data;
	}
	ReleaseResource(); 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
void GL3D::Initial(int width, int height) 
{ 
	//------------------------------------------------	 
	className	=(LPCWSTR)"OpenGL"; 
	windowName	=(LPCWSTR)"OpenGLCube"; 
	winX			=0; 
	winY			=0; 
	winWidth	=width; 
	winHeight	=height; 
	data = new unsigned short [width* height];
	//------------------------------------------------ 
	m_RotX				=120; 
	m_RotY				=0; 
	m_RotZ				=45; 
	m_TransX			=0; 
	m_TransY			=0; 
	m_TransZ			=0; 
	m_glhDC			=NULL; 
	m_glhRC			=NULL; 
	m_Font				=NULL; 
	m_bAutoScaleZ	=false; 
	m_bVisibleWall		=false; 
	m_bLevelPlane		=false; 
	m_bAutoScaleColorMap=false; 
 
	bLMouseDown			=false; 
 
	
	for(int i=0;i<256;i++){ 
		m_ColorIndex[i].R=(float)i/255.0; 
		m_ColorIndex[i].G=m_ColorIndex[i].R; 
		m_ColorIndex[i].B=m_ColorIndex[i].R; 
	} 
	
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
void	GL3D::ReleaseResource() 
{ 
	wglMakeCurrent(NULL,NULL); 
	wglDeleteContext(m_glhRC); 
	FontDelete(m_Font); 
 
	m_glhDC	= NULL; 
	m_glhRC	= NULL; 
	m_Font		= NULL; 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
BOOL GL3D::SetupRC(HDC hdc) 
{ 
	BOOL result; 
	if(m_glhDC!=NULL&&hdc!=NULL){ 
		return true; 
	}else{ 
		ReleaseResource(); 
	} 
	m_glhDC=hdc; 
	if(!bSetupPixelFormat(m_glhDC)){ 
		return false; 
	} 
	m_glhRC	=wglCreateContext(m_glhDC); 
	result			=wglMakeCurrent(m_glhDC,m_glhRC); 
	m_Font		=FontCreate(m_glhDC,"TimesNewRoman",10,0,1); 
	return result; 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
void GL3D::SetViewportSize(int width,int height) 
{ 
	if(height==0) 
		height=1; 
	//---SetViewporttowindowdimensions 
	glViewport(0,0,width,height); 
	m_ViewportWidth	=width; 
	m_ViewportHeight=height; 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
void GL3D::SetClippingVolume(double sizex,double sizey,double sizez) 
{ 
	//Resetcoordinatesystem 
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity(); 
 
	//Establishclippingvolume(left,right,bottom,top,near,far) 
	if(m_ViewportWidth<=m_ViewportHeight){ 
		glOrtho(-sizex, 
			sizex, 
			-sizey*m_ViewportHeight/m_ViewportWidth, 
			sizey*m_ViewportHeight/m_ViewportWidth, 
			-sizez, 
			sizez); 
	}else{ 
		glOrtho(-sizex*m_ViewportWidth/m_ViewportHeight, 
			sizex*m_ViewportWidth/m_ViewportHeight, 
			-sizey, 
			sizey, 
			-sizez, 
			sizez); 
	} 
	m_ClippingVolumeX=sizex; 
	m_ClippingVolumeY=sizey; 
	m_ClippingVolumeZ=sizez; 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
void GL3D::SetColorIndex(GLColor *color_index) 
{ 
	memcpy(color_index,m_ColorIndex,256*sizeof(GLColor)); 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
void GL3D::SetColorIndex(vector<GLColor> &color_index) 
{ 
	if(color_index.size()<2) 
		return; 
	float steps=255.0/(color_index.size()-1); 
	float cnt	=0.0; 
	int	x,x1,x2,index; 
	float r,g,b,dr,dg,db,ratio; 
 
	for(cnt=0.0,index=0;cnt<255.0;cnt+=steps,index++){ 
		x1=cnt; 
		x2=cnt+steps; 
		r=color_index[index].R; 
		g=color_index[index].G; 
		b=color_index[index].B; 
		dr=color_index[index+1].R-r; 
		dg=color_index[index+1].G-g; 
		db=color_index[index+1].B-b; 
		for(x=x1;x<=x2;x++){ 
			ratio=(float)(x-x1)/(x2-x1); 
			m_ColorIndex[x].R=r+dr*ratio; 
			m_ColorIndex[x].G=g+dg*ratio; 
			m_ColorIndex[x].B=b+db*ratio; 
		} 
	} 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
void GL3D::SetRotation(float rotx,float roty,float rotz) 
{ 
	m_RotX=rotx; 
	m_RotY=roty; 
	m_RotZ=rotz; 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
void GL3D::SetTranslation(float transx,float transy,float transz) 
{ 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
void GL3D::SetLevelPlane(double &val) 
{ 
	if(val>255.0) 
		val=255.0; 
	if(val<0.0) 
		val=0.0; 
	//m_LevelPlaneValue=val*(m_DataMax-m_DataMin)/100.0+m_DataMin; 
	m_LevelPlaneValue=val; 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

void GL3D::DrawData(unsigned	short	*pSrc,int	sizex,int	sizey, 
				   int	slicex,int	slicey, 
				   GLenum mode,unsigned short *pMap) 
{ 
	double sizez=512; 
	double max; 
 
	if(pSrc==NULL)return; 
 
	SearchMaxMin(pSrc,m_DataMax,m_DataMin,sizex*sizey); 
	sizez=m_DataMax-m_DataMin; 
	sizez=300; 
 
	//SetClippingVolume(sizex,sizey,sizex+sizey+sizez); 
	max=sizex>sizey?sizex:sizey; 
	SetClippingVolume(max,max,max*2); 
 
	glClearColor(1.0,1.0,1.0,1.0); 
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); 
	glEnable(GL_DEPTH_TEST); 
 
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity(); 
 
	//GLTranslatef(m_TransX,m_TransY,m_TransZ); 
	glRotatef(m_RotX,1.0f,0.0f,0.0f); 
	glRotatef(m_RotY,0.0f,1.0f,0.0f); 
	glRotatef(m_RotZ,0.0f,0.0f,1.0f); 
 
	if(m_bAutoScaleZ){ 
		float scale=(float)(sizex+sizey)/2.0/sizez; 
		glScalef(1.0,1.0,scale); 
	} 
	//GLTranslatef(-sizex/2,-sizey/2,-sizez/2); 
	GLTranslatef(-sizex/4,-sizey/2,-m_DataMax/2); 
	//DrawCoordinate(sizex,sizey,sizez); 
 
	DrawMesh(pSrc,sizex,sizey,slicex,slicey,mode,pMap); 
	DrawCoordinate(sizex,sizey,m_DataMax); 
	if(m_bLevelPlane) 
		DrawLevelPlane(sizex,sizey,m_LevelPlaneValue); 
 
	SwapBuffers(m_glhDC); 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
BOOL GL3D::bSetupPixelFormat(HDC hdc) 
{ 
	PIXELFORMATDESCRIPTOR pfd={ 
		sizeof(PIXELFORMATDESCRIPTOR), 
		1, 
		PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER, 
		PFD_TYPE_RGBA, 
		32, 
		//0,0,0,0,0,0, 
		8,0,8,0,8,0, 
		0,0, 
		0,0,0,0,0, 
		32, 
		0, 
		0, 
		PFD_MAIN_PLANE, 
		0, 
		0,0, 
	}; 
	m_PixelFormat=ChoosePixelFormat(hdc,&pfd); 
	if(!m_PixelFormat)return false; 
	return SetPixelFormat(hdc,m_PixelFormat,&pfd); 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
void GL3D::DrawCoordinate(int sizex,int sizey,double sizez,int slicex, 
						 int slicey,int slicez) 
{ 
	if(m_bVisibleWall){ 
 
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 
		//X-Y 
		float alpha=0.5; 
		float delt=-0.002; 
		glBegin(GL_TRIANGLE_STRIP); 
		glColor4f(1.0,0.5,0.5,alpha); 
		GLVertex3f(sizex,0.0,delt*sizez); 
		GLVertex3f(sizex,sizey,delt*sizez); 
		GLVertex3f(0.0,0.0,delt*sizez); 
		GLVertex3f(0.0,sizey,delt*sizez); 
		glEnd(); 
 
		//Y-Z 
		glBegin(GL_TRIANGLE_STRIP); 
		glColor4f(0.5,1.0,0.5,alpha); 
		GLVertex3f(delt*sizex,sizey,0.0); 
		GLVertex3f(delt*sizex,sizey,sizez); 
		GLVertex3f(delt*sizex,0.0,0.0); 
		GLVertex3f(delt*sizex,0.0,sizez); 
		glEnd(); 
 
		//Z-X 
		glBegin(GL_TRIANGLE_STRIP); 
		glColor4f(0.5,0.5,1.0,alpha); 
		GLVertex3f(0.0,delt*sizey,sizez); 
		GLVertex3f(sizex,delt*sizey,sizez); 
		GLVertex3f(0.0,delt*sizey,0.0); 
		GLVertex3f(sizex,delt*sizey,0.0); 
		glEnd(); 
		glDisable(GL_BLEND); 
	} 
 
 
	glColor4f(0.0,0.0,0.0,1.0); 
	glBegin(GL_LINES); 
	//X 
	glColor4f(1.0,0.0,0.0,1.0); 
	GLVertex3f(0,0,0); 
	GLVertex3f(sizex,0,0); 
 
	//Y 
	glColor4f(0.0,1.0,0.0,1.0); 
	glVertex3f(0,0,0); 
	GLVertex3f(0,sizey,0); 
 
	//Z 
	glColor4f(0.0,0.0,1.0,1.0); 
	GLVertex3f(0,0,0); 
	GLVertex3f(0,0,sizez); 
 
	glEnd(); 
 
	glColor4f(0.0,0.0,0.0,1.0); 
	glBegin(GL_LINE_STRIP); 
	GLVertex3f(sizex,	0.0,0.0); 
	GLVertex3f(sizex,sizey,0.0); 
	GLVertex3f(0.0,sizey,0.0); 
	GLVertex3f(0.0,sizey,sizez); 
	GLVertex3f(0.0,0.0,sizez); 
	GLVertex3f(sizex,0.0,sizez); 
	GLVertex3f(sizex,0.0,0.0); 
	glEnd(); 
 
	//---[} 
	double stepx,stepy,stepz; 
	stepx=(double)(sizex-1)/slicex; 
	stepy=(double)(sizey-1)/slicey; 
	stepz=(sizez)/slicez; 
	glEnable(GL_LINE_STIPPLE); 
	glLineStipple(1,0x5555); 
	glBegin(GL_LINES); 
	//X-Y 
	for(int i=1;i<slicex;i++){ 
		GLVertex3f(stepx*i,0.0,0.0); 
		GLVertex3f(stepx*i,sizey,0.0); 
	} 
	for(int i=1;i<slicey;i++){ 
		GLVertex3f(0.0,stepy*i,0.0); 
		GLVertex3f(sizex,stepy*i,0.0); 
	} 
 
	//Y-Z 
	for(int i=1;i<slicey;i++){ 
		GLVertex3f(0.0,stepy*i,0.0); 
		GLVertex3f(0.0,stepy*i,sizez); 
	} 
	for(int i=1;i<slicez;i++){ 
		GLVertex3f(0.0,0.0,stepz*i); 
		GLVertex3f(0.0,sizey,stepz*i); 
	} 
 
	//Z-X 
	for(int i=1;i<slicez;i++){ 
		GLVertex3f(0.0,0.0,stepz*i); 
		GLVertex3f(sizex,0.0,stepz*i); 
	} 
	for(int i=1;i<slicex;i++){ 
		GLVertex3f(stepx*i,0.0,0.0); 
		GLVertex3f(stepx*i,0.0,sizez); 
	} 
	glEnd(); 
	glDisable(GL_LINE_STIPPLE); 
 
	//postcoordinalvalues 
	double min=m_ViewportWidth<m_ViewportHeight?m_ViewportWidth:m_ViewportHeight; 
	double xoff=60.0*sizex/min; 
	double yoff=60.0*sizey/min; 
	double zoff=60.0*sizez/min; 
	for(int i=0;i<=slicex;i++){ 
		//GLRasterPos3i(stepx*i,sizey+60,0.0); 
		GLRasterPos3i(stepx*i,sizey+yoff,0.0); 
		FontPrintf(m_Font,0,"%4.2f",stepx*i); 
	} 
	for(int i=0;i<=slicey;i++){ 
		//GLRasterPos3i(sizex+60,stepy*i,0.0); 
		GLRasterPos3i(sizex+xoff,stepy*i,0.0); 
		FontPrintf(m_Font,0,"%4.2f",stepy*i); 
	} 
	for(int i=0;i<=slicez;i++){ 
		//GLRasterPos3f(0.0,sizey+60,stepz*i); 
		GLRasterPos3f(0.0,sizey+yoff,stepz*i); 
		FontPrintf(m_Font,0,"%4.2f",stepz*i); 
	} 
 
	//postcoordinalmarks 
	{ 
		glColor3f(0.5,0.5,0.0); 
 
		//GLRasterPos3i(sizex/2,sizey+120,0); 
		GLRasterPos3i(sizex/2,sizey+yoff*2,0); 
		FontPrintf(m_Font,0,"XAxis(pixel)"); 
 
		//GLRasterPos3i(sizex+120,sizey/2,0); 
		GLRasterPos3i(sizex+xoff*2,sizey/2,0); 
		FontPrintf(m_Font,0,"YAxis(pixel)"); 
 
		//GLRasterPos3i(0,0,sizez+30); 
		GLRasterPos3f(0,0,sizez+zoff/2); 
		FontPrintf(m_Font,0,"ZAxis"); 
	} 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
void GL3D::DrawLevelPlane(int sizex,int sizey,double val) 
{ 
	if(m_bLevelPlane){ 
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 
		//X-Y 
		float alpha=0.5; 
		glBegin(GL_TRIANGLE_STRIP); 
		glColor4f(0.5,0.5,0.0,alpha); 
		GLVertex3f(sizex,0.0,val); 
		GLVertex3f(sizex,sizey,val); 
		GLVertex3f(0.0,0.0,val); 
		GLVertex3f(0.0,sizey,val); 
		glEnd(); 
		glDisable(GL_BLEND); 
	} 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
template<typename TSrc,typename TMap> 
void GL3D::DrawMesh(TSrc *pSrc,int sizex,int sizey,int slicex,int slicey, 
				   GLenum mode,TMap *pMap) 
{ 
	float stepx,stepy; 
 
	if(slicex<0||slicey<0) 
	{ 
		//---drawfulldata 
		stepx=stepy=1.0; 
		slicex=sizex-1; 
		slicey=sizey-1; 
	} 
	else 
	{ 
		//---drawslicedata 
		stepx=(double)(sizex-1)/slicex; 
		stepy=(double)(sizey-1)/slicey; 
	} 
 
	float x,y; 
	float map_color; 
	float delt=m_DataMax-m_DataMin; 
	GLColor *color; 
	int i,j; 
	int shift; 
	delt=(delt==0.0)?1.0:delt; 
 
	glPolygonMode(GL_FRONT_AND_BACK,mode); 
	if(pMap==NULL) 
	{ 
		//---usecolorbar:m_ColorIndex 
		for(j=0,y=0.0;j<slicey;j++,y+=stepy) 
		{ 
			glBegin(GL_TRIANGLE_STRIP); 
			for(i=0,x=0.0;i<=slicex;i++,x+=stepx) 
			{ 
				shift=sizex*(int)y+(int)x; 
				color=&m_ColorIndex[int((pSrc[shift]-m_DataMin)*255.0/delt)]; 
				glColor3f(color->R,color->G,color->B); 
				//glEdgeFlag(true); 
				GLVertex3f(x,y,pSrc[shift]); 
 
				shift=sizex*(int)(y+stepy)+(int)x; 
				color=&m_ColorIndex[int((pSrc[shift]-m_DataMin)*255.0/delt)]; 
				glColor3f(color->R,color->G,color->B); 
				//glEdgeFlag(false); 
				GLVertex3f(x,y+stepy,pSrc[shift]); 
			} 
			glEnd(); 
		} 
	} 
	else 
	{ 
		//---usecolormap:pMap 
		double color_map_max=255,color_map_min=0; 
		if(m_bAutoScaleColorMap) 
			SearchMaxMin(pMap,color_map_max,color_map_min,sizex*sizey); 
		float delt=color_map_max-color_map_min; 
 
		glPolygonMode(GL_FRONT_AND_BACK,mode); 
		for(j=0,y=0.0;j<slicey;j++,y+=stepy) 
		{ 
			glBegin(GL_TRIANGLE_STRIP); 
			for(i=0,x=0.0;i<slicex;i++,x+=stepx) 
			{ 
				shift=sizex*(int)y+(int)x; 
				map_color=(float)(pMap[shift]-color_map_min)/delt; 
				glColor3f(map_color,map_color,map_color); 
				GLVertex3f(x,y,pSrc[shift]); 
 
				shift=sizex*(int)(y+stepy)+(int)x; 
				map_color=(float)(pMap[shift]-color_map_min)/delt; 
				glColor3f(map_color,map_color,map_color); 
				GLVertex3f(x,y+stepy,pSrc[shift]); 
			} 
			glEnd(); 
		} 
	} 
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
template<typename TSrc> 
void GL3D::SearchMaxMin(TSrc *pSrc,double &max,double &min,int len) 
{ 
	int m_len; 
	max=-999999999; 
	min=999999999; 
 
	for(int i=0;i<len;i++){ 
		if(max<pSrc[i]){ 
			max=pSrc[i]; 
		}else if(min>pSrc[i]){ 
			min=pSrc[i]; 
		} 
	} 
	m_len=len; 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
 
void GL3D::init(void) 
{ 
	/*setviewingprojection*/ 
	glMatrixMode(GL_PROJECTION); 
	glFrustum(-0.5F,0.5F,-0.5F,0.5F,1.0F,3.0F); 
	//glFrustum(-10.5F,10.5F,-10.5F,10.5F,-2.0F,13.0F); 
 
	/*positionviewer*/ 
	glMatrixMode(GL_MODELVIEW); 
	//glTranslatef(0.0F,0.0F,-2.0F); 
	//glTranslatef(0.0F,0.0F,-2.2F); 
 
	//glTranslatef(-5.0F,0.0F,-5.2F); 
	glTranslatef(-7.0F,1.0F,-5.2F); 
 
	/*positionobject*/ 
	glRotatef(30.0F,1.0F,0.0F,0.0F); 
	glRotatef(30.0F,0.0F,1.0F,0.0F); 
 
	glEnable(GL_DEPTH_TEST); 
	glEnable(GL_LIGHTING); 
	glEnable(GL_LIGHT0); 
} 
 
void 
GL3D::redraw(void) 
{ 
 
	glBegin(GL_QUADS); 
		glNormal3f(0.0F,0.0F,1.0F); 
		glVertex3f(0.5F,0.5F,0.5F);glVertex3f(-0.5F,0.5F,0.5F); 
		glVertex3f(-0.5F,-0.5F,0.5F);glVertex3f(0.5F,-0.5F,0.5F); 
 
		glNormal3f(0.0F,0.0F,-1.0F); 
		glVertex3f(-0.5F,-0.5F,-0.5F);glVertex3f(-0.5F,0.5F,-0.5F); 
		glVertex3f(0.5F,0.5F,-0.5F);glVertex3f(0.5F,-0.5F,-0.5F); 
 
		glNormal3f(0.0F,1.0F,0.0F); 
		glVertex3f(0.5F,0.5F,0.5F);glVertex3f(0.5F,0.5F,-0.5F); 
		glVertex3f(-0.5F,0.5F,-0.5F);glVertex3f(-0.5F,0.5F,0.5F); 
 
		glNormal3f(0.0F,-1.0F,0.0F); 
		glVertex3f(-0.5F,-0.5F,-0.5F);glVertex3f(0.5F,-0.5F,-0.5F); 
		glVertex3f(0.5F,-0.5F,0.5F);glVertex3f(-0.5F,-0.5F,0.5F); 
	 
		glNormal3f(1.0F,0.0F,0.0F); 
		glVertex3f(0.5F,0.5F,0.5F);glVertex3f(0.5F,-0.5F,0.5F); 
		glVertex3f(0.5F,-0.5F,-0.5F);glVertex3f(0.5F,0.5F,-0.5F); 
 
		glNormal3f(-1.0F,0.0F,0.0F); 
		glVertex3f(-0.5F,-0.5F,-0.5F);glVertex3f(-0.5F,-0.5F,0.5F); 
		glVertex3f(-0.5F,0.5F,0.5F);glVertex3f(-0.5F,0.5F,-0.5F); 
	glEnd(); 
 
	SwapBuffers(hDC); 
 
	DrawMesh(data,100,100,this->m_slicex, 
		this->m_slicey,GL_FILL,(unsigned short*)NULL); 
	SwapBuffers(m_glhDC); 
 
} 
 
void 
GL3D::resize(void) 
{ 
	/*setviewporttocoverthewindow*/ 
	glViewport(0,0,winWidth,winHeight); 
} 
 
void 
GL3D::setupPixelFormat(HDC hDC) 
{ 
	PIXELFORMATDESCRIPTOR pfd={ 
		sizeof(PIXELFORMATDESCRIPTOR),/*size*/ 
		1,/*version*/ 
		PFD_SUPPORT_OPENGL| 
		PFD_DRAW_TO_WINDOW| 
		PFD_SUPPORT_GDI| 
		PFD_DOUBLEBUFFER,/*supportdouble-buffering*/ 
		PFD_TYPE_RGBA,/*colortype*/ 
		16,/*preferedcolordepth*/ 
		0,0,0,0,0,0,/*colorbits(ignored)*/ 
		0,/*noalphabuffer*/ 
		0,/*alphabits(ignored)*/ 
		0,/*noaccumulationbuffer*/ 
		0,0,0,0,/*accumbits(ignored)*/ 
		16,/*depthbuffer*/ 
		0,/*nostencilbuffer*/ 
		0,/*noauxiliarybuffers*/ 
		PFD_MAIN_PLANE,/*mainlayer*/ 
		0,/*reserved*/ 
		0,0,0,/*nolayer,visible,damagemasks*/ 
	}; 
	int pixelFormat; 
 
	pixelFormat=ChoosePixelFormat(hDC,&pfd); 
	if(pixelFormat==0){ 
		/*MessageBox(	WindowFromDC(hDC), 
		(LPCSTR)"ChoosePixelFormatfailed.", 
		(LPCSTR)"Error", 
		MB_ICONERROR|MB_OK);*/ 
		//MessageBoxA((LPCSTR)"ChoosePixelFormatfailed.");					 
		exit(1); 
	} 
 
	if(SetPixelFormat(hDC,pixelFormat,&pfd)!=TRUE){ 
		/*MessageBox(	WindowFromDC(hDC), 
		(LPCSTR)"SetPixelFormatfailed.", 
		(LPCSTR)"Error", 
		MB_ICONERROR|MB_OK);*/ 
		//MessageBoxA((LPCSTR)"SetPixelFormatfailed.");							 
 
		exit(1); 
	} 
} 
 
void 
GL3D::setupPalette(HDC hDC) 
{ 
	int						pixelFormat	=GetPixelFormat(hDC); 
	PIXELFORMATDESCRIPTOR	pfd; 
	LOGPALETTE*				pPal; 
	int						paletteSize; 
 
	DescribePixelFormat(hDC,pixelFormat,sizeof(PIXELFORMATDESCRIPTOR),&pfd); 
 
	if(pfd.dwFlags&PFD_NEED_PALETTE){ 
		paletteSize=1<<pfd.cColorBits; 
	}else{ 
		return; 
	} 
 
	pPal=(LOGPALETTE*) 
		malloc(sizeof(LOGPALETTE)+paletteSize*sizeof(PALETTEENTRY)); 
	pPal->palVersion=0x300; 
	pPal->palNumEntries=paletteSize; 
 
	/*buildasimpleRGBcolorpalette*/ 
	{ 
		int redMask=(1<<pfd.cRedBits)-1; 
		int greenMask=(1<<pfd.cGreenBits)-1; 
		int blueMask=(1<<pfd.cBlueBits)-1; 
		int i; 
 
		for(i=0;i<paletteSize;++i){ 
			pPal->palPalEntry[i].peRed	 
				=(((i>>pfd.cRedShift)&redMask)*255)/redMask; 
 
			pPal->palPalEntry[i].peGreen 
				=(((i>>pfd.cGreenShift)&greenMask)*255)/greenMask; 
 
			pPal->palPalEntry[i].peBlue 
				=(((i>>pfd.cBlueShift)&blueMask)*255)/blueMask; 
 
			pPal->palPalEntry[i].peFlags=0; 
		} 
	} 
 
	hPalette=CreatePalette(pPal); 
	free(pPal); 
 
	if(hPalette){ 
		SelectPalette(hDC,hPalette,FALSE); 
		RealizePalette(hDC); 
	} 
} 
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
 
BOOL GL3D::PreCreateWindow(CREATESTRUCT &cs) 
{ 
	//TODO:Addyourspecializedcodehereand/orcallthebaseclass 
	cs.style|=(WS_CLIPCHILDREN|WS_CLIPSIBLINGS); 
 
	return CDialog::PreCreateWindow(cs); 
} 
 
//--------------------------------------------------------------------------- 
 
BOOL GL3D::SetWindowPixelFormat(HDC hDC) 
{ 
	//[INzSvP}<h _ 
	PIXELFORMATDESCRIPTOR pixelDesc= 
	{ 
		sizeof(PIXELFORMATDESCRIPTOR), 
		1, 
		PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL| 
		PFD_DOUBLEBUFFER|PFD_SUPPORT_GDI, 
		PFD_TYPE_RGBA, 
		24, 
		0,0,0,0,0,0, 
		0, 
		0, 
		0, 
		0,0,0,0, 
		32, 
		0, 
		0, 
		PFD_MAIN_PLANE, 
		0, 
		0,0,0 
	}; 
 
	this->m_GLPixelIndex=ChoosePixelFormat(hDC,&pixelDesc); 
	if(this->m_GLPixelIndex==0) 
	{ 
		this->m_GLPixelIndex=1; 
		if(DescribePixelFormat(hDC,this->m_GLPixelIndex,sizeof(PIXELFORMATDESCRIPTOR),&pixelDesc)==0) 
		{ 
			return FALSE; 
		} 
	} 
 
	if(SetPixelFormat(hDC,this->m_GLPixelIndex,&pixelDesc)==FALSE) 
	{ 
		return FALSE; 
	} 
	return TRUE; 
 
} 
//void GL3D::OnMButtonDown(UINTnFlags,CPointpoint) 
//{ 
//	//TODO:Addyourmessagehandlercodehereand/orcalldefault 
//	inta; 
// 
//	a=0; 
// 
//	CDialog::OnMButtonDown(nFlags,point); 
//} 
 
 
void GL3D::OnLButtonDown(UINT nFlags,CPoint point) 
 
{	//TODO: 
 
	bLMouseDown=true; 
 
	LMouseDownX = point.x; 
	LMouseDownY = point.y; 
 
	CDialog::OnLButtonDown(nFlags,point); 
 
} 
 
 
void GL3D::OnMouseMove(UINT nFlags,CPoint point) 
 
{	//TODO: 
 
	if(bLMouseDown){ 
 
		if(	(point.x>rcClient.right)||(point.x<0)|| 
			(point.y>rcClient.bottom)||(point.y<0)){ 
 
				bLMouseDown=false; 
 
		}else{ 
 
			rotz-=(point.x-LMouseDownX); 
			rotx+=(point.y-LMouseDownY); 
 
			LMouseDownX=point.x; 
			LMouseDownY=point.y; 
 
			SetRotation(rotx,roty,rotz); 
 
			this->DrawData( data, 
							winWidth, 
							winHeight, 
							this->m_slicex, 
							this->m_slicey, 
							GL_FILL, 
							(unsigned short*)NULL); 
			//Draw(); 
		} 
	} 
	CDialog::OnMouseMove(nFlags,point); 
 
} 
 
 
 
void GL3D::OnLButtonUp(UINT nFlags,CPoint point) 
 
{ 
	//TODO: 
 
	bLMouseDown=false; 
 
	CDialog::OnLButtonUp(nFlags,point); 
 
} 
 
 
void GL3D::OnNcMouseMove(UINT nHitTest, CPoint point) 
{ 
	// TODO: Add your message handler code here and/or call default 
	bLMouseDown=false; 
 
	CDialog::OnNcMouseMove(nHitTest, point); 
} 
