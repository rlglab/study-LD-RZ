#ifndef __NODEPAGE_H__
#define __NODEPAGE_H__

#include "types.h"
#include "NodePtr.h"


namespace node_manage {
	const uint BitsPerPage = 18 ; // default 1M per page
	const uint MaxNumPages = (1<<(32-BitsPerPage)) ;
	const uint PageSize = (1<<BitsPerPage) ; 
	const uint OffsetMask = PageSize-1 ;

	inline uint getPageIndex ( uint index ) { return (index>>BitsPerPage); }
	inline uint getOffset ( uint index ) { return (index&OffsetMask); }
	inline uint getIndex ( uint page, uint offset ) { return (page<<BitsPerPage) | offset ; }

template<class _NodeClass>
class PageAllocator ;

template<class _NodeClass, class _Policy> class NodeRecycler ; 

template<class _NodeClass>
class NodePage
{
	friend class PageAllocator<_NodeClass> ;
	template<class __NodeClass, class _Policy> friend class NodeRecycler ;
	typedef NodePtr<_NodeClass> UctNodePtr ;
private:
	volatile _NodeClass* m_pStartAddr ;
	uint m_index ;
	uint m_size ;
	uint m_used ;
private:
	/*!
		@breif  initialize this page
		@author T.F. Liao
		@param  index [in] index of this page
		@param  size [in] indicate page size for special case ( first(0) page )
		@return true if initialization successful, false otherwise
	*/
	bool initialize ( uint index, uint size = PageSize ) 
	{
		m_index = index ;
		
		if ( !isValid() ) {
			try {
				m_pStartAddr = new _NodeClass[size] ; 
			} catch ( std::bad_alloc& ) {
				return false ;
				/// if allocation fail, return 
			}
		}

		m_used = 0 ;
		m_size = size ;
		return true;
	}

	void freeMemory () 
	{
		delete[] m_pStartAddr ;
		m_pStartAddr = nullptr ;
	}

public:
	/*!
		@breif  default constructor
		@author T.F. Liao
	*/
	NodePage () 
		: m_pStartAddr ( nullptr ), m_size(0), m_used(0)
	{}

	~NodePage()
	{
		if ( isValid() ) {
			freeMemory();
		}
	}

	/*!
		@breif  check if this page is valid 
		@author T.F. Liao
		@return true if page is valid
	*/
	bool isValid ( ) 
	{
		return m_pStartAddr != nullptr ;
	}

	/*!
		@breif  set page index
		@author T.F. Liao
		@param  index [in] new index to be set
	*/
	void setIndex (uint index) 
	{
		m_index = index ;
	}

	/*!
		@brief  allocate a number of nodes and initialize these nodes
		@author T.F. Liao
		@param  size [in] number of nodes to be allocated
		@return invalid NodePtr if no enough nodes in this page, or NodePtr to first node
	*/
	UctNodePtr allocate ( uint size ) 
	{
		assert(isValid());
		
		if ( m_used + size >= m_size ) 
			return UctNodePtr::NULL_PTR ;

		for ( uint i=m_used; i<m_used+size; ++i ) {
			const_cast<_NodeClass&>(m_pStartAddr[i]).reset(typename _NodeClass::move_type());
		}

		uint offset = m_used ;
		m_used += size ;

		return UctNodePtr ( getIndex(m_index, offset) ) ;
	}
	/*!
		@brief  resolve UctNodePtr to native pointer
		@author T.F. Liao
		@param  node [in] UctNodePtr to be resolved
		@return native pointer
	*/
	_NodeClass* resolve ( UctNodePtr node )
	{
		return resolve(node.getIndex());
	}

	/*!
		@brief  resolve index to native pointer
		@author T.F. Liao
		@param  index [in] index of node ptr
		@return native pointer
	*/
	_NodeClass* resolve ( uint index )
	{
		assert ( getPageIndex(index) == m_index ) ;
		assert ( getOffset(index) < m_used ) ;
		return const_cast<_NodeClass*>(m_pStartAddr+getOffset(index)) ;
	}
};

}

#endif // __NODEPAGE_H__ 
