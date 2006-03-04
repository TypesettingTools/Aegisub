// This file is part of FexTracker and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

#pragma once

template< class type >
	class tenlist {
public:
	int		nVal;
	int		mVal;
	type*	lVal;

	inline tenlist()	
	{
		mVal = 0;
		nVal = 0;
		lVal = 0; 
		//zero everything since we well over-zero it anyway
	}
	inline ~tenlist()	
	{
		free( lVal );
	}

	inline int size()	
	{
		return nVal;
	}

	inline void Add( type t )
	{
		if( nVal+1 >= mVal )
		{
			mVal += 8;
			lVal = (type*)realloc( lVal, sizeof(type)*mVal );
			memset( lVal+nVal, 0x00, sizeof(type)*(mVal-nVal) );  //lVal+nVal, since it'll be multiplied by sizeof(type) due to lVal being a type*
		}
		lVal[nVal++] = t;
	}

	inline void AddStr( type t )
	{
		if( nVal+1 >= mVal )
		{
			mVal += 8;
			lVal = (type*)realloc( lVal, sizeof(type)*mVal );
			memset( lVal+nVal, 0x00, sizeof(type)*(mVal-nVal) );  //lVal+nVal, since it'll be multiplied by sizeof(type) due to lVal being a type*
		}
		strcpy( lVal[nVal++], t );
	}

	inline void Rem( int n )
	{
		if( n>=nVal )
		{
			nVal = 0;
			return;
		}

		for( int i=0;i<nVal;i++ )
		{
			lVal[i] = lVal[i+n];
		}
	}

	type& operator[]( int i ) const
	{ return lVal[i]; }
};
