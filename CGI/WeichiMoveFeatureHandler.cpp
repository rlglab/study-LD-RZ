#include "WeichiMoveFeatureHandler.h"
#include "WeichiThreadState.h"
#include "WeichiPlayoutAgent.h"

const Vector<WeichiMoveFeature,MAX_NUM_GRIDS>& WeichiMoveFeatureHandler::calculateFullBoardCandidates( Color turnColor )
{
	calculateAllMoveFeature(turnColor);
	calculateAllRadiusFeature(turnColor);

	assertToFile( invariance(), const_cast<WeichiBoard*>(&m_board) );

	return m_vMoveFeatureList;
}

const Vector<CandidateEntry,MAX_NUM_GRIDS>& WeichiMoveFeatureHandler::getUCTCandidateList( bool bConsiderPass/*=false*/ )
{
	double dMax = 0.0f, dSum = 0.0f;
	m_vUCTCandidateList.clear();

	if( WeichiConfigure::mcts_use_mm ) {
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			if( m_vMoveFeatureList[*it].hasFixedFeature(MF_NOT_IN_CANDIDATE) ) { continue; }
			dSum += m_vMoveFeatureList[*it].getScore();
		}
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			if( m_vMoveFeatureList[*it].hasFixedFeature(MF_NOT_IN_CANDIDATE) ) { continue; }
			double dProbability = m_vMoveFeatureList[*it].getScore()/dSum;
			if( dProbability>dMax ) { dMax = dProbability; }
			m_vUCTCandidateList.push_back(CandidateEntry(*it,dProbability));
		}
	} else if( WeichiConfigure::mcts_use_lfr ) {
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			if( m_vMoveFeatureList[*it].hasFixedFeature(MF_NOT_IN_CANDIDATE) ) { continue; }
			double dScore = m_vMoveFeatureList[*it].getScore();
			dScore = (dScore>0)? dScore: 0.0f;
			if( dScore>dMax ) { dMax = dScore; }
			m_vUCTCandidateList.push_back(CandidateEntry(*it,dScore));
		}
	}

	// if no candidate or consider pass, add pass move
	if( m_vUCTCandidateList.size()==0 || bConsiderPass ) {
		m_vUCTCandidateList.push_back(CandidateEntry(PASS_MOVE.getPosition(),dMax+1));
	}

	std::sort(m_vUCTCandidateList.begin(),m_vUCTCandidateList.end());

	return m_vUCTCandidateList;

	/*double dMax = 0.0;
	m_vUCTCandidateList.clear();
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		if( m_vMoveFeatureList[*it].hasFixedFeature(MF_NOT_IN_CANDIDATE) ) { continue; }
		const double dScore = m_vMoveFeatureList[*it].getScore();
		if( dScore>dMax ) { dMax = dScore; }
		m_vUCTCandidateList.push_back(CandidateEntry(*it,dScore));
	}

	// if no candidate or consider pass, add pass move
	if( m_vUCTCandidateList.size()==0 || bConsiderPass ) {
		m_vUCTCandidateList.push_back(CandidateEntry(PASS_MOVE.getPosition(),dMax+1));
	}

	std::sort(m_vUCTCandidateList.begin(),m_vUCTCandidateList.end());

	return m_vUCTCandidateList;*/
}

void WeichiMoveFeatureHandler::calculateAllMoveFeature( Color turnColor )
{
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiMove move(turnColor,*it);
		if( isNeedCalculateCandidate(move) ) { calculateNormalCandidate(move); }
		else { m_vMoveFeatureList[*it].addFixedFeature(MF_NOT_IN_CANDIDATE); }
	}

	// Pass candidate
	calculatePassCandidate();

	// full board feature
	findFullBoardNakadeFeature(turnColor);
	findFullBoardCFGDistance();
	findFullBoardDecayFeature();
	findFullBoardAtariFeature(turnColor);

	calculateAllBlockLocalSequenceFeature(turnColor);
}

void WeichiMoveFeatureHandler::calculateAllBlockLocalSequenceFeature( Color turnColor )
{
	m_localSearchHandler.findFullBoardBlockLocalSequence();

	findFullBoardCaptureFeature(turnColor);
	findFullBoardKill2LibFeature(turnColor);
	findFullBoardSave1LibFeature(turnColor);
	findFullBoardSave2LibFeature(turnColor);
}

void WeichiMoveFeatureHandler::calculateAllRadiusFeature( Color turnColor )
{
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		if( m_vMoveFeatureList[*it].hasFixedFeature(MF_NOT_IN_CANDIDATE) ) { continue; }

		const WeichiMove move(turnColor,*it);
		const WeichiGrid& grid = m_board.getGrid(move);
		WeichiMoveFeature& moveFeature = m_vMoveFeatureList[*it];

		HashKey64 accumlateKey = 0;
		for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
			uint iIndex = grid.getRadiusPatternRealIndex(iRadius,move.getColor());
			if( iRadius==3 ) { iIndex = (iIndex<<(RADIUS_NBR_LIB_SIZE*RADIUS_NBR_LIB_BITS)) + (grid.getNbrLibIndex()&0x55); }
			accumlateKey ^= WeichiRadiusPatternTable::getRadiusPatternHashKey(iRadius,iIndex);

			int patternIndex = WeichiGammaTable::getRadiusPatternIndex(accumlateKey);
			if( patternIndex==-1 ) { break; }
			
			moveFeature.addRadiusPatternFeature(patternIndex);
		}
	}
}

bool WeichiMoveFeatureHandler::isNeedCalculateCandidate( const WeichiMove& move )
{
	/*
		skip following position: (will return with MOVE_NONE)
			1. non-empty grid
			2. bad move
			3. illegal move
	*/
	assertToFile( !move.isPass(), const_cast<WeichiBoard*>(&m_board) );

	const WeichiGrid& grid = m_board.getGrid(move);

	if( !grid.isEmpty() ) { return false; }
	if( m_board.isIllegalMove(move,m_ht) ) { return false; }

	PredictPlayInfo& moveInfo = m_vPredictPlayInfoList[move.getPosition()];
	moveInfo.reset();
	m_board.getPredictPlayInfoByPlay(move,moveInfo);
	if( WeichiBadMoveReader::getBadMoveType(m_board,move)!=BADMOVE_NOT_BADMOVE ) { return false; }
	
	return true;
}

