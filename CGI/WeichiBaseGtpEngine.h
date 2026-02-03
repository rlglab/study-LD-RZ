#ifndef WEICHIBASEGTPENGINE_H
#define WEICHIBASEGTPENGINE_H

#include "BaseGtpEngine.h"
#include "concepts.h"
#include "BasicType.h"

class WeichiBaseGtpEngine : public BaseGtpEngine
{
protected:
	MCTS& m_mcts;
	WeichiPlayoutPolicy m_usedPolicy;
	Vector<uint,MAX_NUM_GRIDS> m_vCandidateMoves;

public:
	WeichiBaseGtpEngine( std::ostream& os, MCTS& instance )
		: BaseGtpEngine(os), m_mcts(instance)
	{
		RegisterFunction("empty", this, &WeichiBaseGtpEngine::cmdEmpty, 0);
	}
	virtual void Register()=0;
	virtual string getGoguiAnalyzeCommandsString()=0;

	void cmdEmpty();
	void ensureTerritory();
	void createDirectories();
	Move genPlayoutMove();
	Move genDCNNBestMove();
	Move genDCNNSoftmaxMove();
	Move genDCNNVNMove();

	virtual void checkAndReplaceCommand();
	void beforeRunCommand();
	void afterRunCommand();
};

#endif