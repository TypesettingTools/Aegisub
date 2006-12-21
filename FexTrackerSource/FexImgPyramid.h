// This file is part of FexTracker and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

// FexImgPyramid.h: interface for the FexImgPyramid class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEXIMGPYRAMID_H__B289955E_6660_483D_AF51_CE78F2D03944__INCLUDED_)
#define AFX_FEXIMGPYRAMID_H__B289955E_6660_483D_AF51_CE78F2D03944__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class FexImgPyramidLevel
{
public:
	FexImgPyramidLevel( int sx, int sy );
	~FexImgPyramidLevel();

	int sx, sy; // dimensions of this level
	float CoordMul;

	float*	Img; // actual image data
	float*	GradX; // X gradients
	float*	GradY; // Y gradients

	void Fill( float* Img, float DetectSmoothSigma );
	void Scale( FexImgPyramidLevel* old );
	void Calc( float EdgeDetectSigma );
};

class FexImgPyramid  
{
public:
	FexImgPyramid( float* Img, int SizX, int SizY, float EdgeDetectSigma, float DetectSmoothSigma, int Subsampling, int Levels );
	~FexImgPyramid();

	int Subsampling;

	FexImgPyramidLevel**	lLevels;
	int						nLevels;
};

#endif // !defined(AFX_FEXIMGPYRAMID_H__B289955E_6660_483D_AF51_CE78F2D03944__INCLUDED_)
