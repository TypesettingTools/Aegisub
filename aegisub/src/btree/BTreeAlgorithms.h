//////////////////////////////////////////////////////////////////
///
/// (C) 2007: ScalingWeb.com
///
/// Author: Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __BTreeAlgorithms_h__
#define __BTreeAlgorithms_h__

#include "BTreeNode.h"

///
/// BTree algorithms implementation
///

template
<
	int NodeSize,
	typename KeyT,
	typename DataT,
	template < int, typename, typename > class TController
>
class BTreeAlgorithms : public TController< NodeSize, KeyT, DataT >
{
public:

	typedef BTreeNode< NodeSize, KeyT, DataT > NodeType;
	enum { nodeSize = NodeSize };

	BTreeAlgorithms();
	~BTreeAlgorithms();

	///
	/// Add new key:data pair
	bool add( const KeyT &key, const DataT &data );

	///
	/// Remove pair by key
	bool remove( const KeyT &key );

	///
	/// Remove pair by key and return associated data
	bool remove( const KeyT &key, DataT &data );

	///
	/// Find data by key, or return false if key was not found
	bool find( const KeyT &key, DataT &data );

	///
	/// Find all specified data
	template< class TChecker, class TContainer >
	bool search( TContainer &retList, const KeyT &startKey, 
		bool preciseSearch, TChecker &condition );

	///
	/// Get all specified in container data
	template< class TContainer >
	bool getAll( TContainer &retList );

	///
	/// Change data which belongs to the key is specified
	void changeData( const KeyT &key, const DataT &newData );

	///
	/// Close BTree and release all occupied resources
	void close();

protected:

	int getMedian( NodeType *node, BTreeElement< KeyT, DataT > &elem );

	void splitNodeByKey( NodeType **fullNode, KeyT &key );
	bool splitNode( NodeType *node, 
		BTreeElement< KeyT, DataT > &median, NodeType **nodeRight );

	NodeType* findNode( NodeType *node, const KeyT &key, int &retIndex, 
		int &parentIndex, bool &found );

	bool rebalance( NodeType *node, int parentIndex );
	bool combine( NodeType *leftNode, NodeType *rightNode );
	bool pullOut( NodeType *node, int itemIndex );

	NodeType* rightMost( NodeType *subtree, KeyT &largestKey, DataT &largestData );
	NodeType* leftMost( NodeType *subtree, KeyT &smallestKey, DataT &smallestData );

