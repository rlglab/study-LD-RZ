#ifndef WEICHIMOVEDECISIONMAKER_H
#define WEICHIMOVEDECISIONMAKER_H

#include "BaseDecisionMaker.h"
#include "WeichiMove.h"
#include "WeichiUctNode.h"
#include "WeichiThreadState.h"

class WeichiMoveDecisionMaker : public BaseDecisionMaker<WeichiMove, WeichiUctNode>
{
	typedef NodePtr<WeichiUctNode> UctNodePtr ;
public:
	WeichiMove operator() ( const WeichiThreadState& state, UctNodePtr pRoot) ;
private:
	bool checkStableForEarlyPass( const WeichiThreadState& state, Color toPlay, int& neutralPosition ) ;
	bool checkNeutralPosition( const WeichiThreadState& state, Color toPlay, int& neutralPosition );
};

#endif  
