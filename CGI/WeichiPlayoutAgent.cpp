#include "WeichiPlayoutAgent.h"
#include "WeichiBadMoveReader.h"
#include "WeichiLowLibReader.h"
#include "WeichiLadderReader.h"
#include "WeichiSemeaiHandler.h"

void WeichiPlayoutAgent::findSpecificPolicyCandidates( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vCandidates, WeichiPlayoutPolicy policy )
{
	switch(policy) {
		case POLICY_KILL_SUICIDE:		vCandidates.push_back(genMoveToKillSuicide(lastMove).getPosition()); break;
		case POLICY_REPLY_SAVE_1LIB:	genMoveToReplySaveOneLibBlock(lastMove,vCandidates); break;
		case POLICY_REPLY_KILL_2LIB:	genMoveToReplyKillTwoLibBlock(lastMove,vCandidates); break;
		case POLICY_REPLY_SAVE_2LIB:	genMoveToReplySaveTwoLibBlock(lastMove,vCandidates); break;
		case POLICY_FIGHT_KO:			genMoveToFightKo(vCandidates); break;
		case POLICY_REPLY_NAKADE:		genMoveToReplyLastCANakade(lastMove,vCandidates); break;
		case POLICY_REPLY_GOOD_PATTERN:	genMoveToReplyGoodPattern(lastMove,vCandidates); break;
		default: break;
	}
}

void WeichiPlayoutAgent::calcualteLocalPlayoutFeature( const WeichiMove& lastMove )
{
	if( lastMove.isPass() ) { return; }

	Color turnColor = m_board.getToPlay();
	WeichiProbabilityPlayoutHandler& probabilityHandler = m_state.m_board.m_probabilityHandler;

	probabilityHandler.getProbabilityTable(turnColor).setMaxScore(1.0f);

	addContiguousFeatures(lastMove);

	// save neighbor own block features
	WeichiBitBoard bmCheckIndex;
	const WeichiGrid& lastGrid = m_board.getGrid(lastMove);
	const int *iNeighbor = lastGrid.getStaticGrid().getAdjacentNeighbors();
	for( ; *iNeighbor!=-1; iNeighbor++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNeighbor);
		if( nbrGrid.getColor()!=turnColor ) { continue; }

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if( bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) ) { continue; }
		bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());

		add2345SemeaiFeatures(nbrBlock);
		if( nbrBlock->getLiberty()==1 ) { addSave1LibFeatures(nbrBlock); }
		else if( nbrBlock->getLiberty()==2 ) { addSave2LibFeatures(nbrBlock); }
	}

	// kill last move's block
	const WeichiBlock* lastBlock = lastGrid.getBlock();
	addCASemeaiFeature(lastBlock);
	if( lastBlock->getLiberty()==1 ) { addKill1LibFeatures(lastBlock); }
	else if( lastBlock->getLiberty()==2 ) { addKill2LibFeatures(lastBlock); }

	addNakadeFeatures(lastMove);
	addMakeEyeFeatures(lastMove);
	addFixEyeFeatures(lastMove);
	addDestroyEyeFeatures(lastMove);

	/*if( lastMove.isPass() ) { return; }

	WeichiMove move = PASS_MOVE;
	WeichiBitBoard bmContiguous;
	Color turnColor = m_board.getToPlay();
	WeichiProbabilityPlayoutHandler& probabilityHandler = m_state.m_board.m_probabilityHandler;

	if( WeichiConfigure::sim_kill_suicide ) {
		move = genMoveToKillSuicide(lastMove);
		addProbabilityFeature(move.getPosition(),turnColor,POLICY_KILL_SUICIDE);
		bmContiguous.SetBitOn(move.getPosition());
	}

	if( WeichiConfigure::sim_reply_save_1lib ) {
		genMoveToReplySaveOneLibBlock(lastMove,m_vCandidateMoves);
		for( uint i=0; i<m_vCandidateMoves.size(); i++ ) {
			addProbabilityFeature(m_vCandidateMoves[i],turnColor,POLICY_REPLY_SAVE_1LIB);
			bmContiguous.SetBitOn(m_vCandidateMoves[i]);
		}
	}

	if( WeichiConfigure::sim_reply_kill_2lib ) {
		genMoveToReplyKillTwoLibBlock(lastMove,m_vCandidateMoves);
		for( uint i=0; i<m_vCandidateMoves.size(); i++ ) {
			addProbabilityFeature(m_vCandidateMoves[i],turnColor,POLICY_REPLY_KILL_2LIB);
			bmContiguous.SetBitOn(m_vCandidateMoves[i]);
		}
	}

	if( WeichiConfigure::sim_reply_save_2lib ) {
		genMoveToReplySaveTwoLibBlock(lastMove,m_vCandidateMoves);
		for( uint i=0; i<m_vCandidateMoves.size(); i++ ) {
			addProbabilityFeature(m_vCandidateMoves[i],turnColor,POLICY_REPLY_SAVE_2LIB);
			bmContiguous.SetBitOn(m_vCandidateMoves[i]);
		}
	}

	if( WeichiConfigure::sim_reply_defense_approach_lib ) {
		genMoveToReplyDefenseApproachLib(lastMove,m_vCandidateMoves);
		for( uint i=0; i<m_vCandidateMoves.size(); i++ ) {
			addProbabilityFeature(m_vCandidateMoves[i],turnColor,POLICY_REPLY_DEFENSE_APPROACH_LIB);
			bmContiguous.SetBitOn(m_vCandidateMoves[i]);
		}
	}

	if( WeichiConfigure::sim_reply_nakade ) {
		genMoveToReplyLastCANakade(lastMove,m_vCandidateMoves);
		for( uint i=0; i<m_vCandidateMoves.size(); i++ ) {
			addProbabilityFeature(m_vCandidateMoves[i],turnColor,POLICY_REPLY_NAKADE);
			bmContiguous.SetBitOn(m_vCandidateMoves[i]);
		}
	}

	if( WeichiConfigure::sim_fight_ko ) {
		genMoveToFightKo(m_vCandidateMoves);
		for( uint i=0; i<m_vCandidateMoves.size(); i++ ) {
			addProbabilityFeature(m_vCandidateMoves[i],turnColor,POLICY_FIGHT_KO);
			bmContiguous.SetBitOn(m_vCandidateMoves[i]);
		}
	}

	if( WeichiConfigure::sim_345point_semeai ) {
		genMoveTo345PointSemeai(lastMove,m_vCandidateMoves);
		for( uint i=0; i<m_vCandidateMoves.size(); i++ ) {
			addProbabilityFeature(m_vCandidateMoves[i],turnColor,POLICY_345POINT_SEMEAI);
			bmContiguous.SetBitOn(m_vCandidateMoves[i]);
		}
	}

	if( WeichiConfigure::sim_contiguous ) {
		const WeichiGrid& lastGrid = m_board.getGrid(lastMove);
		for( const int* iNbr=lastGrid.getStaticGrid().get3x3Neighbors(); *iNbr!=-1; iNbr++ ) {
			const WeichiGrid& nbrGrid = m_board.getGrid(*iNbr);
			if( !nbrGrid.isEmpty() ) { continue; }
			bmContiguous.SetBitOn(*iNbr);
		}

		Vector<uint,MAX_NUM_GRIDS> vContiguous;
		bmContiguous.bitScanAll(vContiguous);
		addProbabilityFeatures(vContiguous,turnColor,POLICY_CONTIGUOUS);
	}

	if( WeichiConfigure::sim_reply_good_pattern ) {
		genMoveToReplyGoodPattern(lastMove,m_vCandidateMoves);
		addProbabilityFeatures(m_vCandidateMoves,turnColor,POLICY_REPLY_GOOD_PATTERN);
	}*/
}

