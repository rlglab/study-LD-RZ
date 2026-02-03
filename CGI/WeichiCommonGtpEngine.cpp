#include "WeichiCommonGtpEngine.h"
#include "WeichiGtpUtil.h"
#include "WeichiDynamicKomi.h"
#include "WeichiLogger.h"

void WeichiCommonGtpEngine::Register()
{
	setName(WeichiConfigure::sGtpProgramName, WeichiConfigure::sGtpProgramVersion);

	RegisterFunction("name", this, &WeichiCommonGtpEngine::cmdName, 0);
	RegisterFunction("version", this, &WeichiCommonGtpEngine::cmdVersion, 0);
	RegisterFunction("boardsize", this, &WeichiCommonGtpEngine::cmdBoardsize, 1 );
	RegisterFunction("showboard", this, &WeichiCommonGtpEngine::cmdShowboard, 0);
	RegisterFunction("clear_board", this, &WeichiCommonGtpEngine::cmdClearBoard, 0);
	RegisterFunction("sleep", this, &WeichiCommonGtpEngine::cmdSleep, 1);
	RegisterFunction("loadsgf", this, &WeichiCommonGtpEngine::cmdLoadsgf, 1, 2);
	RegisterFunction("loadjson", this, &WeichiCommonGtpEngine::cmdLoadjson, 1);
	RegisterFunction("komi", this, &WeichiCommonGtpEngine::cmdKomi, 1); 
	RegisterFunction("fixed_handicap", this, &WeichiCommonGtpEngine::cmdFixedHandicap, 1);
	RegisterFunction("place_free_handicap", this, &WeichiCommonGtpEngine::cmdPlaceFreeHandicap, 1);
	RegisterFunction("set_free_handicap", this, &WeichiCommonGtpEngine::cmdSetFreeHandicap, 1, 25);
	RegisterFunction("time_settings", this, &WeichiCommonGtpEngine::cmdTimeSettings, 3);
	RegisterFunction("time_left", this, &WeichiCommonGtpEngine::cmdTimeLeft, 3);
	RegisterFunction("play", this, &WeichiCommonGtpEngine::cmdPlay, 2);
	RegisterFunction("gogui-play_sequence", this, &WeichiCommonGtpEngine::cmdGoguiPlaySequence, 2, 99999);
	RegisterFunction("genmove", this, &WeichiCommonGtpEngine::cmdGenmove, 1);
	RegisterFunction("reg_genmove", this, &WeichiCommonGtpEngine::cmdRegGenmove, 1);	
    RegisterFunction("kgs-genmove_cleanup", this, &WeichiCommonGtpEngine::cmdKgsGenmoveCleanup, 1);
	RegisterFunction("peek", this, &WeichiCommonGtpEngine::cmdPeek, 1);
	RegisterFunction("win_rate", this, &WeichiCommonGtpEngine::cmdWinRate, 0);
	RegisterFunction("final_score", this, &WeichiCommonGtpEngine::cmdFinalScore, 0);
	RegisterFunction("final_status_list", this, &WeichiCommonGtpEngine::cmdFinalStatusList, 1, 1);
	RegisterFunction("set_internal_komi", this, &WeichiCommonGtpEngine::cmdSetInternalKomi, 1, 1);
	RegisterFunction("mcts_bv", this, &WeichiCommonGtpEngine::cmdMctsBV, 1);
	RegisterFunction("show_ko", this, &WeichiCommonGtpEngine::cmdShowKo, 0);

	if( WeichiConfigure::EnableOpeningBook ) {
		m_dbOpening.dbOpeningInit();
	}
}

string WeichiCommonGtpEngine::getGoguiAnalyzeCommandsString()
{
	ostringstream oss;
	oss << "string/=====Common Gtp Command=====/empty\n"
		<< "varc/Reg GenMove/reg_genmove %c\n"
		<< "string/Final Score/final_score\n"
		<< "string/Win Rate/win_rate\n"
		<< "dboard/MCTS BV/mcts_bv %c\n"		
		;

	return oss.str();
}

void WeichiCommonGtpEngine::cmdName()
{
	reply(GTP_SUCC, m_name);
}

void WeichiCommonGtpEngine::cmdVersion()
{
	ostringstream oss ;
	oss << m_version;

	reply(GTP_SUCC, oss.str());
}

void WeichiCommonGtpEngine::cmdBoardsize()
{
	int boardsize = atoi(m_args[0].c_str());

	if( boardsize<=0 || boardsize>=MAX_BOARD_SIZE ) {
		reply(GTP_FAIL, "not support");
	} else {
		changeBoardSize(boardsize);
		reply(GTP_SUCC, "");
	}
}

