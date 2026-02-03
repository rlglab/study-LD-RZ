#include "WeichiParamGtpEngine.h"

void WeichiParamGtpEngine::Register()
{
	RegisterFunction("param_setting", this, &WeichiParamGtpEngine::cmdParamSetting, 0, 2);
	RegisterFunction("set_random_seed", this, &WeichiParamGtpEngine::cmdSetRandomSeed, 1 );
	RegisterFunction("set_dykomi_param", this, &WeichiParamGtpEngine::cmdSetDKomiParam, 2 );
}

string WeichiParamGtpEngine::getGoguiAnalyzeCommandsString()
{
	ostringstream oss;
	oss << "string/=====Param setting=====/empty\n"
		<< "param/Param Setting/param_setting\n"
		;

	return oss.str();
}

void WeichiParamGtpEngine::cmdParamSetting()
{
	string sGenMovePolicyString[5] = { "0.UCT_policy","1.playout_policy","2.dcnn_bestmove_policy","3.dcnn_softmax_policy","4.dcnn_vn_policy" };
	ostringstream oss;
	if ( m_args.size() == 0 ) {
		oss << "[bool] live_gfx " << WeichiConfigure::LiveGfx << '\n'
			<< "[list/0.UCT_policy/1.playout_policy/2.dcnn_bestmove_policy/3.dcnn_softmax_policy/4.dcnn_vn_policy] genmove_policy " << sGenMovePolicyString[WeichiConfigure::GenMovePolicy] << '\n';
	} else if( m_args.size() == 2) {
		if( m_args[0] == "live_gfx" ) {
			WeichiConfigure::LiveGfx = (m_args[1] == "1");
		} else if(m_args[0] == "genmove_policy") {
			WeichiConfigure::GenMovePolicy = m_args[1][0] - '0';
		} else {
			reply(GTP_FAIL, "unknown parameter " + m_args[0]);
		}
	} else {
		reply(GTP_FAIL, "need 0 or 2 arguments");
	}

	reply(GTP_SUCC, oss.str());
}

void WeichiParamGtpEngine::cmdSetRandomSeed()
{
	int seed = atoi(m_args[0].c_str());
	Random::reset(seed);
	reply(GTP_SUCC, "");
}

void WeichiParamGtpEngine::cmdSetDKomiParam()
{
	int iIndex = atoi(m_args[0].c_str());
	double dWeight = atof(m_args[1].c_str());

	switch( iIndex ) {
	case 1:	WeichiConfigure::dkomi_start_step = static_cast<int>(dWeight);	break;
	case 2:	WeichiConfigure::dkomi_4komi = static_cast<int>(dWeight);	break;
	case 3:	WeichiConfigure::dkomi_minus = static_cast<int>(dWeight);	break;
	case 4:	WeichiConfigure::dkomi_slop = static_cast<float>(dWeight);	break;
	case 5:	WeichiConfigure::dkomi_shift = static_cast<float>(dWeight);	break;
	case 6:	WeichiConfigure::dkomi_winrate_weight = static_cast<float>(dWeight);	break;
	default:	reply(GTP_FAIL, "index is out of range");	return;

	}

	reply(GTP_SUCC, "");
}