void WeichiMoveFeatureHandler::calculateNormalCandidate( const WeichiMove& move )
{
	assertToFile( !move.isPass(), const_cast<WeichiBoard*>(&m_board) );

	WeichiMoveFeature& moveFeature = m_vMoveFeatureList[move.getPosition()];

	// fixed feature
	moveFeature.clear();
	addMoveFixedFeature(findDisToBorderFeature(move),moveFeature);
	addMoveFixedFeature(findLongDisToBorderFeature(move),moveFeature);
	addMoveFixedFeature(findDisToPrevMoveFeature(move),moveFeature);
	addMoveFixedFeature(findDisToPreOwnMoveFeature(move),moveFeature);
	addMoveFixedFeature(findSelfAtariFeature(move),moveFeature);
	addMoveFixedFeature(findKillOwnNew2LibFeature(move),moveFeature);
	addMoveFixedFeature(findKoFeature(move),moveFeature);

	// radius pattern attribute
	m_vRadiusPatternAttrList[move.getPosition()].reset();
	if( m_board.hasKo() ) { m_vRadiusPatternAttrList[move.getPosition()].setHasKo(true); }
}

void WeichiMoveFeatureHandler::calculatePassCandidate()
{
	WeichiMoveFeature& moveFeature = m_vMoveFeatureList[PASS_MOVE.getPosition()];
	moveFeature.clear();

	addMoveFixedFeature(findPassMoveFeature(PASS_MOVE),moveFeature);
}

// for normal feature
WeichiMoveFeatureType WeichiMoveFeatureHandler::findDisToBorderFeature( const WeichiMove& move ) const
{
	int iLineNo = m_board.getGrid(move).getStaticGrid().getLineNo();
	return getShiftMoveFeatureType(iLineNo,1,MF_DIS_TO_BORDER_1,MF_DIS_TO_BORDER_5_OR_MORE);
}

WeichiMoveFeatureType WeichiMoveFeatureHandler::findLongDisToBorderFeature( const WeichiMove& move ) const
{
	int iLongLineNo = m_board.getGrid(move).getStaticGrid().getLongLineNo();
	return getShiftMoveFeatureType(iLongLineNo,1,MF_LONG_DIS_TO_BORDER_1,MF_LONG_DIS_TO_BORDER_10);
}

WeichiMoveFeatureType WeichiMoveFeatureHandler::findDisToPrevMoveFeature( const WeichiMove& move ) const
{
	if( !m_board.hasPrevMove() ) { return MF_NOT_IN_CANDIDATE; }

	WeichiMove preMove = m_board.getPrevMove();
	if( preMove.isPass() ) { return MF_NOT_IN_CANDIDATE; }

	int iDis = distanceBetweenTwoMoves(move,preMove);
	return getShiftMoveFeatureType(iDis,2,MF_DIS_TO_PREV_2,MF_DIS_TO_PREV_17_OR_MORE);
}

WeichiMoveFeatureType WeichiMoveFeatureHandler::findDisToPreOwnMoveFeature( const WeichiMove& move ) const
{
	if( !m_board.hasPrevOwnMove() ) { return MF_NOT_IN_CANDIDATE; }

	WeichiMove preOwnMove = m_board.getPrevOwnMove();
	if( preOwnMove.isPass() || move==preOwnMove ) { return MF_NOT_IN_CANDIDATE; }

	int iDis = distanceBetweenTwoMoves(move,preOwnMove);
	return getShiftMoveFeatureType(iDis,2,MF_DIS_TO_PREV_OWN_2,MF_DIS_TO_PREV_OWN_17_OR_MORE);
}

WeichiMoveFeatureType WeichiMoveFeatureHandler::findSelfAtariFeature( const WeichiMove& move ) const
{
	const PredictPlayInfo& moveInfo = m_vPredictPlayInfoList[move.getPosition()];
	if( moveInfo.m_liberty!=1 ) { return MF_NOT_IN_CANDIDATE; }

	const Color oppColor = AgainstColor(move.getColor());
	if( WeichiKnowledgeBase::isKo(m_board,move,oppColor) ) { return MF_NOT_IN_CANDIDATE; }
	if( moveInfo.m_bHasDeadBlocks && !WeichiBadMoveReader::isSnapback(m_board,move) ) { return MF_NOT_IN_CANDIDATE; }

	if( moveInfo.m_blockNumStone>3 && WeichiBadMoveReader::isMakingNakadeShape(m_board, move) ) { return MF_SELF_ATARI_MAKE_NAKADE_SHAPE; }
	else if( m_board.hasKo() ) { return MF_SELF_ATARI_WITH_KO; }
	else if( m_board.getGrid(move).getStonenNbrMap().hasIntersection(m_board.getTwoLibBlocksBitBoard(oppColor)) ) { return MF_SELF_ATARI_WITH_ATARI; }
	else if( WeichiKnowledgeBase::isMakeKo(m_board,move) ) { return MF_SELF_ATARI_1_MAKE_KO; }
	else if( moveInfo.m_blockNumStone==1 ) { return MF_SELF_ATARI_1; }
	else if( moveInfo.m_blockNumStone==2 ) { return MF_SELF_ATARI_2; }
	else { return MF_SELF_ATARI_MORE_THAN_3; }
}

WeichiMoveFeatureType WeichiMoveFeatureHandler::findKillOwnNew2LibFeature( const WeichiMove& move ) const
{
	const PredictPlayInfo& moveInfo = m_vPredictPlayInfoList[move.getPosition()];
	if( moveInfo.m_liberty!=2 ) { return MF_NOT_IN_CANDIDATE; }

	WeichiLocalSearch& localSearch = m_localSearchHandler.getLocalSearch();

	localSearch.reset();
	localSearch.startLocalSearch();
	localSearch.play(move);
	const WeichiBlock* block = m_board.getGrid(move).getBlock();
	WeichiBlockSearchResult result = localSearch.canKill(block,localSearch.getSearchTreeRootNode());
	localSearch.undo();
	localSearch.endLocalSearch();

	if( result!=RESULT_SUCCESS ) { return MF_NOT_IN_CANDIDATE; }
	if( m_board.hasKo() ) { return MF_KILL_OWN_NEW_2LIB_WITH_KO; }
	else if( moveInfo.m_blockNumStone==1 ) { return MF_KILL_OWN_NEW_2LIB_STONE_1; }
	else if( moveInfo.m_blockNumStone==2 ) { return MF_KILL_OWN_NEW_2LIB_STONE_2; }
	else if( moveInfo.m_blockNumStone<=5 ) { return MF_KILL_OWN_NEW_2LIB_STONE_3_TO_5; }
	else if( moveInfo.m_blockNumStone<=10 ) { return MF_KILL_OWN_NEW_2LIB_STONE_6_TO_10; }
	else { return MF_KILL_OWN_NEW_2LIB_STONE_11_OR_MORE; }
}

