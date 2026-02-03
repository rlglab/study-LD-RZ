#ifndef __UCTNODEIMPL_H__
#define __UCTNODEIMPL_H__

#include "types.h"
#include "StatisticData.h"
#include "NodePtr.h"

#include <string>
#include "strops.h"

template<class _NodeClass>
class UctChildIterator ;

/*!
    @brief  the default implementation of UctNode
    @author T.F. Liao
    the implementation is for the minimal use of this framework
    for any new field is needed, inherited it.
*/
template<class _Move,class _Derived>
class UctNodeImpl
{
    friend class UctChildIterator<_Derived> ;

public:
    typedef NodePtr<_Derived> UctNodePtr ;
    typedef _Move move_type;
    
protected:
        UctNodePtr m_firstChild ;
    volatile 
        uint m_nChildren ;

    _Move m_move ;
    StatisticData m_uctStat ;

public:

    /*!
        @brief  default constructor
        @author T.F. Liao
    */
    UctNodeImpl () 
        : m_firstChild ( UctNodePtr::NULL_PTR ),
          m_nChildren(0)
    {
    }

    /*!
        @brief  copy assignment
        @author T.F. Liao
        @param  rhs [in] right hand side of assignment
        @return reference to itself, after clone the rhs
    */
    UctNodeImpl& operator=(UctNodeImpl& rhs) 
    {
        m_move = rhs.m_move ;
        m_uctStat = rhs.m_uctStat ;
        m_firstChild = rhs.m_firstChild ;
        m_nChildren = rhs.m_nChildren ;
        return *this;
    }

    /*!
        @brief  reset data in this node, with initialization new move
        @author T.F. Liao
        @param  move    [in] move to be set in
    */
    void reset ( _Move move )
    {
        setMove ( move ) ;
        m_uctStat.reset();
        setChildren ( UctNodePtr::NULL_PTR, 0 ) ;
    }

    /*!
        @brief  check if there are children in this node 
        @author T.F. Liao
        @return true if there are children, false otherwise
    */
    bool hasChildren () const 
    {
        return m_firstChild.isValid();
    }


	bool isLeafNode () const 
	{
		return !hasChildren();
	}

    /*!
        @brief  get the number of children
        @author T.F. Liao
        @return uint indicate the children num
    */
    uint getNumChildren () const 
    {
        return m_nChildren;
    }

    /*!
        @brief  get UctNodePtr to particular child
        @author T.F. Liao
        @param  idx [in] the index of the child
        @return UctNodePtr to the child
    */
    UctNodePtr getChild ( uint idx ) const 
    {
        assert( hasChildren() );
        assert( idx < m_nChildren ) ;
        return const_cast<UctNodePtr&>(m_firstChild)+idx ;
    }

    /*!
        @brief  get the move from parent to this node
        @author T.F. Liao
        @return move from parent
    */
    _Move getMove () const 
    {
        return m_move;
    }
    /*!
        @brief  set the move from parent to this node
        @author T.F. Liao
        @param  move [in] move from parent
    */
    void setMove ( _Move move ) 
    {
        m_move = move ;
    }

    /*!
        @brief  get reference of UCT data 
        @author T.F. Liao
        @return reference of UCT data
    */
    StatisticData& getUctData() 
    {
        return m_uctStat ;
    }
	const StatisticData& getUctData() const
	{
		return m_uctStat ;
	}

    /*!
        @brief  set the childern infomation
        @author T.F. Liao
        @param  firstChild  [in] the UctNodePtr to first child
        @param  nChildren   [in] total number of children
    */
    void setChildren ( UctNodePtr firstChild, uint nChildren ) 
    {
		if(nChildren == 0)
			clearChildren();
        m_nChildren = nChildren ;
        m_firstChild = firstChild ;
    }

	virtual void clearChildren()
	{
		m_nChildren = 0 ;
		m_firstChild = UctNodePtr::NULL_PTR ; 
	}

};

#endif 