void WeichiPlayoutAgent::addContiguousFeatures( const WeichiMove& lastMove )
{
	Color colorToPlay = m_board.getToPlay();
	const WeichiGrid& lastGrid = m_board.getGrid(lastMove);

	for( const int* iNbr=lastGrid.getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNbr);
		if( !nbrGrid.isEmpty() ) { continue; }

		addProbabilityFeature(*iNbr,colorToPlay,POLICY_REPLY_CONTIGUOUS_2);
	}

	for( const int* iNbr=lastGrid.getStaticGrid().getDiagonalNeighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNbr);
		if( !nbrGrid.isEmpty() ) { continue; }

		addProbabilityFeature(*iNbr,colorToPlay,POLICY_REPLY_CONTIGUOUS_3);
	}

	for( const int* iNbr=lastGrid.getStaticGrid().getDiamondEndNeighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNbr);
		if( !nbrGrid.isEmpty() ) { continue; }
		addProbabilityFeature(*iNbr,colorToPlay,POLICY_REPLY_CONTIGUOUS_4);
	}
}

void WeichiPlayoutAgent::addKill1LibFeatures( const WeichiBlock* block )
{
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&m_board) );

	Color turnColor = m_board.getToPlay();
	const uint lastLiberty = block->getLastLibertyPos();
	const WeichiMove move(turnColor,lastLiberty);

	if( WeichiBadMoveReader::isLifeTerritory(m_board,move) ) { return; }
	if( m_board.isIllegalMove(move,m_state.m_ht) ) { return; }
	
	if( getKB().isSnapback(move) ) { addProbabilityFeature(lastLiberty,turnColor,POLICY_REPLY_KILL_1LIB_SNAPBACK); }
	else { addProbabilityFeature(lastLiberty,turnColor,POLICY_REPLY_KILL_1LIB); }
}

void WeichiPlayoutAgent::addSave1LibFeatures( const WeichiBlock* block )
{
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&m_board) );

	bool bCanSaveByCapture = addSave1LibByCaptureFeatures(block);
	addSave1LibByExtendFeatures(block,bCanSaveByCapture);
}

bool WeichiPlayoutAgent::addSave1LibByCaptureFeatures( const WeichiBlock* block )
{
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&m_board) );

	Color myColor = block->getColor();
	WeichiBitBoard bmNbr1LibBlock;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmNbr1LibBlock);
	if( bmNbr1LibBlock.empty() ) { return false; }

	uint pos;
	bool bCanSave = false;
	while( (pos=bmNbr1LibBlock.bitScanForward())!=-1 ) {
		const WeichiBlock* nbrBlock = m_board.getGrid(pos).getBlock();
		bmNbr1LibBlock -= nbrBlock->getStoneMap();

		uint libertyAfterPlay = m_board.getLibertyAfterPlay(WeichiMove(myColor,nbrBlock->getLastLibertyPos()));
		if( libertyAfterPlay==0 ) { continue; }

		if( libertyAfterPlay==1 ) { addProbabilityFeature(nbrBlock->getLastLibertyPos(),myColor,POLICY_REPLY_SAVE_1LIB_BY_CAPTURE_SNAPBACK); }
		else { addProbabilityFeature(nbrBlock->getLastLibertyPos(),myColor,POLICY_REPLY_SAVE_1LIB_BY_CAPTURE); }
		bCanSave = true;
	}

	return bCanSave;
}

void WeichiPlayoutAgent::addSave1LibByExtendFeatures( const WeichiBlock* block, const bool bCanSaveByCapture )
{
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&m_board) );

	Color myColor = block->getColor();
	uint lastLibPos = block->getLastLibertyPos();
	WeichiMove extendMove(myColor,lastLibPos);

	PredictPlayInfo pInfo;
	m_board.getPredictPlayInfoByPlay(extendMove,pInfo);
	uint libertyAfterPlay = pInfo.m_liberty;

	if( libertyAfterPlay==1 ) {
		addProbabilityFeature(lastLibPos,myColor,POLICY_REPLY_SAVE_1LIB_BY_EXTEND_LIB_1);
		addSave1LibByCaptureExtendNbrFeatures(block,bCanSaveByCapture,pInfo);
	} else if( libertyAfterPlay==2 ) {
		// ladder or not ladder
		WeichiLadderType type = WeichiLadderReader::getLadderType(m_board,block,lastLibPos);
		if( WeichiLadderReader::isInSimpleFastLadder(m_board,myColor,m_board.getGrid(lastLibPos),type) ) {
			addProbabilityFeature(lastLibPos,myColor,POLICY_REPLY_SAVE_1LIB_BY_EXTEND_LIB_2_LADDER);
		} else {
			addProbabilityFeature(lastLibPos,myColor,POLICY_REPLY_SAVE_1LIB_BY_EXTEND_LIB_2);
		}
	} else if( libertyAfterPlay>=3 ) {
		addProbabilityFeature(lastLibPos,myColor,POLICY_REPLY_SAVE_1LIB_BY_EXTEND_LIB_3UP);
	}
}

