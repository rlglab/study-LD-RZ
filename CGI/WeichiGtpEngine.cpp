#include "WeichiGtpEngine.h"

void WeichiGtpEngine::Register()
{
	RegisterFunction("gogui-analyze_commands", this, &WeichiGtpEngine::cmdGoguiAnalyzeCommands, 0);

	m_commonGtp.Register();
	if( WeichiConfigure::EnableDevelopmentGoguiCommand ) {
		m_boardStructureGtp.Register();
		m_kbGtp.Register();
		m_paramGtp.Register();
		m_playoutGtp.Register();
		m_uctGtp.Register();
		m_cnnGtp.Register();
		m_trainGtp.Register();
		m_lifedeathGtp.Register();
	}
}

string WeichiGtpEngine::getGoguiAnalyzeCommandsString()
{
	ostringstream oss;
	oss << m_commonGtp.getGoguiAnalyzeCommandsString();

	if( WeichiConfigure::EnableDevelopmentGoguiCommand ) {
		oss << m_paramGtp.getGoguiAnalyzeCommandsString()
			<< m_lifedeathGtp.getGoguiAnalyzeCommandsString()
			<< m_boardStructureGtp.getGoguiAnalyzeCommandsString()
			<< m_cnnGtp.getGoguiAnalyzeCommandsString()
			<< m_uctGtp.getGoguiAnalyzeCommandsString()
			<< m_playoutGtp.getGoguiAnalyzeCommandsString()
			<< m_kbGtp.getGoguiAnalyzeCommandsString()
			<< m_trainGtp.getGoguiAnalyzeCommandsString();
	}

	return oss.str();
}

void WeichiGtpEngine::cmdGoguiAnalyzeCommands()
{
	reply(GTP_SUCC, getGoguiAnalyzeCommandsString());
}