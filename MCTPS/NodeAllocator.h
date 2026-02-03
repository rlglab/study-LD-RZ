#ifndef NODEALLOCATOR_H
#define NODEALLOCATOR_H

#include "PageAllocator.h"
#include "NodePage.h"

namespace node_manage {

/*!
	@brief  thread level node manager
	@author T.F. Liao
*/
template<class _NodeClass>
class NodeAllocator 
{
	typedef NodePtr<_NodeClass> UctNodePtr ;
private:
	NodePage<_NodeClass>* m_page ;
	PageAllocator<_NodeClass>* m_pGNMr ;
	bool m_bAllocFail ;
public:
	NodeAllocator () 
		: m_pGNMr ( PageAllocator<_NodeClass>::get() ), m_bAllocFail ( false )
	{
		m_pGNMr->pushNodeAllocator(this);
	}

	~NodeAllocator ()
	{
		m_pGNMr->popNodeAllocator(this);
	}

	/*!
		@brief  allocate a number of nodes
		@author T.F. Liao
		@param  nNodes [in] number od nodes to be allocate
		@return NULL_PTR if allocation fail, or UctNodePtr to first child
	*/
	UctNodePtr allocateNodes ( uint nNodes ) 
	{
		UctNodePtr ptr ;
		if ( !m_page || !m_page->isValid() || (ptr=m_page->allocate(nNodes)) == UctNodePtr::NULL_PTR ) {
			m_page = m_pGNMr->allocatePage();
			if ( m_page && m_page->isValid() ) {
				ptr = m_page->allocate(nNodes) ;
			} else {
				m_bAllocFail = true ;
			}
		}
		return ptr ;
	}

	/*!
		@brief  invalidate the cached page and flag
		@author T.F. Liao
	*/
	void invalidate()
	{
		m_page = nullptr;
		m_bAllocFail = false ;
	}
	/*!
		@brief  check if allocation fail before
		@author T.F. Liao
		@return true if allocate fail (no free page availalbe)
	*/
	bool hasAllocFail () const {
		return m_bAllocFail ;
	}

};

}

#endif 
