#ifndef BASEPRUNEPOLICY_H
#define BASEPRUNEPOLICY_H

#include "types.h"
#include "NodePtr.h"
#include "Vector.h"

template<class _NodeClass>
class BasePrunePolicy
{
	typedef NodePtr<_NodeClass> UctNodePtr;
public:
    /*!
        @brief  determine which children should be kept
        @author T.F. Liao
        @return true if parent should be set as unexpanded, otherwise false
        @param  parent [in] pointer to parent node
        @param  keep [out] array that indicate which children should be kept
        @param  after_play [in] indicate the pruning is invoked after a play or not
        the default policy keep all children if parent visit count greater than threshold
        or discard all expanded child
    */
    bool operator() ( _NodeClass* parent, Vector<uint, MAX_NUM_CHILDREN>& keep, bool after_play )
    {
        if ( parent->getUctData().getCount() < 32 )
            return true ; // if visit count is very small, discard all
        keep.clear();
        for ( uint i=0 ; i<parent->getNumChildren() ; ++i ) 
            keep.push_back(i);

        return false; 
    }
	void beforePruning (UctNodePtr root) {}
    /*!
        @brief  update internal data after pruning nodes
        @author T.F. Liao
        @param  pool used size and maximum page size
    */
    void afterPruning ( const size_t pageSize, const size_t maxPageSize, _NodeClass* root ) {}
};


#endif 
