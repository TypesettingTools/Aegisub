// This file is part of FexTracker and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

// FexTracker.cpp : Defines the entry point for the DLL application.
//

#include "StdAfx.h"
#include "stdio.h"

FexTracker::FexTracker( int sx, int sy, int inFeatures )
{
	printf( "[ using FexTracker (c)2006 Hajo Krabbenhoft ]\n" );

	nFeatures = inFeatures;
	minFeatures = 0;
	mFeatures = 8;
	lFeatures = (FexTrackingFeature*) new FexTrackingFeature[mFeatures];
	SizX = sx;
	SizY = sy;
	CurImg = 0;
	CurFrame = 0;

	bDebug = 0;

	float subsampling = float(Cfg.SearchRange) / min(Cfg.WindowX,Cfg.WindowY);

	if (subsampling < 1.0)  {		/* 1.0 = 0+1 */
		PyramidMaxLevels = 1;
	} else if (subsampling <= 3.0)  {	/* 3.0 = 2+1 */
		PyramidMaxLevels = 2;
		PyramidSubsampling = 2;
	} else if (subsampling <= 5.0)  {	/* 5.0 = 4+1 */
		PyramidMaxLevels = 2;
		PyramidSubsampling = 4;
	} else if (subsampling <= 9.0)  {	/* 9.0 = 8+1 */
		PyramidMaxLevels = 2;
		PyramidSubsampling = 8;
	} else {
		/* The following lines are derived from the formula:
		search_range = 
		window_halfwidth * \sum_{i=0}^{nPyramidLevels-1} 8^i,
		which is the same as:
		search_range = 
		window_halfwidth * (8^nPyramidLevels - 1)/(8 - 1).
		Then, the value is rounded up to the nearest integer. */
		float val = (float) (log(7.0*subsampling+1.0)/log(8.0));
		PyramidMaxLevels = (int) (val + 0.99);
		PyramidSubsampling = 8;
	}
}
FexTracker::~FexTracker()
{
	delete [] lFeatures;
	if( CurImg ) delete CurImg;
}

void FexTracker::ProcessImage( float *Img, bool bFirst )
{
	if( bFirst || !CurImg ) 
	{
		CurFrame = 0;
		CurImg = new FexImgPyramid( Img, SizX, SizY, Cfg.EdgeDetectSigma, Cfg.DetectSmoothSigma, PyramidSubsampling, PyramidMaxLevels );
		nActiveFeatures = 0;
		int tmp = nFeatures;
		nFeatures = 0;
		FindFeatures( tmp );
	}
	else
	{
		CountActiveFeatures();
		if( nActiveFeatures<minFeatures ) 
			FindFeatures( minFeatures );
 		NextImg = new FexImgPyramid( Img, SizX, SizY, Cfg.EdgeDetectSigma, Cfg.DetectSmoothSigma, PyramidSubsampling, PyramidMaxLevels );
		TrackFeatures();
		delete CurImg;
		CurImg = NextImg;
		NextImg = 0;
	}
	CurFrame++; 
}

void FexTracker::ProcessingDone()
{
	if( CurImg ) delete CurImg;
	CurImg = 0;
}

void FexTracker::CountActiveFeatures()
{
	nActiveFeatures = 0;
	for( int i=0;i<nFeatures;i++ )
	{
		if( lFeatures[i].StartTime + lFeatures[i].Pos.size() >= CurFrame )
			nActiveFeatures++;
	}
}

FexTrackingFeature* FexTracker::operator [] ( int i )
{ 
	if( i<0 || i>=nFeatures ) return 0; 
	return & lFeatures[i]; 
}

int FexTracker::GetEigenvalueForPoint( int px, int py )
{
	int sx = px - Cfg.WindowX;
	int ex = px + Cfg.WindowX;
	int sy = py - Cfg.WindowY;
	int ey = py + Cfg.WindowY;
	if( sx<0 )sx=0;
	if( sy<0 )sy=0;
	if( ex>SizX-1 )ex=SizX-1;
	if( ey>SizY-1 )ey=SizY-1;

	int imgSX = CurImg->lLevels[0]->sx;
	float* gradx = CurImg->lLevels[0]->GradX;
	float* grady = CurImg->lLevels[0]->GradY;

	register float gxx = 0, gyy = 0, gxy = 0;
	for( int y=sy;y<ey;y++ )
	{
		for( int x=sx;x<ex;x++ )
		{
			float gx = gradx[ imgSX*y + x ];
			float gy = grady[ imgSX*y + x ];
			gxx += gx*gx;
			gyy += gy*gy;
			gxy += gx*gy;
		}
	}

	float val = gxx + gyy - sqrtf((gxx - gyy)*(gxx - gyy) + 4*gxy*gxy);
	if( val>(1<<30) ) val=(1<<30);
	return (int) val;
}


