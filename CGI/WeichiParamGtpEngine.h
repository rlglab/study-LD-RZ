#ifndef WEICHIPARAMGTPENGINE_H
#define WEICHIPARAMGTPENGINE_H

#include "WeichiBaseGtpEngine.h"
#include "concepts.h"
#include "BasicType.h"

class WeichiParamGtpEngine : public WeichiBaseGtpEngine
{
public:
	WeichiParamGtpEngine( std::ostream& os, MCTS& instance )
		: WeichiBaseGtpEngine(os,instance)
	{
	}
	void Register();
	string getGoguiAnalyzeCommandsString();

private:
	void cmdParamSetting();
	void cmdSetRandomSeed();
	void cmdSetDKomiParam();
};

#endif