void WeichiCommonGtpEngine::cmdShowboard()
{
	m_mcts.getState().m_board.showColorBoard();

	//WeichiBitBoard bmZone;
	//bmZone.SetBitOn(WeichiMove::PASS_POSITION);
	//cout << bmZone.BitIsOn(WeichiMove::PASS_POSITION) << endl;
	//cout << bmZone.empty() << endl;
	//cout << bmZone.bitCount() << endl;

	reply(GTP_SUCC, "");
}

void WeichiCommonGtpEngine::cmdClearBoard()
{
	clearBoard();
	reply(GTP_SUCC, "");
}

void WeichiCommonGtpEngine::cmdSleep() 
{
	int seconds = atoi(m_args[0].c_str()) ;
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000*seconds));
	ostringstream oss ;
	oss << "Sleep " << seconds << " seconds" << endl ;

	reply(GTP_SUCC, oss.str());
}

void WeichiCommonGtpEngine::cmdLoadsgf()
{
	std::string filename = m_args[0];
	WeichiGlobalInfo::getTreeInfo().m_problemSgfFilename = filename;
	int limit = (1<<10);
	if ( m_args.size() >= 2 ) 
		limit = atoi(m_args[1].c_str()) ;

	if ( !m_sgfLoader.parseFromFile(filename, limit) ) {
		reply(GTP_FAIL, "open file fail") ;
		return;
	}

	changeBoardSize(m_sgfLoader.getBoardSize());
	changeKomi(m_sgfLoader.getKomi());
    clearBoard();
    WeichiDynamicKomi::setHandicapConfigure(m_sgfLoader.getHandicap());

	if ( !m_mcts.modifyState(m_sgfLoader) ) {
		reply(GTP_FAIL, "bad sgf") ;
		return;
	}

	Color toPlay = m_mcts.getState().getRootTurn();
	reply(GTP_SUCC, (toPlay==COLOR_BLACK?"black":"white") );
}

void WeichiCommonGtpEngine::cmdLoadjson()
{	
	int limit = (1<<10);
	if (m_args.size() >= 2) { limit = atoi(m_args[1].c_str()); }
	std::string filename = m_args[0];	
	boost::property_tree::ptree root;
	boost::property_tree::read_json(filename.c_str(), root);

	string sgffile = root.get<string>("filename");
	string masked_sgfstring = root.get<string>("masked_sgf_str");
	string sWinColor = root.get<string>("winning_color");
	string sBlackSearchGoal = root.get<string>("black_search_goal");
	string sWhiteSearchGoal = root.get<string>("white_search_goal");
	string sBlackKoRule = root.get<string>("black_ko_rule");
	string sWhiteKoRule = root.get<string>("white_ko_rule");
	string sBlackCrucialStone = root.get<string>("black_crucial_stone");
	string sWhiteCrucialStone = root.get<string>("white_crucial_stone");

	if (!m_sgfLoader.parseFromString(masked_sgfstring, limit)) {
		reply(GTP_FAIL, "sgf string fail");
		return;
	}

	changeBoardSize(m_sgfLoader.getBoardSize());
	changeKomi(m_sgfLoader.getKomi());
    clearBoard();
    WeichiDynamicKomi::setHandicapConfigure(m_sgfLoader.getHandicap());

	CERR() << "[Komi]: " << WeichiConfigure::komi << endl;

	if (!m_mcts.modifyState(m_sgfLoader)) {
		reply(GTP_FAIL, "bad sgf") ;
		return;
	}		

	//check the closed area is correct along preset moves
	for (int iThread = 0; iThread < m_mcts.getNumThreads(); ++iThread) {
		const_cast<WeichiThreadState&>(m_mcts.getState(iThread)).m_rootFilter.startFilter();
	}

	WeichiGlobalInfo::getTreeInfo().m_problemSgfFilename = sgffile;
	if (sWinColor == "b") { WeichiConfigure::win_color = 1; }
	else if (sWinColor == "w") { WeichiConfigure::win_color = 2; }	
	WeichiGlobalInfo::getTreeInfo().m_winColor = Color(WeichiConfigure::win_color);
	
	if (sBlackKoRule == "allow_ko") { WeichiConfigure::black_ignore_ko = false; }
	else if(sBlackKoRule == "disallow_ko") { WeichiConfigure::black_ignore_ko = true; }
	else { CERR() << "[WARNING]: Unknown black ko setting.";  }
	if (sWhiteKoRule == "allow_ko") { WeichiConfigure::white_ignore_ko = false; }
	else if (sWhiteKoRule == "disallow_ko") { WeichiConfigure::white_ignore_ko = true; }
	else { CERR() << "[WARNING]: Unknown white ko setting."; }

	CERR() << "[Black Ignore Ko]: " << WeichiConfigure::black_ignore_ko << endl;
	CERR() << "[White Ignore Ko]: " << WeichiConfigure::white_ignore_ko << endl;
		
	WeichiGlobalInfo::getEndGameCondition().setCrucialStones(const_cast<WeichiThreadState&>(m_mcts.getState()), COLOR_BLACK, sBlackCrucialStone);
	WeichiGlobalInfo::getEndGameCondition().setSearchGoal(COLOR_BLACK, sBlackSearchGoal);
	WeichiGlobalInfo::getEndGameCondition().setCrucialStones(const_cast<WeichiThreadState&>(m_mcts.getState()), COLOR_WHITE, sWhiteCrucialStone);
	WeichiGlobalInfo::getEndGameCondition().setSearchGoal(COLOR_WHITE, sWhiteSearchGoal);	
	for (int iThread=0; iThread<m_mcts.getNumThreads(); ++iThread) {
		WeichiGlobalInfo::getEndGameCondition().setBoardCrucialStone(const_cast<WeichiThreadState&>(m_mcts.getState(iThread)));
	}

	// set region
	string sRegion;
	if (root.count("region") != 0) { sRegion = root.get<string>("region"); }
	WeichiGlobalInfo::getEndGameCondition().setRegion(sRegion);

	CERR() << WeichiGlobalInfo::getEndGameCondition().toString() << endl;

	Color toPlay = m_mcts.getState().getRootTurn();
	reply(GTP_SUCC, (toPlay==COLOR_BLACK?"black":"white") );	
}

