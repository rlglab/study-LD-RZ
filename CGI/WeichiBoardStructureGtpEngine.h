#ifndef WEICHIBOARDSTRUCTUREGTPENGINE_H
#define WEICHIBOARDSTRUCTUREGTPENGINE_H

#include "WeichiBaseGtpEngine.h"
#include "concepts.h"
#include "BasicType.h"
#include "GtpRGBColor.h"
#include "WeichiRZoneHandler.h"

class WeichiBoardStructureGtpEngine : public WeichiBaseGtpEngine
{
public:
	WeichiBoardStructureGtpEngine( std::ostream& os, MCTS& instance )
		: WeichiBaseGtpEngine(os,instance)
	{
	}
	void Register();
	string getGoguiAnalyzeCommandsString();

private:
	void cmdBoardStatus();
	void cmdClassSize();
	void cmdSoftGridNumber();
	void cmdCompactGridNumber();
	void cmdRadiusNeighbors();
	void cmdPattern3x3Attribute();
	void cmdClosedArea();
	void cmdClosedAreaOption();
	void cmdNakade();
	void cmdLifeAndDeath();
	void cmdEdgeLine();
	void cmdEdgeLineOption();
};

#endif