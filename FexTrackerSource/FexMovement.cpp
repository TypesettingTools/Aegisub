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

FEXTRACKER_API void LoadMovement( FexMovement* me, const unsigned short* Filename )
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
FEXTRACKER_API void SaveMovement( FexMovement* me, const unsigned short* Filename )
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
/*
const WCHAR* FEXTRACKER_API GetUniqueName()
{
	static WCHAR Name[512];
    time_t long_time;
    time( &long_time );

    swprintf( Name, L"%x%x\n", timeGetTime(), long_time );

	for( DWORD i=0;i<wcslen(Name);i++ )
	{
		Name[i] = towlower( Name[i] );
		if( Name[i]<'a' || Name[i]>'z' )
			Name[i] = '_';
	}
	wcscat( Name, L".fexmove" );
	return Name;
}
*/
FEXTRACKER_API void DeleteMovement( FexMovement* delme )
{
	delete delme;
}
