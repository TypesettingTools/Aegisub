// This file is part of FexTracker and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

// FexTrackerMovement.cpp
//

#include "StdAfx.h"
#include "stdio.h"



void FexTracker::InfluenceFeatures( int Frame, float x, float y, float off )
{
	for( int i=0;i<nFeatures;i++ )
	{
		if( Frame < lFeatures[i].StartTime ) continue;

		int FeatureFrame = Frame - lFeatures[i].StartTime;
		
		if( lFeatures[i].Pos.size() < FeatureFrame  ) continue; //feature was lost

		vec2 p = lFeatures[i].Pos[ FeatureFrame ];

		float dist = sqrtf( (p.x-x)*(p.x-x)+(p.y-y)*(p.y-y) );
		float infl = 1/(dist+1);
		if( infl < 0.1 ) continue;
		lFeatures[i].Influence += infl * off;
		if( lFeatures[i].Influence<0 ) lFeatures[i].Influence = 0;
		if( lFeatures[i].Influence>1 ) lFeatures[i].Influence = 1;
	}
}



#define VEC2LEN(a) sqrtf( (a).x*(a).x+(a).y*(a).y )

FexMovement* FexTracker::GetMovement()
{
	FILE *log = fopen( "movementlog.txt", "wt" );

	int i;
	for( i=0;i<nFeatures;i++ )
	{
		if( lFeatures[i].Influence>0.01 )
			fprintf( log, "Feature(%.2f): %d - %d\n", lFeatures[i].Influence, lFeatures[i].StartTime, lFeatures[i].StartTime+lFeatures[i].Pos.size() );
	}

	FexMovement *m = new FexMovement();
	FexMovementFrame f;

	float FirstInfluenceSum = 0;
	vec2 FirstPos;
	FirstPos.x = FirstPos.y = 0;

	for( i=0;i<nFeatures;i++ )
	{
		if( 0 < lFeatures[i].StartTime ) continue;
		if( lFeatures[i].Pos.size() <= 0  ) continue;

		vec2 p = lFeatures[i].Pos[ 0 ];

		FirstPos.x += p.x * lFeatures[i].Influence;
		FirstPos.y += p.y * lFeatures[i].Influence;
		FirstInfluenceSum += lFeatures[i].Influence; 
	}
	FirstPos.x /= FirstInfluenceSum;
	FirstPos.y /= FirstInfluenceSum;

	float FirstLen = 1;


	vec2* MidOffset = (vec2*) new vec2[nFeatures];
	float* MidOffsetLen = (float*) new float[nFeatures];
	for( int Frame = 0; Frame < CurFrame; Frame ++ )
	{
		//set feature offset
		for( i=0;i<nFeatures;i++ )
		{
			if( Frame == lFeatures[i].StartTime ) 
			{
				MidOffset[i].x = FirstPos.x - lFeatures[i].Pos[0].x;
				MidOffset[i].y = FirstPos.y - lFeatures[i].Pos[0].y;
				//realOffLen / MidOffLen = FirstLen
				// => MidOffLen /= FirstLen;
				MidOffset[i].x /= FirstLen;
				MidOffset[i].y /= FirstLen;
				MidOffsetLen[i] = VEC2LEN( MidOffset[i] );
			}
		}

		//accumulate position
		float NextLen = 0;
		float NextInfluenceSum = 0;
		vec2 NextPos;
		NextPos.x = NextPos.y = 0;

		for( i=0;i<nFeatures;i++ )
		{
			if( Frame < lFeatures[i].StartTime ) continue;
			int FeatureFrame = Frame - lFeatures[i].StartTime;
			if( lFeatures[i].Pos.size() <= FeatureFrame  ) continue; //feature was lost

			vec2 p = lFeatures[i].Pos[ FeatureFrame ];

			NextPos.x += (p.x+MidOffset[i].x*FirstLen) * lFeatures[i].Influence;
			NextPos.y += (p.y+MidOffset[i].y*FirstLen) * lFeatures[i].Influence;
			NextInfluenceSum += lFeatures[i].Influence;
		}

		if( NextInfluenceSum > 0.01 )
		{
			NextPos.x /= NextInfluenceSum;
			NextPos.y /= NextInfluenceSum;
		}
		else 
			NextPos = FirstPos;  //take over last one

		for( i=0;i<nFeatures;i++ )
		{
			if( Frame < lFeatures[i].StartTime ) continue;
			int FeatureFrame = Frame - lFeatures[i].StartTime;
			if( lFeatures[i].Pos.size() <= FeatureFrame  ) continue; //feature was lost

			vec2 p = lFeatures[i].Pos[ FeatureFrame ];

			vec2 realMidOff;
			realMidOff.x = NextPos.x-p.x;
			realMidOff.y = NextPos.y-p.y;
			float realMidOffLen = VEC2LEN( realMidOff );

			NextLen += realMidOffLen/MidOffsetLen[i] *lFeatures[i].Influence;
		}

		if( NextInfluenceSum > 0.01 )
		{
			NextLen /= NextInfluenceSum;
		}
		else 
			NextLen = FirstLen;  //take over last one

		f.Pos = NextPos;
		f.Rot.x = 0;
		f.Rot.y = 0;
		f.Rot.z = 0;
		f.Scale.x = NextLen;
		f.Scale.y = NextLen;
		m->Frames.Add( f );

		FirstPos = NextPos;
		FirstLen = NextLen;
	}

	delete []MidOffset;
	delete []MidOffsetLen;

	fclose( log );

	return m;
}