void WeichiCommonGtpEngine::cmdKomi()
{
	float komi = (float)atof(m_args[0].c_str());	
	//komi = 99.0;		
	//Dual<WeichiBitBoard>& lifeStones = const_cast<WeichiThreadState&>(m_mcts.getState()).m_board.getLADToLifeStones();
	//Dual<WeichiBitBoard>& killStones = const_cast<WeichiThreadState&>(m_mcts.getState()).m_board.getLADToKillStones();

	//lifeStones.reset();
	//killStones.reset();

	//for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
	//	lifeStones.m_white.SetBitOn(*it);
	//	killStones.m_white.SetBitOn(*it);
	//}

	changeKomi(komi);

	reply(GTP_SUCC, "");
}

void WeichiCommonGtpEngine::cmdSetInternalKomi()
{
    float internalKomi = (float)atof(m_args[0].c_str());
    WeichiDynamicKomi::setInternalKomi(static_cast<int>(internalKomi-(WeichiConfigure::komi+WeichiDynamicKomi::Handicap)));

    reply(GTP_SUCC, "");
}

void WeichiCommonGtpEngine::cmdFixedHandicap() 
{
	if( !m_mcts.getState().m_board.isEmptyBoard() ){
		reply(GTP_FAIL, "must be empty board.") ;
		return ;
	}
    clearBoard();

	uint stoneNum = (int)atoi(m_args[0].c_str()) ;

	vector<WeichiMove> handicapMoves ;
	try{
		handicapMoves = WeichiGtpUtil::getHandiCapStones(WeichiConfigure::BoardSize, stoneNum);
	}
	catch( string failedMsg ) {
		reply(GTP_FAIL, failedMsg) ;
		return;
	}

	m_handicap_setter.placeHandicap(handicapMoves);
	if ( !m_mcts.modifyState(m_handicap_setter) ) {
		reply(GTP_SUCC, "handicap failed.") ;
		return ;
	}

	ostringstream oss ;
	for(vector<WeichiMove>::iterator it=handicapMoves.begin() ; it!=handicapMoves.end() ; ++it ) 
		oss << it->toGtpString() << " " ;

	WeichiDynamicKomi::setHandicapConfigure(stoneNum);
	reply(GTP_SUCC, oss.str()) ;
}

void WeichiCommonGtpEngine::cmdPlaceFreeHandicap() 
{
	if( !m_mcts.getState().m_board.isEmptyBoard() ){
		reply(GTP_FAIL, "must be empty board.") ;
		return ;
	}
    clearBoard();

	uint stoneNum = (int)atoi(m_args[0].c_str()) ;

	vector<WeichiMove> handicapMoves ;
	try{
		handicapMoves = WeichiGtpUtil::getHandiCapStones(WeichiConfigure::BoardSize, stoneNum);
	}
	catch( string failedMsg ) {
		reply(GTP_FAIL, failedMsg) ;
		return;
	}

	m_handicap_setter.placeHandicap(handicapMoves);

	if ( !m_mcts.modifyState(m_handicap_setter) ) {
		reply(GTP_SUCC, "handicap failed.") ;
		return ;
	}

	ostringstream oss ;
	for(vector<WeichiMove>::iterator it=handicapMoves.begin() ; it!=handicapMoves.end() ; ++it ) 
		oss << it->toGtpString() << " " ;

	WeichiDynamicKomi::setHandicapConfigure(stoneNum);
	reply(GTP_SUCC, oss.str()) ;
}

