#include "WeichiPlayoutGtpEngine.h"

void WeichiPlayoutGtpEngine::Register()
{
	RegisterFunction("playout_policy_moves", this, &WeichiPlayoutGtpEngine::cmdPlayoutPolicyMoves, 0);
	if( WeichiConfigure::use_probability_playout ) { RegisterFunction("probability_playout_table", this, &WeichiPlayoutGtpEngine::cmdProbabilityPlayoutTable, 0); }
	if( WeichiConfigure::use_probability_playout ) { RegisterFunction("probability_playout_score", this, &WeichiPlayoutGtpEngine::cmdProbabilityPlayoutScore, 1); }
	if( WeichiConfigure::sim_kill_suicide ) { RegisterFunction("playout_kill_suicide", this, &WeichiPlayoutGtpEngine::cmdSpecificPolicyCandidates, 0); }
	if( WeichiConfigure::sim_reply_save_1lib ) { RegisterFunction("playout_reply_save_1lib", this, &WeichiPlayoutGtpEngine::cmdSpecificPolicyCandidates, 0); }
	if( WeichiConfigure::sim_reply_kill_2lib ) { RegisterFunction("playout_reply_kill_2lib", this, &WeichiPlayoutGtpEngine::cmdSpecificPolicyCandidates, 0); }
	if( WeichiConfigure::sim_reply_save_2lib ) { RegisterFunction("playout_reply_save_2lib", this, &WeichiPlayoutGtpEngine::cmdSpecificPolicyCandidates, 0); }
	if( WeichiConfigure::sim_fight_ko ) { RegisterFunction("playout_fight_ko", this, &WeichiPlayoutGtpEngine::cmdSpecificPolicyCandidates, 0); }
	if( WeichiConfigure::sim_reply_nakade ) { RegisterFunction("playout_nakade", this, &WeichiPlayoutGtpEngine::cmdSpecificPolicyCandidates, 0); }
	if( WeichiConfigure::sim_reply_good_pattern ) { RegisterFunction("playout_good_pattern", this, &WeichiPlayoutGtpEngine::cmdSpecificPolicyCandidates, 0); }
	RegisterFunction("playout_statistic", this, &WeichiPlayoutGtpEngine::cmdPlayoutStatistic, 0);
	if( WeichiConfigure::DoPlayoutCacheLog ) { 
		RegisterFunction("playout_cache_play_region", this, &WeichiPlayoutGtpEngine::cmdPlayoutCacheRegion, 1, MAX_NUM_GRIDS);
		RegisterFunction("playout_cache_result_region", this, &WeichiPlayoutGtpEngine::cmdPlayoutCacheRegion, 1, MAX_NUM_GRIDS);
	}
}

string WeichiPlayoutGtpEngine::getGoguiAnalyzeCommandsString()
{
	ostringstream oss;
	oss << "string/=====Playout Information=====/empty\n"
		<< "gfx/playout Policy Moves/playout_policy_moves\n";

	if( WeichiConfigure::use_probability_playout ) { oss << "gfx/probability playout table/probability_playout_table\n"; }
	if( WeichiConfigure::use_probability_playout ) { oss << "string/probability playout score/probability_playout_score %p\n"; }
	if( WeichiConfigure::sim_kill_suicide ) { oss << "gfx/playout kill suicide/playout_kill_suicide\n"; }
	if( WeichiConfigure::sim_reply_save_1lib ) { oss << "gfx/playout replay save 1Lib/playout_reply_save_1lib\n"; }
	if( WeichiConfigure::sim_reply_kill_2lib ) { oss << "gfx/playout replay kill 2Lib/playout_reply_kill_2lib\n"; }
	if( WeichiConfigure::sim_reply_save_2lib ) { oss << "gfx/playout replay save 2Lib/playout_reply_save_2lib\n"; }
	if( WeichiConfigure::sim_fight_ko ) { oss << "gfx/playout fight ko/playout_fight_ko\n"; }
	if( WeichiConfigure::sim_reply_nakade ) { oss << "gfx/playout nakade/playout_nakade\n"; }
	if( WeichiConfigure::sim_reply_good_pattern ) { oss << "gfx/playout good pattern/playout_good_pattern\n"; }
	if( WeichiConfigure::DoPlayoutCacheLog ) {
		oss << "gfx/playout cache play region/playout_cache_play_region %P\n";
		oss << "gfx/playout cache result region/playout_cache_result_region %P\n";
	}

	return oss.str();
}

