// GaussSmooth.cpp: implementation of the CDOG class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <math.h>

#include "GaussSmooth.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDOG::CDOG()
{
	orgImg		= NULL;
	outImg		= NULL;

	gMask		= NULL;
	rImg		= NULL;
	cImg		= NULL;

	nmImg  = NULL;
	hyImg  = NULL;
	tempImg=NULL;

}

CDOG::~CDOG()
{
	if(orgImg)  delete []orgImg;
	if(outImg)	delete []outImg;

	if(gMask)	delete []gMask;
	if(rImg)	delete []rImg;
	if(cImg)	delete []cImg;
	if(nmImg)	delete []nmImg;
	if(hyImg)	delete []hyImg;
	if(tempImg)	delete []tempImg;
	
}

void CDOG::Initialize(int height, int width, float sigma)
{
	orgImg	= new byte  [height*width];
	outImg	= new byte  [height*width];
	dirImg		= new float [height*width];
	rImg		= new float [height*width];
	cImg		= new float [height*width];
	nmImg	= new byte  [height*width];
	hyImg		= new byte  [height*width];
	tempImg	= new byte  [height*width];

	this->height	= height;
	this->width	= width;
	this->sigma	= sigma;

	wsize			= (int) ceil(3*sigma);
	gMask		= new float [2*wsize+1];
}

CDOG::CDOG(int height, int width, float sigma, int low , int high)
{
	Initialize(height, width, sigma);
	this->low	= low;
	this->high	= high;
	GaussMask();
}

void CDOG::GaussMask()
{
	register int j;

   for(j = 0 ; j < 2*wsize+1 ; ++j){
		gMask[j] = -1*(j - wsize)/(sigma*sigma)*(float)exp(-0.5 * (j - wsize)*(j - wsize)/(sigma*sigma)) ;	
   }
	

}

void CDOG::Run(byte *orgImg)
{
	memcpy(this->orgImg, orgImg, height*width*sizeof(char));

	//DOG Filtering
	Filtering();
	//NonMax Supression
	NonMax();
	//hyteresis
	Hysteresis_Th(low,high);

}

void CDOG::Filtering()
{
	register int i,j,k;
	float  isum, isum2;
	int index1;
		
	for(i=0; i<height; i++){
		index1 = i*width;

		for(j=0; j<width; j++){
			isum = 0.0f;
			isum2 = 0.0f;
			for(k=-wsize; k<=wsize; k++){
				if(!(j+k<0 || j+k>=width)){
					isum += orgImg[index1+j+k] * gMask[k+wsize];
				}
				if(!(i+k<0 || i+k >=height)){
					isum2 += orgImg[(i+k)*width+j] * gMask[k+wsize];
				}
			}
			rImg[index1+j] = isum;
			cImg[index1+j] = isum2;
			
			outImg[index1+j] = (unsigned char)sqrt(isum*isum+isum2*isum2);
			dirImg[index1+j] = atan(isum2/isum);
		}
	}
	
}

byte* CDOG::GetOutImg()
{
	return outImg;
}

 

float* CDOG::GetdirImg()
{
	return dirImg;
}

void CDOG::NonMax()
{

	memcpy(tempImg, outImg, height*width);

	float pi1_8 =	(float)PI/8;			// 0.3926  22.5도
	float pi3_8 =	((float)PI*3)/8 ;	// 1.178097f; // 67.5도
	float pi5_8 =  ((float)PI*5)/8;	//	1.570796f;	112.5 
	float pi7_8 =  ((float)PI*7)/8;	//	2.748893f;	157.5
	float theta;
	register int i,j;
	int index;
	byte n,n1,n2;

	for( i = 1 ; i < height-1 ; ++i ){
		index = i*width;
		for( j = 1 ; j < width-1 ; ++j){
			n = tempImg[index + j];
			if( n > 5 ){
				theta = dirImg[index + j];
				if( theta <0 ) 
					theta += (float)PI;


				// 0도 
				if( theta < pi1_8 ||  pi7_8 <= theta ){				
					n1 = tempImg[ (i) * width + (j-1) ];
					n2 = tempImg[ (i) * width + (j+1) ];
					if( n > n1 && n > n2){
						nmImg[index+j] = tempImg[index+j];
					}else{
						if(n == n1 || n == n2){
							tempImg[ index+j ] -= 1;						
						}
						nmImg[index+j] = 0;
					}
				}
				// 45도 
				else if( pi1_8 <= theta && theta < pi3_8 ){			
					n1 = tempImg[ (i+1) * width + (j+1) ];
					n2 = tempImg[ (i-1) * width + (j-1) ];
					if( n > n1 && n > n2){ 
						nmImg[index+j] = tempImg[index+j];
					}else{
						if(n == n1 || n == n2){
							tempImg[ index+j ] -= 1;
						}
						nmImg[index+j] = 0;
					}
				}
				// 90도 
				else if( pi3_8 <= theta && pi5_8 > theta ){
					n1 = tempImg[ (i+1) * width + (j) ];
					n2 = tempImg[ (i-1) * width + (j) ];
					if( n > n1 && n > n2){
						nmImg[index+j] = tempImg[index+j];
					}else{
						if(n == n1 || n == n2){
							tempImg[ index+j ] -= 1;
						}
						nmImg[index+j] = 0;
					}
				}
				// 135도 방향일때 
				else if( pi5_8 <= theta && theta < pi7_8 ){			
					n1 = tempImg[ (i+1) * width + (j-1) ];
					n2 = tempImg[ (i-1) * width + (j+1) ];
					if( n > n1 && n > n2){
						nmImg[index+j] = tempImg[index+j];
					}else{
						if(n == n1 || n == n2){
							tempImg[ index+j ] -= 1;
						}
						nmImg[index+j] = 0;
					}
				}
			}
			else nmImg[index+j] = 0;
		}
	}
}

void CDOG::Hysteresis_Th(int low, int high)
{
	memset(tempImg,0,width*height);
	memset(hyImg,0,width*height);
	int cnt=0;
	register int i,j;
	int index;
	for( i = 0 ; i < height ; ++i ){
		index = i*width;
		for( j = 0 ; j < width ; ++j){

			if ( nmImg[index + j] > high && tempImg[index + j]==0){
				tempImg[index + j]=1;		
				danji(i, j, low);
			}
		}
	}
}

int CDOG::danji(int y, int x, int low)
{
	int i1;
	//x-1 y-1 
	i1 = (y-1)*width+x-1;
	if(y && x && (nmImg[i1] > low) && tempImg[i1]==0 )
	{
		hyImg[i1] = nmImg[i1];
		tempImg[i1] = 1;
		danji(y-1,x-1,low);
	}

	//x-1 y 
	i1 = (y)*width+x-1;
	if(x && (nmImg[i1] > low) && tempImg[i1]==0)
	{
		hyImg[i1] = nmImg[i1];
		tempImg[i1] = 1;
		danji(y,x-1,low);
	}

	//x-1 y+1 
	i1 = (y+1)*width+x-1;
	if(x && y<height-1 && (nmImg[i1] > low) && tempImg[i1]==0)
	{
		hyImg[i1] = nmImg[i1];
		tempImg[i1] = 1;
		danji(y+1,x-1,low);
	}
	return 1;
}
