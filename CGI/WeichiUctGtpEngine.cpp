#include "WeichiUctGtpEngine.h"
#include "WeichiLogger.h"

void WeichiUctGtpEngine::Register()
{
	RegisterFunction("uct_info", this, &WeichiUctGtpEngine::cmdUctInfo, 0);
	if( WeichiConfigure::mcts_use_rave ) { RegisterFunction("rave_info", this, &WeichiUctGtpEngine::cmdRaveInfo, 0); }
	RegisterFunction("bonus", this, &WeichiUctGtpEngine::cmdBonus, 0);
	RegisterFunction("territory", this, &WeichiUctGtpEngine::cmdTerritory, 0);
	RegisterFunction("criticality", this, &WeichiUctGtpEngine::cmdCriticality, 0);
	RegisterFunction("criticality_info", this, &WeichiUctGtpEngine::cmdCriticalityInfo, 1);
	RegisterFunction("static_move_feature", this, &WeichiUctGtpEngine::cmdStaticMoveFeature, 2);
	RegisterFunction("static_move_predictor", this, &WeichiUctGtpEngine::cmdStaticMovePredictor, 0);
	RegisterFunction("static_move_order", this, &WeichiUctGtpEngine::cmdStaticMoveOrder, 0);
	RegisterFunction("static_move_score", this, &WeichiUctGtpEngine::cmdStaticMoveScore, 0);
	RegisterFunction("uct_response_cache_move", this, &WeichiUctGtpEngine::cmdUCTResponseCacheMove, 0);
	RegisterFunction("uct_bw_cache_move", this, &WeichiUctGtpEngine::cmdUCTResponseCacheMove, 1);
	RegisterFunction("uct_wb_cache_move", this, &WeichiUctGtpEngine::cmdUCTResponseCacheMove, 1);
	RegisterFunction("uct_black_cache_record", this, &WeichiUctGtpEngine::cmdUCTCacheRecord, 0);
	RegisterFunction("uct_white_cache_record", this, &WeichiUctGtpEngine::cmdUCTCacheRecord, 0);
	RegisterFunction("uct_search_sequence", this, &WeichiUctGtpEngine::cmdUCTSearchSequence, 1);
}

string WeichiUctGtpEngine::getGoguiAnalyzeCommandsString()
{
	ostringstream oss;
	oss << "string/=====UCT Information=====/empty\n"
		<< "gfx/Uct Info/uct_info\n";

	if( WeichiConfigure::mcts_use_rave ) { oss << "gfx/Rave Info/rave_info\n"; }

	oss	<< "gfx/Bonus/bonus\n"
		<< "dboard/Territory/territory\n"
		<< "gfx/Criticality/criticality\n"
		<< "gfx/Criticality Info/criticality_info %p/\n"
		<< "string/Move Feature/static_move_feature %c %p\n"
		<< "cboard/Move Predictor/static_move_predictor\n"
		<< "gfx/Move Score/static_move_score\n"
		<< "gfx/Move Order/static_move_order\n"
		<< "gfx/UCT Response Cache Move/uct_response_cache_move\n"
		<< "gfx/UCT BW Cache Move/uct_bw_cache_move %p\n"
		<< "gfx/UCT WB Cache Move/uct_wb_cache_move %p\n"
		<< "gfx/UCT Black Cache Record/uct_black_cache_record\n"
		<< "gfx/UCT White Cache Record/uct_white_cache_record\n"
		;

	return oss.str();
}

void WeichiUctGtpEngine::cmdUctInfo()
{
	reply(GTP_SUCC, m_mcts.serializeTree(m_uctInfoSerializer));
}

void WeichiUctGtpEngine::cmdRaveInfo()
{
	reply(GTP_SUCC, m_mcts.serializeTree(m_raveInfoSerializer));
}

void WeichiUctGtpEngine::cmdBonus()
{
	reply(GTP_SUCC, m_mcts.serializeTree(m_bonusSerializer));
}

void WeichiUctGtpEngine::cmdTerritory()
{
	ensureTerritory();
	const Territory& territory = WeichiGlobalInfo::getSearchInfo().m_territory;
	ostringstream oss ;
	oss << endl;
	for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
		double prob = territory.getTerritory(*it);
		oss << setw(6) << std::fixed << setprecision(3) << prob << " " ;
		if( *it%MAX_BOARD_SIZE==WeichiConfigure::BoardSize-1 ) { oss << endl; }
	}
	reply(GTP_SUCC, oss.str());
}