void WeichiPlayoutAgent::addSave1LibByCaptureExtendNbrFeatures( const WeichiBlock* block, const bool bCanSaveByCapture, const PredictPlayInfo& pInfo )
{
	if( !pInfo.m_bHasCombineBlock || bCanSaveByCapture ) { return; }

	uint pos;
	Color myColor = block->getColor();
	Color oppColor = AgainstColor(myColor);
	WeichiBitBoard bmNbr1LibBlock = pInfo.m_bmNewNbrStoneMap & m_board.getOneLibBlocksBitBoard(oppColor);
	if( bmNbr1LibBlock.empty() ) { return; }

	WeichiBitBoard bmNewLib = pInfo.m_bmNewLiberty;
	uint still1LibPos = bmNewLib.bitScanForward();
	const WeichiGrid& still1LibGrid = m_board.getGrid(still1LibPos);
	bool bIsEyeShape = (still1LibGrid.getPattern().getEye(myColor)>0);

	const WeichiBitBoard& bm3x3StoneNbr = still1LibGrid.getStaticGrid().get3x3StoneNbrsMap();
	while( (pos=bmNbr1LibBlock.bitScanForward())!=-1 ) {
		const WeichiBlock* nbrBlock = m_board.getGrid(pos).getBlock();
		bmNbr1LibBlock -= nbrBlock->getStoneMap();

		// avoid to save by capturing feature
		if( !(nbrBlock->getStoneMap()&block->getStonenNbrMap()).empty() ) { continue; }

		if( bIsEyeShape && !(nbrBlock->getStoneMap()&bm3x3StoneNbr).empty() ) { addProbabilityFeature(nbrBlock->getLastLibertyPos(),myColor,POLICY_REPLY_SAVE_1LIB_EYE_SHAPE_BY_CAPTURE_EXTEND_NBR); }
		else { addProbabilityFeature(nbrBlock->getLastLibertyPos(),myColor,POLICY_REPLY_SAVE_1LIB_BY_CAPTURE_EXTEND_NBR); }
	}
}

void WeichiPlayoutAgent::addKill2LibFeatures( const WeichiBlock* block )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

	if( block->getStatus()==LAD_LIFE ) { return; }

	Color turnColor = m_board.getToPlay();

	WeichiBitBoard bmNbr1LibBlock;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmNbr1LibBlock);
	bool bNbrHas1LibBlock = !bmNbr1LibBlock.empty();

	// close door
	WeichiBitBoard bmLiberty = block->getLibertyBitBoard(m_board.getBitBoard());
	Vector<uint,2> vLibPos;
	bmLiberty.bitScanAll(vLibPos);
	uint close_door = WeichiLowLibReader::kill2LibByCloseDoor(m_board,block,vLibPos[0],vLibPos[1]);
	if( close_door!=-1 && !bNbrHas1LibBlock ) { addProbabilityFeature(close_door,turnColor,POLICY_REPLY_KILL_2LIB_BY_COLSE_DOOR); }

	// atari or atari dead
	WeichiBitBoard bmKill, bmUnknown;
	bmLiberty = block->getLibertyBitBoard(m_board.getBitBoard());
	if( WeichiLowLibReader::isBitBoardInBlockCA(m_board,block,bmLiberty) ) { bmKill |= bmLiberty; }
	else {
		WeichiLowLibReader::kill2LibDirectly(m_board,block,vLibPos[0],vLibPos[1],bmKill,bmUnknown);
		WeichiLowLibReader::kill2LibDirectly(m_board,block,vLibPos[1],vLibPos[0],bmKill,bmUnknown);
	}
	if( bNbrHas1LibBlock ) {
		bmUnknown |= bmKill;
		bmKill.Reset();
	}
	addProbabilityFeatures(bmKill,turnColor,POLICY_REPLY_KILL_2LIB_BY_ATARI_DEAD);
	addProbabilityFeatures(bmUnknown,turnColor,POLICY_REPLY_KILL_2LIB_BY_ATARI);
}

void WeichiPlayoutAgent::addSave2LibFeatures( const WeichiBlock* block )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

	addSave2LibByCaptureFeatures(block);
	addSave2LibByExtendAndJumpFeatures(block);
}

void WeichiPlayoutAgent::addSave2LibByCaptureFeatures( const WeichiBlock* block )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

	// capture 1 liberty
	Color myColor = block->getColor();
	WeichiBitBoard bmNbr1LibBlock;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmNbr1LibBlock);

	uint pos;
	while( (pos=bmNbr1LibBlock.bitScanForward())!=-1 ) {
		const WeichiBlock* nbrBlock = m_board.getGrid(pos).getBlock();
		bmNbr1LibBlock -= nbrBlock->getStoneMap();

		addProbabilityFeature(nbrBlock->getLastLibertyPos(),myColor,POLICY_REPLY_SAVE_2LIB_BY_CAPTURE);
	}
}

