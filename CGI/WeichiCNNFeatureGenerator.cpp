#include "WeichiCNNFeatureGenerator.h"

void WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet( WeichiThreadState& state, BaseCNNNet* net, SymmetryType type/*=SYM_NORMAL*/ )
{
	WeichiBitBoard bmLegal;
	float inputFeatures[MAX_CNN_CHANNEL_SIZE*MAX_NUM_GRIDS];
	calculateDCNNFeatures(state,net->getCNNFeatureType(),inputFeatures,bmLegal);
	addToDCNNNet(net,inputFeatures,state.m_board.getToPlay(),bmLegal,type);
}

void WeichiCNNFeatureGenerator::calculateDCNNFeatures( WeichiThreadState& state, WeichiCNNFeatureType featureType, float* inputFeatures, WeichiBitBoard& bmLegal )
{
	// reset features
	int channelSize = CNN_CHANNEL_SIZE[featureType];
	float features[MAX_CNN_CHANNEL_SIZE][MAX_NUM_GRIDS];
	for( int channel=0; channel<channelSize; channel++ ) {
		for( uint pos=0; pos<WeichiConfigure::TotalGrids; pos++ ) {
			features[channel][pos] = 0.0f;
		}
	}

	switch( featureType ) {
		case CNN_FEATURE_G49:	calculateG49Features(state, features, bmLegal); break;
		case CNN_FEATURE_G50:	calculateG50Features(state, features, bmLegal); break;
		case CNN_FEATURE_G51:	calculateG51Features(state, features, bmLegal); break;
		case CNN_FEATURE_F16:	calculateF16Features(state, features, bmLegal); break;
		case CNN_FEATURE_G17:	calculateG17Features(state, features, bmLegal); break;
		case CNN_FEATURE_C49:	calculateC49Features(state, features, bmLegal); break;
		case CNN_FEATURE_C51:	calculateC51Features(state, features, bmLegal); break;
		case CNN_FEATURE_C52:	calculateC52Features(state, features, bmLegal); break;
		case CNN_FEATURE_F18:	calculateF18Features(state, features, bmLegal); break;
		case CNN_FEATURE_C148:	calculateC148Features(state, features, bmLegal); break;
		default:	break;
	}

	for( int channel=0; channel<channelSize; channel++ ) {
		for( uint pos=0; pos<WeichiConfigure::TotalGrids; pos++ ) {
			inputFeatures[channel*WeichiConfigure::TotalGrids+pos] = features[channel][pos];
		}
	}
}

void WeichiCNNFeatureGenerator::addToDCNNNet( BaseCNNNet* net, float* features, Color turnColor, WeichiBitBoard& bmLegal, SymmetryType type/*=SYM_NORMAL*/ )
{
	switch( net->getCNNNetType() ) {
		case CNN_NET: static_cast<WeichiCNNNet*>(net)->push_back(features, turnColor, bmLegal, type); break;
		case CNN_NET_SL: static_cast<WeichiCNNSLNet*>(net)->push_back(features,turnColor,bmLegal,type); break;
		case CNN_NET_BV_VN: static_cast<WeichiCNNBVVNNet*>(net)->push_back(features,turnColor,type); break;
		case CNN_NET_RL: break;
		default: break;
	}
}