WeichiMoveFeatureType WeichiMoveFeatureHandler::findKoFeature( const WeichiMove& move ) const
{
	const Color oppColor = AgainstColor(move.getColor());

	if( WeichiKnowledgeBase::isKo(m_board,move,move.getColor()) ) { return findKoRank(move,MF_KO_SAVE_URGENT); }
	else if( WeichiKnowledgeBase::isKo(m_board,move,oppColor) ) { return findKoRank(move,MF_KO_EAT_URGENT); }

	return MF_NOT_IN_CANDIDATE;
}

WeichiMoveFeatureType WeichiMoveFeatureHandler::findKoRank( const WeichiMove& move, WeichiMoveFeatureType baseType ) const
{
	bool bIsImportant = false;
	const WeichiGrid& grid = m_board.getGrid(move);
	const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors();
	for( ; *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNbr);
		const WeichiBlock* nbrBlock = nbrGrid.getBlock();

		if( nbrBlock->getLiberty()==2 ) { return baseType; }
		else if( nbrBlock->getLiberty()==3 ) { bIsImportant = true; }
	}

	if( bIsImportant ) { return WeichiMoveFeatureType(baseType+1); }
	else { return WeichiMoveFeatureType(baseType+2); }
}

WeichiMoveFeatureType WeichiMoveFeatureHandler::findPassMoveFeature( const WeichiMove& move ) const
{
	assertToFile( move.isPass(), const_cast<WeichiBoard*>(&m_board) );

	if( m_board.hasPrevMove() && m_board.getPrevMove().isPass() ) { return MF_PASS_CONSECUTIVE; }
	else { return MF_PASS_NEW; }
}

// for full board feature
void WeichiMoveFeatureHandler::findFullBoardNakadeFeature( Color turnColor )
{
	WeichiBitBoard bmCACheckIndex;

	if( m_board.hasPrevMove() ) { findNakadeWithPrevMoveFeature(m_board.getPrevMove(),bmCACheckIndex,MF_PREV_OPP_NAKADE); }
	if( m_board.hasPrevOwnMove() ) { findNakadeWithPrevMoveFeature(m_board.getPrevOwnMove(),bmCACheckIndex,MF_PREV_OWN_NAKADE); }

	const FeatureList<WeichiClosedArea,MAX_NUM_CLOSEDAREA>& closedAreaList = m_board.getCloseArea();
	for( uint iIndex=0; iIndex<closedAreaList.getCapacity(); iIndex++ ) {
		if( !closedAreaList.isValidIdx(iIndex) ) { continue; }

		const WeichiClosedArea* closedArea = closedAreaList.getAt(iIndex);
		if( !(bmCACheckIndex&closedArea->getStoneMap()).empty() ) { continue; }

		if( closedArea->getColor()==turnColor ) { addNakadeFeature(closedArea,MF_OWN_NAKADE); }
		else { addNakadeFeature(closedArea,MF_OPP_NAKADE); }
	}
}

void WeichiMoveFeatureHandler::findNakadeWithPrevMoveFeature( const WeichiMove& prevMove, WeichiBitBoard bmCACheckIndex, WeichiMoveFeatureType type )
{
	if( prevMove.isPass() ) { return; }

	const WeichiGrid& prevGrid = m_board.getGrid(prevMove);
	if( !prevGrid.getPattern().getIsSplitCA(prevMove.getColor(),prevMove.getColor()) || !prevGrid.getBlock() ) { return; }

	for( const int *iNbr=prevGrid.getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNbr);
		if( !nbrGrid.isEmpty() ) { continue; }

		const WeichiClosedArea* closedArea = nbrGrid.getClosedArea(prevMove.getColor());
		if( !closedArea || !(bmCACheckIndex&closedArea->getStoneMap()).empty() ) { continue; }

		bmCACheckIndex |= closedArea->getStoneMap();
		
		if( !closedArea->hasNakade() ) { continue; }
		addNakadeFeature(closedArea,type);
	}
}

void WeichiMoveFeatureHandler::addNakadeFeature( const WeichiClosedArea* closedArea, WeichiMoveFeatureType type )
{
	if( !closedArea->hasNakade() ) { return; }

	const uint nakadePos = closedArea->getNakade();
	WeichiMoveFeature& moveFeature = m_vMoveFeatureList[nakadePos];
	addMoveFixedFeature(type,moveFeature);
}

void WeichiMoveFeatureHandler::findFullBoardCFGDistance()
{
	if( m_board.hasPrevMove() && !m_board.getPrevMove().isPass() ) {
		findBlockCFGDistance(m_board.getGrid(m_board.getPrevMove()).getBlock(),MF_CFG_DIS_TO_PREV_1);
	}
	if( m_board.hasPrevOwnMove() && !m_board.getPrevOwnMove().isPass() ) {
		const WeichiBlock* prevOwnBlock = m_board.getGrid(m_board.getPrevOwnMove()).getBlock();
		if( prevOwnBlock ) { findBlockCFGDistance(prevOwnBlock,MF_CFG_DIS_TO_PREV_OWN_1); }
	}
}

