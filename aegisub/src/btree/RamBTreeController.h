//////////////////////////////////////////////////////////////////
///
/// (C) 2007: ScalingWeb.com
///
/// Author: Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __RamBTreeController_h__
#define __RamBTreeController_h__

#include "BTreeNode.h"

///
/// BTreeController with RAM as storage implementation
///

template< int NodeSize, typename KeyT, typename DataT >
class RamBTreeController
{
public:

	typedef BTreeNode< NodeSize, KeyT, DataT > NodeType;

	RamBTreeController();
	virtual ~RamBTreeController();
	bool isOpen();

	inline void flushChanges();
	inline void releaseCache();

	inline void setMaxCacheSize( unsigned int cacheSize );
	inline unsigned int getMaxCacheSize();

	inline void releaseNode( int addr );
	inline bool nodeInCache( int addr );

	void assign( RamBTreeController *copy );

	void clear();

protected:

	// Storage related operations
	inline NodeType* newNode();
	inline void deleteNode( NodeType *node );
	inline bool loadNode( NodeType **node, int addr );
	inline int rootAddr();
	inline void rootAddr( int addr );

	void rclear( NodeType *node );

	void closeController();

	// Data
	NodeType *root_;

};

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::RamBTreeController

template< int NodeSize, typename KeyT, typename DataT >
RamBTreeController< NodeSize, KeyT, DataT >::RamBTreeController() : 
	root_( 0 )
{}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::RamBTreeController

template< int NodeSize, typename KeyT, typename DataT >
RamBTreeController< NodeSize, KeyT, DataT >::~RamBTreeController()
{
	clear();
}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::isOpen

template< int NodeSize, typename KeyT, typename DataT >
bool RamBTreeController< NodeSize, KeyT, DataT >::isOpen()
{
	return true;
}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::newNode

template< int NodeSize, typename KeyT, typename DataT >
typename RamBTreeController< NodeSize, KeyT, DataT >::NodeType* 
		RamBTreeController< NodeSize, KeyT, DataT >::newNode()
{
	NodeType *newnode = new NodeType();
	newnode->addr_ = ( int ) newnode;

	return newnode;
}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::deleteNode

template< int NodeSize, typename KeyT, typename DataT >
void RamBTreeController< NodeSize, KeyT, DataT >::deleteNode( NodeType* node )
{
	delete node;
}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::loadNode

template< int NodeSize, typename KeyT, typename DataT >
bool RamBTreeController< NodeSize, KeyT, DataT >::loadNode( NodeType **node, int addr )
{
	if ( !addr )
	{
		*node = 0;
		return true;
	}

	*node = ( NodeType* ) addr;
	return true;
}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::releaseCache

template< int NodeSize, typename KeyT, typename DataT >
void RamBTreeController< NodeSize, KeyT, DataT >::flushChanges()
{}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::releaseCache

template< int NodeSize, typename KeyT, typename DataT >
void RamBTreeController< NodeSize, KeyT, DataT >::releaseCache()
{}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::rootAddr

template< int NodeSize, typename KeyT, typename DataT >
int RamBTreeController< NodeSize, KeyT, DataT >::rootAddr()
{
	return ( int ) root_;
}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::closeController

template< int NodeSize, typename KeyT, typename DataT >
void RamBTreeController< NodeSize, KeyT, DataT >::closeController()
{}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::rootAddr

template< int NodeSize, typename KeyT, typename DataT >
void RamBTreeController< NodeSize, KeyT, DataT >::rootAddr( int addr )
{
	root_ = ( NodeType* ) addr;
}

template< int NodeSize, typename KeyT, typename DataT >
inline void RamBTreeController< NodeSize, KeyT, DataT >::setMaxCacheSize( unsigned int )
{}

template< int NodeSize, typename KeyT, typename DataT >
inline unsigned int RamBTreeController< NodeSize, KeyT, DataT >::getMaxCacheSize()
{}

template< int NodeSize, typename KeyT, typename DataT >
inline void RamBTreeController< NodeSize, KeyT, DataT >::releaseNode( int )
{}

template< int NodeSize, typename KeyT, typename DataT >
inline bool RamBTreeController< NodeSize, KeyT, DataT >::nodeInCache( int )
{
	return true;
}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::assign

template< int NodeSize, typename KeyT, typename DataT >
void RamBTreeController< NodeSize, KeyT, DataT >::assign( RamBTreeController *rhv )
{
	root_ = rhv->root_;
	rhv->root_ = 0;
}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::clear

template< int NodeSize, typename KeyT, typename DataT >
void RamBTreeController< NodeSize, KeyT, DataT >::clear()
{
	rclear( root_ );
	root_ = 0;
}

//	===================================================================
//	RamBTreeController< NodeSize, KeyT, DataT >::rclear

template< int NodeSize, typename KeyT, typename DataT >
void RamBTreeController< NodeSize, KeyT, DataT >::rclear( NodeType *node )
{
	if ( !node ) return;

	if ( node->less_ )
	{
		NodeType *less = 0;

		if ( loadNode( &less, node->less_ ) )
		{
			rclear( less );
		}
	}

	for ( int i = 0; i < node->count(); i++ )
	{
		if ( node->elems_[ i ].link_ )
		{
			NodeType *kid = 0;

			if ( loadNode( &kid, node->elems_[ i ].link_ ) )
			{
				rclear( kid );
			}
		}
	}

	delete node;
}

#endif // __RamBTreeController_h__