void WeichiCNNFeatureGenerator::calculateG49Features( WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal )
{
	WeichiBoard& board = state.m_board;
	OpenAddrHashTable& ht = state.m_ht;
	Color myColor = board.getToPlay();
	Color oppColor = AgainstColor(myColor);

	/*
	Google features:
		channel 0    : zeros (all filled with 0)
		channel 1    : ones (all filled with 1)
		channel 2~4  : our stone / opp stone / empty
		channel 5~12 : liberty
		channel 13~20: turns since (last 8 moves)
		channel 21~28: capture size
		channel 29~36: self-atari size
		channel 37~44: liberty after move
		channel 45   : ladder capture
		channel 46   : ladder escape
		channel 47   : sensibleness
		channel 48   : ko
	*/
	
	// channel 1: ones
	for( uint i=0; i<WeichiConfigure::TotalGrids; i++ ) { features[1][i] = 1.0; }

	// channel 13~20: turns since
	const Vector<WeichiMove,MAX_GAME_LENGTH>& vMoves = board.getMoveList();
	for( int i=vMoves.size()-1; i>=0; i-- ) {
		const WeichiMove& move = vMoves[i];
		if( move.isPass() ) { continue; }

		const uint compactPosition = move.getCompactPosition();
		int iTurns = vMoves.size()-i;
		if( iTurns>=8 ) { break; }
		int channelIndex = iTurns + 12;
		features[channelIndex][compactPosition] = 1.0;
	}

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiGrid& grid = board.getGrid(*it);
		const WeichiMove move(myColor, *it);
		const uint compactPosition = move.getCompactPosition();

		// legal
		if( !board.isIllegalMove(WeichiMove(board.getToPlay(),*it),ht) &&
			WeichiBadMoveReader::getBadMoveType(board,WeichiMove(board.getToPlay(),*it))==BADMOVE_NOT_BADMOVE ) { bmLegal.SetBitOn(*it); }

		// channel 2~4: our stone / opp stone / empty
		if( grid.getColor()==myColor ) { features[2][compactPosition] = 1.0; }
		else if( grid.getColor()==oppColor ) { features[3][compactPosition] = 1.0; }
		else { features[4][compactPosition] = 1.0; }

		// channel 5~12: liberty
		const WeichiBlock* block = grid.getBlock();
		if( block ) {
			int liberty = block->getLiberty();
			int channelIndex = (liberty<8)? liberty+4: 12;
			features[channelIndex][compactPosition] = 1.0;
		}

		// following feature should be empty grid
		if( !grid.isEmpty() ) { continue; }

		PredictPlayInfo moveInfo;
		board.getPredictPlayInfoByPlay(move,moveInfo);
		// channel 21~28: capture size
		if( moveInfo.m_bHasDeadBlocks ) {
			int deadStone = moveInfo.m_bmDeadStone.bitCount();
			int channelIndex = (deadStone<8)? deadStone+20: 28;
			features[channelIndex][compactPosition] = 1.0;
		}
		// channel 29~36: self-atari size
		if( moveInfo.m_liberty==1 ) {
			int iStoneNum = moveInfo.m_blockNumStone;
			int channelIndex = (iStoneNum<8)? iStoneNum+28: 36;
			features[channelIndex][compactPosition] = 1.0;
		}
		// channel 37~44: liberty after move
		int iLibertyAfterMove = moveInfo.m_liberty;
		int channelIndex = (iLibertyAfterMove<8)? iLibertyAfterMove+36: 44;
		features[channelIndex][compactPosition] = 1.0;

		// channel 47: sensibleness
		if( board.isIllegalMove(move,ht) ) {
			if( grid.getPattern().getTrueEye(myColor) ) { features[47][compactPosition] = 1.0; }
		}
	}

	uint pos;
	WeichiLocalSearchHandler& localSearchHandler = state.m_localSearchHandler;
	localSearchHandler.findFullBoardBlockLocalSequence();
	{
		// channel 45: ladder capture
		WeichiBitBoard bmOneLibBlocks = board.getOneLibBlocksBitBoard(oppColor);
		while( (pos=bmOneLibBlocks.bitScanForward())!=-1 ) {
			const WeichiBlock* oneLibBlock = board.getGrid(pos).getBlock();
			bmOneLibBlocks -= oneLibBlock->getStoneMap();

			const WeichiLocalSequence* killSequence = oneLibBlock->getKillLocalSequence();
			for( uint iNum=0; iNum<killSequence->getNumSuccess(); iNum++ ) {
				const uint pos = killSequence->getSuccessPos(iNum);
				const uint compactPosition = WeichiMove(pos).getCompactPosition();
				features[45][compactPosition] = 1.0;
			}
		}
	}

	{
		// channel 46: ladder escape
		WeichiBitBoard bmOneLibBlocks = board.getOneLibBlocksBitBoard(myColor);
		while( (pos=bmOneLibBlocks.bitScanForward())!=-1 ) {
			const WeichiBlock* oneLibBlock = board.getGrid(pos).getBlock();
			bmOneLibBlocks -= oneLibBlock->getStoneMap();

			const WeichiLocalSequence* saveSequence = oneLibBlock->getSaveLocalSequence();
			for( uint iNum=0; iNum<saveSequence->getNumSuccess(); iNum++ ) {
				const uint pos = saveSequence->getSuccessPos(iNum);
				const uint compactPosition = WeichiMove(pos).getCompactPosition();
				features[46][compactPosition] = 1.0;
			}
		}
	}

	// channel 48
	if( board.hasKo() ) {
		WeichiMove m(board.m_status.m_ko);
		features[48][m.getCompactPosition()] = 1.0;
	}
}

