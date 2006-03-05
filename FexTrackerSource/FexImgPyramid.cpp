// This file is part of FexTracker and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

// FexImgPyramid.cpp: implementation of the FexImgPyramid class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "FexImgPyramid.h"
#include "FexGenericFilter_Include.h"

void BaseFloatImage_GaussEdgeDetect( float* Img, int sizx, int sizy, float sigma, float* GradX, float* GradY );
void BaseFloatImage_GaussSmooth( float* Img, int sizx, int sizy, float sigma, float* Out );
void BaseFloatImage_LanczosRescale( float* in, int inSx, int inSy, float* out, int outSx, int outSy );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//turn off image debugging
#ifndef imdebug
#define imdebug // 
#endif


FexImgPyramidLevel::FexImgPyramidLevel( int isx, int isy )
{
	sx = isx;
	sy = isy;
	Img = new float[ sx*sy ];
	GradX = new float[ sx*sy ];
	GradY = new float[ sx*sy ];
}

FexImgPyramidLevel::~FexImgPyramidLevel()
{
	delete [] Img;
	delete [] GradX;
	delete [] GradY;
}

void FexImgPyramidLevel::Fill( float* iImg, float DetectSmoothSigma )
{
	imdebug("lum b=32f w=%d h=%d %p /255", sx, sy, iImg);
	BaseFloatImage_GaussSmooth( iImg, sx, sy, DetectSmoothSigma, Img );
	imdebug("lum b=32f w=%d h=%d %p /255", sx, sy, Img);
}

void FexImgPyramidLevel::Scale( FexImgPyramidLevel* old )
{
	imdebug("lum b=32f w=%d h=%d %p /255", old->sx, old->sy, old->Img);
	BaseFloatImage_LanczosRescale( old->Img, old->sx, old->sy, Img, sx, sy );
	imdebug("lum b=32f w=%d h=%d %p /255", sx, sy, Img);
}

void FexImgPyramidLevel::Calc( float EdgeDetectSigma )
{
	imdebug("lum b=32f w=%d h=%d %p /255", sx, sy, Img);
	BaseFloatImage_GaussEdgeDetect( Img, sx, sy, EdgeDetectSigma, GradX, GradY );
	imdebug("lum b=32f w=%d h=%d %p /2", sx, sy, GradX);
	imdebug("lum b=32f w=%d h=%d %p /2", sx, sy, GradY);
}



FexImgPyramid::FexImgPyramid( float* Img, int SizX, int SizY, float EdgeDetectSigma, float DetectSmoothSigma, int iSubsampling, int Levels )
{
	int i;
	
	Subsampling = iSubsampling;

	if( Levels == -1 ) Levels = 999;
	int mLvl = 0;
	int tsm = min(SizX,SizY);
	while( tsm>1 && tsm%2==0 )
	{
		tsm/=Subsampling;
		++mLvl;
	}
	if( Levels > mLvl ) Levels = mLvl;
	if( Levels < 1 ) Levels = 1;

	nLevels = Levels;
	lLevels = new FexImgPyramidLevel*[ nLevels ];

	lLevels[0] = new FexImgPyramidLevel( SizX, SizY );
	lLevels[0]->Fill( Img, DetectSmoothSigma );
	for( i=1;i<nLevels;i++ )
	{
		SizX /= Subsampling;
		SizY /= Subsampling;
		lLevels[i] = new FexImgPyramidLevel( SizX, SizY );
		lLevels[i]->Scale( lLevels[i-1] );
	}

	float cmul = 1.f;
	for( i=0;i<nLevels;i++ )
	{
		lLevels[i]->CoordMul = cmul;
		lLevels[i]->Calc( EdgeDetectSigma );
		cmul /= Subsampling;
	}
}

FexImgPyramid::~FexImgPyramid()
{
	for( int i=0;i<nLevels;i++ )
		delete lLevels[i];
	delete [] lLevels;
}