void WeichiPlayoutAgent::addSave2LibByExtendAndJumpFeatures( const WeichiBlock* block )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

	Vector<uint,2> vLibPos;
	Vector<uint,2> vLibAfterPlay;
	block->getLibertyPositions(m_board.getBitBoard(),vLibPos);
	Color myColor = block->getColor();

	vLibAfterPlay.resize(2);
	for( uint i=0; i<2; i++ ) {
		uint extend_pos = vLibPos[i];
		WeichiMove extendMove(myColor,extend_pos);
		vLibAfterPlay[i] = m_board.getLibertyAfterPlay(extendMove);
		if( vLibAfterPlay[i]==1 ) { continue; }

		addProbabilityFeature(vLibPos[i],myColor,POLICY_REPLY_SAVE_2LIB_BY_EXTEND);
	}

	/* Jump case: (@ is previous move, in this case, ! is better)
		-----------
		@O ! OX
		XO ! OX
		XXOOOOX
		 XXXXX
		PS: liberty must be adjacent
	*/
	if( !m_board.getGrid(vLibPos[0]).getStonenNbrMap().BitIsOn(vLibPos[1]) ) { return; }

	uint jumpDir;
	if( vLibAfterPlay[1]>2 && (jumpDir=m_board.getGrid(vLibPos[0]).getPattern().get2LibJump(myColor))>0 ) {
		uint jumpPos = m_board.getGrid(vLibPos[0]).getStaticGrid().getNeighbor(m_board.realDirectionDIR4of(jumpDir));
		addProbabilityFeature(jumpPos,myColor,POLICY_REPLY_SAVE_2LIB_BY_JUMP);
	}
	if( vLibAfterPlay[0]>2 && (jumpDir=m_board.getGrid(vLibPos[1]).getPattern().get2LibJump(myColor))>0 ) {
		uint jumpPos = m_board.getGrid(vLibPos[1]).getStaticGrid().getNeighbor(m_board.realDirectionDIR4of(jumpDir));
		addProbabilityFeature(jumpPos,myColor,POLICY_REPLY_SAVE_2LIB_BY_JUMP);
	}
}

void WeichiPlayoutAgent::addNakadeFeatures( const WeichiMove& lastMove )
{
	WeichiBitBoard bmCheckIndex;
	const WeichiGrid& lastGrid = m_board.getGrid(lastMove);

	for( const int *iNbr=lastGrid.getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& grid = m_board.getGrid(*iNbr);
		if( !grid.isEmpty() ) { continue; }

		const WeichiClosedArea* closedArea = grid.getClosedArea(lastMove.getColor());
		if( !closedArea || !closedArea->hasNakade() ) { continue; }
		if( !(closedArea->getStoneMap()&bmCheckIndex).empty() ) { continue; }

		bmCheckIndex |= closedArea->getStoneMap();
		addProbabilityFeature(closedArea->getNakade(),m_board.getToPlay(),POLICY_REPLY_NAKADE);
	}
}

void WeichiPlayoutAgent::addMakeEyeFeatures( const WeichiMove& lastMove )
{	
	const WeichiGrid& lastGrid = m_board.getGrid(lastMove);

	for( const int *iNbr=lastGrid.getStaticGrid().getDiamondEndNeighbors(); *iNbr!=-1; iNbr++ ) { addMakeEyeDetailFeatures(m_board.getGrid(*iNbr)); }
	for( const int *iNbr=lastGrid.getStaticGrid().getDiagonalNeighbors(); *iNbr!=-1; iNbr++ ) { addMakeEyeDetailFeatures(m_board.getGrid(*iNbr)); }
}

void WeichiPlayoutAgent::addMakeEyeDetailFeatures( const WeichiGrid& grid )
{
	if( !grid.isEmpty() ) { return; }

	uint pos;
	Color turnColor = m_board.getToPlay();

	if( grid.getPattern().getMakeTrueEye(turnColor) ) {
		uint makeTrueEye = grid.getPattern().getMakeTrueEye(turnColor);
		pos = grid.getStaticGrid().getNeighbor(m_board.realDirectionDIR4of(makeTrueEye));
		addProbabilityFeature(pos,turnColor,POLICY_REPLY_MAKE_TRUE_EYE);
	} else if( grid.getPattern().getMakePotentialTrueEye(turnColor) ) {
		uint makePotentialTrueEye = grid.getPattern().getMakePotentialTrueEye(turnColor);
		pos = grid.getStaticGrid().getNeighbor(m_board.realDirectionDIR4of(makePotentialTrueEye));
		addProbabilityFeature(pos,turnColor,POLICY_REPLY_MAKE_POTENTIAL_TRUE_EYE);
	} else if( grid.getPattern().getMakeFalseEye(turnColor) ) {
		uint makeFalseEye = grid.getPattern().getMakeFalseEye(turnColor);
		pos = grid.getStaticGrid().getNeighbor(m_board.realDirectionDIR4of(makeFalseEye));
		addProbabilityFeature(pos,turnColor,POLICY_REPLY_MAKE_FALSE_EYE);
	}
}

void WeichiPlayoutAgent::addFixEyeFeatures( const WeichiMove& lastMove )
{
	uint pos;
	Color turnColor = m_board.getToPlay();
	const WeichiGrid& lastGrid = m_board.getGrid(lastMove);

	for( const int *iNbr=lastGrid.getStaticGrid().getDiagonalNeighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& grid = m_board.getGrid(*iNbr);
		if( !grid.getPattern().getEye(turnColor) ) { continue; }

		if( grid.getPattern().getFixTrueEye(turnColor) ) {
			uint fixTrueEye = grid.getPattern().getFixTrueEye(turnColor);
			const Vector<uint,8>& vPoint = StaticBoard::getPattern3x3Direction(fixTrueEye);
			for( uint i=0; i<vPoint.size(); i++ ) {
				pos = grid.getStaticGrid().getNeighbor(vPoint[i]);
				addProbabilityFeature(pos,turnColor,POLICY_REPLY_FIX_TRUE_EYE);
			}
		} else if( grid.getPattern().getFixPotentialTrueEye(turnColor) ) {
			uint fixPotentialTrueEye = grid.getPattern().getFixPotentialTrueEye(turnColor);
			const Vector<uint,8>& vPoint = StaticBoard::getPattern3x3Direction(fixPotentialTrueEye);
			for( uint i=0; i<vPoint.size(); i++ ) {
				pos = grid.getStaticGrid().getNeighbor(vPoint[i]);
				addProbabilityFeature(pos,turnColor,POLICY_REPLY_FIX_POTENTIAL_TRUE_EYE);
			}
		} else if( grid.getPattern().getFixFalseEye(turnColor) ) {
			uint fixFalseEye = grid.getPattern().getFixFalseEye(turnColor);
			const Vector<uint,8>& vPoint = StaticBoard::getPattern3x3Direction(fixFalseEye);
			for( uint i=0; i<vPoint.size(); i++ ) {
				pos = grid.getStaticGrid().getNeighbor(vPoint[i]);
				addProbabilityFeature(pos,turnColor,POLICY_REPLY_FIX_FALSE_EYE);
			}
		}
	}
}