void WeichiPlayoutGtpEngine::cmdPlayoutPolicyMoves()
{
	genPlayoutMove();

	ostringstream oss;
	oss << "SQUARE";
	for(uint i=0; i<m_vCandidateMoves.size(); i++) {
		oss << ' ' << WeichiMove(m_vCandidateMoves[i]).toGtpString();
	}
	oss << "\nTEXT " << getWeichiPlayoutPolicyString(m_usedPolicy);

	reply(GTP_SUCC, oss.str());
}

void WeichiPlayoutGtpEngine::cmdProbabilityPlayoutTable()
{
	const WeichiThreadState& state = m_mcts.getState();
	const WeichiBoard& board = state.m_board;
	WeichiProbabilityPlayoutHandler& probabilityHandler = const_cast<WeichiProbabilityPlayoutHandler&>(board.getProbabilityPlayoutHandler());

	Color turnColor = board.getToPlay();
	WeichiPlayoutAgent agent(const_cast<WeichiThreadState&>(state));
	agent.calcualteLocalPlayoutFeature(board.getPrevMove());

	const Vector<ProbabilityGrid,MAX_NUM_GRIDS>& vProbGrid = probabilityHandler.getProbGrid(turnColor);
	const double dTotalScore = probabilityHandler.getProbabilityTable(turnColor).getTotalScore();

	ostringstream oss ;
	oss << "INFLUENCE" << std::fixed << std::setprecision(2);
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiGrid& grid = board.getGrid(*it);
		if( !grid.isEmpty() ) { continue; }
		oss << " " << WeichiMove(*it).toGtpString() << " " << vProbGrid[*it].getScore() / dTotalScore;
	}
	oss << "\n";
	oss << "LABEL" << std::fixed << std::setprecision(2);
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiGrid& grid = board.getGrid(*it);
		if( !grid.isEmpty() ) { continue; }
		oss << " " << WeichiMove(*it).toGtpString() << " " << vProbGrid[*it].getScore();
	}
	oss << "\n";

	probabilityHandler.recover(turnColor);

	CERR() << "Total Score = " << dTotalScore << endl;
	reply(GTP_SUCC, oss.str());
}

void WeichiPlayoutGtpEngine::cmdProbabilityPlayoutScore()
{
	string sPosition = m_args[0];
	transform(sPosition.begin(),sPosition.end(),sPosition.begin(),::toupper);
	Move move(COLOR_BLACK, sPosition);

	const WeichiThreadState& state = m_mcts.getState();
	const WeichiBoard& board = state.m_board;
	WeichiProbabilityPlayoutHandler& probabilityHandler = const_cast<WeichiProbabilityPlayoutHandler&>(board.getProbabilityPlayoutHandler());

	Color turnColor = board.getToPlay();

	WeichiPlayoutAgent agent(const_cast<WeichiThreadState&>(state));
	agent.calcualteLocalPlayoutFeature(board.getPrevMove());

	const double dTotalScore = probabilityHandler.getProbabilityTable(turnColor).getTotalScore();
	const Vector<ProbabilityGrid,MAX_NUM_GRIDS>& vProbGrid = probabilityHandler.getProbGrid(turnColor);
	const ProbabilityGrid& probGrid = vProbGrid[move.getPosition()];

	CERR() << "Total Score = " << dTotalScore << ", move score = " << probGrid.getScore() << endl;
	Vector<uint,MAX_NUM_PLAYOUT_FEATURE_GROUPS> vFeatures = probGrid.getFeatureIndexs();
	for( uint i=0; i<vFeatures.size(); i++ ) {
		if( i==vFeatures.size()-1 ) { CERR() << "pattern3x3 " << vFeatures[i]; }
		else { CERR() << getWeichiPlayoutPolicyString(static_cast<WeichiPlayoutPolicy>(vFeatures[i]-2*MAX_3X3PATTERN_SIZE)); }
		CERR() << "(" << vFeatures[i] << ") :" << WeichiGammaTable::getPlayoutFeatureValue(vFeatures[i]) << endl;
	}

	probabilityHandler.recover(turnColor);

	reply(GTP_SUCC, "");
}

