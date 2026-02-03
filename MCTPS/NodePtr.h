#ifndef __NODEPTR_H__
#define __NODEPTR_H__

#include <cassert>
#include "types.h"
/*!
    @brief  encapsulate pointer behavior into single 32bits int size
*/
template<class _NodeClass>
class NodePtr 
{
private:
    volatile uint m_index ;
    static const uint s_null_index ;

public:
    static const NodePtr NULL_PTR ;

public:
    /*!
        @brief  constructor
        @author T.F. Liao
        @param  idx [in]    index of this node in node pool
    */
    explicit NodePtr ( uint idx = s_null_index /* default null */ ) 
        : m_index ( idx ) 
    {
    }

    /*!
        @brief  copy constructor
        @author T.F. Liao
        @param  rhs [in]    NodePtr to be copied
    */
    NodePtr ( const NodePtr& rhs )
        : m_index ( rhs.m_index ) 
    {
    }


    /*!
        @brief  copy assignment with const version as parameter
        @author T.F. Liao
        @param  rhs [in]    NodePtr to be copied
        @return reference to its self
    */
    NodePtr& operator=(const NodePtr& rhs ) 
    {
        m_index = rhs.m_index ;
        return *this ;
    }
  
   /*!
        @brief  the indirection operator
        @author T.F. Liao
        @return reference to node
    */
    _NodeClass& operator* () 
    {
        assert ( isValid() ) ;
        return * resolve(m_index) ;
    }
    /*!
        @brief  the const version of indirection operator
        @author T.F. Liao
        @return const reference to node
    */
    const _NodeClass& operator* () const 
    {
        assert ( isValid() ) ;
        return * resolve(m_index) ;
    }

    /*!
        @brief  the dereference operator
        @author T.F. Liao
        @return pointer to node
    */
    _NodeClass* operator->() 
    {
        assert ( isValid() ) ;
        return resolve(m_index) ;
    }

    /*!
        @brief  the const version dereference operator
        @author T.F. Liao
        @return const pointer to node
    */
    const _NodeClass* operator->() const 
    {
        assert ( isValid() ) ;
        return resolve(m_index) ;
    }

    /*!
        @brief  check if NodePtr is valid
        @author T.F. Liao
        @return true if NodePtr is valid (not null)
    */
    bool isValid () const 
    {
        return m_index != s_null_index ;
    }

    /*!
        @brief  check if NodePtr is null pointer
        @author T.F. Liao
        @return true if NodePtr is null
    */
    bool isNull () const 
    {
        return m_index == s_null_index ;
    }

    /*!
        @brief  method to remove volatile 
        @author T.F. Liao
        @return NodePtr without volatile
    */
    NodePtr devolatile () volatile
    {
        return NodePtr(m_index) ;
    }

    /*!
        @brief  const version method to remove volatile 
        @author T.F. Liao
        @return const NodePtr without volatile
    */
    NodePtr devolatile () volatile const
    {
        return NodePtr(m_index) ;
    }

    /*!
        @brief  check if this NodePtr is less than rhs
        @author T.F. Liao
        @param  rhs [in]    another NodePtr to be compared
        @return true  if this NodePtr is less than rhs
    */
    bool operator< ( const NodePtr& rhs ) const { return m_index < rhs.m_index ; }

    /*!
        @brief  check if this NodePtr is greater than rhs
        @author T.F. Liao
        @param  rhs [in]    another NodePtr to be compared
        @return true  if this NodePtr is greater than rhs
    */
    bool operator> ( const NodePtr& rhs ) const { return m_index > rhs.m_index ; }

    /*!
        @brief  check if this NodePtr is less than or equal to rhs
        @author T.F. Liao
        @param  rhs [in]    another NodePtr to be compared
        @return true  if this NodePtr is less than or equal to rhs
    */
    bool operator<= ( const NodePtr& rhs ) const { return m_index <= rhs.m_index ; }

    /*!
        @brief  check if this NodePtr is equal to rhs
        @author T.F. Liao
        @param  rhs [in]    another NodePtr to be compared
        @return true  if this NodePtr is equal to rhs
    */
    bool operator== ( const NodePtr& rhs ) const { return this->m_index == rhs.m_index ; }

    /*!
        @brief  check if this NodePtr is not equal to rhs
        @author T.F. Liao
        @param  rhs [in]    another NodePtr to be compared
        @return true  if this NodePtr is not equal to rhs
    */
    bool operator!= ( const NodePtr& rhs ) const { return this->m_index != rhs.m_index ; }

    /*!
        @brief  addition operator
        @author T.F. Liao
        @param  offset [in] offset to add
        @return NodePtr after add offset
    */
    NodePtr operator+ ( uint offset ) const 
    {
        return NodePtr(m_index+offset) ;
    }

    /*!
        @brief  increment operator
        @author T.F. Liao
    */
    void operator++ () { ++ m_index ; }

    /*!
        @brief  get the index of NodePtr 
        @author T.F. Liao
        @return index of NodePtr
    */
    uint getIndex() const { return m_index; }
private:
    static _NodeClass* resolve ( uint index ) ;
    
};

template<class _NodeClass>
const uint NodePtr<_NodeClass>::s_null_index = 0 ;

#include "PageAllocator.h"

template<class _NodeClass>
_NodeClass* NodePtr<_NodeClass>::resolve( uint index )
{
    return node_manage::PageAllocator<_NodeClass>::get()->resolve(index) ;
}

template<class _NodeClass>
const NodePtr<_NodeClass> NodePtr<_NodeClass>::NULL_PTR = NodePtr<_NodeClass>(s_null_index);


#endif 