void WeichiPlayoutAgent::addDestroyEyeFeatures( const WeichiMove& lastMove )
{
	uint pos;
	Color turnColor = m_board.getToPlay();
	Color oppColor = AgainstColor(turnColor);
	const WeichiGrid& lastGrid = m_board.getGrid(lastMove);

	for( const int *iNbr=lastGrid.getStaticGrid().get3x3Neighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& grid = m_board.getGrid(*iNbr);
		if( !grid.isEmpty() ) { continue; }

		// destroy make eye
		if( grid.getPattern().getMakeTrueEye(oppColor) ) {
			uint makeTrueEye = grid.getPattern().getMakeTrueEye(oppColor);
			pos = grid.getStaticGrid().getNeighbor(m_board.realDirectionDIR4of(makeTrueEye));
			addProbabilityFeature(pos,turnColor,POLICY_REPLY_DESTROY_MAKE_TRUE_EYE);
		} else if( grid.getPattern().getMakePotentialTrueEye(oppColor) ) {
			uint makePotentialTrueEye = grid.getPattern().getMakePotentialTrueEye(oppColor);
			pos = grid.getStaticGrid().getNeighbor(m_board.realDirectionDIR4of(makePotentialTrueEye));
			addProbabilityFeature(pos,turnColor,POLICY_REPLY_DESTROY_MAKE_POTENTIAL_TRUE_EYE);
		} else if( grid.getPattern().getMakeFalseEye(oppColor) ) {
			uint makeFalseEye = grid.getPattern().getMakeFalseEye(oppColor);
			pos = grid.getStaticGrid().getNeighbor(m_board.realDirectionDIR4of(makeFalseEye));
			addProbabilityFeature(pos,turnColor,POLICY_REPLY_DESTROY_MAKE_FALSE_EYE);
		}

		// destroy fix eye
		if( grid.getPattern().getFixTrueEye(oppColor) ) {
			uint fixTrueEye = grid.getPattern().getFixTrueEye(oppColor);
			const Vector<uint,8>& vPoint = StaticBoard::getPattern3x3Direction(fixTrueEye);
			for( uint i=0; i<vPoint.size(); i++ ) {
				pos = grid.getStaticGrid().getNeighbor(vPoint[i]);
				addProbabilityFeature(pos,turnColor,POLICY_REPLY_DESTROY_FIX_TRUE_EYE);
			}
		} else if( grid.getPattern().getFixPotentialTrueEye(oppColor) ) {
			uint fixPotentialTrueEye = grid.getPattern().getFixPotentialTrueEye(oppColor);
			const Vector<uint,8>& vPoint = StaticBoard::getPattern3x3Direction(fixPotentialTrueEye);
			for( uint i=0; i<vPoint.size(); i++ ) {
				pos = grid.getStaticGrid().getNeighbor(vPoint[i]);
				addProbabilityFeature(pos,turnColor,POLICY_REPLY_DESTROY_FIX_POTENTIAL_TRUE_EYE);
			}
		} else if( grid.getPattern().getFixFalseEye(oppColor) ) {
			uint fixFalseEye = grid.getPattern().getFixFalseEye(oppColor);
			const Vector<uint,8>& vPoint = StaticBoard::getPattern3x3Direction(fixFalseEye);
			for( uint i=0; i<vPoint.size(); i++ ) {
				pos = grid.getStaticGrid().getNeighbor(vPoint[i]);
				addProbabilityFeature(pos,turnColor,POLICY_REPLY_DESTROY_FIX_FALSE_EYE);
			}
		}
	}
}

void WeichiPlayoutAgent::add2345SemeaiFeatures( const WeichiBlock* block )
{
	Vector<uint,MAX_NUM_GRIDS> vSemeaiPoints;
	WeichiSemeaiResult semeaiResult = WeichiSemeaiHandler::getSemeaiType(m_board,block,NULL,vSemeaiPoints);
	if( semeaiResult!=SEMEAI_BASIC && semeaiResult!=SEMEAI_CONNECT ) { return; }

	Color myColor = m_board.getToPlay();
	Color oppColor = AgainstColor(myColor);

	uint pos;
	WeichiBitBoard bmNbrSemeaiBlock = (block->getStonenNbrMap()&m_board.getStoneBitBoard(oppColor));
	while( (pos=bmNbrSemeaiBlock.bitScanForward())!=-1 ) {
		const WeichiBlock* semeaiBlock = m_board.getGrid(pos).getBlock();
		bmNbrSemeaiBlock -= semeaiBlock->getStoneMap();

		if( semeaiBlock->getLiberty()>block->getLiberty() ) { continue; }
		semeaiResult = WeichiSemeaiHandler::getSemeaiType(m_board,semeaiBlock,block,vSemeaiPoints);
		if( semeaiResult!=SEMEAI_BASIC && semeaiResult!=SEMEAI_CONNECT ) { continue; }

		WeichiPlayoutPolicy policy = (semeaiResult==SEMEAI_BASIC)? POLICY_REPLY_2345LIB_SEMEAI: POLICY_REPLY_2345LIB_SEMEAI_CONNECT;
		for( uint i=0; i<vSemeaiPoints.size(); i++ ) { addProbabilityFeature(vSemeaiPoints[i],myColor,policy); }
	}

	/*if( block->getStatus()==LAD_LIFE ) { return; }
	if( block->getLiberty()<2 || block->getLiberty()>5 ) { return; }

	Color myColor = m_board.getToPlay();
	Color oppColor = AgainstColor(myColor);

	if( !WeichiLowLibReader::isSemeaiBlock(m_board,block) ) { return; }

	uint pos;
	bool bSetReduceLiberty = false;
	WeichiBitBoard bmNbrSemeaiBlock = (block->getStonenNbrMap()&m_board.getStoneBitBoard(oppColor));
	while( (pos=bmNbrSemeaiBlock.bitScanForward())!=-1 ) {
		const WeichiBlock* semeaiBlock = m_board.getGrid(pos).getBlock();
		bmNbrSemeaiBlock -= semeaiBlock->getStoneMap();

		if( semeaiBlock->getLiberty()>block->getLiberty()+1 ) { continue; }
		if( !WeichiLowLibReader::isSemeaiBlock(m_board,semeaiBlock) ) { continue; }

		uint libPos;
		WeichiBitBoard bmLiberty = semeaiBlock->getLibertyBitBoard(m_board.getBitBoard());
		while( (libPos=bmLiberty.bitScanForward())!=-1 ) {
			libPos = adjustSemeaiPoint(libPos);
			addProbabilityFeature(libPos,myColor,POLICY_REPLY_2345LIB_SEMEAI);
		}

		if( !bSetReduceLiberty ) {
			bSetReduceLiberty = true;
			bmLiberty = block->getLibertyBitBoard(m_board.getBitBoard())-semeaiBlock->getLibertyBitBoard(m_board.getBitBoard());
			while( (libPos=bmLiberty.bitScanForward())!=-1 ) {
				addProbabilityFeature(libPos,myColor,POLICY_REPLY_REDUCE_OWN_2345LIB_SEMEAI);
			}
		}
	}*/
}