	// Internal search methods
	template < class TChecker, class TContainer >
	bool allKeys( NodeType *node, TContainer &retList, int elemIndex, 
		const KeyT &startKey, TChecker &condition );
	template < class TChecker, class TContainer >
	bool allKeys( NodeType *node, TContainer &retList, TChecker &condition );
	template< class TContainer >
	bool allKeys( NodeType *node, TContainer &retList );

};

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::BTreeAlgorithms

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::BTreeAlgorithms()
{}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::~BTreeAlgorithms

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::~BTreeAlgorithms()
{
	close();
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::add

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::add( const KeyT &key, 
												const DataT &data )
{
	if ( !isOpen() ) return false;

	if ( !root_ )
	{
		int addr = rootAddr();
		if ( !addr )
		{
			root_ = newNode();
			root_->flags_ = NodeType::NodeChanged;
			rootAddr( root_->addr_ );
		}
		else
		{
			if ( !loadNode( &root_, addr ) )
			{
				return false;
			}
		}
	}

	if ( !root_ ) return false;

	// Find targeted node
	bool found = false;
	KeyT copyKey( key );
	DataT copyData( data );
	int retIndex = -1, parentIndex = 0;

	NodeType *node = findNode( root_, copyKey, retIndex, parentIndex, found );

	if ( found || !node )
	{
		// Duplicate keys are not accepted
		releaseCache();
		return false;
	}

	if ( !node->isFull() )
	{
		// Just add to not full node
		node->add( copyKey, copyData );
		releaseCache();
		return true;
	}
	else
	{
		BTreeElement< KeyT, DataT > median;
		median.key_ = key;
		median.data_ = data;
		NodeType *nodeRight = 0;
		NodeType *parent = 0;
		if ( !loadNode( &parent, node->parent_ ) )
		{
			releaseCache();
			return false;
		}

		// Split node and get median
		if ( !splitNode( node, median, &nodeRight ) )
		{
			releaseCache();
			return false;
		}

		while ( 0 != parent )
		{
			// Add median to the parent
			if ( parent->isFull() )
			{
				if ( !splitNode( parent, median, &nodeRight ) )
				{
					releaseCache();
					return false;
				}

				node = parent;

				// Move up
				if ( !loadNode( &parent, parent->parent_ ) )
				{
					releaseCache();
					return false;
				}
			}
			else
			{
				parent->add( median );
				parent->flags_ = NodeType::NodeChanged;
				nodeRight->parent_ = parent->addr_;
				nodeRight->flags_ = NodeType::NodeChanged;
				break;
			}
		}

		if ( !parent )
		{
			// The root node!
			root_ = newNode();
			rootAddr( root_->addr_ );
			node->parent_ = root_->addr_;
			node->flags_ = NodeType::NodeChanged;
			nodeRight->parent_ = root_->addr_;
			nodeRight->flags_ = NodeType::NodeChanged;

			root_->add( median.key_, median.data_ );
			root_->elems_[ 0 ].link_ = nodeRight->addr_;
			root_->less_ = node->addr_;
		}
	}

	releaseCache();
	return true;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::remove

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::remove( const KeyT &key )
{
	if ( !root_ )
	{
		return false;
	}

	// Find targeted node
	bool found = false;
	int retIndex = -1, parentIndex = 0;

	NodeType *node = findNode( root_, key, retIndex, parentIndex, found );

	if ( !found || !node )
	{
		// Key not found
		releaseCache();
		return false;
	}

	if ( !node->hasChilds() )
	{
		node->removeAt( retIndex );
		if ( !rebalance( node, parentIndex ) )
		{
			releaseCache();
			return false;
		}
	}
	else
	{
		if ( !pullOut( node, retIndex ) )
		{
			releaseCache();
			return false;
		}
	}

	releaseCache();
	return true;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::remove

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::remove( const KeyT &key, 
														DataT &data )
{
	if ( !root_ )
	{
		return false;
	}

	// Find targeted node
	bool found = false;
	int retIndex = -1, parentIndex = 0;

	NodeType *node = findNode( root_, key, retIndex, parentIndex, found );

	if ( !found || !node )
	{
		// Key not found
		releaseCache();
		return false;
	}
	else
	{
		data = node->elems_[ retIndex ].data_;
	}

	if ( !node->hasChilds() )
	{
		node->removeAt( retIndex );
		if ( !rebalance( node, parentIndex ) )
		{
			releaseCache();
			return false;
		}
	}
	else
	{
		if ( !pullOut( node, retIndex ) )
		{
			releaseCache();
			return false;
		}
	}

	releaseCache();
	return true;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::find

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::find( const KeyT &key, 
								DataT &data )
{
	if ( !root_ )
	{
		return false;
	}

	int index = 0, parentIndex = 0;
	bool found = false;
	NodeType *node = findNode( root_, key, index, parentIndex, found );

	if ( !node || index < 0 )
	{
		releaseCache();
		return false;
	}

	if ( found )
	{
		// Key found
		data = node->elems_[ index ].data_;
		releaseCache();
		return true;
	}

	releaseCache();

	// Key not found
	return false;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::search

template < int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
template < class TChecker, class TContainer >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::search( 
		TContainer &retList, const KeyT &startKey, bool preciseSearch,
		TChecker &condition )
{
	retList.clear();

	// Find targeted node
	bool found = false;
	int retIndex = -1, parentIndex = 0;
	NodeType *node = findNode( root_, startKey, retIndex, parentIndex, found );

	if ( ( preciseSearch && !found ) || !node )
	{
		releaseCache();
		return false;
	}

	if ( !allKeys( node, retList, retIndex, startKey, condition ) )
	{
		releaseCache();
		return false;
	}

	releaseCache();
	return true;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::getAll

template < int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
template < class TContainer >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::getAll( TContainer &retList )
{
	if ( !allKeys( root_, retList ) )
	{
		releaseCache();
		return false;
	}

	releaseCache();
	return true;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::changeData

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
void BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::changeData( const KeyT &key, 
											const DataT &newData )
{
	if ( !root_ )
	{
		return;
	}

	int index = 0, parentIndex = 0;
	bool found = false;
	NodeType *node = findNode( root_, key, index, parentIndex, found );

	if ( !node || index < 0 )
	{
		releaseCache();
		return;
	}

	if ( found )
	{
		// Key found
		node->elems_[ index ].data_ = newData;
		node->flags_ = NodeType::NodeChanged;
		releaseCache();
		return;
	}

	releaseCache();
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::allKeys

template < int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
template < class TChecker, class TContainer >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::allKeys( 
		NodeType *node, TContainer &retList, int elemIndex,
		const KeyT &startKey, TChecker &condition )
{
	if ( !node ) return true;

	if ( -1 == elemIndex )
	{
		node->find( startKey, elemIndex );
		if ( -1 == elemIndex )
		{
			return false;
		}
	}

	int i = elemIndex;
	//if ( i > 0 ) i--;

	while ( i < node->size_ )
	{
		if ( condition.satisfy( node->elems_[ i ].key_ ) )
		{
			retList.push_back( TContainer::ElemType( node->elems_[ i ] ) );

			if ( node->elems_[ i ].link_ )
			{
				// Examine each child item
				NodeType *child = 0;
				bool wasInCache = nodeInCache( node->elems_[ i ].link_ );
				if ( !loadNode( &child, node->elems_[ i ].link_ ) ) return false;
				allKeys( child, retList, condition );
				if ( child && !wasInCache ) releaseNode( child->addr_ );
			}
		}
		else
		{
			break;
		}

		// Move to node end
		i++;
	}

	if ( !node->parent_ )
	{
		// End of search has reached
		return true;
	}

	// Move to up
	NodeType *parent = 0;
	bool wasInCache = nodeInCache( node->parent_ );
	if ( !loadNode( &parent, node->parent_ ) )
	{
		return false;
	}

	bool ret = allKeys( parent, retList, -1, startKey, condition );
	if ( parent && !wasInCache ) releaseNode( parent->addr_ );
	return ret;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::allKeys

template < int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
template < class TChecker, class TContainer >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::allKeys( 
	NodeType *node, TContainer &retList, TChecker &condition )
{
	if ( !node ) return true;

	if ( node->less_ )
	{
		NodeType *less = 0;

		bool wasInCache = nodeInCache( node->less_ );
		if ( !loadNode( &less, node->less_ ) ) return false;
		if ( !allKeys( less, retList ) )
		{
			// End of the condition had reached
			return false;
		}

		if ( less && !wasInCache ) releaseNode( less->addr_ );
	}

	for ( int i = 0; i < node->size_; i++ )
	{
		// Apply condition
		if ( !condition.satisfy( node->elems_[ i ].key_ ) )
		{
			// Stop key reached
			return false;
		}

		retList.push_back( TContainer::ElemType( node->elems_[ i ] ) );

		if ( node->elems_[ i ].link_ )
		{
			NodeType *kid = 0;
			bool wasInCache = nodeInCache( node->elems_[ i ].link_ );
			if ( !loadNode( &kid, node->elems_[ i ].link_ ) ) return false;
			if ( !allKeys( kid, retList ) )
			{
				// End of tree or condition had reached
				return false;
			}
			if ( kid && !wasInCache ) releaseNode( kid->addr_ );
		}
	}

	return true;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::allKeys

template < int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
template < class TContainer >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::allKeys( 
	NodeType *node, TContainer &retList )
{
	if ( !node ) return true;

	if ( node->less_ )
	{
		NodeType *less = 0;

		bool wasInCache = nodeInCache( node->less_ );
		if ( !loadNode( &less, node->less_ ) ) return false;
		if ( !allKeys( less, retList ) ) return false;
		if ( less && !wasInCache ) releaseNode( less->addr_ );
	}

	for ( int i = 0; i < node->count(); i++ )
	{
		retList.push_back( TContainer::ElemType( node->elems_[ i ] ) );

		if ( node->elems_[ i ].link_ )
		{
			NodeType *kid = 0;

			bool wasInCache = nodeInCache( node->elems_[ i ].link_ );
			if ( !loadNode( &kid, node->elems_[ i ].link_ ) ) return false;
			if ( !allKeys( kid, retList ) ) return false;
			if ( kid && !wasInCache ) releaseNode( kid->addr_ );
		}
	}

	return true;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::close

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
void BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::close()
{
	setMaxCacheSize( 0 );
	releaseCache();
	delete root_;
	root_ = 0;

	closeController();
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::rebalance

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::rebalance( NodeType *node, int parentIndex )
{
	int minimal = node->maxKeys >> 1;

	if ( node->count() >= minimal )
	{
		// Node is balanced
		return true;
	}

	NodeType *parent = 0;
	if ( !loadNode( &parent, node->parent_ ) )
	{
		return false;
	}

	NodeType *leftNode = 0, *rightNode = 0;
	NodeType *combinedNode = 0;

	if ( 0 == parentIndex - 1 )
	{
		if ( !loadNode( &leftNode, parent->less_ ) ) return false;
	}
	else if ( parentIndex >= 2 )
	{
		if ( !loadNode( &leftNode, parent->elems_[ parentIndex - 2 ].link_ ) ) return false;
	}

	if ( parent && parentIndex < parent->count() )
	{
		if ( !loadNode( &rightNode, parent->elems_[ parentIndex ].link_ ) ) return false;
	}

	if ( leftNode && leftNode->count() > minimal )
	{
		// Check left
		BTreeElement< KeyT, DataT > parentElem = parent->elems_[ parentIndex - 1 ];
		parentElem.link_ = node->less_;
		node->less_ = leftNode->elems_[ leftNode->count() - 1 ].link_;
		node->add( parentElem );
		node->flags_ = NodeType::NodeChanged;

		if ( node->less_ )
		{
			NodeType *less = 0;
			if ( !loadNode( &less, node->less_ ) ) return false;
			less->parent_ = node->addr_;
			less->flags_ = NodeType::NodeChanged;
		}

		BTreeElement< KeyT, DataT > largest;
		leftNode->removeAt( leftNode->count() - 1, largest );
		parent->elems_[ parentIndex - 1 ].key_ = largest.key_;
		parent->elems_[ parentIndex - 1 ].data_ = largest.data_;
		parent->flags_ = NodeType::NodeChanged;
	}
	else if ( rightNode && rightNode->count() > minimal )
	{
		// Check right
		BTreeElement< KeyT, DataT > parentElem = parent->elems_[ parentIndex ];
		parentElem.link_ = rightNode->less_;
		node->add( parentElem );

		if ( rightNode->less_ )
		{
			NodeType *less = 0;
			if ( !loadNode( &less, rightNode->less_ ) ) return false;
			less->parent_ = node->addr_;
			less->flags_ = NodeType::NodeChanged;
		}

		BTreeElement< KeyT, DataT > smallest;
		rightNode->removeAt( 0, smallest );
		rightNode->less_ = smallest.link_;
		parent->elems_[ parentIndex ].key_ = smallest.key_;
		parent->elems_[ parentIndex ].data_ = smallest.data_;
		parent->flags_ = NodeType::NodeChanged;
	}
	else
	{
		// Combine nodes
		if ( leftNode )
		{
			int index = leftNode->add( parent->elems_[ parentIndex - 1 ].key_,
				parent->elems_[ parentIndex - 1 ].data_ );
			leftNode->elems_[ index ].link_ = node->less_;
			if ( node->less_ )
			{
				NodeType *less = 0;
				if ( !loadNode( &less, node->less_ ) ) return false;
				less->parent_ = leftNode->addr_;
				less->flags_ = NodeType::NodeChanged;
			}

			if ( !combine( leftNode, node ) ) return false;
			combinedNode = leftNode;
			parent->removeAt( parentIndex - 1 );
		}
		else if ( rightNode )
		{
			int index = node->add( parent->elems_[ parentIndex ].key_,
				parent->elems_[ parentIndex ].data_ );
			node->elems_[ index ].link_ = rightNode->less_;

			if ( rightNode->less_ )
			{
				NodeType *less = 0;
				if ( !loadNode( &less, rightNode->less_ ) ) return false;
				less->parent_ = node->addr_;
				less->flags_ = NodeType::NodeChanged;
			}

			if ( !combine( node, rightNode ) ) return false;
			combinedNode = node;
			parent->removeAt( parentIndex );
		}

		parentIndex = -1;

		if ( parent && parent->parent_ )
		{
			// Find parent index
			NodeType *pp = 0;
			if ( !loadNode( &pp, parent->parent_ ) ) return false;

			if ( pp->less_ == parent->addr_ )
			{
				parentIndex = 0;
			}
			else
			{
				// Check parents parent
				for ( int i = 0; i < pp->count(); i++ )
				{
					if ( pp->elems_[ i ].link_ == parent->addr_ )
					{
						parentIndex = i + 1;
						break;
					}
				}
			}

			if ( parentIndex >= 0 )
			{
				if ( !rebalance( parent, parentIndex ) ) return false;
			}
		}
		else
		{
			// It is root
			if ( parent && parent->isEmpty() )
			{
				deleteNode( root_ );
				root_ = combinedNode;
				rootAddr( root_->addr_ );
				combinedNode->parent_ = 0;
				combinedNode->flags_ = NodeType::NodeChanged;
			}
		}
	}

	return true;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::combine

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::combine( NodeType *leftNode, 
													NodeType *rightNode )
{
	if ( leftNode->count() + rightNode->count() > leftNode->maxKeys )
	{
		// No space for combining
		return true;
	}

	for ( int i = 0; i < rightNode->count(); i++ )
	{
		leftNode->elems_[ leftNode->count() ] = rightNode->elems_[ i ];

		if ( rightNode->elems_[ i ].link_ )
		{
			NodeType *link = 0;
			if ( !loadNode( &link, rightNode->elems_[ i ].link_ ) ) return false;
			link->parent_ = leftNode->addr_;
			link->flags_ = NodeType::NodeChanged;
		}

		leftNode->size_++;
		leftNode->flags_ = NodeType::NodeChanged;
	}

	deleteNode( rightNode );
	return true;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::pullOut

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::pullOut( NodeType *node, 
																	int itemIndex )
{
	NodeType *leftSubtree = 0, *rightSubtree = 0;

	// Get subtrees
	if ( 0 == itemIndex )
	{
		if ( !loadNode( &leftSubtree, node->less_ ) ) return false;
	}
	else
	{
		if ( !loadNode( &leftSubtree, node->elems_[ itemIndex - 1 ].link_ ) ) return false;
	}

	if ( !loadNode( &rightSubtree, node->elems_[ itemIndex ].link_ ) ) return false;
	if ( !leftSubtree || !rightSubtree ) return false;

	if ( leftSubtree->count() > rightSubtree->count() )
	{
		// Left subtree is greater
		if ( leftSubtree->hasChilds() )
		{
			KeyT largestKey;
			DataT largestData;
			NodeType *largestNode = rightMost( leftSubtree, largestKey, largestData );
			node->elems_[ itemIndex ].key_ = largestKey;
			node->elems_[ itemIndex ].data_ = largestData;
			node->flags_ = NodeType::NodeChanged;
			largestNode->removeAt( largestNode->count() - 1 );

			NodeType *largestParent = 0;
			if ( !loadNode( &largestParent, largestNode->parent_ ) ) return false;
			if ( !rebalance( largestNode, largestParent->count() ) ) return false;
		}
		else
		{
			node->elems_[ itemIndex ].key_ = leftSubtree->elems_[ leftSubtree->count() - 1 ].key_;
			node->elems_[ itemIndex ].data_ = leftSubtree->elems_[ leftSubtree->count() - 1 ].data_;
			node->flags_ = NodeType::NodeChanged;
			leftSubtree->removeAt( leftSubtree->count() - 1 );
			if ( !rebalance( leftSubtree, itemIndex ) ) return false;
		}
	}
	else
	{
		// Right subtree is greater
		if ( rightSubtree->hasChilds() )
		{
			KeyT smallestKey;
			DataT smallestData;
			NodeType *smallestNode = leftMost( rightSubtree, smallestKey, smallestData );
			node->elems_[ itemIndex ].key_ = smallestKey;
			node->elems_[ itemIndex ].data_ = smallestData;
			node->flags_ = NodeType::NodeChanged;
			smallestNode->removeAt( 0 );
			if ( !rebalance( smallestNode, 0 ) ) return false;
		}
		else
		{
			node->elems_[ itemIndex ].key_ = rightSubtree->elems_[ 0 ].key_;
			node->elems_[ itemIndex ].data_ = rightSubtree->elems_[ 0 ].data_;
			node->flags_ = NodeType::NodeChanged;
			rightSubtree->removeAt( 0 );
			if ( !rebalance( rightSubtree, itemIndex + 1 ) ) return false;
		}
	}

	return true;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::rightMost

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
typename BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::NodeType* 
		BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::rightMost( NodeType *subtree, 
												KeyT &largestKey, DataT &largestData )
{
	if ( subtree->elems_[ subtree->count() - 1 ].link_ )
	{
		NodeType *rightMostLink = 0;
		if ( !loadNode( &rightMostLink, subtree->elems_[ subtree->count() - 1 ].link_ ) )
		{
			return 0;
		}

		return rightMost( rightMostLink, largestKey, largestData );
	}
	else
	{
		largestKey = subtree->elems_[ subtree->count() - 1 ].key_;
		largestData = subtree->elems_[ subtree->count() - 1 ].data_;
		return subtree;
	}
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::leftMost

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
typename BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::NodeType* 
			BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::
			leftMost( NodeType *subtree, KeyT &smallestKey, DataT &smallestData )
{
	if ( subtree->less_ )
	{
		NodeType *leftMostLink = 0;
		if ( !loadNode( &leftMostLink, subtree->less_ ) )
		{
			return 0;
		}

		return leftMost( leftMostLink, smallestKey, smallestData );
	}
	else
	{
		smallestKey = subtree->elems_[ 0 ].key_;
		smallestData = subtree->elems_[ 0 ].data_;
		return subtree;
	}
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::splitNode

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
bool BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::splitNode( NodeType *node, 
						BTreeElement< KeyT, DataT > &median, NodeType **nodeRight )
{
	BTreeElement< KeyT, DataT > insertElem( median );
	int medianIndex = getMedian( node, median );

	NodeType nodeLeftTmp;
	*nodeRight = newNode();

	// Replace element
	node->removeAt( medianIndex );
	node->add( insertElem );

	( *nodeRight )->less_ = median.link_;
	if ( median.link_ )
	{
		NodeType *link = 0;
		if ( !loadNode( &link, median.link_ ) ) return false;
		link->parent_ = ( *nodeRight )->addr_;
		link->flags_ = NodeType::NodeChanged;
	}
	nodeLeftTmp.less_ = node->less_;

	for ( int i = 0; i < node->size_; i++ )
	{
		if ( node->elems_[ i ].key_ < median.key_ )
		{
			nodeLeftTmp.elems_[ nodeLeftTmp.size_ ] = node->elems_[ i ];
			nodeLeftTmp.size_++;
		}
		else
		{
			( *nodeRight )->elems_[ ( *nodeRight )->size_ ] = node->elems_[ i ];

			if ( node->elems_[ i ].link_ )
			{
				NodeType *link = 0;
				
				bool wasInCache = nodeInCache( node->elems_[ i ].link_ );
				if ( !loadNode( &link, node->elems_[ i ].link_ ) ) return false;
				link->parent_ = ( *nodeRight )->addr_;
				link->flags_ = NodeType::NodeChanged;
				if ( link && !wasInCache ) releaseNode( link->addr_ );
			}

			( *nodeRight )->size_++;
		}
	}

	// Prepare result
	( *nodeRight )->parent_ = node->parent_;
	nodeLeftTmp.parent_ = node->parent_;
	nodeLeftTmp.addr_ = node->addr_;
	*node = nodeLeftTmp;

	( *nodeRight )->flags_ = NodeType::NodeChanged;
	node->flags_ = NodeType::NodeChanged;

	median.link_ = ( *nodeRight )->addr_;
	return true;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::getMedian

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
int BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::getMedian( 
	BTreeNode< NodeSize, KeyT, DataT > *node, 
	BTreeElement< KeyT, DataT > &elem )
{
	int medianRange = node->size_ >> 1;
	elem = node->elems_[ medianRange ];
	return medianRange;
}

//	===================================================================
//	BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::findNode

template< int NodeSize, typename KeyT, typename DataT, 
template < int, typename, typename > class TController >
typename BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::NodeType* 
		BTreeAlgorithms< NodeSize, KeyT, DataT, TController >::
		findNode( NodeType *node, const KeyT &key, int &retIndex, int &parentIndex, bool &found )
{
	if ( !node ) return 0;

	found = false;
	retIndex = -1;

	if ( node->find( key, retIndex ) )
	{
		// Key found
		found = true;
		return node;
	}

	int link = 0;

	if ( retIndex > 0 )
	{
		link = node->elems_[ retIndex - 1 ].link_;
	}
	else
	{
		link = node->less_;
	}

	if ( !link )
	{
		// No more kids
		return node;
	}
	else if ( link == node->addr_ )
	{
		// Simple cyclic traversal skip (in case of corrapted index)
		return 0;
	}

	NodeType *nextLink = 0;
	if ( !loadNode( &nextLink, link ) ) return 0;

	parentIndex = retIndex;
	return findNode( nextLink, key, retIndex, parentIndex, found );
}

#endif // __BTreeAlgorithms_h__
