#include "WeichiBadMoveReader.h"

WeichiBadMoveReader::WeichiBadMoveReader()
{
}

WeichiBadMoveType WeichiBadMoveReader::getBadMoveType( const WeichiBoard& board, const WeichiMove& move )
{
	if( move.isPass() ) { return BADMOVE_NOT_BADMOVE; }
	if( isLifeTerritory(board, move) ) { return BADMOVE_LIFE_TERRITORY; }
	if( isPotentialTrueEye(board, move) ) { return BADMOVE_POTENTIAL_TRUE_EYE; }
	if( isFillingOwnEyeShape(board, move) ) { return BADMOVE_FILLING_OWN_EYE_SHAPE; }

	//return BADMOVE_NOT_BADMOVE;
	PredictPlayInfo moveInfo;
	board.getPredictPlayInfoByPlay(move, moveInfo);
	return getBadMoveType(board, move, moveInfo);
}

WeichiBadMoveType WeichiBadMoveReader::getBadMoveType( const WeichiBoard& board, const WeichiMove& move, const PredictPlayInfo& moveInfo )
{
	// bad move means never put stone on this position
	// PS: put fast function in the head!!
	assertToFile( !move.isPass(), const_cast<WeichiBoard*>(&board) );

	if( isTwoLibFalseEye(board, move, moveInfo) ) { return BADMOVE_TWO_LIB_FALSE_EYE; }
	if( isNotMakingNakadeInOppCA(board, move, moveInfo) ) { return BADMOVE_NOT_MAKING_NAKADE_IN_OPP_CA; }

	// do not check follow when there is a ko in UCT
	if( !board.isPlayoutStatus() && board.hasKo() ) { return BADMOVE_NOT_BADMOVE; }
	if( isBadSelfAtari(board, move, moveInfo) ) { return BADMOVE_SELF_ATARI; }
	if( isMutualAtari(board, move, moveInfo) ) { return BADMOVE_MUTUAL_ATARI; }

	return BADMOVE_NOT_BADMOVE;
}

bool WeichiBadMoveReader::isBadSelfAtari( const WeichiBoard& board, const WeichiMove& move, const PredictPlayInfo& moveInfo )
{
	// some condition:
	//	1. must be 1 liberty after play
	//	2. must not be making nakade shape
	//	3. if neighbor have something can eat, eat it
	if( moveInfo.m_liberty!=1 ) { return false; }
	if( moveInfo.m_blockNumStone<3 ) { return false; }
	if( !isMakingNakadeShape(board, move) ) { return true; }

	Color oppColor = AgainstColor(move.getColor());
	if( !(moveInfo.m_bmPreNbrStoneMap&board.getOneLibBlocksBitBoard(oppColor)).empty() ) { return true; }

	return false;
}

bool WeichiBadMoveReader::isLifeTerritory( const WeichiBoard& board, const WeichiMove& move )
{
	assertToFile( board.getGrid(move).isEmpty(), const_cast<WeichiBoard*>(&board) );

	if( !WeichiConfigure::use_closed_area ) { return false; }

	Color myColor = move.getColor();
	const WeichiGrid& grid = board.getGrid(move);

	// don't put stone in own life territory or opponent life territory
	const WeichiClosedArea *ownCA = grid.getClosedArea(myColor);
	if( ownCA && ownCA->getStatus()==LAD_LIFE ) { return true; }
	const WeichiClosedArea *oppCA = grid.getClosedArea(AgainstColor(myColor));
	if( oppCA && oppCA->getStatus()==LAD_LIFE ) { return true; }

	return false;
}

bool WeichiBadMoveReader::isPotentialTrueEye( const WeichiBoard& board, const WeichiMove& move )
{
	assertToFile( board.getGrid(move).isEmpty(), const_cast<WeichiBoard*>(&board) );

	// potential true eye means if we put stone continuously, it will be true eye
	// TODO: may be ko for opponent if avoid
	const WeichiGrid& grid = board.getGrid(move);
	if( grid.getPattern().getPotentialTrueEye(move.getColor()) ) { return true; }

	return false;
}

