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

	void Load( const char* Filename );
	void Save( const char* Filename );

	const char* GetUniqueName();

	tenlist<FexMovementFrame>	Frames;
};

void FEXTRACKER_API DeleteMovement( FexMovement* delme );

#endif // !defined(AFX_FEXMOVEMENT_H__63D8ADD8_4EA1_4C56_8D6F_7B587A1A61A4__INCLUDED_)
