#ifndef H_JOBLEVELMCTS
#define H_JOBLEVELMCTS
#include <string>
#include "concepts.h"
#include "WeichiUctNodeStatus.h"

using namespace std;

class WeichiJLMctsHandler
{
typedef NodePtr<WeichiUctNode> UctNodePtr;

public:
	WeichiJLMctsHandler(MCTS& instance);
	void playSgfPathString(string path);
	void set7x7JobLevelConfigurations();
	void solve();
	string getResult() const;

	static string normalizePathStringFromJLMCTS(string path);

private:
	void reset();
	void computeOutputInformation();
	string getGeneratedMoveResult() const;
	string getWinRateResult() const;
	string getICResult() const;
	string getVCResult() const;
	string getRootRZoneResult() const;
	string getCurrentRZoneResult() const;
	string getRootWinStatusResult() const;
	string getCurrentWinStatusResult() const;
	string getRootLegalBitMap() const;
	string getCurrentLegalBitMap() const;
	string getLastResult() const;
	string getAIEndMessage() const;
	string toEditorBitBoardValueString(WeichiBitBoard bm) const;
	
private:
	MCTS& m_mcts;
	UctNodePtr m_bestNode;
	WeichiMove m_best_move;	
	StatisticData m_bestSD;
};

#endif