bool WeichiBadMoveReader::isMutualAtari( const WeichiBoard& board, const WeichiMove& move, const PredictPlayInfo& myInfo )
{
	assertToFile( board.getGrid(move).isEmpty(), const_cast<WeichiBoard*>(&board) );

	const WeichiMove oppMove(AgainstColor(move.getColor()),move.getPosition());
	const WeichiGrid& grid = board.getGrid(move);

	// after put stone should be one liberty and should be >=3 stone (because 3 is the less nakade shape)
	if( grid.getLiberty()>=2 ) { return false; }

	if( myInfo.m_liberty!=1 || myInfo.m_blockNumStone==1 ) { return false; }

	PredictPlayInfo oppInfo;
	board.getPredictPlayInfoByPlay(oppMove,oppInfo);
	if( oppInfo.m_liberty!=1 || oppInfo.m_blockNumStone==1 ) { return false; }

	// avoid making nakade shape
	if( isMakingNakadeShape(board, move) && isSharedLibertyWithNbrBlock(board, myInfo) ) { return false; }
	if( isMakingNakadeShape(board, oppMove) && isSharedLibertyWithNbrBlock(board, oppInfo) ) { return false; }

	return true;
}

bool WeichiBadMoveReader::isMakingNakadeShape( const WeichiBoard& board, const WeichiMove& move )
{
	const WeichiGrid& grid = board.getGrid(move.getPosition());
	WeichiBitBoard bmNbrStoneMap = grid.getStonenNbrMap();
	WeichiBitBoard bmShape;
	uint pos;
	while( (pos=bmNbrStoneMap.bitScanForward()) != -1 ) {
		const WeichiBlock* nbrBlock = board.getGrid(pos).getBlock();
		if ( !nbrBlock || nbrBlock->getColor() != move.getColor() ) { continue; }
		if ( nbrBlock->getNumStone() > 6 ) { return false; }

		bmShape |= nbrBlock->getStoneMap();
	}
	bmShape.SetBitOn(move.getPosition());

	return WeichiKnowledgeBase::isNakadeShape(board, bmShape);
}

bool WeichiBadMoveReader::isFillingOwnEyeShape ( const WeichiBoard& board, const WeichiMove& move )
{
	assertToFile( board.getGrid(move).isEmpty(), const_cast<WeichiBoard*>(&board) );

	// if move is not in any CA or there are >8 grids in own CA, this situation is not decided by this function
	const WeichiClosedArea* closedArea = board.getGrid(move.getPosition()).getClosedArea(move.getColor());
	if ( !closedArea || closedArea->getNumStone() > 8 ) { return false; }

	// check if there is only 1 block which surrounds CA (>2 blocks is too complicated)
	if ( closedArea->getNumBlock() > 1 ) { return false; }

	// block should not be LAD_LIFE
	const WeichiBlock* block = board.getBlock(closedArea->getBlockID(0));
	if ( block->getStatus() == LAD_LIFE ) { return false; }
	
	// if move can split into >2 CA, move is not bad
	const WeichiBitBoard& bmClosedArea = closedArea->getStoneMap();
	WeichiBitBoard bmShape = bmClosedArea;
	bmShape.SetBitOff(move.getPosition());
	if ( bmShape.getNumArea(2) > 1 ) { return false; }

	// if (CA - move.getPosition() - another empty grid) forms nakade shape, move is bad move
	Color oppColor = AgainstColor(move.getColor());
	WeichiBitBoard bmEmptyInShape = bmShape - board.getStoneBitBoard(oppColor);
	uint pos;
	while( (pos=bmEmptyInShape.bitScanForward()) != -1 ) {
		bmShape.SetBitOff(pos);
		if ( WeichiKnowledgeBase::isNakadeShape(board, bmShape, false) ) { return true; }
		bmShape.SetBitOn(pos);
	}

	return false;
}