uint WeichiPlayoutAgent::adjustSemeaiPoint( const uint position )
{
	/* Adjust Semeai Point:
		@XX!OOOX
		XOOOO OX
		XXXXOOOX
		XXXXXXXX
		
		PS: change ! to @
	*/
	Color myColor = m_board.getToPlay();

	const WeichiGrid& semeaiGrid = m_board.getGrid(position);
	if( semeaiGrid.getPattern().getAdjGridCount(myColor)==0 ) { return position; }

	WeichiMove move(myColor,position);
	WeichiBitBoard bmLibertyAfterPlay;
	m_board.getLibertyBitBoardAfterPlay(move,bmLibertyAfterPlay);
	if( bmLibertyAfterPlay.bitCount()!=1 ) { return position; }

	uint anotherLibPos = bmLibertyAfterPlay.bitScanForward();
	if( m_board.getLibertyAfterPlay(WeichiMove(myColor,anotherLibPos))>1 ) { return anotherLibPos; }
	
	return position;
}

void WeichiPlayoutAgent::addCASemeaiFeature( const WeichiBlock* block )
{
	if( !m_board.isLastMoveCaptureBlock() ) { return; }
	if( block->getLiberty()<2 || block->getLiberty()>5 ) { return; }
	if( m_board.getCurrentMoveStack().m_vDeadBlocks.size()>1 ) { return; }

	Color myColor = m_board.getToPlay();
	WeichiBitBoard bmDeadStone = m_board.getDeadStoneBitBoard();
	uint caPoint = bmDeadStone.bitScanForward();
	const WeichiClosedArea* ca = m_board.getGrid(caPoint).getClosedArea(block->getColor());
	if( !ca || ca->getNumStone()!=4 ) { return; }
	if( !m_board.getGrid(caPoint).getPattern().getTofuFour(myColor) ) { return; }
	if( !WeichiLowLibReader::isSemeaiBlock(m_board,block) ) { return; }

	uint pos;
	WeichiBitBoard bmStone = ca->getStoneMap()-m_board.getBitBoard();
	while( (pos=bmStone.bitScanForward())!=-1 ) {
		addProbabilityFeature(pos,myColor,POLICY_REPLY_CA_SEMEAI);
	}
}

WeichiMove WeichiPlayoutAgent::genMoveToKillSuicide( const WeichiMove& lastMove )
{
	assertToFile( !lastMove.isPass(), const_cast<WeichiBoard*>(&m_board) );

	/*
		1. last move block must be 1 liberty
		2. put stone in opponent last liberty shouldn't be illegal move or snapback
	*/
	const WeichiBlock* lastBlock = m_board.getGrid(lastMove).getBlock();
	if( lastBlock->getLiberty()!=1 ) { return PASS_MOVE; }

	const uint lastLiberty = lastBlock->getLastLiberty(m_board.getBitBoard());
	const WeichiMove move(m_board.getToPlay(),lastLiberty);
	
	if( WeichiBadMoveReader::isLifeTerritory(m_board,move) ) { m_bmSkipMove.SetBitOn(lastLiberty); return PASS_MOVE; }
	if( m_board.isIllegalMove(move,m_state.m_ht) ) { m_bmSkipMove.SetBitOn(lastLiberty); return PASS_MOVE; }
	if( getKB().isSnapback(move) ) { m_bmSkipMove.SetBitOn(lastLiberty); return PASS_MOVE; }
	if( getKB().isBadMoveForKillSuicide(move,lastBlock) ) { m_bmSkipMove.SetBitOn(lastLiberty); return PASS_MOVE; }
	
	return move;
}

void WeichiPlayoutAgent::genMoveToReplySaveOneLibBlock( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vSaveMoves )
{
	vSaveMoves.clear();

	Color myColor = m_board.getToPlay();
	const WeichiGrid& lastGrid = m_board.getGrid(lastMove);
	
	WeichiBitBoard bmCheckIndex;
	WeichiBitBoard bmSaveMove,bmUnknownMove;
	const int *iNeighbor = lastGrid.getStaticGrid().getAdjacentNeighbors();
	for( ; *iNeighbor!=-1; iNeighbor++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNeighbor);
		if( nbrGrid.getColor()!=myColor ) { continue; }

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if( nbrBlock->getLiberty()>1 || bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) ) { continue; }

		bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
		WeichiLowLibReader::save1Lib(m_board,m_state.m_ht,nbrBlock,bmSaveMove,bmUnknownMove,m_bmSkipMove);
	}

	if( !bmSaveMove.empty() ) { bmSaveMove.bitScanAll(vSaveMoves); }
	else if( !bmUnknownMove.empty() && Random::nextInt(2)==1 ) { bmUnknownMove.bitScanAll(vSaveMoves); }
}

