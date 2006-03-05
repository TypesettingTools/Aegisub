// This file is part of FexTracker and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FEXTRACKER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FEXTRACKER_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifndef AEGISUB
#ifdef FEXTRACKER_EXPORTS
#define FEXTRACKER_API __declspec(dllexport)
#else
#define FEXTRACKER_API __declspec(dllimport)
#endif
#else
#define FEXTRACKER_API 
#endif



class FEXTRACKER_API FexTrackerConfig
{
public:
	inline FexTrackerConfig() : 
		FeatureNumber(0),
		EdgeDetectSigma(1.f),
		WindowX(3), WindowY(3), 
		SearchRange(15),
		MaxIterations(10),
		MinDeterminant(0.01f),
		MinDisplacement(0.1f),
		MaxResidue(10.f),
		IgnoreLightning(0),
		DetectSmoothSigma(0.9f),
		MinDistanceSquare(100.f)
	{};

	int		FeatureNumber;

	int		WindowX, WindowY;				//static const int window_size = 7;
	int		SearchRange;

	float	DetectSmoothSigma;			//static const float pyramid_sigma_fact = 0.9f;
	float	EdgeDetectSigma;				//static const float grad_sigma = 1.0f;

	int		MaxIterations;				//static const int max_iterations = 10;
	float	MinDeterminant;			//static const float min_determinant = 0.01f;
	float	MinDisplacement;				//static const float min_displacement = 0.1f;
	float	MaxResidue;				//static const float max_residue = 10.0f;

	bool	IgnoreLightning;				//static const KLT_BOOL lighting_insensitive = FALSE;

	float	MinDistanceSquare;				//static const int mindist = 10;

				//static const int min_eigenvalue = 1;
				//static const float smooth_sigma_fact = 0.1f;
				//static const KLT_BOOL sequentialMode = FALSE;
				///* for affine mapping*/
				//static const int affineConsistencyCheck = -1;
				//static const int affine_window_size = 15;
				//static const int affine_max_iterations = 10;
				//static const float affine_max_residue = 10.0;
				//static const float affine_min_displacement = 0.02f;
				//static const float affine_max_displacement_differ = 1.5f;
};

typedef struct{
	float x, y;
}vec2;

typedef struct{
	float x, y, z;
}vec3;

class FexImgPyramid;
class FexTrackingFeature;
class FexMovement;

class FEXTRACKER_API FexTracker 
{
public:
	FexTracker( int sx, int sy, int nFeatures );
	~FexTracker();
//config
	FexTrackerConfig  Cfg;
//work
	void ProcessImage( float *Img, bool bFirst=0 ); //we assume grayscale image here
	void ProcessingDone();	// call after last call to ProcessImage to clear temporary storage

//point -> movement
	void InfluenceFeatures( int Frame, float x, float y, float off );
	FexMovement* GetMovement();

//feature access
	FexTrackingFeature* operator [] ( int i );
	inline int GetCount(){ return nFeatures; };
	inline int GetFrame(){ return CurFrame; };
	inline int GetSizeX(){ return SizX; };
	inline int GetSizeY(){ return SizY; };

	bool bDebug;
	int minFeatures;
private:
	int SizX, SizY;
	int PyramidSubsampling;
	int PyramidMaxLevels;
	FexImgPyramid*	CurImg;
	FexImgPyramid*	NextImg;

	void FindFeatures( int minFeatures );
	void TrackFeatures();

	bool TrackOneFeature( int lvl, vec2 op, vec2& np );
	int GetEigenvalueForPoint( int px, int py );
	void GetDiffForPointset( int lvl, vec2 op, vec2 np, float* diff );
	void GetGradForPointset( int lvl, vec2 op, vec2 np, float* gradx, float* grady );

	void CountActiveFeatures();

//result
	FexTrackingFeature*		lFeatures;
	int						nFeatures;
	int						nActiveFeatures;
	int						mFeatures;

	int CurFrame;
};



FEXTRACKER_API void FexBaseResize( float* out, int newx, int newy, float* in, int sizx, int sizy );
FEXTRACKER_API void GaussKernelWidths( float sigma, int *gauss_width, int *gaussderiv_width );
FEXTRACKER_API void GaussEdgeDetect( float* Img, int sizx, int sizy, float sigma, float* GradX, float* GradY );
FEXTRACKER_API void GaussSmooth( float* Img, int sizx, int sizy, float sigma, float* Out );