void WeichiCommonGtpEngine::cmdSetFreeHandicap()
{
	if( !m_mcts.getState().m_board.isEmptyBoard() ) {
		reply(GTP_FAIL, "must be empty board.") ;
		return ;
	}
    clearBoard();

	vector<WeichiMove> handicapMoves ;
	for( vector<std::string>::iterator it = m_args.begin() ; it!=m_args.end() ; ++it ) {
		WeichiMove wm(COLOR_BLACK, *it);
		handicapMoves.push_back(wm);
	}

	m_handicap_setter.placeHandicap(handicapMoves);
	if ( !m_mcts.modifyState(m_handicap_setter) ) {
		reply(GTP_SUCC, "handicap failed.") ;
		return ;
	}

	WeichiDynamicKomi::setHandicapConfigure(static_cast<int>(handicapMoves.size()));
	reply(GTP_SUCC, "") ;
}

void WeichiCommonGtpEngine::cmdTimeSettings()
{
	float main_time =  (float)atof(m_args[0].c_str()) ;
	float byo_yomi_time =  (float)atof(m_args[1].c_str()) ;
	float byo_yomi_stones =  (float)atof(m_args[2].c_str()) ;

	WeichiConfigure::TimeControlMainTime = main_time ;
	//WeichiConfigure::TimeControlByoYomiTime = byo_yomi_time ;
	//WeichiConfigure::TimeControlByoYomiStones = byo_yomi_stones ;

	setTime ( main_time ) ;

	reply(GTP_SUCC, "") ;
}

void WeichiCommonGtpEngine::cmdTimeLeft()
{
	Color c = toColor(m_args[0][0]) ;
	float seconds = (float)atof(m_args[1].c_str()) ; 
	float hands = (float)atof(m_args[2].c_str()) ; 

	WeichiGlobalInfo::getTreeInfo().m_dRemainTime.get(c) = seconds / (hands+1) ;

	reply(GTP_SUCC, "") ;
}

void WeichiCommonGtpEngine::cmdPlay()
{
	Color c = toColor(m_args[0].at(0));
	string sMove = m_args[1] ;

    transform(sMove.begin(),sMove.end(), sMove.begin(), ::toupper) ;
    WeichiGlobalInfo::getSearchInfo().m_territory.clear();    // TODO: clean genmove clear territory
	WeichiGlobalInfo::getSearchInfo().m_passTerritory.clear();

	if ( sMove == "RESIGN" ) {
		m_resign = true ;
		reply ( GTP_SUCC, "" ) ;
		return ;
	}

	Move m(c, sMove);
	if ( m.getColor() == COLOR_NONE || ! m_mcts.play(m) ) {
		reply ( GTP_FAIL, "invalid color or coordinate" ) ;
		return;
	}

	WeichiGlobalInfo::get()->logInfo(m);

	reply ( GTP_SUCC, "" ) ; 
}

void WeichiCommonGtpEngine::cmdGoguiPlaySequence()
{
	for ( uint i = 0; i < m_args.size(); i += 2 ) {
		Color c = toColor(m_args[i].at(0));
		string sMove = m_args[i+1];

		transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
		WeichiGlobalInfo::getSearchInfo().m_territory.clear();    // TODO: clean genmove clear territory
		WeichiGlobalInfo::getSearchInfo().m_passTerritory.clear();

		WeichiMove m(c, sMove);
		if ( m.getColor() == COLOR_NONE || !m_mcts.play(m) ) {
			reply(GTP_FAIL, "invalid color or coordinate");
			return;
		}
	}

	reply(GTP_SUCC, "");
}

void WeichiCommonGtpEngine::cmdGenmove()
{
	Color turnColor = toColor(m_args[0].at(0));
	reply(GTP_SUCC, genOneMCTSMove(turnColor, true));
}

void WeichiCommonGtpEngine::cmdRegGenmove()
{
	Color turnColor = toColor(m_args[0].at(0));	
	reply(GTP_SUCC, genOneMCTSMove(turnColor, false));
}

void WeichiCommonGtpEngine::cmdKgsGenmoveCleanup()
{
    Color turnColor = toColor(m_args[0].at(0));
	bool bEarlyPassBackup = WeichiConfigure::EarlyPass;
	bool bCaptureDeadStones = WeichiConfigure::CaptureDeadStones;
	WeichiConfigure::EarlyPass = false;
	WeichiConfigure::CaptureDeadStones = true;
	string sReply = genOneMCTSMove(turnColor, true);
	WeichiConfigure::EarlyPass = bEarlyPassBackup;
	WeichiConfigure::CaptureDeadStones = bCaptureDeadStones;
	
	reply(GTP_SUCC, sReply);
}