void WeichiPlayoutAgent::genMoveToReplyKillTwoLibBlock( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vKillMoves )
{
	vKillMoves.clear();

	const WeichiBlock* lastBlock = m_board.getGrid(lastMove).getBlock();
	if( lastBlock->getLiberty()!=2 ) { return; }

	WeichiBitBoard bmKillMove,bmUnknownMove;
	WeichiLowLibReader::kill2Lib(m_board,m_state.m_ht,lastBlock,bmKillMove,bmUnknownMove,m_bmSkipMove);

	if( !bmKillMove.empty() ) { bmKillMove.bitScanAll(vKillMoves); }
	else if( !bmUnknownMove.empty() && Random::nextInt(2)==1 ) { bmUnknownMove.bitScanAll(vKillMoves); }
}

void WeichiPlayoutAgent::genMoveToReplySaveTwoLibBlock( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vSaveMoves )
{
	vSaveMoves.clear();

	Color myColor = m_board.getToPlay();
	const WeichiGrid& lastGrid = m_board.getGrid(lastMove);

	WeichiBitBoard bmCheckIndex;
	WeichiBitBoard bmSaveMove,bmUnknownMove;
	const int *iNeighbor = lastGrid.getStaticGrid().getAdjacentNeighbors();
	for( ; *iNeighbor!=-1; iNeighbor++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNeighbor);
		if( nbrGrid.getColor()!=myColor ) { continue; }

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if( nbrBlock->getLiberty()!=2 || bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) ) { continue; }

		bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
		getKB().save2Lib(nbrBlock,bmSaveMove,bmUnknownMove);
	}

	if( !bmSaveMove.empty() ) { bmSaveMove.bitScanAll(vSaveMoves); }
	else if( !bmUnknownMove.empty() && Random::nextInt(2)==1 ) { bmUnknownMove.bitScanAll(vSaveMoves); }
}

void WeichiPlayoutAgent::genMoveToReplyDefenseApproachLib( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vDefenseMoves )
{
	vDefenseMoves.clear();

	uint pos;
	Color myColor = m_board.getToPlay();
	WeichiBitBoard bmApproachLib = m_board.getApproachLibBlockPos() - m_board.getBitBoard();
	while( (pos=bmApproachLib.bitScanForward())!=-1 ) {
		const WeichiGrid& defenseGrid = m_board.getGrid(pos);
		if( defenseGrid.getPattern().getEmptyAdjGridCount()>0 ) { continue; }
		if( defenseGrid.getPattern().getAdjGridCount(lastMove.getColor())!=1 ) { continue; }

		const WeichiMove defenseMove(myColor,pos);
		PredictPlayInfo defenseInfo;
		m_board.getPredictPlayInfoByPlay(defenseMove,defenseInfo);
		
		if( defenseInfo.m_liberty==0 ) { continue; }
		else if( defenseInfo.m_liberty==1 ) {
			if( !defenseInfo.m_bHasCombineBlock ) {
				const uint anyAgjNbr = m_board.realDirectionDIR4of(defenseGrid.getPattern().getAnyAdjNeighbor(myColor));
				const WeichiBlock* nbrBlock = m_board.getGrid(defenseGrid,anyAgjNbr).getBlock();
				assertToFile( nbrBlock && nbrBlock->getLiberty()==2 && nbrBlock->getColor()==myColor, const_cast<WeichiBoard*>(&m_board) );

				WeichiBitBoard bmSave,bmUnknown;
				getKB().save2Lib(nbrBlock,bmSave,bmUnknown);

				if( !bmSave.empty() ) { bmSave.bitScanAll(vDefenseMoves); }
				else if( !bmUnknown.empty() && Random::nextInt(2)==1 ) { bmUnknown.bitScanAll(vDefenseMoves); }
			}
		} else {
			for( const int *iNbr=defenseGrid.getStaticGrid().getDiagonalNeighbors(); *iNbr!=-1; iNbr++ ) {
				const WeichiGrid& diagonalGrid = m_board.getGrid(*iNbr);
				if( !diagonalGrid.isEmpty() ) { continue; }
				if( diagonalGrid.getPattern().getTrueEye(myColor) ) { continue; }
				if( !diagonalGrid.getPattern().getPotentialTrueEye(myColor) ) { continue; }

				/*m_board.toColorString();
				cerr << toChar(lastMove.getColor()) << ' ' << lastMove.toGtpString() << endl;
				cerr << "reply " << WeichiMove(pos).toGtpString() << endl;
				int k;
				cin >> k;*/

				vDefenseMoves.push_back(pos);
				break;
			}
		}
	}
}

void WeichiPlayoutAgent::genMoveToReplyLastCANakade( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vNakadeMoves )
{
	vNakadeMoves.clear();

	WeichiBitBoard bmCheckIndex;
	const WeichiGrid& lastGrid = m_board.getGrid(lastMove);
	
	for( const int *iNbr=lastGrid.getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& grid = m_board.getGrid(*iNbr);
		if( !grid.isEmpty() ) { continue; }

		const WeichiClosedArea* closedArea = grid.getClosedArea(lastMove.getColor());
		if( !closedArea || !closedArea->hasNakade() ) { continue; }
		if( !(closedArea->getStoneMap()&bmCheckIndex).empty() ) { continue; }

		bmCheckIndex |= closedArea->getStoneMap();
		vNakadeMoves.push_back(closedArea->getNakade());
	}
}

