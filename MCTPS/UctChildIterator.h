#ifndef __UCTCHILDITERATOR__
#define __UCTCHILDITERATOR__

#include "NodePtr.h"
/*!
    @brief  iterator that help developer to iterate a set of child
    @author T.F. Liao
*/
template<class _NodeClass>
class UctChildIterator
{
    typedef NodePtr<_NodeClass> UctNodePtr ;
private:
    uint m_firstIdx ;
    _NodeClass* m_first ;
    _NodeClass* m_current ;
    _NodeClass* m_end ;
public:
    /*!
        @brief  constructor with single NodePtr, generate iterator of its all children
        @author T.F. Liao
        @param  parent  [in] parent that children to be iterated
    */
    UctChildIterator ( UctNodePtr parent ) 
    {
        assert ( parent.isValid() ) ;
        if ( !parent->hasChildren() ) {
            m_first = m_current = NULL ;
            m_end = NULL ;
            m_firstIdx = 0 ;
        } else {
            UctNodePtr child = parent->m_firstChild ;
            m_first = m_current = child.operator->() ;
            m_end = m_current + parent->m_nChildren ;
            m_firstIdx = child.getIndex();
        }
    }

    /*!
        @brief  constructor with first node and total number of nodes, generate iterator of its all children
        @author T.F. Liao
        @param  first   [in] first node to be iterated
        @param  nNodes  [in] total number of node to be iterated
    */
    UctChildIterator ( UctNodePtr first, uint nNodes ) 
    {
        assert ( first.isValid() ) ;
        m_first = m_current = first.operator->() ;
        m_end = m_current + nNodes ;
        m_firstIdx = first.getIndex();
    }

    /*!
        @brief  the const version of indirection operator 
        @author T.F. Liao
        @return refernce to curent node
    */
    const _NodeClass& operator*() const { return *m_current ; }
    /*!
        @brief  the indirection operator
        @author T.F. Liao
        @return refernce to curent node
    */
    _NodeClass& operator*() { return *m_current ; }

    /*!
        @brief  the const version of dereference operator 
        @author T.F. Liao
        @return pointer to curent node
    */
    const _NodeClass* operator->() const { return m_current ; }
    /*!
        @brief  the indirection operator 
        @author T.F. Liao
        @return pointer to curent node
    */
    _NodeClass* operator->() { return m_current ; }

    /*!
        @brief  cast to UctNodePtr
        @author T.F. Liao
        @return UctNodePtr that casted from current node
    */
    operator UctNodePtr () const { return UctNodePtr(m_firstIdx+(uint)(m_current-m_first)) ; }

    /*!
        @brief  the increment operator
        @author T.F. Liao
    */
    void operator++ () { ++m_current ; }

    /*!
        @brief  cast to boolean
        @author T.F. Liao
        @return boolean indicate that if end if reached
    */
    operator bool() const { return m_current != m_end ; }

	UctNodePtr getUctNodePtr() const
	{
		return UctNodePtr(m_firstIdx+(uint)(m_current-m_first)) ;
	}
};

#endif 
