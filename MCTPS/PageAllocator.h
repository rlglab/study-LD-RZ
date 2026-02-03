#ifndef __PAGEALLOCATOR_H__
#define __PAGEALLOCATOR_H__

#include "types.h"
#include "Configure.h"
#include "NodePage.h"
#include "SpinLock.h"
#include "Vector.h"

template<class _NodeClass>
class NodePtr ;

namespace node_manage {

	template<class _NodeClass>
	class NodeAllocator ;

/*!
	@brief  global (process level) node manager
	@author T.F. Liao
	implement with singleton, ensure only one instance
*/
template<class _NodeClass>
class PageAllocator
{
	friend class NodePtr<_NodeClass> ;
	
private:
	SpinLock m_lock ;
	bool m_allocateFail ;
	Vector<NodePage<_NodeClass>, MaxNumPages> m_pages ;
	uint m_used ;

	Vector<NodeAllocator<_NodeClass>*, MAX_NUM_THREADS+MAX_NUM_WORKERS+1> m_nodeAllocators ;

	static PageAllocator* s_pInstance ;

public:
	/*!
		@brief  get the real position of compressed NodePtr
		@author T.F. Liao
		@param  index [in] compressed index of pointer
		@return pointer to real position
	*/
	_NodeClass* resolve ( uint index ) 
	{
		uint pageIndex = getPageIndex(index) ; 

		assert(pageIndex<m_pages.size()) ;
		assert(m_pages[pageIndex].isValid());

		NodePage<_NodeClass>& page = m_pages[pageIndex];
		return page.resolve(index) ; 
	}
private:
	/*!
		@brief  private constructor
		@author T.F. Liao
	*/
	PageAllocator ( ) 
		: m_allocateFail ( false ), m_used ( 0 )
	{
		s_pInstance = this ; // can be remove, since initialize set this variable
	}
	/*!
		@brief  private destructor, delete all allocate pages
		@author T.F. Liao
	*/
	~PageAllocator ()
	{
		for ( int i=0;i<m_pages.size();++i ) {
			if ( m_pages[i].isValid() ) 
				delete [] m_pages[i].getStartAddr();
		}
	}

	int getNextPageIndex() 
	{
		int index = -1;
		m_lock.lock();

		if ( m_used < m_pages.size() ) { 
			index = m_used ++ ;
		} else if ( m_used == m_pages.size() && 
			m_used < Configure::MaxPageNum &&
			m_used < node_manage::MaxNumPages ) { 
				// no free page to reuse, and max page number not exceed
				m_pages.push_back(NodePage<_NodeClass>());
				index = m_used ++ ;
		}
		m_lock.unlock();
		return index ;
	}

public:
	/*!
		@brief  initialize GlobalNodeManager
		@author T.F. Liao
	*/
	static void initialize () 
	{
		assert (s_pInstance==NULL);
		s_pInstance = new PageAllocator();
	}
	
	/*!
		@brief  get singleton instance of GlobalNodeManager
		@author T.F. Liao
		@return pointer to GlobalNodeManager  
	*/
	static PageAllocator * get () 
	{
		assert (s_pInstance!=NULL);
		return s_pInstance ;
	}

	/*!
		@brief  allocate a page of node (for NodeAllocator use only)
		@author T.F. Liao
		@return pointer of NodePage indicate allocated page,
				if return nullptr, no more page is available
	*/
	NodePage<_NodeClass>* allocatePage () 
	{
		if ( m_used >= Configure::MaxPageNum || m_used >= node_manage::MaxNumPages ) {
			/// early report
			return nullptr;
		}

		int index = getNextPageIndex() ;

		if ( index == -1 ) {
			m_allocateFail = true ;
			return nullptr;
		}

		NodePage<_NodeClass>& page = m_pages[index] ;
		
		if ( !page.initialize(index) ) {
			m_allocateFail = true ;
			return nullptr;
		}

		return &page;
	}
	/*!
		@brief  check if all pages are used
		@author T.F. Liao
		@return true if out of page
	*/
	bool isOutOfPage () const 
	{
		return m_allocateFail ;
	}

	/*!
		@brief  replace first n pages with pages passed in
		@author T.F. Liao
		@param  pages   [in]    new pages after prune tree
	*/
	void replacePages ( Vector<NodePage<_NodeClass>, MaxNumPages>& pages )
	{
		for ( uint i=0 ; i<pages.size() ; ++i ) {
			m_pages[i].freeMemory() ;
			m_pages[i] = pages[i] ;
		}
		m_used = pages.size() ;
		m_allocateFail = false ;
	}

	/*!
		@brief  reset the node manager
		@author T.F. Liao
	*/
	void reset()
	{
		m_used = 0 ;
		invalidateNodeAllocator();
	}

	int getUsedPages()
	{
		return m_used;
	}

	void pushNodeAllocator( NodeAllocator<_NodeClass>* nodeAllocator )
	{
		m_nodeAllocators.push_back( nodeAllocator );
	}

	void popNodeAllocator( NodeAllocator<_NodeClass>* nodeAllocator )
	{
		for( uint i=0 ; i<m_nodeAllocators.size() ; ++i ) {
			if( m_nodeAllocators[i] == nodeAllocator ) {
				m_nodeAllocators.erase_no_order(i);
				break;
			}
		}
	}

	void invalidateNodeAllocator()
	{
		for( uint i=0 ; i<m_nodeAllocators.size() ; ++i ) {
			m_nodeAllocators[i]->invalidate();
		}
	}
};


template<class _NodeClass>
PageAllocator<_NodeClass>* PageAllocator<_NodeClass>::s_pInstance = NULL;

}

#endif 