void WeichiMoveFeatureHandler::findBlockCFGDistance( const WeichiBlock* block, WeichiMoveFeatureType baseType )
{
	uint pos;
	const int LIBERTY_WALL = 5;

	WeichiBitBoard bmCFG[5];
	WeichiBitBoard bmAccumulation;

	bmAccumulation = bmCFG[0] = block->getStoneMap();
	for( int dis=1; dis<4; dis++ ) {
		bmCFG[dis] = bmAccumulation.dilate();
		bmCFG[dis] -= bmAccumulation;
		bmCFG[dis] &= StaticBoard::getMaskBorder();
		bmAccumulation |= bmCFG[dis];

		if( dis==3 ) { break; }

		WeichiBitBoard bmNewCFGStone = (bmCFG[dis] & m_board.getBitBoard());
		while( (pos=bmNewCFGStone.bitScanForward())!=-1 ) {
			const WeichiBlock* cfgBlock = m_board.getGrid(pos).getBlock();
			bmNewCFGStone -= cfgBlock->getStoneMap();

			if( cfgBlock->getLiberty()>LIBERTY_WALL ) { bmCFG[dis] -= cfgBlock->getStoneMap(); }
			else { bmCFG[dis] |= cfgBlock->getStoneMap(); }
		}
	}
	bmCFG[4] = (~bmCFG[4] - bmAccumulation) & StaticBoard::getMaskBorder();

	for( int dis=1; dis<=4; dis++ ) {
		bmCFG[dis] -= m_board.getBitBoard();
		setBitBoardFeature(bmCFG[dis],WeichiMoveFeatureType(baseType+dis-1));
	}
}

void WeichiMoveFeatureHandler::findFullBoardDecayFeature()
{
	WeichiBitBoard bmNearPrevBlockCheckIndex;
	findDecayNearPrevious(bmNearPrevBlockCheckIndex);

	uint pos;
	WeichiBitBoard bmBlock = m_board.getOneLibBlocksBitBoard(COLOR_BLACK) | m_board.getOneLibBlocksBitBoard(COLOR_WHITE)
		| m_board.getTwoLibBlocksBitBoard(COLOR_BLACK) | m_board.getTwoLibBlocksBitBoard(COLOR_WHITE);

	// calculate decay
	WeichiBitBoard bmDecay;
	Vector<uint,MAX_NUM_GRIDS> vDecay;
	vDecay.setAllAs(-1,MAX_NUM_GRIDS);
	while( (pos=bmBlock.bitScanForward())!=-1 ) {
		const WeichiBlock* block = m_board.getGrid(pos).getBlock();
		bmBlock -= block->getStoneMap();

		bool bIsNearPrevBlockDecay = (bmNearPrevBlockCheckIndex.BitIsOn(block->getiFirstGrid()));
		if( block->getLiberty()==1 ) {
			const uint lastLiberty = block->getLastLiberty(m_board.getBitBoard());
			findBlockDecayFeature(block,lastLiberty,bmDecay,vDecay);
			if( bIsNearPrevBlockDecay ) { addMoveFixedFeature(MF_BLOCK_NEAR_PREV_DECAY,m_vMoveFeatureList[lastLiberty]); }
		} else if( block->getLiberty()==2 ) {
			Vector<uint,2> vLibPos;
			block->getLibertyPositions(m_board.getBitBoard(),vLibPos);
			for( uint i=0; i<2; i++ ) {
				findBlockDecayFeature(block,vLibPos[i],bmDecay,vDecay);
				if( bIsNearPrevBlockDecay ) { addMoveFixedFeature(MF_BLOCK_NEAR_PREV_DECAY,m_vMoveFeatureList[vLibPos[i]]); }
			}
		}
	}

	// set decay
	while( (pos=bmDecay.bitScanForward())!=-1 ) {
		uint offset = vDecay[pos];
		WeichiMoveFeature& moveFeature = m_vMoveFeatureList[pos];
		addMoveFixedFeature(WeichiMoveFeatureType(MF_BLOCK_DECAY_1_TO_5+offset),moveFeature);
	}
}

void WeichiMoveFeatureHandler::findDecayNearPrevious( WeichiBitBoard& bmNearPrevBlockCheckIndex )
{
	const uint moveNumber = m_board.getMoveList().size();
	const FeatureList<WeichiBlock,MAX_GAME_LENGTH>& blockLists = m_board.getBlockList();

	for( uint iIndex=0; iIndex<blockLists.getCapacity(); iIndex++ ) {
		if( !blockLists.isValidIdx(iIndex) ) { continue; }

		const WeichiBlock* block = blockLists.getAt(iIndex);
		if( !block->isUsed() ) { continue; }
		if( block->getLiberty()>2 ) { continue; }
		if( block->getModifyMoveNumber()!=moveNumber ) { continue; }

		Color againstBlkColor = AgainstColor(block->getColor());
		WeichiBitBoard bmNbrBlock = m_board.getOneLibBlocksBitBoard(againstBlkColor) | m_board.getTwoLibBlocksBitBoard(againstBlkColor);
		bmNbrBlock &= block->getStonenNbrMap();

		uint pos;
		while( (pos=bmNbrBlock.bitScanForward())!=-1 ) {
			const WeichiBlock* nbrBlock = m_board.getGrid(pos).getBlock();
			bmNbrBlock -= nbrBlock->getStoneMap();

			if( nbrBlock->getModifyMoveNumber()==moveNumber ) { continue; }
			bmNearPrevBlockCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
		}
	}
}

void WeichiMoveFeatureHandler::findBlockDecayFeature( const WeichiBlock* block, const uint pos, WeichiBitBoard& bmAllDecay, Vector<uint,MAX_NUM_GRIDS>& vDecay )
{
	const uint decay = m_board.getMoveList().size() - block->getModifyMoveNumber();

	uint decayIndex = 0;
	if( decay<=5 ) { decayIndex = 0; }
	else if( decay<=10 ) { decayIndex = 1; }
	else if( decay<=20 ) { decayIndex = 2; }
	else { decayIndex = 3; }

	if( bmAllDecay.BitIsOn(pos) ) {
		if( decayIndex<vDecay[pos] ) { vDecay[pos] = decayIndex; }
	} else {
		bmAllDecay.SetBitOn(pos);
		vDecay[pos] = decayIndex;
	}
}

