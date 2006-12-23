// This file is part of FexTracker and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

// FexTracker.cpp : Defines the entry point for the DLL application.
//

#ifndef MIN
#define MIN(a,b) ((a)<(b))?(a):(b)
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b))?(a):(b)
#endif

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

	float subsampling = float(Cfg.SearchRange) / float(MIN(Cfg.WindowX,Cfg.WindowY));

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
	// Receive new image to track
	// This assumes it chronologically directly follows the previously processed image
	if( bFirst || !CurImg ) 
	{
		// First image in series
		// Initialise a few things
		CurFrame = 0;
		CurImg = new FexImgPyramid( Img, SizX, SizY, Cfg.EdgeDetectSigma, Cfg.DetectSmoothSigma, PyramidSubsampling, PyramidMaxLevels );
		nActiveFeatures = 0;
		int tmp = nFeatures;
		nFeatures = 0;
		// Find initial features
		FindFeatures( tmp );
	}
	else
	{
		// Check if we've lost too many features, and find some more if that's the case
		CountActiveFeatures();
		if( nActiveFeatures<minFeatures ) 
			FindFeatures( minFeatures );
		// Build image pyramid
 		NextImg = new FexImgPyramid( Img, SizX, SizY, Cfg.EdgeDetectSigma, Cfg.DetectSmoothSigma, PyramidSubsampling, PyramidMaxLevels );
		// Now correlate the features to the new image
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
		// If the feature has a known position for the active frame, it's active
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
	// Determine window in the image to process
	int sx = px - Cfg.WindowX;
	int ex = px + Cfg.WindowX;
	int sy = py - Cfg.WindowY;
	int ey = py + Cfg.WindowY;
	// Clip against the edges of the image
	if( sx<0 )sx=0;
	if( sy<0 )sy=0;
	if( ex>SizX-1 )ex=SizX-1;
	if( ey>SizY-1 )ey=SizY-1;

	// Stride for the image
	int imgSX = CurImg->lLevels[0]->sx;
	// Pointers to X and Y gradient vectors
	float* gradx = CurImg->lLevels[0]->GradX;
	float* grady = CurImg->lLevels[0]->GradY;

	// Accumulated entries into the correlation matrix [gxx gxy; gxy gyy]
	register float gxx = 0, gyy = 0, gxy = 0;
	// Loop over points inside the window
	for( int y=sy;y<ey;y++ )
	{
		for( int x=sx;x<ex;x++ )
		{
			// Get X and Y gradient values of this point
			float gx = gradx[ imgSX*y + x ];
			float gy = grady[ imgSX*y + x ];
			// Add to the matrix entries
			gxx += gx*gx;
			gyy += gy*gy;
			gxy += gx*gy;
		}
	}

	// Calculate the eigenvalue L for the correlation matrix
	// 0 = det([gxx-L gxy; gxy gyy-L]) = (gxx-L)(gyy-L) - gxy*gxy = L*L + L*(-gxx-gyy) + gxx*gyy - gxy*gxy
	// Only the smaller of the two eigenvalues has interest, and a factor 1/4 isn't relevant for comparison,
	// so this is the smallest solution to the second-order polynomial.
	float val = gxx + gyy - sqrtf((gxx - gyy)*(gxx - gyy) + 4*gxy*gxy);
	// Limit the value
	if( val>(1<<30) ) val=(1<<30);
	return (int) val;
}


// An int triple (?!) denoting a coordinate pair and an eigenvalue for that position
typedef struct{
	int val, x, y;
}littleFeature;


// Swap two triples of ints (in reality two littleFeature)
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

// Sort a list of int-triples (littleFeature structs)
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
	// Detect new features, so there's at least minFeatures available

	// First calculate eigenvalues for each pixel in the image...
	int nli=0; // Number of LIttle features
	littleFeature *list = new littleFeature[SizX*SizY];
	for( int y=0;y<SizY;y++ )
	{
		for( int x=0;x<SizX;x++ )
		{
			int v = GetEigenvalueForPoint( x, y );
			// ... if the eigenvalue for a pixel is larger than zero, include it in the list...
			if( v>0 )
			{
				list[nli].val = v;
				list[nli].x = x;
				list[nli].y = y;
				nli++;
			}
		}
	}

	// ... and sort the list
	_quicksort( (int*)list, nli );
	// I'll call these "interest points", since they're just candidates for features...

	int oldN = nFeatures;

	// Look through all newly found interest-points and add the most interesting to our
	// list of features, until we have at least minFeatures
	for( int i=0;i<nli && nActiveFeatures<minFeatures;i++ )
	{
		// Check if this interest point is too close to an existing feature, to avoid excessive clustering
		int j;
		for( j=0;j<nFeatures;j++ )
		{
			// Check that we didn't lose this feature
			if( lFeatures[j].StartTime + lFeatures[j].Pos.size() < CurFrame  ) continue;

			// Calculate distance between the interest point of the outer loop and this feature
			float dx = list[i].x - lFeatures[j].Pos[ CurFrame - lFeatures[j].StartTime ].x;
			float dy = list[i].y - lFeatures[j].Pos[ CurFrame - lFeatures[j].StartTime ].y;
			float sqr = dx*dx+dy*dy;
			// And see if it's close enough
			if( sqr < Cfg.MinDistanceSquare ) break;
		}
		if( j!=nFeatures ) continue; // Found an existing feature too close, so skip this interest point

		// Check if we need to allocate more space for features
		if( nFeatures >= mFeatures )
		{
			// Allocate new, larger feature list and copy old features into new list
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
			// ... finally replacing the old list
			delete [] lFeatures;
			lFeatures = nlFeatures;
		}

		// Add this interest point to the end of the feature list
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

	// Subtract 1 from the start time of all newly found features
	for( int j=oldN;j<nFeatures;j++ )
		lFeatures[j].StartTime = MAX(0,lFeatures[j].StartTime-1);


	delete []list;
}