void WeichiUctGtpEngine::cmdCriticality()
{
	ensureTerritory();
	const Territory& territory = WeichiGlobalInfo::getSearchInfo().m_territory;
	ostringstream oss ;
	oss << "INFLUENCE " ;
	for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
		double prob = territory.getCriticality(*it);
		oss << WeichiMove(*it).toGtpString() << " " << setw(6) << std::fixed << setprecision(3) << prob << " " ;
	}
	oss << "\nLABEL " ;
	for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
		double prob = territory.getCriticality(*it);
		oss << WeichiMove(*it).toGtpString() << " " << setw(6) << std::fixed << setprecision(3) << prob << " " ;
	}
	reply(GTP_SUCC, oss.str());
}

void WeichiUctGtpEngine::cmdCriticalityInfo()
{
	ensureTerritory();
	string sPosition = m_args[0];
	transform(sPosition.begin(),sPosition.end(),sPosition.begin(),::toupper);
	Move move( COLOR_NONE, sPosition );

	const Territory& territory = WeichiGlobalInfo::getSearchInfo().m_territory;
	territory.ShowCriticalityInfo(move.getPosition());

	reply(GTP_SUCC, "");
}

void WeichiUctGtpEngine::cmdStaticMoveFeature()
{
	Color c = toColor(m_args[0].at(0));
	string sMove = m_args[1];

	transform(sMove.begin(),sMove.end(), sMove.begin(), ::toupper);

	Move move ( c, sMove ) ;

	preCaculateForStaticEval();

	double dFeatureScore = 1.0f;
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	WeichiMoveFeatureHandler& moveFeatureHandler = state.m_moveFeatureHandler;
	const Vector<WeichiMoveFeature,MAX_NUM_GRIDS>& vMoveFeatureList = moveFeatureHandler.calculateFullBoardCandidates(c);
	WeichiMoveFeature moveFeature = vMoveFeatureList[move.getPosition()];

	// fixed feature
	WeichiMoveFeatureType type;
	while( (type=moveFeature.scanForwardFixedFeature())!=-1 ) {
		if( WeichiConfigure::mcts_use_mm ) { dFeatureScore = WeichiGammaTable::getMMFeatureValue(type); }
		else if( WeichiConfigure::mcts_use_lfr ) { dFeatureScore = WeichiGammaTable::getLFRFeatureValue(type); }
		CERR() << getWeichiMoveFeatureTypeName(type) << ' ' << dFeatureScore << '(' << type << ')' << '\n';
	}

	// radius pattern
	for( uint iRadius=0; iRadius<moveFeature.getRadiusPatternFeatureSize(); iRadius++ ) {
		uint index = moveFeature.getRadiusPatternFeature(iRadius);
		if( WeichiConfigure::mcts_use_mm ) { dFeatureScore = WeichiGammaTable::getMMFeatureValue(index); }
		else if( WeichiConfigure::mcts_use_lfr ) { dFeatureScore = WeichiGammaTable::getLFRFeatureValue(index); }
		CERR() << "Radius " << (iRadius+MIN_RADIUS_SIZE) << ' '<< dFeatureScore << '(' << index << ')' << '\n';
	}

	ostringstream oss;
	oss << "score: " << moveFeature.getScore() << '\n';

	reply(GTP_SUCC, oss.str());
}

void WeichiUctGtpEngine::cmdStaticMovePredictor()
{
	double dMinScore = DBL_MAX;
	double dMaxScore = -DBL_MAX;
	Vector<double,MAX_NUM_GRIDS> vScore;
	vScore.setAllAs(-DBL_MAX,MAX_NUM_GRIDS);

	preCaculateForStaticEval();

	const WeichiBoard& board = m_mcts.getState().m_board;
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	WeichiMoveFeatureHandler& moveFeatureHandler = state.m_moveFeatureHandler;

	moveFeatureHandler.calculateFullBoardCandidates(board.getToPlay());
	const Vector<CandidateEntry,MAX_NUM_GRIDS>& vUctCandidateList = moveFeatureHandler.getUCTCandidateList();

	for( uint i=0; i<vUctCandidateList.size(); i++ ) {
		vScore[vUctCandidateList[i].getPosition()] = vUctCandidateList[i].getScore();
		if( vScore[vUctCandidateList[i].getPosition()]>dMaxScore ) { dMaxScore = vScore[vUctCandidateList[i].getPosition()]; }
		if( vScore[vUctCandidateList[i].getPosition()]<dMinScore ) { dMinScore = vScore[vUctCandidateList[i].getPosition()]; }
	}

	ostringstream oss;
	GtpColorGradient colorGradient(RGB_GREEN,RGB_BLUE,dMinScore,dMaxScore);

	oss << endl;
	CERR() << "Min value=" << dMinScore << "; Max value=" << dMaxScore << endl;
	for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
		if( vScore[*it]==-DBL_MAX ) { oss << "\"\" "; }
		else { oss << colorGradient.colorOf(vScore[*it]).toString() << " "; }
		if( *it%MAX_BOARD_SIZE==WeichiConfigure::BoardSize-1 ) { oss << endl; }
	}

	reply(GTP_SUCC, oss.str());
}

