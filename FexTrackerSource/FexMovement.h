// This file is part of FexTracker and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

// FexMovement.h: interface for the FexMovement class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEXMOVEMENT_H__63D8ADD8_4EA1_4C56_8D6F_7B587A1A61A4__INCLUDED_)
#define AFX_FEXMOVEMENT_H__63D8ADD8_4EA1_4C56_8D6F_7B587A1A61A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct
{
	vec2 Pos;
	vec3 Rot;
	vec2 Scale;
}FexMovementFrame;

#include "tenlist.h"

class FexMovement  
{
public:
	FexMovement();
	~FexMovement();
	wchar_t* FileName;

	tenlist<FexMovementFrame>	Frames;
};

FEXTRACKER_API FexMovement* CreateMovement();
FEXTRACKER_API void LoadMovement( FexMovement* me, const wchar_t* Filename );
FEXTRACKER_API void SaveMovement( FexMovement* me, const wchar_t* Filename );
FEXTRACKER_API void DeleteMovement( FexMovement* delme );

#endif // !defined(AFX_FEXMOVEMENT_H__63D8ADD8_4EA1_4C56_8D6F_7B587A1A61A4__INCLUDED_)
