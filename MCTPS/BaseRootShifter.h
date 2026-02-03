#ifndef BASEROOTSHIFTER_H
#define BASEROOTSHIFTER_H

#include "UctChildIterator.h"

template<class _Move, class _UctNode>
class BaseRootShifter
{
    typedef NodePtr<_UctNode> UctNodePtr ;
public:
    /*!
        @brief  functor determines how to move root node after a move is played
        @author T.F. Liao
        @param  root    [in] the original root node
        @param  move    [in] the move played
        @return UctNodePtr to the new root node, or NULL_PTR if no such node in tree
        the default implementation is to select the child with move is exactly the same as passed move
    */
    UctNodePtr operator()( UctNodePtr root, _Move move ) 
    {
        if ( root.isNull() || !root->hasChildren() ) 
            return UctNodePtr::NULL_PTR;
        
        for ( UctChildIterator<_UctNode> it(root) ; it ; ++ it ) {
            if ( it->getMove() == move ) {
                return it ;
            }
        }
        return UctNodePtr::NULL_PTR ;
    }
	void verifyAfterPlay( UctNodePtr pNode, bool bIsFirst=false )
	{
		;
	}
};

#endif
 