void WeichiUctGtpEngine::cmdStaticMoveOrder()
{
	preCaculateForStaticEval();

	const WeichiBoard& board = m_mcts.getState().m_board;
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	WeichiMoveFeatureHandler& moveFeatureHandler = state.m_moveFeatureHandler;

	moveFeatureHandler.calculateFullBoardCandidates(board.getToPlay());
	const Vector<CandidateEntry,MAX_NUM_GRIDS>& vUCTCandidateList = moveFeatureHandler.getUCTCandidateList();

	ostringstream oss;
	uint preOrder = 1;
	double dPreMoveScore = vUCTCandidateList[0].getScore();

	oss << "LABEL ";
	// TODO: add pass order
	for( uint i=0; i<vUCTCandidateList.size(); i++ ) {
		if( vUCTCandidateList[i].getPosition()==PASS_MOVE.getPosition() ) { continue; }
		if( vUCTCandidateList[i].getScore()==dPreMoveScore ) {
			oss << WeichiMove(vUCTCandidateList[i].getPosition()).toGtpString() << " " << preOrder << " ";
		} else {
			oss << WeichiMove(vUCTCandidateList[i].getPosition()).toGtpString() << " " << (i+1) << " ";
			dPreMoveScore = vUCTCandidateList[i].getScore();
			preOrder = i+1;
		}
	}
	oss << endl;

	reply(GTP_SUCC, oss.str());
}

void WeichiUctGtpEngine::cmdStaticMoveScore()
{
	double dMinScore = DBL_MAX;
	double dMaxScore = -DBL_MAX;

	ostringstream oss;
	oss << "LABEL ";

	preCaculateForStaticEval();

	const WeichiBoard& board = m_mcts.getState().m_board;
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	WeichiMoveFeatureHandler& moveFeatureHandler = state.m_moveFeatureHandler;

	moveFeatureHandler.calculateFullBoardCandidates(board.getToPlay());
	const Vector<CandidateEntry,MAX_NUM_GRIDS>& vUctCandidateList = moveFeatureHandler.getUCTCandidateList();

	for( uint i=0; i<vUctCandidateList.size(); i++ ) {
		oss << WeichiMove(vUctCandidateList[i].getPosition()).toGtpString() << ' ' << vUctCandidateList[i].getScore() << ' ';
		if( vUctCandidateList[i].getScore()>dMaxScore ) { dMaxScore = vUctCandidateList[i].getScore(); }
		if( vUctCandidateList[i].getScore()<dMinScore ) { dMinScore = vUctCandidateList[i].getScore(); }
	}

	CERR() << "Min value=" << dMinScore << "; Max value=" << dMaxScore << endl;
	reply(GTP_SUCC, oss.str());
}

void WeichiUctGtpEngine::cmdUCTResponseCacheMove()
{
	const WeichiThreadState& state = m_mcts.getState();
	const WeichiBoard& board = state.m_board;

	WeichiMove preMove;
	Color turnColor = board.getToPlay();
	if( m_command=="uct_response_cache_move" ) {
		if( !board.hasPrevMove() ) { reply(GTP_SUCC, ""); return; }
		else { preMove = board.getPrevMove(); }
	} else {
		string sPosition = m_args[0];
		transform(sPosition.begin(),sPosition.end(),sPosition.begin(),::toupper);
		preMove = Move(COLOR_NONE,sPosition);

		if( m_command=="uct_bw_cache_move" ) { turnColor = COLOR_WHITE; }
		else if( m_command=="uct_wb_cache_move" ) { turnColor = COLOR_BLACK; }
		else { reply(GTP_FAIL, "error command!"); return; }
	}

	ostringstream oss;
	WeichiMove replyMove = WeichiGlobalInfo::getSearchInfo().m_moveCache.getReplyMove(board, preMove);
	oss << replyMove.toGtpString();

	reply(GTP_SUCC, oss.str());
}