typedef struct{
	int val, x, y;
}littleFeature;



#define SWAP3(list, i, j)               \
{register int *pi, *pj, tmp;            \
     pi=list+3*(i); pj=list+3*(j);      \
                                        \
     tmp=*pi;    \
     *pi++=*pj;  \
     *pj++=tmp;  \
                 \
     tmp=*pi;    \
     *pi++=*pj;  \
     *pj++=tmp;  \
                 \
     tmp=*pi;    \
     *pi=*pj;    \
     *pj=tmp;    \
}

void _quicksort(int *pointlist, int n)
{
  unsigned int i, j, ln, rn;

  while (n > 1)
  {
    SWAP3(pointlist, 0, n/2);
    for (i = 0, j = n; ; )
    {
      do
        --j;
      while (pointlist[3*j] < pointlist[0]);
      do
        ++i;
      while (i < j && pointlist[3*i] > pointlist[0]);
      if (i >= j)
        break;
      SWAP3(pointlist, i, j);
    }
    SWAP3(pointlist, j, 0);
    ln = j;
    rn = n - ++j;
    if (ln < rn)
    {
      _quicksort(pointlist, ln);
      pointlist += 3*j;
      n = rn;
    }
    else
    {
      _quicksort(pointlist + 3*j, rn);
      n = ln;
    }
  }
}
#undef SWAP3


void FexTracker::FindFeatures( int minFeatures )
{
	int nli=0;
	littleFeature *list = new littleFeature[SizX*SizY];
	for( int y=0;y<SizY;y++ )
	{
		for( int x=0;x<SizX;x++ )
		{
			int v = GetEigenvalueForPoint( x, y );
			if( v>0 )
			{
				list[nli].val = v;
				list[nli].x = x;
				list[nli].y = y;
				nli++;
			}
		}
	}

	_quicksort( (int*)list, nli );

	int oldN = nFeatures;

	for( int i=0;i<nli && nActiveFeatures<minFeatures;i++ )
	{
		int j;
		for( j=0;j<nFeatures;j++ )
		{
			if( lFeatures[j].StartTime + lFeatures[j].Pos.size() < CurFrame  ) continue; //feature was lost

			float dx = list[i].x - lFeatures[j].Pos[ CurFrame - lFeatures[j].StartTime ].x;
			float dy = list[i].y - lFeatures[j].Pos[ CurFrame - lFeatures[j].StartTime ].y;
			float sqr = dx*dx+dy*dy;
			if( sqr < Cfg.MinDistanceSquare ) break;
		}
		if( j!=nFeatures ) continue;

		if( nFeatures >= mFeatures )
		{
			mFeatures = nFeatures+9;
			mFeatures -= mFeatures%8;
			FexTrackingFeature * nlFeatures = (FexTrackingFeature*) new FexTrackingFeature[mFeatures];
			for( int cpy=0;cpy<nFeatures;cpy++ )
			{
				nlFeatures[ cpy ].Eigenvalue = lFeatures[ cpy ].Eigenvalue;
				nlFeatures[ cpy ].StartTime = lFeatures[ cpy ].StartTime;
				nlFeatures[ cpy ].Influence = lFeatures[ cpy ].Influence;
				for( int cpy2=0;cpy2<lFeatures[ cpy ].Pos.size();cpy2++ )
					nlFeatures[ cpy ].Pos.Add( lFeatures[ cpy ].Pos[cpy2] );
			}
			delete [] lFeatures;
			lFeatures = nlFeatures;
		}

		lFeatures[nFeatures].Eigenvalue = list[i].val;
		vec2 pt;
		pt.x = (float)list[i].x;
		pt.y = (float)list[i].y;
		lFeatures[nFeatures].Pos.Add( pt );
		lFeatures[nFeatures].StartTime = CurFrame;
		lFeatures[nFeatures].Influence = 0;
		nFeatures++;
		nActiveFeatures++;
	}

	for( int j=oldN;j<nFeatures;j++ )
		lFeatures[j].StartTime = max(0,lFeatures[j].StartTime-1);


	delete []list;
}

