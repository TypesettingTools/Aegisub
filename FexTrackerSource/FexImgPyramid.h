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

	int sx, sy;
	float CoordMul;

	float*	Img;
	float*	GradX;
	float*	GradY;

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