void WeichiPlayoutGtpEngine::cmdSpecificPolicyCandidates()
{
	WeichiPlayoutPolicy policy;
	if( m_command=="playout_kill_suicide" ) { policy = POLICY_KILL_SUICIDE; }
	else if( m_command=="playout_reply_save_1lib" ) { policy = POLICY_REPLY_SAVE_1LIB; }
	else if( m_command=="playout_reply_kill_2lib" ) { policy = POLICY_REPLY_KILL_2LIB; }
	else if( m_command=="playout_reply_save_2lib" ) { policy = POLICY_REPLY_SAVE_2LIB; }
	else if( m_command=="playout_fight_ko" ) { policy = POLICY_FIGHT_KO; }
	else if( m_command=="playout_good_pattern" ) { policy = POLICY_REPLY_GOOD_PATTERN; }
	else { reply(GTP_FAIL, "error command!"); }

	ostringstream oss;
	oss << "SQUARE";
	findPlayoutPolicyCandidates(m_vCandidateMoves,policy);
	for(uint i=0; i<m_vCandidateMoves.size(); i++) {
		oss << ' ' << WeichiMove(m_vCandidateMoves[i]).toGtpString();
	}
	reply(GTP_SUCC, oss.str());
}

void WeichiPlayoutGtpEngine::cmdPlayoutStatistic()
{
	if( !WeichiConfigure::ShowPlayoutPolicyStatistic ) {
		reply(GTP_FAIL, "playout statistic calculation off");
		return;
	}

	reply(GTP_SUCC, WeichiGlobalInfo::get()->getPolicyStatString(vector<WeichiPlayoutPolicy>()));
}

void WeichiPlayoutGtpEngine::cmdPlayoutCacheRegion()
{
	WeichiBitBoard bmRegion;
	for( uint i=0; i<m_args.size(); i++ ) {
		string sMove = m_args[i];
		transform(sMove.begin(),sMove.end(), sMove.begin(), ::toupper);
		Move move(COLOR_NONE,sMove);
		
		bmRegion.SetBitOn(move.getPosition());
	}
	CERR() << endl;

	if( m_command=="playout_cache_play_region" ) { WeichiGlobalInfo::getLogInfo().m_cacheLogger.setCachePlayRegion(bmRegion); }
	else if( m_command=="playout_cache_result_region" ) { WeichiGlobalInfo::getLogInfo().m_cacheLogger.setCacheResultRegion(bmRegion); }

	reply(GTP_SUCC, "");
}

void WeichiPlayoutGtpEngine::findPlayoutPolicyCandidates( Vector<uint,MAX_NUM_GRIDS>& vCandidates, WeichiPlayoutPolicy policy )
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	WeichiPlayoutAgent playoutAgent(state);
	WeichiMove lastmove = PASS_MOVE;

	if( state.m_board.hasPrevMove() ) { lastmove = state.m_board.getPrevMove(); }
	if( !lastmove.isPass() ) { playoutAgent.findSpecificPolicyCandidates(lastmove,m_vCandidateMoves,policy); }
}