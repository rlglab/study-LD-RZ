#ifndef WEICHIPLAYOUTGTPENGINE_H
#define WEICHIPLAYOUTGTPENGINE_H

#include "WeichiBaseGtpEngine.h"
#include "concepts.h"
#include "BasicType.h"

class WeichiPlayoutGtpEngine : public WeichiBaseGtpEngine
{
public:
	WeichiPlayoutGtpEngine( std::ostream& os, MCTS& instance )
		: WeichiBaseGtpEngine(os,instance)
	{
	}
	void Register();
	string getGoguiAnalyzeCommandsString();

private:
	void cmdPlayoutPolicyMoves();
	void cmdProbabilityPlayoutTable();
	void cmdProbabilityPlayoutScore();
	void cmdSpecificPolicyCandidates();
	void cmdPlayoutStatistic();
	void cmdPlayoutCacheRegion();

	void findPlayoutPolicyCandidates( Vector<uint,MAX_NUM_GRIDS>& vCandidates, WeichiPlayoutPolicy policy );
};

#endif