void WeichiCNNFeatureGenerator::calculateG50Features( WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal )
{
	/*
	CGI features:
		channel 0~48	: same as google features
		channel 49		: turn color
	*/

	calculateG49Features(state,features,bmLegal);
	calculateTurnColorFeature(state,features[49]);
}

void WeichiCNNFeatureGenerator::calculateG51Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal)
{
	/*
	CGI features:
	channel 0~48	: same as google features
	channel 49		: turn color
	channel 50		: border
	*/

	calculateG49Features(state, features, bmLegal);
	calculateTurnColorFeature(state, features[49]);
	calculateBorderFeature(state, features[50], bmLegal);
}

void WeichiCNNFeatureGenerator::calculateF16Features( WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal )
{
	WeichiBoard& board = state.m_board;
	OpenAddrHashTable& ht = state.m_ht;
	Color myColor = board.getToPlay();
	Color oppColor = AgainstColor(myColor);

	/*
	CGI features:
		channel 0~5	  : own 1~6+ liberty 
		channel 6~11  : opp 1~6+ liberty
		channel 12    : empty position
		channel 13    : history
		channel 14    : ko
		channel 15    : lowLib (dead)
	*/

	// channel 0~12, 15
	WeichiMoveFeatureHandler& moveFeatureHandler = state.m_moveFeatureHandler;
	moveFeatureHandler.calculateFullBoardCandidates(myColor);
	const Vector<WeichiMoveFeature,MAX_NUM_GRIDS>& vMoveFeatureList = moveFeatureHandler.calculateFullBoardCandidates(myColor);
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiMoveFeature& moveFeature = vMoveFeatureList[*it];
		const WeichiGrid& grid = board.getGrid(*it);
		const Color gridColor = grid.getColor();
		const WeichiBlock* block = grid.getBlock();
		const WeichiMove m(gridColor, *it);
		int lib = 0;
		if( block ) {
			lib = block->getLiberty()-1;
			if( lib>5 ) { lib = 5; }
		}

		// legal
		if( !board.isIllegalMove(WeichiMove(board.getToPlay(),*it),ht) &&
			WeichiBadMoveReader::getBadMoveType(board,WeichiMove(board.getToPlay(),*it))==BADMOVE_NOT_BADMOVE ) { bmLegal.SetBitOn(*it); }

		if ( gridColor == myColor ) { features[0+lib][m.getCompactPosition()] = 1.0; }
		else if ( gridColor == oppColor ) { features[6+lib][m.getCompactPosition()] = 1.0; }
		else { features[12][m.getCompactPosition()] = 1.0; }

		if ( moveFeature.hasFixedFeature(MF_SAVE_1LIB_FAILED_BY_SEARCH)
		  || moveFeature.hasFixedFeature(MF_SAVE_1LIB_FAILED_NO_SEARCH)
		  || moveFeature.hasFixedFeature(MF_SAVE_1LIB_FAILED_WITH_KO)
		  || moveFeature.hasFixedFeature(MF_SAVE_2LIB_FAILED) ) {
			// failed
			features[15][m.getCompactPosition()] = 1.0;
		}
	}

	// channel 13
	const Vector<WeichiMove,MAX_GAME_LENGTH>& vMoves = board.getMoveList();
	for ( int i=vMoves.size()-1 ; i>=0 ; --i ) {
		// history = e^(-0.1*t), t is how long the stone has been placed
		float history = 0;
		int playedMoveNumber = i+1;
		history = exp(-0.1f * (vMoves.size() - playedMoveNumber));
		
		if( vMoves[i].isPass() ) { continue; }
		features[13][vMoves[i].getCompactPosition()] = history;
	}

	// channel 14
	if ( board.hasKo() ) {
		WeichiMove m(board.m_status.m_ko);
		features[14][m.getCompactPosition()] = 1.0;
	}
}