void FexTracker::TrackFeatures()
{
	for( int i=0;i<nFeatures;i++ )
	{
		if( lFeatures[i].StartTime + lFeatures[i].Pos.size() < CurFrame  ) continue; //feature was lost

		int FeatureFrame = CurFrame - lFeatures[i].StartTime;

		float orig_px = lFeatures[i].Pos[FeatureFrame-1].x;
		float orig_py = lFeatures[i].Pos[FeatureFrame-1].y;

		vec2 op;
		op.x = orig_px * CurImg->lLevels[ CurImg->nLevels-1 ]->CoordMul / CurImg->Subsampling;
		op.y = orig_py * CurImg->lLevels[ CurImg->nLevels-1 ]->CoordMul / CurImg->Subsampling;
		vec2 np;
		np = op;

		int l;
		for( l=CurImg->nLevels-1;l>=0;l-- )
		{
			op.x *= CurImg->Subsampling;
			op.y *= CurImg->Subsampling;
			np.x *= CurImg->Subsampling;
			np.y *= CurImg->Subsampling;
			if( !TrackOneFeature( l, op, np ) ) break;
		}
		if( l!=-1 ) continue; //we aborted

		if( np.x<0 || np.y<0 || np.x>SizX || np.y>SizY ) continue;

		lFeatures[i].Pos.Add( np );
	}
}

bool FexTracker::TrackOneFeature( int lvl, vec2 op, vec2& np )
{
	static float bordereps = 1.1f;
	if( op.x - Cfg.WindowX < bordereps || op.x + Cfg.WindowX > CurImg->lLevels[lvl]->sx - bordereps ) return 0;
	if( op.y - Cfg.WindowY < bordereps || op.y + Cfg.WindowY > CurImg->lLevels[lvl]->sy - bordereps ) return 0;
	if( np.x - Cfg.WindowX < bordereps || np.x + Cfg.WindowX > CurImg->lLevels[lvl]->sx - bordereps ) return 0;
	if( np.y - Cfg.WindowY < bordereps || np.y + Cfg.WindowY > CurImg->lLevels[lvl]->sy - bordereps ) return 0;

	int isx = (Cfg.WindowX*2+1);
	int imsiz = isx*(Cfg.WindowY*2+1);
	float *diff = new float[imsiz];
	float *gradx = new float[imsiz];
	float *grady = new float[imsiz];

	bool bOk = 1;
	for( int iteration=0;iteration<Cfg.MaxIterations;iteration++ )
	{
		GetDiffForPointset( lvl, op, np, diff );
		GetGradForPointset( lvl, op, np, gradx, grady );
/*
		imdebug("lum b=32f w=%d h=%d %p /255", isx, imsiz/isx, diff);
		imdebug("lum b=32f w=%d h=%d %p /255", isx, imsiz/isx, gradx);
		imdebug("lum b=32f w=%d h=%d %p /255", isx, imsiz/isx, grady);
*/
		register float gx, gy, di;

		float gxx = 0, gyy = 0, gxy = 0, ex = 0, ey = 0;
		for( int i=0;i<imsiz;i++ )
		{
			di = diff[i];
			gx = gradx[i];
			gy = grady[i];

			gxx += gx*gx;
			gyy += gy*gy;
			gxy += gx*gy;
			ex += di*gx;
			ey += di*gy;
		}

		float det = gxx*gyy - gxy*gxy;
		if( det < Cfg.MinDeterminant )
		{
			bOk = 0;
			break;
		}

		float dx = (gyy*ex - gxy*ey)/det;
		float dy = (gxx*ey - gxy*ex)/det;
		np.x += dx;
		np.y += dy;

		if( ( np.x - Cfg.WindowX < bordereps || np.x + Cfg.WindowX > CurImg->lLevels[lvl]->sx - bordereps )
		||  ( np.y - Cfg.WindowY < bordereps || np.y + Cfg.WindowY > CurImg->lLevels[lvl]->sy - bordereps ) )
		{
			bOk = 0;
			break;
		}

		if( fabs(dx) < Cfg.MinDisplacement && fabs(dy) < Cfg.MinDisplacement )break;
	}
	delete [] gradx;
	delete [] grady;

	if( bOk )
	{
		GetDiffForPointset( lvl, op, np, diff );

		float sum = 0;
		for( int i=0;i<imsiz;i++ )
			sum += fabsf( diff[i] );

		if( sum / float(imsiz) > Cfg.MaxResidue ) bOk = 0;
	}

	delete [] diff;
	return bOk;
}