void WeichiMoveFeatureHandler::findFullBoardAtariFeature( Color turnColor )
{
	const Color beAtariColor = AgainstColor(turnColor);
	const WeichiBlock* prevBlock = (m_board.hasPrevMove() && !m_board.getPrevMove().isPass()) ? m_board.getGrid(m_board.getPrevMove()).getBlock(): NULL;

	uint pos;
	WeichiBitBoard bmTwoLiberty;
	WeichiBitBoard bmDoubleAtari;
	WeichiBitBoard bmTwoLibBlocks = m_board.getTwoLibBlocksBitBoard(beAtariColor);
	while( (pos=bmTwoLibBlocks.bitScanForward())!=-1 ) {
		const WeichiBlock* twoLibBlock = m_board.getGrid(pos).getBlock();
		bmTwoLibBlocks -= twoLibBlock->getStoneMap();

		bmDoubleAtari |= (twoLibBlock->getStonenNbrMap()&bmTwoLiberty);
		bmTwoLiberty |= twoLibBlock->getStonenNbrMap();

		Vector<uint,2> vLibPos;
		twoLibBlock->getLibertyPositions(m_board.getBitBoard(),vLibPos);
		for( uint i=0; i<vLibPos.size(); i++ ) {
			WeichiMoveFeature& moveFeature = m_vMoveFeatureList[vLibPos[i]];

			if( twoLibBlock==prevBlock ) { addMoveFixedFeature(MF_ATARI_PREV,moveFeature); }
			else { addMoveFixedFeature(MF_ATARI,moveFeature); }

			// attribute
			if( m_board.hasKo() ) { addMoveFixedFeature(MF_ATARI_ATTR_WITH_KO,moveFeature); }
			if( m_vPredictPlayInfoList[vLibPos[i]].m_liberty==1 ) { addMoveFixedFeature(MF_ATARI_ATTR_WITH_OWN_1LIB,moveFeature); }
		}
	}

	// double atari attribute
	bmDoubleAtari -= m_board.getBitBoard();
	while( (pos=bmDoubleAtari.bitScanForward())!=-1 ) {
		WeichiMove atariMove(turnColor,pos);
		if( !isGoodDoubleAtari(atariMove,beAtariColor) ) { continue; }

		addMoveFixedFeature(MF_ATARI_ATTR_DOUBLE,m_vMoveFeatureList[pos]);
	}
}

