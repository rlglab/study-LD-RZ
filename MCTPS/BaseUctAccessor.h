#ifndef _BASEUCTACCESSOR_
#define _BASEUCTACCESSOR_

#include "NodePtr.h"
#include "NodeAllocator.h"
#include "UctChildIterator.h"
#include "StatisticData.h"
#include <cmath>
#include <cfloat>

/*!
	@brief  the default implementation of UctAccessor
	@author T.F. Liao
*/
template<
	class _Move, 
	class _PlayoutResult, 
	class _UctNode, 
	class _ThreadGameState
>
class BaseUctAccessor
{
	typedef NodePtr<_UctNode> UctNodePtr ;
protected:
	_ThreadGameState& m_state ;
	UctNodePtr m_pRoot ;
	node_manage::NodeAllocator<_UctNode>& m_nodeAllocator;

public:
	/*!
		@brief  constructor
		@author T.F. Liao
		@param  state [in] reference to _ThreadGameState
		@param  lnm [in] reference to local (thread level) node manager
	*/
	BaseUctAccessor ( _ThreadGameState& state, node_manage::NodeAllocator<_UctNode>& na ) 
		: m_state ( state ), m_nodeAllocator ( na )
	{
	}
	/*!
		@brief  select a leaf node from UCT
		@author T.F. Liao
		@return UctNodePtr to the selected node
		the default implementation select node from root to leaf, and expand children if threshold is reached
	*/
	UctNodePtr selectNode ( ) 
	{
		UctNodePtr pNode = m_pRoot ;

		m_state.m_path.push_back(pNode) ;
#if NATIVE_PTR_PATH
		m_state.m_native_path.push_back(pNode.operator->());
#endif
		while ( pNode->hasChildren() )  {
			pNode = selectChild ( pNode ) ;
			m_state.m_path.push_back(pNode) ;
#if NATIVE_PTR_PATH
			m_state.m_native_path.push_back(pNode.operator->());
#endif
			m_state.play(pNode->getMove());
		}

		if ( pNode->getUctData().getCount() >= Configure::ExpandThreshold) {
			if ( !m_state.isTerminal() ) {
				m_state.startExpansion();
				if ( expandChildren ( pNode ) ) {
					pNode = selectChild(pNode) ;
					m_state.m_path.push_back(pNode) ;
#if NATIVE_PTR_PATH
					m_state.m_native_path.push_back(pNode.operator->());
#endif
					m_state.play(pNode->getMove());
				}
				m_state.endExpansion();
			}
		}
		return pNode;
	}

	/*!
		@brief  update the UCT according to simulation result
		@author T.F. Liao
		@param  result  [in] simulation result
		The default implementation assume that 
			1. the game is two-player game
			2. result can be cast to boolean and indicate the status of root node (win or loss)
	*/
	void update ( _PlayoutResult result ) 
	{
		Vector<UctNodePtr, MAX_TREE_DEPTH>& path = m_state.m_path ;
		float w = (result?0.0f:1.0f);

		for ( uint i=0 ; i<path.size() ; ++i ) {
			UctNodePtr p = path[i] ;
			/// TODO: fix this
			p->getUctData().add( w, 1.0f ) ;
			w = 1-w;
		}
	}
	void beforeChangeTree(){}
	/*!
		@brief  set the root node that cached in this class
		@author T.F. Liao
		@param  pRoor   [in] the UctNodePtr to root of UCT
		the method will be invoke after root changed (after genmove, play, or prune tree)
	*/
	virtual void setRoot ( UctNodePtr pRoot ) 
	{
		m_pRoot = pRoot ;
	}

protected:
	/*!
		@brief  expand the children of pNode
		@author T.F. Liao
		@param  pNode   [in] node that to be expanded
		@return expansion success or not
		the default implementation is that
			1. get all possible move by invoke generateAllMoves in _ThreadGameState
			2. allocate memory from node manager
				2.1. in case that no memory is available, return false without expansion
			3. initialize all children and connect pointer from pNode
	*/
	virtual bool expandChildren ( UctNodePtr pNode ) 
	{
		Vector<_Move, MAX_NUM_CHILDREN> moves ;
		m_state.generateAllMoves(moves) ; 
		uint sz = moves.size();

		assert ( sz != 0 ) ;

		UctNodePtr firstChild = m_nodeAllocator.allocateNodes(sz);
		if ( firstChild.isNull() ) {
			// allocation fail
			return false ;
		}
		UctChildIterator<_UctNode> it ( firstChild, sz ) ;
		for ( uint i=0;i<sz;++i ) {
			it->reset(moves[i]) ;
			it->getUctData().add(0.5, 1); // first play policy
			++it ;
		}
		pNode->setChildren(firstChild, sz) ;
		return true ;
	}

	/*!
		@brief  select a child from children of pNode
		@author T.F. Liao
		@param  pNode   [in] parent node of children to be selected
		@return UctNodePtr to the selected node
		the default implementation select the child with highest score according to UCB formula
	*/
	virtual UctNodePtr selectChild ( UctNodePtr pNode ) 
	{
		assert ( pNode->hasChildren() ) ;

		UctNodePtr pBest = UctNodePtr::NULL_PTR ;
		StatisticData::data_type bestScore = -FLT_MAX;
		double logVal = log ( (double)pNode->getUctData().getCount() ) ;

		// for ( uint i=0;i<pNode->getNumChildren();++i ) {
		for ( UctChildIterator<_UctNode> it(pNode) ; it ; ++ it ) {
			// UctNodePtr pChild = pNode->getChild(i);
			_UctNode& child = *it ;
			StatisticData& UctData = child.getUctData();
			StatisticData::data_type score = UctData.getMean() + 
				Configure::UCBWeight * sqrt ( logVal/UctData.getCount() ) ;

			if ( score > bestScore ) {
				bestScore = score ;
				pBest = it ;
			}
		}

		return pBest;
	}

};

#endif 
