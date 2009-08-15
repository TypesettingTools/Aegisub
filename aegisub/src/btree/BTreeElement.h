//////////////////////////////////////////////////////////////////
///
/// (C) 2007: ScalingWeb.com
///
/// Author: Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __BTreeElement_h__
#define __BTreeElement_h__

///
/// Class BTreeElement
///

template< typename KeyT, typename DataT >
class BTreeElement
{
public:

	BTreeElement();
	BTreeElement( const KeyT &key, const DataT &data );
	BTreeElement( const KeyT &key, const DataT &data, int link );
	BTreeElement( const BTreeElement< KeyT, DataT > &copy );

	~BTreeElement();

// Data

	KeyT key_;
	DataT data_;
	int link_;
};

//	===================================================================
//	BTreeElement< KeyT, DataT >::BTreeElement

template< typename KeyT, typename DataT >
BTreeElement< KeyT, DataT >::BTreeElement() :
link_( 0 )
{}

//	===================================================================
//	BTreeElement< KeyT, DataT >::BTreeElement

template< typename KeyT, typename DataT >
BTreeElement< KeyT, DataT >::BTreeElement( const KeyT &key, const DataT &data ) :
key_( key ),
data_( data ),
link_( 0 )
{}

//	===================================================================
//	BTreeElement< KeyT, DataT >::BTreeElement

template< typename KeyT, typename DataT >
BTreeElement< KeyT, DataT >::BTreeElement( const KeyT &key, const DataT &data, int link )
:	key_( key ),
	data_( data ),
	link_( link )
{}

//	===================================================================
//	BTreeElement< KeyT, DataT >::BTreeElement

template< typename KeyT, typename DataT >
BTreeElement< KeyT, DataT >::BTreeElement( 
	const BTreeElement< KeyT, DataT > &copy )
:	key_( copy.key_ ),
	data_( copy.data_ ),
	link_( copy.link_ )
{}

//	===================================================================
//	BTreeElement< KeyT, DataT >::~BTreeElement

template< typename KeyT, typename DataT >
BTreeElement< KeyT, DataT >::~BTreeElement()
{}

#endif // __BTreeElement_h__