bool WeichiBadMoveReader::isNotMakingNakadeInOppCA ( const WeichiBoard& board, const WeichiMove& move, const PredictPlayInfo& moveInfo )
{
	assertToFile( board.getGrid(move).isEmpty(), const_cast<WeichiBoard*>(&board) );

	// if move is not in any CA or there are >8 grids in opponent CA, this situation is not decided by this function
	const WeichiClosedArea* closedArea = board.getGrid(move.getPosition()).getClosedArea(AgainstColor(move.getColor()));
	if ( !closedArea || closedArea->getNumStone() > 8 ) { return false; }

	// check if there is only 1 block which surrounds CA (>2 blocks is too complicated)
	if ( closedArea->getNumBlock() > 1 ) { return false; }

	// if move kill opponent stones, move is not bad
	if ( moveInfo.m_bHasDeadBlocks ) { return false; }

	// if move is suicide move and blockNumStone is not CA->getNumStone()-1, the block will be killed and opponent is life, therefore move is bad
	if ( moveInfo.m_liberty == 1 && moveInfo.m_blockNumStone < closedArea->getNumStone()-1 ) { return true; }

	// if my liberty > opp liberty, move is not bad
	const WeichiBlock* oppSurroundedBlock = board.getBlock(closedArea->getBlockID(0));
	WeichiBitBoard bmOppLiberty = oppSurroundedBlock->getStonenNbrMap() - board.getBitBoard();
	bmOppLiberty.SetBitOff(move.getPosition());
	if ( moveInfo.m_liberty > bmOppLiberty.bitCount() ) { return false; }

	return !isMakingNakadeShape(board, move);
}

bool WeichiBadMoveReader::isTwoLibFalseEye( const WeichiBoard& board, const WeichiMove& move, const PredictPlayInfo& moveInfo )
{
	// some condition:
	//	1. after play must be two liberty
	//	2. should be false eye
	//  3. move is not in small opponent CA (move may make nakade shape)
	//	4. opponent can't put stone here
	//	5. after play have no way to run again or eat neighbor
	//	6. after play opponent have no way to kill it
	if( moveInfo.m_liberty!=2 ) { return false; }

	Color myColor = move.getColor();
	const WeichiGrid& grid = board.getGrid(move);
	if( !grid.getPattern().getFalseEye(myColor) ) { return false; }

	// move is not in opponent CA
	const WeichiClosedArea* closedArea = board.getGrid(move.getPosition()).getClosedArea(AgainstColor(move.getColor()));
	if ( closedArea && closedArea->getNumStone() <= 8 ) { return false; }

	Color oppColor = AgainstColor(myColor);
	const WeichiMove oppMove(oppColor,move.getPosition());
	if( board.checkSuicideRule(oppMove) ) { return false; }

	if( !(moveInfo.m_bmNewNbrStoneMap&board.getOneLibBlocksBitBoard(oppColor)).empty() ) { return false; }
	Vector<uint,2> vLibPos;
	WeichiBitBoard bmNewLiberty = moveInfo.m_bmNewLiberty;
	bmNewLiberty.bitScanAll(vLibPos);
	const WeichiMove runAgainMove1(myColor,vLibPos[0]);
	const WeichiMove runAgainMove2(myColor,vLibPos[1]);
	if( board.getLibertyAfterPlay(runAgainMove1)>2 || board.getLibertyAfterPlay(runAgainMove2)>2 ) { return false; }

	for( uint i=0; i<vLibPos.size(); i++ ) {
		const WeichiMove killMove(oppColor,vLibPos[i]);
		PredictPlayInfo killInfo;
		board.getPredictPlayInfoByPlay(killMove,killInfo);
		if( killInfo.m_liberty==0 || isBadSelfAtari(board,killMove,killInfo) ) { continue; }

		return true;
	}

	return false;
}