void WeichiCNNFeatureGenerator::calculateG17Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal)
{
	const int LAST_N_BOARD = 8;
	const WeichiBoard& board = state.m_board;
	const Vector<Dual<WeichiBitBoard>, MAX_GAME_LENGTH>& vbmBoard = board.getBitBoardList();

	Color turnColor = board.getToPlay();
	Color oppColor = AgainstColor(turnColor);
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		if (!board.isIllegalMove(WeichiMove(board.getToPlay(), *it), state.m_ht)) { bmLegal.SetBitOn(*it); }

		int compactPos = WeichiMove::toCompactPosition(*it);
		for (int i = 0; i < LAST_N_BOARD; i++) {
			if (vbmBoard.size() > i) {
				features[2 * i][compactPos] = vbmBoard[vbmBoard.size() - i - 1].get(turnColor).BitIsOn(*it);
				features[2 * i + 1][compactPos] = vbmBoard[vbmBoard.size() - i - 1].get(oppColor).BitIsOn(*it);
			} else {
				features[2 * i][compactPos] = features[2 * i + 1][compactPos] = 0;
			}
		}
	}

	calculateTurnColorFeature(state, features[16]);
}

void WeichiCNNFeatureGenerator::calculateC49Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal)
{
	/*
	CGI features:
		channel 0~15 : our stone / opp stone's position in last 8 turns
		channel 16~23: 1~8+ liberty
		channel 24~31: capture size
		channel 32~39: self-Atari size
		channel 40~47: liberty after move
		channel 48   : turn color
	*/
	const WeichiBoard& board = state.m_board;
	Color turnColor = board.getToPlay();
	Color oppColor = AgainstColor(turnColor);

	state.m_rootFilter.startFilter();
	const Vector<Dual<WeichiBitBoard>, MAX_GAME_LENGTH>& vbmBoard = board.getBitBoardList();
	WeichiBitBoard bmTerritory = state.m_board.getAliveArea(COLOR_BLACK) | state.m_board.getAliveArea(COLOR_WHITE);
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		const WeichiGrid& grid = board.getGrid(*it);
		const WeichiMove move(turnColor, *it);
		const uint compactPosition = move.getCompactPosition();
		if (WeichiConfigure::dcnn_relaxed_child_generation) {
			if (!board.isIllegalMove(move, state.m_ht) && !grid.getPattern().getTrueEye(turnColor)) { bmLegal.SetBitOn(move.getCompactPosition()); }
		} else {
			bool isTerritory = false;
			if (bmTerritory.BitIsOn(*it)) { isTerritory = true; }
			if (!board.isIllegalMove(move, state.m_ht) && !isTerritory) { bmLegal.SetBitOn(move.getCompactPosition()); }			
			// ignore ko
			if (WeichiConfigure::black_ignore_ko && turnColor == COLOR_BLACK && board.isKoEatPlay(move)) { bmLegal.SetBitOff(move.getCompactPosition()); }
			if (WeichiConfigure::white_ignore_ko && turnColor == COLOR_WHITE && board.isKoEatPlay(move)) { bmLegal.SetBitOff(move.getCompactPosition()); }
		}

		// channel 0~15: our stone / opp stone's position in last 8 turns
		const int LAST_N_BOARD = 8;
		for (int i = 0; i < LAST_N_BOARD; i++) {
			if (vbmBoard.size() > i) {
				features[2 * i][compactPosition] = vbmBoard[vbmBoard.size() - i - 1].get(turnColor).BitIsOn(*it);
				features[2 * i + 1][compactPosition] = vbmBoard[vbmBoard.size() - i - 1].get(oppColor).BitIsOn(*it);
			} else {
				features[2 * i][compactPosition] = features[2 * i + 1][compactPosition] = 0;
			}
		}

		// channel 16~23: 1~8+ liberty
		const WeichiBlock* block = grid.getBlock();
		if (block) {
			int liberty = block->getLiberty();
			int channelIndex = (liberty < 8) ? liberty + 15 : 23;
			features[channelIndex][compactPosition] = 1.0;
		}

		// following feature should be empty grid
		if (!grid.isEmpty()) { continue; }

		PredictPlayInfo moveInfo;
		board.getPredictPlayInfoByPlay(move, moveInfo);
		// channel 24~31: capture size
		if (moveInfo.m_bHasDeadBlocks) {
			int deadStone = moveInfo.m_bmDeadStone.bitCount();
			int channelIndex = (deadStone < 8) ? deadStone + 23 : 31;
			features[channelIndex][compactPosition] = 1.0;
		}

		// channel 32~39: self-Atari size
		if (moveInfo.m_liberty == 1) {
			int iStoneNum = moveInfo.m_blockNumStone;
			int channelIndex = (iStoneNum < 8) ? iStoneNum + 31 : 39;
			features[channelIndex][compactPosition] = 1.0;
		}

		// channel 40~47: liberty after move
		if (moveInfo.m_liberty > 0) {
			int iLibertyAfterMove = moveInfo.m_liberty;
			int channelIndex = (iLibertyAfterMove < 8) ? iLibertyAfterMove + 39 : 47;
			features[channelIndex][compactPosition] = 1.0;
		}
	}

	// channel 48: turn color
	calculateTurnColorFeature(state, features[48]);

	// moves are generated only inside the region
	WeichiBitBoard bmRegion = WeichiGlobalInfo::getEndGameCondition().getRegion();
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		if (bmRegion.empty()) { break; }

		const WeichiMove move(turnColor, *it);
		if (bmLegal.BitIsOn(move.getCompactPosition()) && !bmRegion.BitIsOn(move.getPosition())) {
			bmLegal.SetBitOff(move.getCompactPosition());
		}
	}

	// check able to pass
	if (WeichiConfigure::enable_pass_by_playout) {
		WeichiPlayoutResult result = state.eval();
		if (result.getWinner() == turnColor) { bmLegal.SetBitOn(PASS_MOVE.getCompactPosition()); }
	} else {
		if (bmLegal.bitCount() == 0) { bmLegal.SetBitOn(PASS_MOVE.getCompactPosition()); }
	}
}