void WeichiCommonGtpEngine::cmdPeek()
{
	Color turnColor = m_mcts.getState().getRootTurn();
	string sReply = genOneMCTSMove(turnColor, false);

	vector<WeichiUctNode*> vChild;
	for ( UctChildIterator<WeichiUctNode> it(WeichiGlobalInfo::getTreeInfo().m_pRoot) ; it ; ++it ) {
		vChild.push_back(&(*it));
	}
	sort(vChild.begin(), vChild.end(),
		[] ( const WeichiUctNode* lhs, const WeichiUctNode* rhs ) {
			return lhs->getUctData().getCount() > rhs->getUctData().getCount();
		}
	);

	int bestN;
	istringstream iss(m_args[0]);
	iss >> bestN;

	ostringstream oss;
	for ( int i=0; i<bestN; ++i ) {
		if ( i > 0 ) { oss << ' '; }
		oss << vChild[i]->getMove().toGtpString() << ' ';
		oss << vChild[i]->getUctData().getCount() << ' ';
		oss << vChild[i]->getUctData().getMean() << ';';
	}

	reply(GTP_SUCC, oss.str());
}

void WeichiCommonGtpEngine::cmdWinRate()
{
	string poRate = ToString(WeichiGlobalInfo::getTreeInfo().m_bestSD.getMean(),3);
	string vnRate = ToString(WeichiGlobalInfo::getTreeInfo().m_bestValueSD.getMean(),3);
	string winRate = "po rate: " + poRate + ", vn rate: " + vnRate;
	reply(GTP_SUCC, winRate);
}

void WeichiCommonGtpEngine::cmdFinalScore()
{
	WeichiPlayoutResult result = const_cast<WeichiThreadState&>(m_mcts.getState()).eval();
	/*ensureTerritory();
    float black = 0.0, white = WeichiConfigure::komi + WeichiDynamicKomi::Handicap;
    const Territory& territory = WeichiGlobalInfo::getSearchInfo().m_territory;

    for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
        double prob = territory.getTerritory(*it);

        if ( prob >=  WeichiConfigure::TerritoryThreshold ) ++black ;
        if ( prob <= -WeichiConfigure::TerritoryThreshold ) ++white ;
    }
    WeichiPlayoutResult result(black-white);*/

	reply(GTP_SUCC, result.toString());
}

void WeichiCommonGtpEngine::cmdFinalStatusList()
{
	if ( m_args[0] != "dead" ) {
		reply(GTP_FAIL, "only dead detected") ;
		return ;
	}
	ensureTerritory();
	const Territory& territory = WeichiGlobalInfo::getSearchInfo().m_territory;
	const WeichiBoard& board = m_mcts.getState().m_board ;
	ostringstream oss ;

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		Color c = board.getGrid(*it).getColor();
		double prob = territory.getTerritory(*it) ;
		if ( prob >  WeichiConfigure::TerritoryThreshold && c == COLOR_WHITE )
			oss << WeichiMove(c, *it).toGtpString() << ' ';
		if ( prob < -WeichiConfigure::TerritoryThreshold && c == COLOR_BLACK )
			oss << WeichiMove(c, *it).toGtpString() << ' ';
	} 

	reply(GTP_SUCC, oss.str());
}

void WeichiCommonGtpEngine::cmdMctsBV()
{
	//Color turnColor = toColor(m_args[0].at(0));
	//genOneMCTSMove(turnColor, false) ;

	ostringstream oss;
	//NodePtr<WeichiUctNode> root = WeichiGlobalInfo::getTreeInfo().m_pRoot;

	//vector<float> vTerritory;
	//vTerritory.resize(MAX_NUM_GRIDS, 0.0f);	
	//if( root->getBVID()!= -1  ) {
	//	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
	//		float fProb = WeichiGlobalInfo::getBVDataTable().getAvgBVData(root->getBVID(), WeichiMove::toCompactPosition(*it)) ;
	//		if (vTerritory[*it] == 0.0f) { vTerritory[*it] = fProb; } 
	//		else { vTerritory[*it] += fProb; } 
	//	}
	//}
	//
	//oss << endl ;
	//std::streamsize default_precision = std::cout.precision();
	//for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
	//	oss << setw(6) << std::fixed << setprecision(3) << vTerritory[*it] << " " ;
	//	if( *it%MAX_BOARD_SIZE==WeichiConfigure::BoardSize-1 ) { oss << endl; }		
	//}
	//oss.unsetf(ios::fixed);
	//oss << setprecision(default_precision);

    reply(GTP_SUCC, oss.str());
}

void WeichiCommonGtpEngine::cmdShowKo()
{
	const WeichiBoard& board = m_mcts.getState().m_board;
	ostringstream oss;
	oss << endl;
	oss << "Ko: " << board.getKo() << endl;
	oss << "Ko Eater: " << board.getKoEater() << endl;

	reply(GTP_SUCC, oss.str());
}

