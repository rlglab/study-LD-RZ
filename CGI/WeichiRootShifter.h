#ifndef WEICHIROOTSHIFTER_H
#define WEICHIROOTSHIFTER_H

#include "WeichiUctNode.h"
#include "BaseRootShifter.h"

class WeichiRootShifter : public BaseRootShifter<WeichiMove, WeichiUctNode>
{
public:
	WeichiRootShifter() {}
	void verifyAfterPlay ( NodePtr<WeichiUctNode> pNode, bool bIsFirst = false )
	{
		/*vector<NodePtr<WeichiUctNode> > path;
		path.push_back(pNode);
		CERR() << "nodes in queue:" << endl;
		int count = 1;
		verifyNode(path, count);
		CERR() << "===" << endl;*/
	}

	void verifyNode(vector<NodePtr<WeichiUctNode> >& path, int& count)
	{
		NodePtr<WeichiUctNode> pNode = path.back();
		if (!pNode->isCNNNode() && pNode->isInCNNQueue()) {
			for (NodePtr<WeichiUctNode> node : path) {
				CERR() << count << ' ' << node->getMove().toGtpString() << ' ';
			}
			CERR() << endl;
			count++;
		}

		//pNode->releaseExpandToken();
		//pNode->releaseUpdateToken();
		if (pNode->hasChildren()) {
			for (uint i = 0; i < pNode->getNumChildren(); ++i) {
				path.push_back(pNode->getChild(i));
				verifyNode(path, count);
				path.pop_back();
			}
		}
	}
};

#endif 
 
