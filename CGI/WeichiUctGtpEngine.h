#ifndef WEICHIUCTGTPENGINE_H
#define WEICHIUCTGTPENGINE_H

#include "WeichiBaseGtpEngine.h"
#include "concepts.h"
#include "BasicType.h"
#include "GtpColorGradient.h"

class WeichiUctGtpEngine : public WeichiBaseGtpEngine
{
private:
	typedef NodePtr<WeichiUctNode> UctNodePtr ;

	class WeichiMoveUctInfoSerializer {
	public:
		std::string operator() ( UctNodePtr node );
	};

	class WeichiMoveRaveInfoSerializer {
	public:
		std::string operator() ( UctNodePtr node );
	};

	class WeichiMoveBonusSerializer {
	public:
		std::string operator() ( UctNodePtr node );
	};

	WeichiMoveUctInfoSerializer m_uctInfoSerializer;
	WeichiMoveRaveInfoSerializer m_raveInfoSerializer;
	WeichiMoveBonusSerializer m_bonusSerializer;

public:
	WeichiUctGtpEngine( std::ostream& os, MCTS& instance )
		: WeichiBaseGtpEngine(os,instance)
	{
	}
	void Register();
	string getGoguiAnalyzeCommandsString();

private:
	void cmdUctInfo();
	void cmdRaveInfo();
	void cmdBonus();
	void cmdTerritory();
	void cmdCriticality();
	void cmdCriticalityInfo();
	void cmdStaticMoveFeature();
	void cmdStaticMovePredictor();
	void cmdStaticMoveOrder();
	void cmdStaticMoveScore();
	void cmdUCTResponseCacheMove();
	void cmdUCTCacheRecord();
	void cmdUCTSearchSequence();

	void preCaculateForStaticEval();
};

#endif