string WeichiCommonGtpEngine::genOneMCTSMove( Color turnColor, bool bWithPlay )
{
	if (WeichiConfigure::counting_rule=="Japanese" && !WeichiConfigure::use_dynamic_komi) {
		// TODO: need to consider handicap games
		if( turnColor==COLOR_BLACK ) { WeichiDynamicKomi::setInternalKomi(1); }
		else if( turnColor==COLOR_WHITE ) { WeichiDynamicKomi::setInternalKomi(-1); }
	}

	const WeichiBoard& board = m_mcts.getState().m_board;
	Color toPlay = board.getToPlay();

	if (turnColor!=toPlay) { m_mcts.play(Move(toPlay)); }
	WeichiGlobalInfo::get()->cleanSummaryInfo();

	// gen from database
	Move m = genmoveFromDB(turnColor);
	if (m!=Move()) { return m.toGtpString(); }

	// gen by mode
	bool bDisplayMessage = false;
	if (WeichiConfigure::GenMovePolicy==UCT_GENMOVE) {
		bDisplayMessage = true;
		adjustTimeLimit(turnColor);
		if( WeichiConfigure::DoPlayoutCacheLog ) { WeichiGlobalInfo::getLogInfo().m_cacheLogger.reset(board); }
		m = m_mcts.genmove(false);
		if( WeichiConfigure::DoPlayoutCacheLog ) { WeichiGlobalInfo::getLogInfo().m_cacheLogger.closeCacheLogFile(); }
		if( WeichiConfigure::DoLog ) {
			WeichiLogger::logWinRate(board, m);
			WeichiLogger::logTreeInfo(board, m, false, WeichiLogger::isBestSequenceImportantChild);
		}
	} else if (WeichiConfigure::GenMovePolicy==PLAYOUT_GENMOVE) {
		m = genPlayoutMove();
	} else if (WeichiConfigure::GenMovePolicy==DCNN_GENBESTMOVE) {
		m = genDCNNBestMove();
	} else if (WeichiConfigure::GenMovePolicy==DCNN_GENSOFTMAXMOVE) {
		m = genDCNNSoftmaxMove();
	} else if (WeichiConfigure::GenMovePolicy==DCNN_GENVNMOVE) {
		m = genDCNNVNMove();
	}

	if (board.getMoveList().size() > WeichiConfigure::TotalGrids * 2) { m = WeichiMove(turnColor); }
	
	// double check and adjust move
	WeichiGlobalInfo::get()->calculateSureTerritory();
	if (checkImitate()) {
		CERR() << "Detect imitate, play at tianyuan." << endl;
		int tianyuanPos = MAX_BOARD_SIZE*(WeichiConfigure::BoardSize / 2) + WeichiConfigure::BoardSize / 2;
		m = WeichiMove(turnColor, tianyuanPos);
		if (bWithPlay) { m_mcts.play(m); }
		if (bDisplayMessage) { CERR() << WeichiGlobalInfo::get()->getSearchSummaryInfoString(board) << endl; }
		return m.toGtpString();
	} else if (checkMoveToResign() && !checkCloseGame()) {
		if( bDisplayMessage ) { CERR() << WeichiGlobalInfo::get()->getSearchSummaryInfoString(board) << endl; }
		return "resign";
	} else {
		// adjust dynamic komi && display search summary
		if( bWithPlay && WeichiConfigure::use_dynamic_komi ) { WeichiDynamicKomi::adjustDykomi( board.getBitBoard().bitCount(), board.getMoveList().size() ); }

		if( bWithPlay ) { m_mcts.play(m); }
		if( WeichiConfigure::DoLog ) { WeichiLogger::logSgfRecord(board, true); }
		if( bDisplayMessage ) { CERR() << WeichiGlobalInfo::get()->getSearchSummaryInfoString(board) << endl; }

		if (checkMoveToPass(board)) { m = PASS_MOVE; }
	}
	WeichiGlobalInfo::get()->logPredictData();

	saveWinRateDropSgf();
	return m.toGtpString();
}

Move WeichiCommonGtpEngine::genmoveFromDB ( Color c )
{
	if( !WeichiConfigure::EnableOpeningBook || WeichiGlobalInfo::getSearchInfo().m_outOfOpeningBook ) {
		WeichiGlobalInfo::getSearchInfo().m_outOfOpeningBook = true;
		return Move();
	}

	if( !WeichiConfigure::alwaysBelieveOpeningBook && Configure::SimCtrl == Configure::SIMCTRL_TIME ) {
		double thinkTime = getNextThinkTime(c, getMovesToGo()) ;
		double averageSimulationCounts = WeichiConfigure::predictedSimulationCount ;
		m_dbOpening.setSimulationCountThreshold( thinkTime * averageSimulationCounts ) ;
	}

	const Vector<WeichiMove, MAX_NUM_GRIDS> presetList = m_mcts.getState().m_board.getPresetList() ;
	const Vector<WeichiMove, MAX_GAME_LENGTH> movesList = m_mcts.getState().m_board.getMoveList() ;	

	uint totalSize = presetList.size() + movesList.size() ;
	DbMove* dbMoves = new DbMove[ totalSize ];

	//	Collect preset list and move list into dbMoves 
	for( uint index = 0 ; index < presetList.size() ; index++ ) 
		dbMoves[index].setValue(WeichiGtpUtil::WeichiMoveToLocation(presetList[index]).getMoveValue());				

	for( uint index = 0 ; index < movesList.size() ; index++ )
		dbMoves[index+presetList.size()].setValue(WeichiGtpUtil::WeichiMoveToLocation(movesList[index]).getMoveValue()) ;	

	// Get opening book move if existed
	DbMove returnDbMove = m_dbOpening.getNextOpeningMove(dbMoves, totalSize ) ;    

	// indicate no such move in DB
	if( returnDbMove == NONEMOVE ) return PASS_MOVE ; 

	WeichiMove wm = WeichiGtpUtil::dbMoveToWeichiMove(m_mcts.getState().m_board.getToPlay(),returnDbMove);

	m_dbOpening.dbOpeningClear();

	delete [] dbMoves ;
	return wm ; 
}

