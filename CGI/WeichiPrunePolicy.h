#ifndef WEICHIPRUNEPOLICY_H
#define WEICHIPRUNEPOLICY_H

#include "WeichiUctNode.h"
#include "BasePrunePolicy.h"
#include "WeichiConfigure.h"
#include <iostream>

class WeichiPrunePolicy : public BasePrunePolicy<WeichiUctNode>
{
typedef NodePtr<WeichiUctNode> UctNodePtr;
private:
	double m_limit ;
public:
	WeichiPrunePolicy()
		: m_limit ( WeichiConfigure::ReuseThreshold )
	{}

	bool operator() ( WeichiUctNode* parent, Vector<uint, MAX_NUM_CHILDREN>& keep, bool after_play )
	{
		if ( parent->getUctDataWithoutVirtualLoss().getCount() < m_limit ) {
			if (parent->isCNNNode()) {
				parent->removeCNNNode();
				parent->removeInCNNQueue();
			}
			
			return true ; // if visit count is very small, discard all
		}

		keep.clear();
		for ( uint i=0 ; i<parent->getNumChildren() ; ++i ) {
			keep.push_back(i);
		}

		return false;
	}

	void beforePruning(UctNodePtr root) {}
	
	void afterPruning ( const size_t pageSize, const size_t maxPageSize, WeichiUctNode* root )
	{
		double rate = static_cast<double>(pageSize) / maxPageSize;
		CERR() << "After pruning, pool used rate: " << rate*100.0 << "% (" << pageSize << '/' << maxPageSize << ')' << std::endl;

		if ( rate > 0.25 ) { m_limit *= 2; }
		else { m_limit = WeichiConfigure::ReuseThreshold; }

		WeichiGlobalInfo::getRemoteInfo().m_cnnJobTable.revert();
		WeichiGlobalInfo::getCNNJobQueue().reCollectJobWithPruneTree(root);
	}
};

#endif 
 
