// This file is part of FexTracker and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

// FexMovement.cpp: implementation of the FexMovement class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "stdio.h"
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
//#include <mmsystem.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FexMovement::FexMovement()
{
	FileName = 0;
}

FexMovement::~FexMovement()
{
	if( FileName ) delete []FileName;
}

FEXTRACKER_API FexMovement* CreateMovement()
{
	return new FexMovement();
}

FEXTRACKER_API void LoadMovement( FexMovement* me, const wchar_t* Filename )
{
	me->Frames.nVal = 0;

	me->FileName = new WCHAR[ wcslen(Filename)+1 ];
	wcscpy( me->FileName, Filename );

	FILE *fi = _wfopen( Filename, L"rt" );
	if( !fi ) return;
	int CurFeat = -1;
	char Line[512];
	while( !feof(fi) )
	{
		Line[0]=0;
		fgets( Line, 510, fi );
		if( !Line[0] ) break;
		FexMovementFrame f;
		int r = sscanf( Line, "(%f %f)(%f %f %f)(%f %f)", 
			&f.Pos.x, &f.Pos.y, 
			&f.Rot.x, &f.Rot.y, &f.Rot.z, 
			&f.Scale.x, &f.Scale.y );
		if( r != 7 ) continue;
		me->Frames.Add( f );
	}
	fclose( fi );
}
FEXTRACKER_API void SaveMovement( FexMovement* me, const wchar_t* Filename )
{
	FILE *fi = _wfopen( Filename, L"wt" );
	if( !fi ) return;
	for( int i=0;i<me->Frames.size();i++ )
	{
		const FexMovementFrame f = me->Frames[i];
		fprintf( fi, "(%f %f)(%f %f %f)(%f %f)\n", 
			f.Pos.x, f.Pos.y, 
			f.Rot.x, f.Rot.y, f.Rot.z, 
			f.Scale.x, f.Scale.y );
	}
	fclose( fi );
}

FEXTRACKER_API void DeleteMovement( FexMovement* delme )
{
	delete delme;
}