bool WeichiCommonGtpEngine::checkImitate()
{
	const WeichiBoard& board = m_mcts.getState().m_board;
	Color toPlay = board.getToPlay();

	if (toPlay == COLOR_BLACK && board.getMoveList().size() >= 10) {
		int numBlack = board.getStoneBitBoard(COLOR_BLACK).bitCount();
		int numWhite = board.getStoneBitBoard(COLOR_WHITE).bitCount();
		if (numBlack == numWhite) {
			uint pos;
			const int center = WeichiConfigure::BoardSize / 2;
			WeichiBitBoard bmBoard = board.getStoneBitBoard(COLOR_BLACK);
			while( (pos=bmBoard.bitScanForward())!=-1 ) {
				int imitatePos = 378 - pos;
				if (!board.getStoneBitBoard(COLOR_WHITE).BitIsOn(imitatePos)) { return false; }
			}
			return true;
		}
	}

	return false;
}

bool WeichiCommonGtpEngine::checkCloseGame()
{
	if (!WeichiConfigure::ResignWhenCloseGame) { return false; }

	Color turnColor = m_mcts.getState().m_board.getToPlay();
	float fTerritoryDiff = WeichiGlobalInfo::getTreeInfo().m_fTerritory.m_black - WeichiGlobalInfo::getTreeInfo().m_fTerritory.m_white - WeichiConfigure::komi;
	fTerritoryDiff = (turnColor == COLOR_BLACK) ? fTerritoryDiff : -fTerritoryDiff;

	return (WeichiGlobalInfo::getTreeInfo().m_fTerritory.m_black + WeichiGlobalInfo::getTreeInfo().m_fTerritory.m_white) > 300 && fTerritoryDiff > -10;
}

bool WeichiCommonGtpEngine::checkMoveToResign()
{
	if (WeichiConfigure::GenMovePolicy == UCT_GENMOVE) {
		return WeichiGlobalInfo::getTreeInfo().m_bestSD.getMean() < WeichiConfigure::ResignWinrateThreshold;
	}

	return false;
}

bool WeichiCommonGtpEngine::checkMoveToPass( const WeichiBoard& board )
{
	// for japanese's rule
	/*return (board.hasPrevMove()
			&& board.getPrevMove().getPosition()==PASS_MOVE.getPosition()
			&& WeichiGlobalInfo::getTreeInfo().m_bestSD.getMean()>0.75 );*/
	return false;
}

void WeichiCommonGtpEngine::setTime( float seconds )
{
	WeichiGlobalInfo::getTreeInfo().m_dRemainTime.m_black = seconds;
	WeichiGlobalInfo::getTreeInfo().m_dRemainTime.m_white = seconds;
}

void WeichiCommonGtpEngine::adjustTimeLimit( Color c )
{
	if( !WeichiConfigure::DynamicTimeControl ) { return; }

	float moves_to_go = getMovesToGo() ;
	float remain = WeichiGlobalInfo::getTreeInfo().m_dRemainTime.get(c);
	float time_to_think = getNextThinkTime(c, moves_to_go) ;

	Configure::SimulationTimeLimit = time_to_think ;
	CERR() << "moves to go: " << moves_to_go << endl;
	CERR() << "remain time: " << remain << endl;
	CERR() << "think time: " << time_to_think << endl;
}

void WeichiCommonGtpEngine::changeBoardSize( uint boardsize )
{
	if ( WeichiConfigure::BoardSize != boardsize ) {
		StaticBoard::initializeBoardSizeArgument(boardsize);
		m_dbOpening.changeBoardSize(boardsize);
	}
	clearBoard();
}

void WeichiCommonGtpEngine::changeKomi( float komi )
{
	WeichiConfigure::komi = komi ;
	WeichiDynamicKomi::setHandicapConfigure(WeichiDynamicKomi::Handicap);
}