inline float Interpolate( float *img, int ImgSX, float x, float y )
{
  int xt = (int) x;  /* coordinates of top-left corner */
  int yt = (int) y;
  float ax = x - xt;
  float ay = y - yt;
  float *ptr = img + (ImgSX*yt) + xt;

  return ( (1-ax) * (1-ay) * *ptr +
           ax   * (1-ay) * *(ptr+1) +
           (1-ax) *   ay   * *(ptr+(ImgSX)) +
           ax   *   ay   * *(ptr+(ImgSX)+1) );
}

void FexTracker::GetDiffForPointset( int lvl, vec2 op, vec2 np, float* diff )
{
	float* img1 = CurImg->lLevels[lvl]->Img;
	int isx1 = CurImg->lLevels[lvl]->sx;
	float* img2 = NextImg->lLevels[lvl]->Img;
	int isx2 = NextImg->lLevels[lvl]->sx;
	for( int y = -Cfg.WindowY; y <= Cfg.WindowY; y++ )
	{
		for( int x = -Cfg.WindowX; x <= Cfg.WindowX; x++ )
		{
			*diff++ = Interpolate(img1,isx1,op.x+x,op.y+y) - Interpolate(img2,isx2,np.x+x,np.y+y);
		}
	}
}
void FexTracker::GetGradForPointset( int lvl, vec2 op, vec2 np, float* gradx, float* grady )
{
	int isx = CurImg->lLevels[lvl]->sx;

	float* gx1 = CurImg->lLevels[lvl]->GradX;
	float* gx2 = NextImg->lLevels[lvl]->GradX;

	float* gy1 = CurImg->lLevels[lvl]->GradY;
	float* gy2 = NextImg->lLevels[lvl]->GradY;

	for( int y = -Cfg.WindowY; y <= Cfg.WindowY; y++ )
	{
		for( int x = -Cfg.WindowX; x <= Cfg.WindowX; x++ )
		{
			*gradx++ = Interpolate(gx1,isx,op.x+x,op.y+y) + Interpolate(gx2,isx,np.x+x,np.y+y);
			*grady++ = Interpolate(gy1,isx,op.x+x,op.y+y) + Interpolate(gy2,isx,np.x+x,np.y+y);
		}
	}
}


/*

  static float _minEigenvalue(float gxx, float gxy, float gyy)
{
  return (float) ((gxx + gyy - sqrt((gxx - gyy)*(gxx - gyy) + 4*gxy*gxy))/2.0f);
}

//gen eigenvalue matrix:
gxx = 0;  gxy = 0;  gyy = 0;
for (yy = y-window_hh ; yy <= y+window_hh ; yy++)
{
  for (xx = x-window_hw ; xx <= x+window_hw ; xx++)  
  {
    gx = *(gradx->data + ncols*yy+xx);
    gy = *(grady->data + ncols*yy+xx);
    gxx += gx * gx;
    gxy += gx * gy;
    gyy += gy * gy;
  }
}


//get eigenvalue number
val = _minEigenvalue(gxx, gxy, gyy);


for every frame:
for every feature:
through all pyramid levels from lowres to highres:

calculate diff, gradx, grady
gen eigenvalue matrix 
error vector = [gradx, grady]*imdiff

float det = gxx*gyy - gxy*gxy;
if (det < small)  return KLT_SMALL_DET;
*dx = (gyy*ex - gxy*ey)/det;
*dy = (gxx*ey - gxy*ex)/det;

add [dx,dy] to search position


*/