void FexTracker::TrackFeatures()
{
	for( int i=0;i<nFeatures;i++ )
	{
		// Check if this feature was already lost in an earlier frame
		if( lFeatures[i].StartTime + lFeatures[i].Pos.size() < CurFrame  ) continue;

		int FeatureFrame = CurFrame - lFeatures[i].StartTime;

		float orig_px = lFeatures[i].Pos[FeatureFrame-1].x;
		float orig_py = lFeatures[i].Pos[FeatureFrame-1].y;

		vec2 op; // Original point
		// Calculate position of original point on top level of the pyramid
		op.x = orig_px * CurImg->lLevels[ CurImg->nLevels-1 ]->CoordMul / CurImg->Subsampling;
		op.y = orig_py * CurImg->lLevels[ CurImg->nLevels-1 ]->CoordMul / CurImg->Subsampling;
		vec2 np; // New point
		np = op; // Assume no motion initially

		int l;
		for( l=CurImg->nLevels-1;l>=0;l-- )
		{
			// Move coordinates one level down in the pyramid
			op.x *= CurImg->Subsampling;
			op.y *= CurImg->Subsampling;
			np.x *= CurImg->Subsampling;
			np.y *= CurImg->Subsampling;
			// And try to track the feature
			if( !TrackOneFeature( l, op, np ) ) break;
		}
		// Did the loop finish? If not, tracking failed and the feature was lost
		if( l!=-1 ) continue;

		// Tracked outside the frame? Feature lost.
		if( np.x<0 || np.y<0 || np.x>SizX || np.y>SizY ) continue;

		// Otherwise add the new position to the feature's point list
		lFeatures[i].Pos.Add( np );
	}
}

bool FexTracker::TrackOneFeature( int lvl, vec2 op, vec2& np )
{
	// Motion estimate one feature on one level in the image pyramid
	// @op (in) is the coordinate for the point on the previous frame
	// @np (out) is the coordinate for the new location of the point, based on the information in this level of the pyramid

	// Border epsilon, defines what is "too close" to the edge of the image
	static float bordereps = 1.1f;
	// Check that the point isn't already "almost outside" the image, and let tracking fail if it is
	if( op.x - Cfg.WindowX < bordereps || op.x + Cfg.WindowX > CurImg->lLevels[lvl]->sx - bordereps ) return 0;
	if( op.y - Cfg.WindowY < bordereps || op.y + Cfg.WindowY > CurImg->lLevels[lvl]->sy - bordereps ) return 0;
	if( np.x - Cfg.WindowX < bordereps || np.x + Cfg.WindowX > CurImg->lLevels[lvl]->sx - bordereps ) return 0;
	if( np.y - Cfg.WindowY < bordereps || np.y + Cfg.WindowY > CurImg->lLevels[lvl]->sy - bordereps ) return 0;

	// Temporary images for holding data in the window around the feature
	// Desired width of the window
	int isx = (Cfg.WindowX*2+1);
	// Desired number of pixels in the window
	int imsiz = isx*(Cfg.WindowY*2+1);
	// Simple difference between image data in window around current frame/position and next frame/position
	float *diff = new float[imsiz];
	// Something with gradients around old/new position
	float *gradx = new float[imsiz];
	float *grady = new float[imsiz];

	bool bOk = 1;
	// Iteratively obtain better precision motion estimation (FIXME)
	for( int iteration=0;iteration<Cfg.MaxIterations;iteration++ )
	{
		// Calculate diffs and gradients ((FIXME)
		GetDiffForPointset( lvl, op, np, diff );
		GetGradForPointset( lvl, op, np, gradx, grady );
/*
		imdebug("lum b=32f w=%d h=%d %p /255", isx, imsiz/isx, diff);
		imdebug("lum b=32f w=%d h=%d %p /255", isx, imsiz/isx, gradx);
		imdebug("lum b=32f w=%d h=%d %p /255", isx, imsiz/isx, grady);
*/

		// Calculate gradient correlation matrix and some other matrix related to gradients and differences
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

		// Too small determinant in the gradient corr. matrix? (what does that mean?)
		float det = gxx*gyy - gxy*gxy;
		if( det < Cfg.MinDeterminant )
		{
			bOk = 0;
			break;
		}

		// So apparently those two matrices together tell something about the movement
		float dx = (gyy*ex - gxy*ey)/det;
		float dy = (gxx*ey - gxy*ex)/det;
		np.x += dx;
		np.y += dy;

		// Check if the feature moved too close to a border, in which case it's lost
		if( ( np.x - Cfg.WindowX < bordereps || np.x + Cfg.WindowX > CurImg->lLevels[lvl]->sx - bordereps )
		||  ( np.y - Cfg.WindowY < bordereps || np.y + Cfg.WindowY > CurImg->lLevels[lvl]->sy - bordereps ) )
		{
			bOk = 0;
			break;
		}

		// If the feature didn't move enough in this iteration, assume its motion is properly estimated
		if( fabs(dx) < Cfg.MinDisplacement && fabs(dy) < Cfg.MinDisplacement )break;
	}
	delete [] gradx;
	delete [] grady;

	// I think this checks if there's too large a difference between the image at the current and the next frame
	// around the motion estimated feature
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
	// Bilinear interpolation between (x,y) and ((int)x,(int)y)
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
	// Calculate the difference between the current frame and the next frame
	// locally around the feature point
	// (using the currently motion estimated coordinates for the new position)
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
	// Dark magic
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