void WeichiCNNFeatureGenerator::calculateC51Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal)
{
	calculateC49Features(state, features, bmLegal);
	
	int pos;
	Color turnColor = state.m_board.getToPlay();

	WeichiBitBoard bmToLifeStone = state.m_board.getLADToLifeStones().get(turnColor);
	WeichiBitBoard bmToKillStone = state.m_board.getLADToKillStones().get(turnColor);

	//CERR() << "To Play:" << toChar(turnColor) << endl;
	//CERR() << "bmLife:" << endl << bmToLifeStone.toString() << endl;
	//CERR() << "bmKill:" << endl << bmToKillStone.toString() << endl;

	while ((pos = bmToLifeStone.bitScanForward()) != -1) { features[49][WeichiMove::toCompactPosition(pos)] = 1.0f; }
	while ((pos = bmToKillStone.bitScanForward()) != -1) { features[50][WeichiMove::toCompactPosition(pos)] = 1.0f; }
}

void WeichiCNNFeatureGenerator::calculateC52Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal)
{
	calculateC51Features(state, features, bmLegal);

	WeichiBoard& board = state.m_board;
	Color turnColor = board.getToPlay();
	bool bKoAvailable = board.getLADKoAvailable().get(turnColor);
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		const WeichiGrid& grid = board.getGrid(*it);
		const WeichiMove move(turnColor, *it);

		if (bKoAvailable) { features[51][move.getCompactPosition()] = 1.0f; }
		else if (state.m_board.isKoEatPlay(move)) { bmLegal.SetBitOff(move.getCompactPosition()); }
	}
}