void WeichiCommonGtpEngine::clearBoard()
{
	createDirectories();
	WeichiDynamicKomi::setHandicapConfigure(0);
	setTime(WeichiConfigure::TimeControlMainTime);
	WeichiGlobalInfo::getTreeInfo().m_bestSD.reset();
	WeichiGlobalInfo::getTreeInfo().m_nMove = 0;
	WeichiGlobalInfo::getTreeInfo().m_predictOrder = WeichiGlobalInfo::getTreeInfo().OUT_OF_ORDER;
	WeichiGlobalInfo::getTreeInfo().m_vPredictOrder.setAllAs(WeichiGlobalInfo::getTreeInfo().OUT_OF_ORDER, MAX_NUM_GRIDS);
	WeichiGlobalInfo::getTreeInfo().m_vPredictInfoOrder.setAllAs(StatisticData(), MAX_NUM_GRIDS);
	WeichiGlobalInfo::getSearchInfo().m_outOfOpeningBook = false ;
	WeichiGlobalInfo::getEndGameCondition().reset();
	WeichiGlobalInfo::resetZone();
	Configure::ExpandThreshold = INIT_EXPAND_THRESHOLD;
	m_resign = false;
	m_mcts.newGame();
}

void WeichiCommonGtpEngine::saveWinRateDropSgf()
{
	if( !WeichiConfigure::winratedropSaveSgf ) { return; }

	// if win rate drop, save sgf (success if previous win rate > SuperiorThreshold && current win rate < InferiorThreshold)
	if( WeichiGlobalInfo::getTreeInfo().m_lastSD.getMean() > WeichiConfigure::SuperiorThreshold &&
		WeichiGlobalInfo::getTreeInfo().m_bestSD.getMean() < WeichiConfigure::InferiorThreshold  )
	{
		stringstream sFileName;
		sFileName << TimeSystem::getTimeString("Y_m_d_H_i_s") << "(" << m_args[0].at(0) << ")";
		sFileName << "[" << WeichiGlobalInfo::getTreeInfo().m_lastSD.getMean() << "-" << WeichiGlobalInfo::getTreeInfo().m_bestSD.getMean() << "].sgf";
		ofstream fout(sFileName.str().c_str(), ios::out) ;
		if( fout ) {
			fout << m_mcts.getState().m_board.toSgfFileString() << endl;		   		  
			fout.close();
		}
	}

	// save current win rate
	WeichiGlobalInfo::getTreeInfo().m_lastSD.reset(WeichiGlobalInfo::getTreeInfo().m_bestSD.getMean(), WeichiGlobalInfo::getTreeInfo().m_bestSD.getCount());
}

// for HandicapSetter
bool WeichiCommonGtpEngine::HandicapSetter::operator()( WeichiThreadState& state ) const
{
	state.resetThreadState();
	WeichiBoard& board = state.m_board ;

	// set AB and AW
	for ( uint i=0;i<m_preset.size();++i ) {
		WeichiMove m = m_preset[i];
		if ( !board.preset(m) ) {
			CERR() << "fail: " << m.toSgfString() << endl;
			return false;
		}
	}
	return true ;
}

void WeichiCommonGtpEngine::HandicapSetter::placeHandicap( vector<WeichiMove>& handicaps )
{
	this->clear() ;
	for(vector<WeichiMove>::iterator it=handicaps.begin() ; it!=handicaps.end() ; ++it ) 
		this->addPreset(*it);
}

float WeichiCommonGtpEngine::getMovesToGo() 
{
	if( !WeichiConfigure::DynamicTimeControl ) return 0.0f ;

	const WeichiBoard& board = m_mcts.getState().m_board ;
	float hand_no = (float)(board.getMoveList().size()) ;
	float remain_step = WeichiConfigure::TimeControlRemainStep ;
	float initial_step = WeichiConfigure::TimeControlInitialStep ;

	float moves_to_go = (initial_step - hand_no )*WeichiConfigure::TimeControlParameter + remain_step;
	if( moves_to_go < remain_step ) { moves_to_go = remain_step ; }

	return moves_to_go ;
}


float WeichiCommonGtpEngine::getNextThinkTime( Color c , float moves_to_go )
{
	if( !WeichiConfigure::DynamicTimeControl ) return Configure::SimulationTimeLimit ;

	float remain = WeichiGlobalInfo::getTreeInfo().m_dRemainTime.get(c);
	float time_to_think = remain / moves_to_go ;

	if( WeichiConfigure::TimeControlByoYomiStones > 0 ) {
		if( time_to_think < WeichiConfigure::TimeControlByoYomiTime / WeichiConfigure::TimeControlByoYomiStones ) {
			time_to_think = WeichiConfigure::TimeControlByoYomiTime / WeichiConfigure::TimeControlByoYomiStones;
		}
	}

	return time_to_think ;
} 
