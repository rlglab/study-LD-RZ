#ifndef WEICHIGTPENGINE_H
#define WEICHIGTPENGINE_H

#include <iostream>
#include "WeichiBaseGtpEngine.h"
#include "WeichiBoardStructureGtpEngine.h"
#include "WeichiCommonGtpEngine.h"
#include "WeichiKBGtpEngine.h"
#include "WeichiParamGtpEngine.h"
#include "WeichiPlayoutGtpEngine.h"
#include "WeichiUctGtpEngine.h"
#include "WeichiCNNGtpEngine.h"
#include "WeichiTrainGtpEngine.h"
#include "WeichiLifeDeathGtpEngine.h"

class WeichiGtpEngine : public WeichiBaseGtpEngine
{
private:
	WeichiBoardStructureGtpEngine m_boardStructureGtp;
	WeichiCommonGtpEngine m_commonGtp;
	WeichiKbGtpEngine m_kbGtp;
	WeichiParamGtpEngine m_paramGtp;
	WeichiPlayoutGtpEngine m_playoutGtp;
	WeichiUctGtpEngine m_uctGtp;
	WeichiCNNGtpEngine m_cnnGtp;
	WeichiTrainGtpEngine m_trainGtp;
	WeichiLifeDeathGtpEngine m_lifedeathGtp;

public:
    WeichiGtpEngine( std::ostream& os, MCTS& instance )
		: WeichiBaseGtpEngine(os,instance)
		, m_boardStructureGtp(os,instance)
		, m_commonGtp(os,instance)
		, m_kbGtp(os,instance)
		, m_paramGtp(os,instance)
		, m_playoutGtp(os,instance)
		, m_uctGtp(os,instance)
		, m_cnnGtp(os,instance)
		, m_trainGtp(os,instance)
		, m_lifedeathGtp(os,instance)
	{
		createDirectories();
		Register();
	}
	void Register();
	string getGoguiAnalyzeCommandsString();

private:
	void cmdGoguiAnalyzeCommands();
};

#endif  
