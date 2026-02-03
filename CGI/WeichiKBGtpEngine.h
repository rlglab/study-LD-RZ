#ifndef WEICHIKBGTPENGINE_H
#define WEICHIKBGTPENGINE_H

#include "WeichiBaseGtpEngine.h"
#include "concepts.h"
#include "BasicType.h"

class WeichiKbGtpEngine : public WeichiBaseGtpEngine
{
public:
	WeichiKbGtpEngine( std::ostream& os, MCTS& instance )
		: WeichiBaseGtpEngine(os,instance)
	{
	}
	void Register();
	string getGoguiAnalyzeCommandsString();

private:
	void cmdSaveBlock();
	void cmdKillBlock();
	void cmdSimpleFastLadderPath();
	void cmdComplicateLadderPath();
	void cmdBadMoves();
	void cmdLifeTerritory();
	void cmdPotentialTrueEye();
	void cmdTwoLibertyFalseEye();
	void cmdSelfAtari();
	void cmdMutualAtariMoves();
	void cmdFillingOwnEyeShapeMoves();
	void cmdNotMakingNakadeInOppCA();
};

#endif