void WeichiUctGtpEngine::cmdUCTCacheRecord()
{
	Color findColor = COLOR_NONE;
	if( m_command=="uct_black_cache_record" ) { findColor = COLOR_BLACK; }
	else if( m_command=="uct_white_cache_record" ) { findColor = COLOR_WHITE; }
	else { reply(GTP_FAIL, "error command!"); return; }

	const WeichiThreadState& state = m_mcts.getState();
	const WeichiBoard& board = state.m_board;

	ostringstream oss;
	oss << "SQUARE";
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiMove move(findColor,*it);
		WeichiMove replyMove = WeichiGlobalInfo::getSearchInfo().m_moveCache.getReplyMove(board, move);

		if (!replyMove.isPass()) { oss << ' ' << move.toGtpString(); }
	}

	reply(GTP_SUCC, oss.str());
}

void WeichiUctGtpEngine::cmdUCTSearchSequence()
{
	SgfLoader sgfLoader;
	string sSgfString = m_args[0];
	sgfLoader.parseFromString(sSgfString);

	m_mcts.newGame();
	for (uint i = 0; i < sgfLoader.getPlayMove().size(); i++) { m_mcts.play(sgfLoader.getPlayMove()[i]); }
	
	WeichiMove playMove = m_mcts.genmove(false);
	string sResult = WeichiLogger::getTreeInfo(m_mcts.getState().m_board, playMove, true, WeichiLogger::isBestSequenceImportantChild);

	reply(GTP_SUCC, sResult);
}

void WeichiUctGtpEngine::preCaculateForStaticEval()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());

	state.startPreSimulation();
	state.endPreSimulation();
}

// for Serialize
std::string WeichiUctGtpEngine::WeichiMoveUctInfoSerializer::operator() ( UctNodePtr node )
{
	ostringstream oss ;
	oss << "INFLUENCE" << std::fixed << std::setprecision(2);
	for ( UctChildIterator<WeichiUctNode> it(node) ; it ; ++it ) {
		oss << " " << it->getMove().toGtpString() ;
		double value = it->getUctData().getMean() * 2 - 1;
		if(it->getColor() == COLOR_WHITE)
			value *= -1;
		oss << " " << value;
	}
	oss << "\n";
	oss << "LABEL" << std::fixed << std::setprecision(0);
	for ( UctChildIterator<WeichiUctNode> it(node) ; it ; ++it ) {
		oss << " " << it->getMove().toGtpString() ;
		oss << " " << it->getUctData().getCount();
	}
	oss << "\n";
	return oss.str();
}

std::string WeichiUctGtpEngine::WeichiMoveBonusSerializer::operator() ( UctNodePtr node )
{
	ostringstream oss ;
	double dBonus;
	oss << "LABEL" << std::fixed << std::setprecision(3);
	for ( UctChildIterator<WeichiUctNode> it(node) ; it ; ++it ) {
		if( WeichiConfigure::mcts_use_mm ) {
			double dGamma = exp(it->getProb()) - 1;
			dBonus = dGamma / (1 + dGamma);
		}
		oss << " " << it->getMove().toGtpString();
		oss << " " << dBonus;
	}
	oss << "\n";
	return oss.str();
}

std::string WeichiUctGtpEngine::WeichiMoveRaveInfoSerializer::operator() ( UctNodePtr node )
{
	ostringstream oss ;
	oss << "INFLUENCE" << std::fixed << std::setprecision(2);
	for ( UctChildIterator<WeichiUctNode> it(node) ; it ; ++it ) {
		oss << " " << it->getMove().toGtpString() ;
		double value = it->getRaveData().getMean() * 2 - 1;
		if(it->getColor() == COLOR_WHITE)
			value *= -1;
		oss << " " << value;
	}
	oss << "\n";
	oss << "LABEL" << std::fixed << std::setprecision(0);
	for ( UctChildIterator<WeichiUctNode> it(node) ; it ; ++it ) {
		oss << " " << it->getMove().toGtpString() ;
		oss << " " << it->getRaveData().getCount();
	}
	oss << "\n";
	return oss.str();
}