void WeichiPlayoutAgent::genMoveToFightKo( Vector<uint,MAX_NUM_GRIDS>& vCandidateMoves )
{
	vCandidateMoves.clear();

	if( m_board.m_status.m_lastKo==-1 || Random::nextInt(2)==0 ) { return; }

	// try to eat neighbor to solve ko
	Color myColor = m_board.getToPlay();
	Color oppColor = AgainstColor(myColor);
	uint koEater = m_board.m_status.m_koEater;
	for( const int *iNbr = m_board.getGrid(koEater).getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNbr);
		if( nbrGrid.getColor()!=oppColor ) { continue; }

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if( nbrBlock->getLiberty()!=1 ) { continue; }

		uint lastLiberty = nbrBlock->getLastLiberty(m_board.getBitBoard());
		const WeichiMove killMove(myColor,lastLiberty);
		if( m_board.isIllegalMove(killMove,m_state.m_ht) ) { continue; }

		vCandidateMoves.push_back(lastLiberty);
		return ;
	}

	// try save ko if cannot eat neighbor
	const WeichiMove saveMove(myColor,m_board.m_status.m_lastKo);
	if( m_board.getLibertyAfterPlay(saveMove)>1 && !m_board.isIllegalMove(saveMove,m_state.m_ht) ) {
		vCandidateMoves.push_back(saveMove.getPosition());
	}
}

void WeichiPlayoutAgent::genMoveToReplyGoodPattern( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vGoodPatternMoves )
{
	vGoodPatternMoves.clear();

	Color colorToPlay = m_board.getToPlay();

	const WeichiGrid& lastGrid = m_board.getGrid(lastMove);
	for( const int* iNbr=lastGrid.getStaticGrid().get3x3Neighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNbr);
		assertToFile( nbrGrid.getPatternIndex() < pattern33::TABLE_SIZE, const_cast<WeichiBoard*>(&m_board) ) ;
		if( !nbrGrid.isEmpty() || !nbrGrid.getPattern().getGoodPattern(colorToPlay) ) { continue; }
		vGoodPatternMoves.push_back(*iNbr);
	}
}

void WeichiPlayoutAgent::genMoveTo345PointSemeai( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vSemeaiMoves )
{
	vSemeaiMoves.clear();

	Color myColor = m_board.getToPlay();
	Color oppColor = AgainstColor(myColor);

	WeichiBitBoard bm345PointSemeai;
	const WeichiGrid& lastGrid = m_board.getGrid(lastMove);
	for( const int* iNbr=lastGrid.getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNbr);
		if( nbrGrid.getColor()!=myColor ) { continue; }

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if( nbrBlock->getLiberty()<3 || nbrBlock->getLiberty()>5 ) { continue; }
		if( !WeichiKnowledgeBase::isSemeaiBlock(m_board,nbrBlock) ) { continue; }

		// block can extend liberty
		uint pos;
		WeichiBitBoard bmNbrSemeaiBlock = (nbrBlock->getStonenNbrMap()&m_board.getStoneBitBoard(oppColor));
		while( (pos=bmNbrSemeaiBlock.bitScanForward())!=-1 ) {
			const WeichiBlock* semeaiBlock = m_board.getGrid(pos).getBlock();
			bmNbrSemeaiBlock -= semeaiBlock->getStoneMap();

			if( semeaiBlock->getLiberty()>5 ) { continue; }
			if( !WeichiKnowledgeBase::isSemeaiBlock(m_board,semeaiBlock) ) { continue; }
			bm345PointSemeai |= semeaiBlock->getLibertyBitBoard(m_board.getBitBoard());
		}
	}

	bm345PointSemeai.bitScanAll(vSemeaiMoves);
	/*m_board.showColorBoard();
	cerr << "size= " << vSemeaiMoves.size() << endl;
	cerr << "color= " << toChar(myColor) << ", last move= " << lastMove.toGtpString() << endl;
	for( uint i=0; i<vSemeaiMoves.size(); i++ ) {
		cerr << WeichiMove(vSemeaiMoves[i]).toGtpString() << ' ';
	}
	cerr << endl;
	int k;
	cin >> k;*/
}

WeichiMove WeichiPlayoutAgent::genRandomMove()
{
    Color toPlay = m_board.getToPlay();

    /// method:
    ///     split candidate list into two parts ( ignore and normal )
    ///     in the beginning, ignore is empty, all candidates are normal
    ///     select move from candidates, if not a good move, swap it to ignore part
	uint size = m_board.m_candidates.getNumCandidate() ;
    uint ignoresize = 0;
    CandidateList& candidates = m_state.m_board.m_candidates;

    while ( size ) {
        // uint idx = rand()%size + ignoresize ;
        uint idx = Random::nextInt(size) + ignoresize ;
        uint pos = candidates[idx] ;

        const WeichiMove move(toPlay, pos);
        RandomMoveType type = checkRandomMove(move);
        if ( type == IGNORE_MOVE ) {
            candidates.swap(idx, ignoresize) ;
            ++ ignoresize, --size ;
            continue ;
        } else if ( type == REMOVE_CANDIDATE ) {
            candidates.removeCandidate(pos) ;
            continue ;
        } else {
            return move ;
        }
    }

    return WeichiMove(toPlay) ;
}

WeichiMove WeichiPlayoutAgent::randomSelectMove( Color turnColor, const Vector<uint,MAX_NUM_GRIDS>& moveset )
{
	if( moveset.size()==0 ) { return PASS_MOVE; }

	uint size = moveset.size();
	uint offSet = Random::nextInt(size);

	for( uint i=0; i<size; i++ ) {
		uint pos = moveset[(i+offSet)%size];
		const WeichiMove move(turnColor,pos);

		if( checkRandomMove(move)==FINE ) { return move; }
	}

	return PASS_MOVE;
}

WeichiPlayoutAgent::RandomMoveType WeichiPlayoutAgent::checkRandomMove( const WeichiMove& move )
{
	if( move.isPass() ) { return FINE; }

	const uint position = move.getPosition();
	if( m_bmSkipMove.BitIsOn(position) ) { return IGNORE_MOVE; }
	if (m_board.isIllegalMove(move, m_state.m_ht)) { m_bmSkipMove.SetBitOn(position); return IGNORE_MOVE; }
	if( WeichiBadMoveReader::getBadMoveType(m_board,move)!=BADMOVE_NOT_BADMOVE ) { m_bmSkipMove.SetBitOn(position); return IGNORE_MOVE; }

    return FINE;
}