//////////////////////////////////////////////////////////////////
///
/// (C) 2007: ScalingWeb.com
///
/// Author: Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __BTreeNode_h__
#define __BTreeNode_h__

#include "BTreeElement.h"

///
/// Represents one BTree node in a storage
///

#pragma pack( 1 )
template< int NodeSize, typename KeyT, typename DataT >
class BTreeNode
{
public:

	enum Flags
	{
		NodeChanged = 1
	};

	typedef BTreeElement< KeyT, DataT > ElemType;

	BTreeNode();
	~BTreeNode();

	inline int add( const KeyT &key, const DataT &data );
	int add( const ElemType &elem );

	void removeAt( int index );
	void removeAt( int index, ElemType &removed );

	void clear();

	bool find( const KeyT &key, int &index ) const;

	bool hasChilds() const;
	bool isEmpty() const;
	bool isFull() const;

	int count() const;

	enum { maxKeys = NodeSize };

// Data

	int addr_;

	int less_;
	ElemType elems_[ NodeSize ];
	int size_;
	int parent_;
	uint8_t flags_;
};
#pragma pack()

#include "string.h"

//	===================================================================
//	BTreeNode< NodeSize, KeyT, DataT >::BTreeNode

template< int NodeSize, typename KeyT, typename DataT >
BTreeNode< NodeSize, KeyT, DataT >::BTreeNode()
:	addr_( 0 ),
	less_( 0 ),
	size_( 0 ),
	parent_( 0 ),
	flags_( 0 )
{
	// Clear RAM garbage
	::memset( &elems_, 0, sizeof( ElemType )*NodeSize );
}

//	===================================================================
//	BTreeNode< NodeSize, KeyT, DataT >::~BTreeNode

template< int NodeSize, typename KeyT, typename DataT >
BTreeNode< NodeSize, KeyT, DataT >::~BTreeNode()
{}

//	===================================================================
//	BTreeNode< NodeSize, KeyT, DataT >::add

template< int NodeSize, typename KeyT, typename DataT >
inline int BTreeNode< NodeSize, KeyT, DataT >::add( const KeyT &key, const DataT &data )
{
	bool added = false;

	int i = 0;
	for ( i = 0; i < size_; i++ )
	{
		if ( elems_[ i ].key_ > key )
		{
			// Move all greater elems
			::memmove( elems_ + i + 1, elems_ + i, 
				( size_ - i )*sizeof( ElemType ) );

			// Assign new value
			elems_[ i ].key_ = key;
			elems_[ i ].data_ = data;
			elems_[ i ].link_ = 0;
			added = true;
			break;
		}
	}

	if ( !added )
	{
		elems_[ size_ ].key_ = key;
		elems_[ size_ ].data_ = data;
		i = size_;
	}

	size_++;
	flags_ = NodeChanged;
	return i;
}

//	===================================================================
//	BTreeNode< NodeSize, KeyT, DataT >::add

template< int NodeSize, typename KeyT, typename DataT >
int BTreeNode< NodeSize, KeyT, DataT >::add( const typename ElemType &elem )
{
	int index = add( elem.key_, elem.data_ );
	elems_[ index ].link_ = elem.link_;
	return index;
}

//	===================================================================
//	BTreeNode< NodeSize, KeyT, DataT >::removeAt

template< int NodeSize, typename KeyT, typename DataT >
void BTreeNode< NodeSize, KeyT, DataT >::removeAt( int index )
{
	elems_[ index ] = ElemType();
	::memmove( elems_ + index, elems_ + index + 1, 
		( size_ - index - 1 )*sizeof( ElemType ) );

	size_--;
	flags_ = NodeChanged;
}

//	===================================================================
//	BTreeNode< NodeSize, KeyT, DataT >::removeAt

template< int NodeSize, typename KeyT, typename DataT >
void BTreeNode< NodeSize, KeyT, DataT >::removeAt( int index, 
									typename ElemType &removed )
{
	removed = elems_[ index ];
	elems_[ index ] = ElemType();

	::memmove( elems_ + index, elems_ + index + 1, 
		( size_ - index - 1 )*sizeof( ElemType ) );

	size_--;
	flags_ = NodeChanged;
}

//	===================================================================
//	BTreeNode< NodeSize, KeyT, DataT >::clear

template< int NodeSize, typename KeyT, typename DataT >
void BTreeNode< NodeSize, KeyT, DataT >::clear()
{
	for ( int i = 0; i < size_; i++ )
	{
		elems_[ i ] = ElemType();
	}

	size_ = 0;
	flags_ = 0;
}

//	===================================================================
//	BTreeNode< NodeSize, KeyT, DataT >::find

template< int NodeSize, typename KeyT, typename DataT >
bool BTreeNode< NodeSize, KeyT, DataT >::find( const KeyT &key, int &index ) const
{
	int lb = 0, ub = size_ - 1;

	while( lb <= ub )
	{
		index = ( lb + ub ) >> 1;

		if( key < elems_[ index ].key_ )
		{
			ub = index - 1;
		}
		else
		{
			if( key > elems_[ index ].key_ )
			{
				lb = index + 1;
			}
			else
			{
				return true;
			}
		}
	}

	index = lb;
	return false;
}

//	===================================================================
//	BTreeNode< NodeSize, KeyT, DataT >::hasChilds

template< int NodeSize, typename KeyT, typename DataT >
bool BTreeNode< NodeSize, KeyT, DataT >::hasChilds() const
{
	return less_ != 0;
}

//	===================================================================
//	BTreeNode< NodeSize, KeyT, DataT >::isEmpty

template< int NodeSize, typename KeyT, typename DataT >
bool BTreeNode< NodeSize, KeyT, DataT >::isEmpty() const
{
	return size_ == 0;
}

//	===================================================================
//	BTreeNode< NodeSize, KeyT, DataT >::isFull

template< int NodeSize, typename KeyT, typename DataT >
bool BTreeNode< NodeSize, KeyT, DataT >::isFull() const
{
	return size_ == maxKeys;
}

//	===================================================================
//	BTreeNode< NodeSize, KeyT, DataT >::count

template< int NodeSize, typename KeyT, typename DataT >
int BTreeNode< NodeSize, KeyT, DataT >::count() const
{
	return size_;
}

#endif // __BTreeNode_h__