bool WeichiBadMoveReader::isSnapback( const WeichiBoard& board, const WeichiMove& move )
{
	/*
		1. scan neighbor, should have following property
			a. at least 1 own neighbor (all own block must be 1 liberty)
			b. just 1 opponent block which must be 1 liberty & 1 stone
			c. 0 empty grid
		2. set move pos bit off in neighbor block's nbrStoneMap and check is empty, if true is snapback
		3. 1 liberty opponent block should connect with own block
	*/
	uint opponentPos = -1;
	Color moveColor = move.getColor();
	const WeichiGrid& snapBackGrid = board.getGrid(move);
	const int *iNeighbor = snapBackGrid.getStaticGrid().getAdjacentNeighbors();
	WeichiBitBoard bmStoneMap;
	WeichiBitBoard bmStoneNbrMap;
	WeichiBitBoard bmCheckIndex;

	// check rule 1
	for( ; *iNeighbor!=-1; iNeighbor++ ) {
		const WeichiGrid& nbrGrid = board.getGrid(*iNeighbor);
		const WeichiBlock *nbrBlock = nbrGrid.getBlock();

		if( nbrGrid.isEmpty() ) {
			return false;
		} else if( nbrGrid.getColor()==moveColor ) {
			if( !bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) ) {
				if( nbrBlock->getLiberty()>1 ) { return false; }

				bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
				bmStoneMap |= nbrBlock->getStoneMap();
				bmStoneNbrMap |= nbrBlock->getStonenNbrMap();
			}
		} else {
			// must be opponent color
			assert( nbrGrid.getColor()==AgainstColor(moveColor) );
			if( nbrBlock->getLiberty()==1 ) {
				if( nbrBlock->getNumStone()==1 && opponentPos==-1 ) {
					opponentPos = *iNeighbor;
				} else {
					return false;
				}
			}
		}
	}
	if( opponentPos==-1 ) { return false; }

	// check rule 2
	bmStoneNbrMap -= board.getBitBoard();
	bmStoneNbrMap.SetBitOff(move.getPosition());

	if( bmStoneNbrMap.empty() ) {
		// check rule 3
		const WeichiGrid& opponentGrid = board.getGrid(opponentPos);
		bmStoneMap &= opponentGrid.getStaticGrid().getStoneNbrsMap();
		if( !bmStoneMap.empty() ) {
			// just for debug use
			assertToFile( invariance_snapback(board,move), const_cast<WeichiBoard*>(&board) );
			return true;
		}
	}

	return false;
}

bool WeichiBadMoveReader::isSharedLibertyWithNbrBlock( const WeichiBoard& board, const PredictPlayInfo& info )
{
	uint pos;
	WeichiBitBoard bmNewNbrStoneMap = info.m_bmNewNbrStoneMap;
	while( (pos=bmNewNbrStoneMap.bitScanForward())!=-1 ) {
		const WeichiBlock* nbrBlock = board.getGrid(pos).getBlock();

		if( !info.m_bmNewLiberty.hasIntersection(nbrBlock->getLibertyBitBoard(board.getBitBoard())) ) { return false; }
		bmNewNbrStoneMap -= nbrBlock->getStoneMap();
	}

	return true;
}

bool WeichiBadMoveReader::invariance_snapback( const WeichiBoard& board, const WeichiMove& move )
{
	WeichiBitBoard bmStoneAfterPlay;
	board.getLibertyBitBoardAfterPlay(move,bmStoneAfterPlay);
	assertToFile( bmStoneAfterPlay.bitCount()==1, const_cast<WeichiBoard*>(&board) );

	Color moveColor = move.getColor();
	uint libertyPos = move.getPosition();
	uint snapbackPos = bmStoneAfterPlay.bitScanForward();
	const WeichiGrid& libertyGrid = board.getGrid(libertyPos);
	const WeichiGrid& snapBackGrid = board.getGrid(snapbackPos);
	const int *iNeighbor = libertyGrid.getStaticGrid().getAdjacentNeighbors();

	for( ; *iNeighbor!=-1; iNeighbor++ ) {
		const WeichiGrid& nbrGrid = board.getGrid(*iNeighbor);
		if( nbrGrid.getColor()==moveColor && nbrGrid.getBlock()->getLiberty()==1 ) {
			assertToFile( nbrGrid.getBlock()->getLastLiberty(board.getBitBoard())==libertyPos, const_cast<WeichiBoard*>(&board) );
			if( snapBackGrid.getBlock()->isNeighbor(nbrGrid.getBlock()) ) {
				return true;
			}
		}
	}

	assertToFile( false, const_cast<WeichiBoard*>(&board) );
	return false;
}