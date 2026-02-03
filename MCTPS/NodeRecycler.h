#ifndef NODERECYCLER_H
#define NODERECYCLER_H

#include "PageAllocator.h"
#include "NodePtr.h"

#include "BasePrunePolicy.h"

namespace node_manage {

/*!
	@brief  The Node Recycler, used for prune tree
	@author T.F. Liao
*/
template<class _NodeClass, class _PrunePolicy = BasePrunePolicy<_NodeClass> >
class NodeRecycler
{
	typedef NodePtr<_NodeClass> UctNodePtr;

	_PrunePolicy m_policy ;
	Vector<NodePage<_NodeClass>, MaxNumPages> m_pages ;
	NodePage<_NodeClass> m_page ;

	bool m_prune_after_play ;

public:
	/*!
		@brief  reuse the node pool, start with new root
		@author T.F. Liao
		@param  root    [in] UctNodePtr to root node
		@param  after_play [in] indicate the pruning is invoked after a play or not
		@return UctNodePtr to new root node
	*/
	UctNodePtr reuse ( UctNodePtr root, bool after_play )
	{
		m_policy.beforePruning (root);
		newPage();
		m_page.allocate(1) ; // for NULL_PTR

		UctNodePtr newRoot = m_page.allocate(1) ;
		_NodeClass* p = m_page.resolve(newRoot) ; 
		*p = *root ;
		m_prune_after_play = after_play ;
		reuse_r ( p ) ;
		PageAllocator<_NodeClass>::get()->replacePages(m_pages) ;
		PageAllocator<_NodeClass>::get()->invalidateNodeAllocator() ;

		m_policy.afterPruning ( m_pages.size(), Configure::MaxPageNum, p);

		for ( uint i=0; i<m_pages.size(); ++i ) {
			m_pages[i].m_pStartAddr = nullptr;
		}
		m_pages.clear();
		m_page.m_pStartAddr = nullptr;

		return newRoot ;
	}

private:

	/*!
		@brief  allocate a new page for node reusing (extracted method)
		@author T.F. Liao
	*/
	void newPage ( )
	{
		m_page = NodePage<_NodeClass> ( ) ;
		m_page.initialize(m_pages.size()) ;
		m_pages.push_back(m_page) ;
	}

	/*!
		@brief  reuse the node (recursive method)
		@author T.F. Liao
		@param  node    [in] node of the root of subtree to be reuse
	*/
	void reuse_r ( _NodeClass* node )
	{
		Vector<uint, MAX_NUM_CHILDREN> keeplist ;
		bool discard_all = m_policy ( node, keeplist, m_prune_after_play ) ;

		if ( discard_all || keeplist.size() == 0 ) {
			node->setChildren ( UctNodePtr::NULL_PTR, 0 ) ;
		} else {
			uint sz = keeplist.size();
			UctNodePtr pNewChildren = m_page.allocate(sz) ;
			if ( pNewChildren.isNull() ) {
				newPage();
				pNewChildren = m_page.allocate(sz) ;
				assert ( pNewChildren.isValid() ) ;
			}
			_NodeClass* p = m_page.resolve(pNewChildren) ;

			for ( uint i=0 ; i<sz ; ++i, ++p ) {
				* p = * ( node->getChild(keeplist[i]) ) ;
				reuse_r ( p ) ;
			}
			node->setChildren ( pNewChildren, sz ) ;
		}
	}
};

}
#endif 
