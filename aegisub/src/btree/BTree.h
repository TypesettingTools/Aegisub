//////////////////////////////////////////////////////////////////
///
/// (C) 2007: ScalingWeb.com
///
/// Author: Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __BTree_h__
#define __BTree_h__

#include "BTreeElement.h"
#include <memory>

///
/// BTree auxiliary structures
///

// BTree pair element container
template< typename KeyT, typename DataT >
struct BTreePair
{
	BTreePair() {}

	BTreePair( const KeyT &setkey, const DataT &setdata )
	:	key( setkey ),
		data( setdata )
		{}

	BTreePair( const BTreeElement< KeyT, DataT > &elem )
	:	key( elem.key_ ),
		data( elem.data_ )
		{}

	KeyT key;
	DataT data;
};

// BTree key element container
template< typename KeyT, typename DataT >
struct BTreeKey
{
	BTreeKey() {}

	BTreeKey( const KeyT &setkey )
	:	key( setkey )
		{}

	BTreeKey( const KeyT &setkey, const DataT & )
	:	key( setkey )
		{}

	BTreeKey( const BTreeElement< KeyT, DataT > &elem )
	:	key( elem.key_ )
		{}

	KeyT key;
};

// BTree data element container
template< typename KeyT, typename DataT >
struct BTreeData
{
	BTreeData( const DataT &setdata )
	:	data( setdata )
		{}

	BTreeData( const KeyT &, const DataT &dat )
	:	data( dat )
		{}

	BTreeData( const BTreeElement< KeyT, DataT > &elem )
	:	data( elem.data_ )
		{}

	DataT data;
};

///
/// BTree iterators support. TSuper class must implement
/// push_back( ElemType &elem ) method.
///

template
<
	typename KeyT, 
	typename DataT,
	class TSuper,
	template < typename, typename > class TElement
>
class BTreeIterator : public TSuper
{
public:

	///
	/// Type of container element
	typedef TElement< KeyT, DataT > ElemType;
};

///
/// Template container for BTree search results
///

template
<
	typename KeyT, 
	typename DataT,
	template < typename > class TSuper,
	template < typename, typename > class TElement
>
class BTreeContainer : public TSuper< TElement< KeyT, DataT > >
{
public:

	///
	/// Type of container element
	typedef TElement< KeyT, DataT > ElemType;
};

///
/// Template container for BTree search results using 2-arg templates
///

template
<
	typename KeyT, 
	typename DataT,
	template < typename, typename > class TSuper,
	template < typename, typename > class TElement,
	template < typename > class TSuperAllocator = std::allocator
>
class BTreeContainerStd : public TSuper< TElement< KeyT, DataT >, 
							TSuperAllocator< TElement< KeyT, DataT > > >
{
public:

	///
	/// Type of container element
	typedef TElement< KeyT, DataT > ElemType;
};

///
/// Represents BTree condition for search
///

template< class TChecker >
class BTreeCondition : public TChecker
{
public:

	///
	/// This method returns true if passed param is valid and
	/// node search have to keep going.
	/// TChecker must implement: bool satisfy( const KeyT & ) method

	using TChecker::satisfy;
};

#include "BTreeAlgorithms.h"

#endif // __BTree_h__