bool WeichiMoveFeatureHandler::isGoodDoubleAtari( const WeichiMove& atariMove, Color beAtariColor )
{
	if( m_board.getLibertyAfterPlay(atariMove)==1 && !m_board.isKoEatPlay(atariMove) ) { return false; }

	uint pos;
	const WeichiGrid& doubleAtariGrid = m_board.getGrid(atariMove);
	WeichiBitBoard bmBeAtariBlock = doubleAtariGrid.getStonenNbrMap() & m_board.getTwoLibBlocksBitBoard(beAtariColor);
	WeichiBitBoard bmSurroundAtariBlock;
	while( (pos=bmBeAtariBlock.bitScanForward())!=-1 ) {
		const WeichiBlock* beAtariBlock = m_board.getGrid(pos).getBlock();
		assertToFile( beAtariBlock && beAtariBlock->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

		bmSurroundAtariBlock = beAtariBlock->getStonenNbrMap();
	}
	if( !(bmSurroundAtariBlock&m_board.getOneLibBlocksBitBoard(atariMove.getColor())).empty() ) { return false; }

	return true;
}

void WeichiMoveFeatureHandler::findFullBoardCaptureFeature( Color turnColor )
{
	BlockLocalSequenceData data;
	const Color findColor = AgainstColor(turnColor);

	findFullBoardCaptureSuccessFeature(findColor,data);
}

void WeichiMoveFeatureHandler::findFullBoardCaptureSuccessFeature( Color findColor, BlockLocalSequenceData& data )
{
	uint pos;
	const WeichiBlock* prevBlock = (m_board.hasPrevMove() && !m_board.getPrevMove().isPass()) ? m_board.getGrid(m_board.getPrevMove()).getBlock(): NULL;
	WeichiBitBoard bmOneLibBlocks = m_board.getOneLibBlocksBitBoard(findColor);
	while( (pos=bmOneLibBlocks.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmOneLibBlocks -= oneLibBlock->getStoneMap();

		const WeichiLocalSequence* killSequence = oneLibBlock->getKillLocalSequence();
		assertToFile( killSequence, const_cast<WeichiBoard*>(&m_board) );

		for( uint iNum=0; iNum<killSequence->getNumSuccess(); iNum++ ) {
			const uint pos = killSequence->getSuccessPos(iNum);
			WeichiMoveFeature& moveFeature = m_vMoveFeatureList[pos];

			const WeichiLocalSequence* saveSequence = oneLibBlock->getSaveLocalSequence();
			if( saveSequence->getResult()!=RESULT_SUCCESS ) {
				addMoveFixedFeature(MF_CAPTURE_SUCCESS_WITH_DEAD,moveFeature);
			} else {
				// num stone
				data.m_bmAlreadySet.SetBitOn(pos);
				data.m_vNumStone[pos] += oneLibBlock->getNumStone();
			}

			// attribute
			if( oneLibBlock==prevBlock ) { addMoveFixedFeature(MF_CAPTURE_ATTR_SUICIDE,moveFeature); }
			else if( m_board.hasPrevMove() && !m_board.getPrevMove().isPass() && m_board.getGrid(m_board.getPrevMove()).getStonenNbrMap().BitIsOn(pos) ) {
				addMoveFixedFeature(MF_CAPTURE_ATTR_PREVENT_CONNECTION,moveFeature);
			} else if( m_vPredictPlayInfoList[pos].m_liberty==1 && oneLibBlock->getNumStone()==1 ) {
				const uint decay = m_board.getMoveList().size() - oneLibBlock->getModifyMoveNumber();
				if( decay<=5 ) { addMoveFixedFeature(MF_CAPTURE_ATTR_KO_DECAY_1_TO_5,moveFeature); }
				else { addMoveFixedFeature(MF_CAPTURE_ATTR_KO_DECAY_MORE_THAN_5,moveFeature); }
			}
		}
	}

	setNumStoneFeature(data.m_bmAlreadySet,data.m_vNumStone,MF_CAPTURE_SUCCESS_WITH_LIFE_STONE_1);
}

void WeichiMoveFeatureHandler::findFullBoardKill2LibFeature( Color turnColor )
{
	BlockLocalSequenceData data;
	const Color findColor = AgainstColor(turnColor);

	findFullBoardKill2LibSuccessFeature(findColor,data);
	findFullBoardKill2LibUnknownFeature(findColor,data);
	findFullBoardKill2LibFailedFeature(findColor,data);
}

void WeichiMoveFeatureHandler::findFullBoardKill2LibSuccessFeature( Color findColor, BlockLocalSequenceData& data )
{
	uint pos;
	const WeichiBlock* prevBlock = (m_board.hasPrevMove() && !m_board.getPrevMove().isPass()) ? m_board.getGrid(m_board.getPrevMove()).getBlock(): NULL;
	WeichiBitBoard bmTwoLibBlocks = m_board.getTwoLibBlocksBitBoard(findColor);
	while( (pos=bmTwoLibBlocks.bitScanForward())!=-1 ) {
		const WeichiBlock* twoLibBlock = m_board.getGrid(pos).getBlock();
		bmTwoLibBlocks -= twoLibBlock->getStoneMap();
		if( twoLibBlock->getStatus()==LAD_LIFE ) { continue; }

		const WeichiLocalSequence* killSequence = twoLibBlock->getKillLocalSequence();
		assertToFile( killSequence, const_cast<WeichiBoard*>(&m_board) );

		for( uint iNum=0; iNum<killSequence->getNumSuccess(); iNum++ ) {
			const uint pos = killSequence->getSuccessPos(iNum);
			WeichiMoveFeature& moveFeature = m_vMoveFeatureList[pos];

			const WeichiLocalSequence* saveSequence = twoLibBlock->getSaveLocalSequence();
			if( saveSequence->getResult()!=RESULT_SUCCESS ) {
				addMoveFixedFeature(MF_KILL_2LIB_SUCCESS_WITH_DEAD,moveFeature);
			} else {
				// num stone
				data.m_bmAlreadySet.SetBitOn(pos);
				data.m_vNumStone[pos] += twoLibBlock->getNumStone();
			}

			// attribute
			if( prevBlock && twoLibBlock->getStoneMap().BitIsOn(m_board.getPrevMove().getPosition()) ) {
				addMoveFixedFeature(MF_KILL_2LIB_ATTR_PREV,moveFeature);
			}
		}
	}

	setNumStoneFeature(data.m_bmAlreadySet,data.m_vNumStone,MF_KILL_2LIB_SUCCESS_WITH_LIFE_STONE_1);
}

void WeichiMoveFeatureHandler::findFullBoardKill2LibUnknownFeature( Color findColor, BlockLocalSequenceData& data )
{
	uint pos;
	WeichiBitBoard bmTwoLibBlocks = m_board.getTwoLibBlocksBitBoard(findColor);
	while( (pos=bmTwoLibBlocks.bitScanForward())!=-1 ) {
		const WeichiBlock* twoLibBlock = m_board.getGrid(pos).getBlock();
		bmTwoLibBlocks -= twoLibBlock->getStoneMap();
		if( twoLibBlock->getStatus()==LAD_LIFE ) { continue; }

		const WeichiLocalSequence* killSequence = twoLibBlock->getKillLocalSequence();
		assertToFile( killSequence, const_cast<WeichiBoard*>(&m_board) );

		for( uint iNum=0; iNum<killSequence->getNumUnknown(); iNum++ ) {
			const uint pos = killSequence->getUnknownPos(iNum);
			if( data.m_bmAlreadySet.BitIsOn(pos) ) { continue; }
			data.m_bmAlreadySet.SetBitOn(pos);

			WeichiMoveFeature& moveFeature = m_vMoveFeatureList[pos];
			addMoveFixedFeature(MF_KILL_2LIB_UNKNOWN,moveFeature);
		}
	}
}

void WeichiMoveFeatureHandler::findFullBoardKill2LibFailedFeature( Color findColor, BlockLocalSequenceData& data )
{
	uint pos;
	WeichiBitBoard bmTwoLibBlocks = m_board.getTwoLibBlocksBitBoard(findColor);
	while( (pos=bmTwoLibBlocks.bitScanForward())!=-1 ) {
		const WeichiBlock* twoLibBlock = m_board.getGrid(pos).getBlock();
		bmTwoLibBlocks -= twoLibBlock->getStoneMap();
		if( twoLibBlock->getStatus()==LAD_LIFE ) { continue; }

		const WeichiLocalSequence* killSequence = twoLibBlock->getKillLocalSequence();
		assertToFile( killSequence, const_cast<WeichiBoard*>(&m_board) );

		for( uint iNum=0; iNum<killSequence->getNumFailed(); iNum++ ) {
			const uint pos = killSequence->getFailedPos(iNum);
			if( data.m_bmAlreadySet.BitIsOn(pos) ) { continue; }
			data.m_bmAlreadySet.SetBitOn(pos);

			WeichiMoveFeature& moveFeature = m_vMoveFeatureList[pos];
			addMoveFixedFeature(MF_KILL_2LIB_FAILED,moveFeature);
		}
	}
}

void WeichiMoveFeatureHandler::findFullBoardSave1LibFeature( Color turnColor )
{
	BlockLocalSequenceData data;

	findFullBoardSave1LibSuccessFeature(turnColor,data);
	findFullBoardSave1LibUnknownFeature(turnColor,data);
	findFullBoardSave1LibFailedFeature(turnColor,data);
}

void WeichiMoveFeatureHandler::findFullBoardSave1LibSuccessFeature( Color findColor, BlockLocalSequenceData& data )
{
	uint pos;
	const WeichiBlock* prevBlock = (m_board.hasPrevMove() && !m_board.getPrevMove().isPass()) ? m_board.getGrid(m_board.getPrevMove()).getBlock(): NULL;
	WeichiBitBoard bmOneLibBlocks = m_board.getOneLibBlocksBitBoard(findColor);
	while( (pos=bmOneLibBlocks.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmOneLibBlocks -= oneLibBlock->getStoneMap();

		const WeichiLocalSequence* saveSequence = oneLibBlock->getSaveLocalSequence();
		assertToFile( saveSequence, const_cast<WeichiBoard*>(&m_board) );

		const uint lastLibertyPos = oneLibBlock->getLastLiberty(m_board.getBitBoard());
		for( uint iNum=0; iNum<saveSequence->getNumSuccess(); iNum++ ) {
			const uint pos = saveSequence->getSuccessPos(iNum);
			WeichiMoveFeature& moveFeature = m_vMoveFeatureList[pos];

			const WeichiLocalSequence* killSequence = oneLibBlock->getKillLocalSequence();
			if( killSequence->getResult()!=RESULT_SUCCESS ) {
				addMoveFixedFeature(MF_SAVE_1LIB_SUCCESS_WITH_LIFE,moveFeature);
			} else {
				// num stone
				data.m_bmAlreadySet.SetBitOn(pos);
				data.m_vNumStone[pos] += oneLibBlock->getNumStone();
			}

			// attribute
			if( prevBlock && oneLibBlock->getStonenNbrMap().BitIsOn(m_board.getPrevMove().getPosition()) ) {
				addMoveFixedFeature(MF_SAVE_1LIB_ATTR_BY_PREV_MOVE,moveFeature);
			}

			// radius pattern attribute
			if( pos==lastLibertyPos ) { m_vRadiusPatternAttrList[pos].setHasSave1LibSuccess(true); }
		}
	}

	setNumStoneFeature(data.m_bmAlreadySet,data.m_vNumStone,MF_SAVE_1LIB_SUCCESS_WITH_DEAD_STONE_1);
}

void WeichiMoveFeatureHandler::findFullBoardSave1LibUnknownFeature( Color findColor, BlockLocalSequenceData& data )
{
	uint pos;
	WeichiBitBoard bmOneLibBlocks = m_board.getOneLibBlocksBitBoard(findColor);
	while( (pos=bmOneLibBlocks.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmOneLibBlocks -= oneLibBlock->getStoneMap();

		const WeichiLocalSequence* saveSequence = oneLibBlock->getSaveLocalSequence();
		assertToFile( saveSequence, const_cast<WeichiBoard*>(&m_board) );

		for( uint iNum=0; iNum<saveSequence->getNumUnknown(); iNum++ ) {
			const uint pos = saveSequence->getUnknownPos(iNum);
			if( data.m_bmAlreadySet.BitIsOn(pos) ) { continue; }
			data.m_bmAlreadySet.SetBitOn(pos);

			WeichiMoveFeature& moveFeature = m_vMoveFeatureList[pos];
			addMoveFixedFeature(MF_SAVE_1LIB_UNKNOWN,moveFeature);
		}
	}
}

void WeichiMoveFeatureHandler::findFullBoardSave1LibFailedFeature( Color findColor, BlockLocalSequenceData& data )
{
	uint pos;
	WeichiBitBoard bmOneLibBlocks = m_board.getOneLibBlocksBitBoard(findColor);
	while( (pos=bmOneLibBlocks.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmOneLibBlocks -= oneLibBlock->getStoneMap();

		const WeichiLocalSequence* saveSequence = oneLibBlock->getSaveLocalSequence();
		assertToFile( saveSequence, const_cast<WeichiBoard*>(&m_board) );

		for( uint iNum=0; iNum<saveSequence->getNumFailed(); iNum++ ) {
			const uint pos = saveSequence->getFailedPos(iNum);
			if( data.m_bmAlreadySet.BitIsOn(pos) ) { continue; }
			data.m_bmAlreadySet.SetBitOn(pos);

			WeichiMoveFeature& moveFeature = m_vMoveFeatureList[pos];
			if( m_board.hasKo() ) { addMoveFixedFeature(MF_SAVE_1LIB_FAILED_WITH_KO,moveFeature); }
			else if( m_vPredictPlayInfoList[pos].m_liberty<=1 ) { addMoveFixedFeature(MF_SAVE_1LIB_FAILED_NO_SEARCH,moveFeature); }
			else { addMoveFixedFeature(MF_SAVE_1LIB_FAILED_BY_SEARCH,moveFeature); }
		}
	}
}

void WeichiMoveFeatureHandler::findFullBoardSave2LibFeature( Color turnColor )
{
	BlockLocalSequenceData data;

	findFullBoardSave2LibSuccessFeature(turnColor,data);
	findFullBoardSave2LibUnknownFeature(turnColor,data);
	findFullBoardSave2LibFailedFeature(turnColor,data);
}

void WeichiMoveFeatureHandler::findFullBoardSave2LibSuccessFeature( Color findColor, BlockLocalSequenceData& data )
{
	uint pos;
	const WeichiBlock* prevBlock = (m_board.hasPrevMove() && !m_board.getPrevMove().isPass()) ? m_board.getGrid(m_board.getPrevMove()).getBlock(): NULL;
	WeichiBitBoard bmTwoLibBlocks = m_board.getTwoLibBlocksBitBoard(findColor);
	while( (pos=bmTwoLibBlocks.bitScanForward())!=-1 ) {
		const WeichiBlock* twoLibBlock = m_board.getGrid(pos).getBlock();
		bmTwoLibBlocks -= twoLibBlock->getStoneMap();
		if( twoLibBlock->getStatus()==LAD_LIFE ) { continue; }

		const WeichiLocalSequence* saveSequence = twoLibBlock->getSaveLocalSequence();
		assertToFile( saveSequence, const_cast<WeichiBoard*>(&m_board) );

		for( uint iNum=0; iNum<saveSequence->getNumSuccess(); iNum++ ) {
			const uint pos = saveSequence->getSuccessPos(iNum);
			WeichiMoveFeature& moveFeature = m_vMoveFeatureList[pos];

			const WeichiLocalSequence* killSequence = twoLibBlock->getKillLocalSequence();
			if( killSequence->getResult()!=RESULT_SUCCESS ) {
				addMoveFixedFeature(MF_SAVE_2LIB_SUCCESS_WITH_LIFE,moveFeature);
			} else {
				// with dead num stone
				data.m_bmAlreadySet.SetBitOn(pos);
				data.m_vNumStone[pos] += twoLibBlock->getNumStone();
			}

			// attribute
			if( prevBlock && twoLibBlock->getStonenNbrMap().BitIsOn(m_board.getPrevMove().getPosition()) ) {
				addMoveFixedFeature(MF_SAVE_2LIB_ATTR_BY_PREV_MOVE,moveFeature);
			}
		}
	}

	setNumStoneFeature(data.m_bmAlreadySet,data.m_vNumStone,MF_SAVE_2LIB_SUCCESS_WITH_DEAD_STONE_1);
}

void WeichiMoveFeatureHandler::findFullBoardSave2LibUnknownFeature( Color findColor, BlockLocalSequenceData& data )
{
	uint pos;
	WeichiBitBoard bmTwoLibBlocks = m_board.getTwoLibBlocksBitBoard(findColor);
	while( (pos=bmTwoLibBlocks.bitScanForward())!=-1 ) {
		const WeichiBlock* twoLibBlock = m_board.getGrid(pos).getBlock();
		bmTwoLibBlocks -= twoLibBlock->getStoneMap();
		if( twoLibBlock->getStatus()==LAD_LIFE ) { continue; }

		const WeichiLocalSequence* saveSequence = twoLibBlock->getSaveLocalSequence();
		assertToFile( saveSequence, const_cast<WeichiBoard*>(&m_board) );

		for( uint iNum=0; iNum<saveSequence->getNumUnknown(); iNum++ ) {
			const uint pos = saveSequence->getUnknownPos(iNum);
			if( data.m_bmAlreadySet.BitIsOn(pos) ) { continue; }
			data.m_bmAlreadySet.SetBitOn(pos);

			WeichiMoveFeature& moveFeature = m_vMoveFeatureList[pos];
			addMoveFixedFeature(MF_SAVE_2LIB_UNKNOWN,moveFeature);
		}
	}
}

void WeichiMoveFeatureHandler::findFullBoardSave2LibFailedFeature( Color findColor, BlockLocalSequenceData& data )
{
	uint pos;
	WeichiBitBoard bmTwoLibBlocks = m_board.getTwoLibBlocksBitBoard(findColor);
	while( (pos=bmTwoLibBlocks.bitScanForward())!=-1 ) {
		const WeichiBlock* twoLibBlock = m_board.getGrid(pos).getBlock();
		bmTwoLibBlocks -= twoLibBlock->getStoneMap();
		if( twoLibBlock->getStatus()==LAD_LIFE ) { continue; }

		const WeichiLocalSequence* saveSequence = twoLibBlock->getSaveLocalSequence();
		assertToFile( saveSequence, const_cast<WeichiBoard*>(&m_board) );

		for( uint iNum=0; iNum<saveSequence->getNumFailed(); iNum++ ) {
			const uint pos = saveSequence->getFailedPos(iNum);
			if( data.m_bmAlreadySet.BitIsOn(pos) ) { continue; }
			data.m_bmAlreadySet.SetBitOn(pos);

			WeichiMoveFeature& moveFeature = m_vMoveFeatureList[pos];
			addMoveFixedFeature(MF_SAVE_2LIB_FAILED,moveFeature);
		}
	}
}

// common feature to use
void WeichiMoveFeatureHandler::setNumStoneFeature( WeichiBitBoard& bmStone, Vector<uint,MAX_NUM_GRIDS>& vStoneNum, WeichiMoveFeatureType baseType )
{
	uint pos;
	while( (pos=bmStone.bitScanForward())!=-1 ) {
		const WeichiGrid& grid = m_board.getGrid(pos);
		assertToFile( grid.isEmpty(), const_cast<WeichiBoard*>(&m_board) );

		WeichiMoveFeature& moveFeature = m_vMoveFeatureList[pos];
		if( vStoneNum[pos]==1 ) { addMoveFixedFeature(baseType,moveFeature); }
		else if( vStoneNum[pos]==2 ) { addMoveFixedFeature(WeichiMoveFeatureType(baseType+1),moveFeature); }
		else if( vStoneNum[pos]<=5 ) { addMoveFixedFeature(WeichiMoveFeatureType(baseType+2),moveFeature); }
		else if( vStoneNum[pos]<=10 ) { addMoveFixedFeature(WeichiMoveFeatureType(baseType+3),moveFeature); }
		else { addMoveFixedFeature(WeichiMoveFeatureType(baseType+4),moveFeature); }
	}
}

// invariance
bool WeichiMoveFeatureHandler::invariance() const
{
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		assertToFile( invariance_checkMoveFeature(m_vMoveFeatureList[*it]), const_cast<WeichiBoard*>(&m_board));
	}
	assertToFile( invariance_checkMoveFeature(m_vMoveFeatureList[PASS_MOVE.getPosition()]), const_cast<WeichiBoard*>(&m_board));

	return true;
}

bool WeichiMoveFeatureHandler::invariance_checkMoveFeature( WeichiMoveFeature moveFeature ) const
{
	if( moveFeature.hasFixedFeature(MF_NOT_IN_CANDIDATE) ) { return true; }

	// each feature group can only appear one feature
	for( uint iIndex=0; iIndex<MOVEFEATURE_TYPE_NUM; iIndex++ ) {
		uint currentIndex = MOVEFEATURE_START_INDEX[iIndex];
		uint nextIndex = (iIndex==MOVEFEATURE_TYPE_NUM-1) ? MOVE_FEATURE_SIZE: MOVEFEATURE_START_INDEX[iIndex+1];
		assertToFile( invariance_checkOnlyOneBitOn(moveFeature,currentIndex,nextIndex), const_cast<WeichiBoard*>(&m_board));
	}

	return true;
}

bool WeichiMoveFeatureHandler::invariance_checkOnlyOneBitOn( WeichiMoveFeature moveFeature, uint startIndex, uint endIndex ) const
{
	assertToFile( endIndex>startIndex, const_cast<WeichiBoard*>(&m_board) );

	vector<string> vFeautres;	// use string vector to check just only for debug convenient
	for( uint iIndex=startIndex; iIndex<endIndex; iIndex++ ) {
		if( !moveFeature.hasFixedFeature(static_cast<WeichiMoveFeatureType>(iIndex)) ) { continue; }
		vFeautres.push_back(getWeichiMoveFeatureTypeName(static_cast<WeichiMoveFeatureType>(iIndex)));
	}

	//assertToFile( vFeautres.size()<=1, const_cast<WeichiBoard*>(&m_board) );

	return true;
}