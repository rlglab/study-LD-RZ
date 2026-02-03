#ifndef BASEDECISIONMAKER_H
#define BASEDECISIONMAKER_H

#include "NodePtr.h"
#include "UctChildIterator.h"
#include "StatisticData.h"
#include <cfloat>

template<class _Move, class _UctNode>
class BaseDecisionMaker
{
    typedef NodePtr<_UctNode> UctNodePtr ;
public:
    /*!
        @brief  functor determines how to select a move as final decision
        @author T.F. Liao
        @param  node    [in] the node to make decision
        @return _Move as the best move selected, or a game-specified default move in case no children available
        the default implementation is to select move in the child with max visit count
    */
    _Move operator()( UctNodePtr node ) 
    {
        if ( node.isNull() || !node->hasChildren() ) 
            return _Move();

        _Move best = _Move();
        StatisticData::data_type bestVisit = - DBL_MAX ;

        for ( UctChildIterator<_UctNode> it(node) ; it ; ++ it ) {
            if ( it->getUctData().getCount() > bestVisit ) {
                bestVisit = it->getUctData().getCount();
                best = it->getMove();
            }
        }
        return best ;
    }
};

#endif 
