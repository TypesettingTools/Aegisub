// FexTrackerMovement.cpp
//

#include "StdAfx.h"
#include "stdio.h"
//#include "mmsystem.h"



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

/*
void LineTracing( haAssLine *c, vec2& TracePos, vec2& TraceScale, int Frame, vec2 InScreenPlace )
{
	TracePos = vec2(0,0);
	TraceScale = vec2(0,0);

	vec2 Sum1Pos = vec2(0,0);
	vec2 Sum2Pos = vec2(0,0);
	float TraceScaleV=0;

	float wsum = 0;

	for( int i=0;i<c->FeatureIdList.GetSize();i++ )
	{
		tenFeatureLink l = c->FeatureIdList[i];
		tenFeature f = Trace_GetFeatureByID( l.id, Frame );
		if( f.id < 0 || l.weight<0.01 )
			continue;
		Sum1Pos += vec2( 
			(f.x)/float(ncols)*float(AssRes[0]),
			(f.y)/float(nrows)*float(AssRes[1])
			)*l.weight;
		Sum2Pos += vec2( 
			(l.sub.x)/float(ncols)*float(AssRes[0]),
			(l.sub.y)/float(nrows)*float(AssRes[1])
			)*l.weight;
		wsum += l.weight;
	}
	if( wsum>0.01 )
	{
		Sum1Pos /= wsum;
		Sum2Pos /= wsum;
		TracePos = Sum1Pos - Sum2Pos;

		wsum = 0;
		CArray<float, float&> DistList;
		CArray<float, float&> WeightList;

//now we know weighted mid point, gen vectors from points to mid
		for( int i=0;i<c->FeatureIdList.GetSize();i++ )
		{
			tenFeatureLink l = c->FeatureIdList[i];
			tenFeature f = Trace_GetFeatureByID( l.id, Frame );
			if( f.id < 0 || l.weight<0.01  )
				continue;
			vec2 mid = vec2( 
				(f.x)/float(ncols)*float(AssRes[0]),
				(f.y)/float(nrows)*float(AssRes[1])
				);
			vec2 midToMe = mid - Sum1Pos;
			vec2 stdMidToMe = vec2( 
				(l.sub.x)/float(ncols)*float(AssRes[0]),
				(l.sub.y)/float(nrows)*float(AssRes[1])
				) - Sum2Pos;

			float stdlen = sqrtf( stdMidToMe.x*stdMidToMe.x + stdMidToMe.y*stdMidToMe.y );
			if( stdlen < 3 )
			{//too much error amplification.skip
				continue;
			}

			float len = sqrtf( midToMe.x*midToMe.x + midToMe.y*midToMe.y );
			float scale = len/stdlen;
	//		TRACE( "%f\n", scale );
			float addme = scale*l.weight;
			TraceScaleV += addme;
			DistList.Add( scale );
			WeightList.Add( l.weight );
			wsum += l.weight;
		}
		TraceScaleV /= wsum;

		float TraceScaleV2=0;
		for( i=0;i<DistList.GetSize();i++ )
		{
			TraceScaleV2 += ((DistList[i]-TraceScaleV)*0.5f+TraceScaleV)*WeightList[i];
		}
		TraceScaleV2 /= wsum;

		TraceScale = vec2(1,1)*TraceScaleV2;

		vec2 MidToItemStd = InScreenPlace - Sum2Pos;
		TracePos += MidToItemStd*(TraceScaleV2-1);
	}
	else
	{
		TracePos = vec2(0,0);
		TraceScale = vec2(1,1);
	}
}
*/

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

