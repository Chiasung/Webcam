// GaussSmooth.h: interface for the CDOG class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAUSSSMOOTH_H__6365D726_33AF_4BBD_94DF_FF492DDBD74C__INCLUDED_)
#define AFX_GAUSSSMOOTH_H__6365D726_33AF_4BBD_94DF_FF492DDBD74C__INCLUDED_

#include <windows.h>
#include <iostream>
#include <vector>


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


///============================================
using namespace std;

#define IN
#define OUT

typedef unsigned char byte;
#define null 0
#define PI				3.141592653589793238462643383279
#define ROUND(A)        ((A)<0.0?-((int)(-(A)+0.5)):((int)((A)+0.5)))

///============================================


class CDOG  
{
public:
	int danji(int y, int x,int low);
	void Hysteresis_Th(int low, int high);
	void NonMax();
	float* GetdirImg();
	byte* GetOutImg();
	byte* GetNonMax(){ return nmImg;}
	byte* GetHyster(){ return hyImg;}

	void Run(byte* orgImg);
	CDOG(int height, int width, float sigma,int low , int high);
	void Initialize(int height, int width, float sigma);
	CDOG();
	virtual ~CDOG();

private:
	byte *outImg;
	float *dirImg;
	float* cImg;
	float *rImg;
	byte *nmImg;	//nonmax
	byte *hyImg;	//hys

	byte *tempImg;	//In

	int wsize;
	void Filtering();
	float sigma;
	int low, high;

	float* gMask;
	void GaussMask();
	unsigned char *orgImg;
	int width;
	int height;
};

#endif // !defined(AFX_GAUSSSMOOTH_H__6365D726_33AF_4BBD_94DF_FF492DDBD74C__INCLUDED_)