void WeichiCNNFeatureGenerator::calculateF18Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal)
{
	const int LAST_N_BOARD = 8;
	const WeichiBoard& board = state.m_board;
	const Vector<Dual<WeichiBitBoard>, MAX_GAME_LENGTH>& vbmBoard = board.getBitBoardList();

	Color turnColor = board.getToPlay();
	Color oppColor = AgainstColor(turnColor);
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		WeichiMove move(turnColor,*it);		
		if (!board.isIllegalMove(move, state.m_ht)) { bmLegal.SetBitOn(move.getCompactPosition()); }

		// ignore ko
		if (WeichiConfigure::black_ignore_ko && state.m_board.getToPlay() == COLOR_BLACK) {
			if (state.m_board.isKoEatPlay(move)) { bmLegal.SetBitOff(move.getCompactPosition()); }
		}

		if (WeichiConfigure::white_ignore_ko && state.m_board.getToPlay() == COLOR_WHITE) {
			if (state.m_board.isKoEatPlay(move)) { bmLegal.SetBitOff(move.getCompactPosition()); }
		}

		int compactPos = WeichiMove::toCompactPosition(*it);
		for (int i = 0; i < LAST_N_BOARD; i++) {
			if (vbmBoard.size() > i) {
				features[2 * i][compactPos] = vbmBoard[vbmBoard.size() - i - 1].get(turnColor).BitIsOn(*it);
				features[2 * i + 1][compactPos] = vbmBoard[vbmBoard.size() - i - 1].get(oppColor).BitIsOn(*it);
			} else {
				features[2 * i][compactPos] = features[2 * i + 1][compactPos] = 0;
			}
		}
	}

	calculateTurnColorFeature(state, features[16]);
	for (uint i = 0; i<WeichiConfigure::TotalGrids; i++) { features[17][i] = 1 - features[16][i]; }

	// moves are generated only inside the region
	WeichiBitBoard bmRegion = WeichiGlobalInfo::getEndGameCondition().getRegion();
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		if (bmRegion.empty()) { break; }

		const WeichiMove move(turnColor, *it);
		if (bmLegal.BitIsOn(move.getCompactPosition()) && !bmRegion.BitIsOn(move.getPosition())) {
			bmLegal.SetBitOff(move.getCompactPosition());
		}
	}

	return;
}

void WeichiCNNFeatureGenerator::calculateC148Features(WeichiThreadState & state, float features[][MAX_NUM_GRIDS], WeichiBitBoard & bmLegal)
{
	calculateC49Features(state, features, bmLegal);

	int move_number = state.m_board.getMoveList().size();
	move_number = (move_number >= 98 ? 98 : move_number);
	for (uint pos = 0; pos < WeichiConfigure::TotalGrids; pos++) { features[move_number + 49][pos] = 1.0; }
}

void WeichiCNNFeatureGenerator::calculateTurnColorFeature( WeichiThreadState& state, float features[] )
{
	Color myColor = state.m_board.getToPlay();
	
	// turn color (black for 1)
	if( myColor!=COLOR_BLACK ) { return; }
	for( uint i=0; i<WeichiConfigure::TotalGrids; i++ ) { features[i] = 1.0; }
}

void WeichiCNNFeatureGenerator::calculateBorderFeature( WeichiThreadState& state, float features[], WeichiBitBoard& bmLegal )
{
	const uint BORDER = WeichiConfigure::BoardSize - 1;

	for (uint i = 0; i < WeichiConfigure::BoardSize; i++){
		features[0 * WeichiConfigure::BoardSize + i] = 1.0f;
		features[BORDER * WeichiConfigure::BoardSize + i] = 1.0f;
		features[i * WeichiConfigure::BoardSize + 0] = 1.0f;
		features[i * WeichiConfigure::BoardSize + BORDER] = 1.0f;
	}
}