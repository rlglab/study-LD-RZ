#include "WeichiKnowledgeBase.h"

bool WeichiKnowledgeBase::isKo( const WeichiBoard& board, const WeichiMove& move, Color koColor )
{
	assertToFile( board.getGrid(move).isEmpty(), const_cast<WeichiBoard*>(&board) );

	const WeichiGrid& grid = board.getGrid(move);
	if( !grid.getPattern().getFalseEye(koColor) ) { return false; }

	bool bIsKo = false;
	for( const int* iNbr=grid.getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiBlock* nbrBlock = board.getGrid(*iNbr).getBlock();
		assertToFile( nbrBlock && nbrBlock->getColor()==koColor, const_cast<WeichiBoard*>(&board) );

		if( nbrBlock->getLiberty()!=1 ) { continue; }

		// must only have one 1-liberty block
		if( bIsKo ) { return false; }
		else { bIsKo = true; }
	}

	return bIsKo;
}

bool WeichiKnowledgeBase::isMakeKo( const WeichiBoard& board, const WeichiMove& move )
{
	assertToFile( board.getGrid(move).isEmpty(), const_cast<WeichiBoard*>(&board) );

	const WeichiGrid& grid = board.getGrid(move);
	const uint makeKoDir = grid.getPattern().getMakeKoCheckDir(move.getColor());
	if( !makeKoDir ) { return false; }

	const WeichiGrid& checkGrid = board.getGrid(grid,board.realDirectionDIR4of(makeKoDir));
	const Color oppColor = AgainstColor(move.getColor());
	if( !checkGrid.getPattern().getMakeKoCheckDir(oppColor) ) { return false; }

	// check this ko will not eat any neighbor
	for( const int* iNbr=grid.getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& nbrGrid = board.getGrid(*iNbr);
		if( nbrGrid.isEmpty() ) { continue; }
		assertToFile( nbrGrid.getColor()==oppColor, const_cast<WeichiBoard*>(&board) );

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if( nbrBlock->getLiberty()==1 ) { return false; }
	}

	return true;
}

bool WeichiKnowledgeBase::isSemeaiBlock( const WeichiBoard& board, const WeichiBlock* block )
{
	const Color ownColor = block->getColor();
	//const Color oppColor = AgainstColor(ownColor);

	uint pos;
	WeichiBitBoard bmLiberty = block->getLibertyBitBoard(board.getBitBoard());
	while( (pos=bmLiberty.bitScanForward())!=-1 ) {
		WeichiMove move(ownColor,pos);
		if( board.getLibertyAfterPlay(move)>block->getLiberty() ) { return false; }
	}

	return true;
	/*if( (block->getStonenNbrMap()&board.getStoneBitBoard(oppColor)).empty() ) { return false; }

	WeichiBitBoard bmLiberty1 = block->getLibertyBitBoard(board.getBitBoard());
	cerr << bmLiberty1.toString() << endl;

	WeichiBitBoard bmLiberty1Dilate = (bmLiberty1.dilate()-block->getStonenNbrMap()-board.getStoneBitBoard(oppColor)) & StaticBoard::getMaskBorder();
	cerr << bmLiberty1Dilate.toString() << endl;

	return bmLiberty1Dilate.empty();*/
}

bool WeichiKnowledgeBase::isNakadeShape( const WeichiBoard& board, const WeichiBitBoard& bmShape, bool bCheckTwoLine/*=true*/ )
{
	uint numStone = bmShape.bitCount();
	if ( numStone == 0 ) { return false; }

	// nakade stone num > 6 is impossible
	if ( numStone > 6 ) { return false; }

	// check if bmShape is connected
	WeichiBitBoard bmShapeCopy = bmShape;
	if ( bmShapeCopy.getNumArea(2) > 1 ) { return false; }

	// nakade stone num <= 3 is definitely nakade shape
	if ( numStone <= 3 ) { return true; }

	// initialize bounding box
	BoundingBox shapeBoundingBox;
	bmShapeCopy = bmShape;
	uint pos;
	while( (pos=bmShapeCopy.bitScanForward())!=-1 ) {
		shapeBoundingBox.combine(pos);
	}

	// bounding box will not be larger than 3
	if( (shapeBoundingBox.getUpBound()-shapeBoundingBox.getDownBound()) > 2 ) { return false; }
	if( (shapeBoundingBox.getRightBound()-shapeBoundingBox.getLeftBound()) > 2 ) { return false; }

	// initialize shape pattern index
	const WeichiGrid& centerGrid = board.getGrid(shapeBoundingBox.getCenterPosition());
	uint shapePatternIndex = 0;
	Color simulationColor = COLOR_BLACK;
	bmShapeCopy = bmShape;
	while( (pos=bmShapeCopy.bitScanForward())!=-1 ) {
		const WeichiGrid& putStoneGrid = board.getGrid(pos);
		const int* indexDiff = putStoneGrid.getStaticGrid().getPatternIndexDifference(simulationColor) ;
		const int* nbr33 = putStoneGrid.getStaticGrid().get3x3Neighbors();		
		const int* rel33 = putStoneGrid.getStaticGrid().get3x3Relations();
		for( ; *nbr33!=-1 ; nbr33++, rel33++  ) {
			if( *nbr33 == centerGrid.getPosition() ) {
				shapePatternIndex += indexDiff[*rel33];
				break;
			}
		}
	}

	// use pattern to check whether bmShape is nakade shape or not
	const pattern33::Pattern33& pattern33 = StaticBoard::getPattern33(shapePatternIndex);
	bool bIsNakadePattern = false;
	Color centerColor = COLOR_NONE;
	if ( bmShape.BitIsOn(centerGrid.getPosition()) ) { centerColor = simulationColor; }
	bIsNakadePattern = (bIsNakadePattern || pattern33.getNakade(simulationColor, centerColor));

	// for SZ shape nakade case
	bool bIsSZNakadePattern = pattern33.getSZNakade(simulationColor, centerColor) > 0;
	if ( bIsSZNakadePattern ) {
		// bmShape's liberty must >= 3
		WeichiBitBoard bmShapeLiberty = bmShape.dilate() - bmShape - board.getBitBoard();
		bIsNakadePattern = (bIsNakadePattern || bmShapeLiberty.bitCount() >= 3);
	}

	// for two line nakade case
	if( bCheckTwoLine && centerGrid.getStaticGrid().getXLineNo()==2 && centerGrid.getStaticGrid().getYLineNo()==2 ) {
		uint line2NakadePatternDir = pattern33.get2LineNakade(simulationColor, centerColor);
		bIsNakadePattern = (bIsNakadePattern || isTwoLineNakadeShape(board, centerGrid, line2NakadePatternDir));
	}

	return bIsNakadePattern;
}

bool WeichiKnowledgeBase::isTwoLineNakadeShape( const WeichiBoard& board, const WeichiGrid& centerGrid, const uint line2NakadePatternDir )
{
	if( line2NakadePatternDir==0 ) { return false; }

	const Vector<uint,8>& vWallPos = StaticBoard::getPattern3x3Direction(line2NakadePatternDir);
	assertToFile( vWallPos.size()==2, const_cast<WeichiBoard*>(&board) );
	for( uint i=0; i<vWallPos.size(); i++ ) {
		const WeichiGrid& wallGrid = board.getGrid(centerGrid,vWallPos[i]);
		if( wallGrid.getStaticGrid().getLineNo()!=1 ) { return false; }
	}

	return true;
}

bool WeichiKnowledgeBase::isBadMoveForKillSuicide( const WeichiMove& move, const WeichiBlock* suicideBlock ) const
{
	// see for reason why avoid some kill suicide
	const WeichiGrid& grid = m_board.getGrid(move);
	const WeichiClosedArea* closedArea = grid.getClosedArea(move.getColor());

	if( !closedArea || closedArea->getNumStone()>2 ) { return false; }
	if( suicideBlock->getNumStone()!=1 ) { return false; }

	// all closed area's block should be >1 liberty & put stone should near combine all block
	for( uint iNumBlock=0; iNumBlock<closedArea->getNumBlock(); iNumBlock++ ) {
		const WeichiBlock* nbrBlock = m_board.getBlock(closedArea->getBlockID(iNumBlock));
		if( nbrBlock->getLiberty()==1 ) { return false; }
		if( !nbrBlock->getStonenNbrMap().BitIsOn(move.getPosition()) ) { return false; }
	}

	PredictPlayInfo moveInfo;
	m_board.getPredictPlayInfoByPlay(move,moveInfo);
	if( moveInfo.m_liberty!=2 ) { return false; }

	moveInfo.m_bmNewLiberty.SetBitOff(suicideBlock->getiFirstGrid());
	const uint outerLibPos = moveInfo.m_bmNewLiberty.bitScanForward();
	WeichiMove oppAtariMove(suicideBlock->getColor(),outerLibPos);

	return (m_board.getLibertyAfterPlay(oppAtariMove)>1);
}

void WeichiKnowledgeBase::save1Lib( const WeichiBlock* block, WeichiBitBoard& bmSave, WeichiBitBoard& bmUnknown ) const
{
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&m_board) );

	Color myColor = block->getColor();
	const uint runPos = block->getLastLiberty(m_board.getBitBoard());
	const WeichiGrid& runGrid = m_board.getGrid(runPos);

	bool bCanKillNbr1Lib = killNeighbor1Lib(block,bmSave);

	if( (runGrid.getLiberty()>=3) || (runGrid.getLiberty()==2 && bCanKillNbr1Lib) ) {
		bmSave.SetBitOn(runPos);
	} else {
		const WeichiMove runMove(myColor,runPos);
		WeichiBitBoard bmRunLib;
		uint runLib = m_board.getLibertyBitBoardAndLibertyAfterPlay(runMove,bmRunLib);

		if( runLib>2 ) {
			bmSave.SetBitOn(runPos);
		} else if( runLib==2 ) {
			Vector<uint,2> vLibPos;
			bmRunLib.bitScanAll(vLibPos);

			bool bIsUnknown = false;
			Color oppColor = AgainstColor(myColor);
			for( uint i=0; i<vLibPos.size(); i++ ) {
				const WeichiMove killMove(oppColor,vLibPos[i]);
				if( !m_board.getGrid(killMove).isEmpty() ) { bIsUnknown = true; continue; }
				WeichiBitBoard bmKillLib;
				m_board.getLibertyBitBoardAfterPlay(killMove,bmKillLib);
				bmKillLib.SetBitOff(runPos);
				if( bmKillLib.bitCount()<=1 ) { continue; }

				const WeichiMove runAgainMove(myColor,vLibPos[1-i]);
				if( !m_board.getGrid(runAgainMove).isEmpty() ) { bIsUnknown = true; continue; }
				WeichiBitBoard bmRunAgainLib;
				m_board.getLibertyBitBoardAfterPlay(runAgainMove,bmRunAgainLib);
				bmRunAgainLib.SetBitOff(runPos);
				bmRunAgainLib.SetBitOff(vLibPos[i]);
				uint runAgainLib = bmRunAgainLib.bitCount();
				if( runAgainLib<2 ) { return; }
				else if( runAgainLib>2 ) { continue; }
				else { bIsUnknown = true; }
			}

			if( bIsUnknown ) { bmUnknown.SetBitOn(runPos); }
			else { bmSave.SetBitOn(runPos); }
		} else {
			// TODO: replace suicide move??
		}
	}
}

void WeichiKnowledgeBase::save2Lib( const WeichiBlock* block, WeichiBitBoard& bmSave, WeichiBitBoard& bmUnknown ) const
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

	Color myColor = block->getColor();
	bool bCanKillNbr1Lib = killNeighbor1Lib(block,bmSave);
	bool bCanKillNbr2Lib = killNeighbor2Lib(block,bmSave,bmUnknown);

	Vector<uint,2> vLibPos;
	block->getLibertyPositions(m_board.getBitBoard(),vLibPos);

	const WeichiMove move1(myColor,vLibPos[0]);
	const WeichiMove move2(myColor,vLibPos[1]);

	if( bCanKillNbr1Lib || bCanKillNbr2Lib ) {
		if( m_board.getLibertyAfterPlay(move1)>2 ) { bmSave.SetBitOn(move1.getPosition()); }
		if( m_board.getLibertyAfterPlay(move2)>2 ) { bmSave.SetBitOn(move2.getPosition()); }
	} else {
		WeichiBitBoard bmLibAfterPlay1,bmLibAfterPlay2;
		uint libAfterPlay1 = m_board.getLibertyBitBoardAndLibertyAfterPlay(move1,bmLibAfterPlay1);
		uint libAfterPlay2 = m_board.getLibertyBitBoardAndLibertyAfterPlay(move2,bmLibAfterPlay2);

		if( libAfterPlay1>2 ) { bmSave.SetBitOn(move1.getPosition()); }
		if( libAfterPlay2>2 ) { bmSave.SetBitOn(move2.getPosition()); }

		if( libAfterPlay1<=2 && libAfterPlay2<=2 ) {
			if( libAfterPlay1==2 ) {
				const WeichiGrid& grid1 = m_board.getGrid(move1);
				if( grid1.getPattern().getFalseEye(myColor) ) {
					bmLibAfterPlay1.SetBitOff(move2.getPosition());
					uint newLib = bmLibAfterPlay1.bitScanForward();
					const WeichiMove newMove(myColor,newLib);
					if( m_board.getLibertyAfterPlay(newMove)>2 ) { bmSave.SetBitOn(newLib); }
				} else {
					bmUnknown.SetBitOn(move1.getPosition());
				}
			}
			if( libAfterPlay2==2 ) {
				const WeichiGrid& grid2 = m_board.getGrid(move2);
				if( grid2.getPattern().getFalseEye(myColor) ) {
					bmLibAfterPlay2.SetBitOff(move1.getPosition());
					uint newLib = bmLibAfterPlay2.bitScanForward();
					const WeichiMove newMove(myColor,newLib);
					if( m_board.getLibertyAfterPlay(newMove)>2 ) { bmSave.SetBitOn(newLib); }
				} else {
					bmUnknown.SetBitOn(move2.getPosition());
				}
			}
		}
	}
}

bool WeichiKnowledgeBase::kill1Lib( const WeichiBlock* block, WeichiBitBoard& bmKill ) const
{
	// check isIllegal move or snapback
	// TODO: maybe can use pattern to fast kill?
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&m_board) );

	uint kill_pos = block->getLastLiberty(m_board.getBitBoard());
	WeichiMove killMove(AgainstColor(block->getColor()),kill_pos);
	if( isSnapback(killMove) || m_board.isIllegalMove(killMove, m_ht) ) { return false; }

	bmKill.SetBitOn(kill_pos);
	return true;
}

bool WeichiKnowledgeBase::kill2Lib( const WeichiBlock* block, WeichiBitBoard& bmKill, WeichiBitBoard& bmUnknow ) const
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );
	
	if( block->getStatus()==LAD_LIFE ) { return false; }

	WeichiBitBoard bmNbr1Lib;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmNbr1Lib);
	if( !bmNbr1Lib.empty() && !isBitBoardInOwnCA(m_board,block,bmNbr1Lib) ) { return false; }

	WeichiBitBoard bmLiberty = block->getLibertyBitBoard(m_board.getBitBoard());
	if( isBitBoardInOwnCA(m_board,block,bmLiberty) ) {
		bmKill |= bmLiberty;
		return true;
	} else {
		bool bCanKill = false;
		Vector<uint,2> vLibPos;
		bmLiberty.bitScanAll(vLibPos);

		if( kill2LibDirectly(block,vLibPos[0],vLibPos[1],bmKill,bmUnknow) ) { bCanKill = true; }
		if( kill2LibDirectly(block,vLibPos[1],vLibPos[0],bmKill,bmUnknow) ) { bCanKill = true; }
		if( kill2LibByCloseDoor(block,vLibPos[0],vLibPos[1],bmKill) ) { bCanKill = true; }

		return bCanKill;
	}
}

bool WeichiKnowledgeBase::kill2LibDirectly( const WeichiBlock* block, uint killPos, uint runPos, WeichiBitBoard& bmKill, WeichiBitBoard& bmUnknow ) const
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );
	assertToFile( block->getStatus()!=LAD_LIFE, const_cast<WeichiBoard*>(&m_board) );

	Color oppColor = block->getColor();
	Color myColor = AgainstColor(oppColor);
	
	const WeichiMove killMove(myColor,killPos);
	if( m_board.getLibertyAfterPlay(killMove)==1 ) { return false; }

	const WeichiMove runMove(oppColor,runPos);
	WeichiBitBoard bmRunLib;
	m_board.getLibertyBitBoardAfterPlay(runMove,bmRunLib);
	bmRunLib.SetBitOff(killPos);
	uint runLib = bmRunLib.bitCount();

	if( runLib<=1 ) {
		bmKill.SetBitOn(killPos);
		return true;
	} else if( runLib==2 ) {
		bmUnknow.SetBitOn(killPos);
		return false;
	}

	return false;
}

bool WeichiKnowledgeBase::kill2LibByCloseDoor( const WeichiBlock* block, uint pos1, uint pos2, WeichiBitBoard& bmKill ) const
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );
	assertToFile( block->getStatus()!=LAD_LIFE, const_cast<WeichiBoard*>(&m_board) );

	Color myColor = AgainstColor(block->getColor());
	const WeichiGrid& grid1 = m_board.getGrid(pos1);
	const WeichiGrid& grid2 = m_board.getGrid(pos2);
	int dir1 = grid1.getPattern().getCloseDoor(myColor);
	int dir2 = grid2.getPattern().getCloseDoor(myColor);

	if( dir1==0 || dir2==0 ) { return false; }

	const WeichiBlock* dirBlock = m_board.getGrid(grid1,m_board.realDirectionDIR4of(dir1)).getBlock();
	assertToFile( dirBlock && dirBlock->getColor()==myColor, const_cast<WeichiBoard*>(&m_board) );
	if( dirBlock->getLiberty()<=2 ) { return false; }

	dirBlock = m_board.getGrid(grid2,m_board.realDirectionDIR4of(dir2)).getBlock();
	assertToFile( dirBlock && dirBlock->getColor()==myColor, const_cast<WeichiBoard*>(&m_board) );
	if( dirBlock->getLiberty()<=2 ) { return false; }

	uint kill_pos = grid2.getStaticGrid().getNeighbor(StaticBoard::getRevDirect(m_board.realDirectionDIR4of(dir2)));
	assertToFile( m_board.getGrid(kill_pos).isEmpty(), const_cast<WeichiBoard*>(&m_board) );
	bmKill.SetBitOn(kill_pos);

	return true;
}

bool WeichiKnowledgeBase::killNeighbor1Lib( const WeichiBlock* block, WeichiBitBoard& bmKill ) const
{
	uint pos;
	bool bCanKill = false;
	WeichiBitBoard bmNbr1LibBlock;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmNbr1LibBlock);
	while( (pos=bmNbr1LibBlock.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmNbr1LibBlock -= oneLibBlock->getStoneMap();

		if( kill1Lib(oneLibBlock,bmKill) ) { bCanKill = true; }
	}

	return bCanKill;
}

bool WeichiKnowledgeBase::killNeighbor2Lib( const WeichiBlock* block, WeichiBitBoard& bmKill, WeichiBitBoard& bmUnknown ) const
{
	uint pos;
	bool bCanKill = false;
	Vector<uint,2> vLibPos;
	WeichiBitBoard bmNbr2LibBlock;
	m_board.findNeighborTwoLibBlocksBitBoard(block,bmNbr2LibBlock);
	while( (pos=bmNbr2LibBlock.bitScanForward())!=-1 ) {
		const WeichiBlock* twoLibBlock = m_board.getGrid(pos).getBlock();
		bmNbr2LibBlock -= twoLibBlock->getStoneMap();

		if( twoLibBlock->getStatus()==LAD_LIFE ) { continue; }

		WeichiBitBoard bmLiberty = twoLibBlock->getLibertyBitBoard(m_board.getBitBoard());
		if( isBitBoardInOwnCA(m_board,twoLibBlock,bmLiberty) ) {
			bmKill |= bmLiberty;
			bCanKill = true;
		} else {
			bmLiberty.bitScanAll(vLibPos);
			if( kill2LibDirectly(twoLibBlock,vLibPos[0],vLibPos[1],bmKill,bmUnknown) ) { bCanKill = true; }
			if( kill2LibDirectly(twoLibBlock,vLibPos[1],vLibPos[0],bmKill,bmUnknown) ) { bCanKill = true; }
		}
	}

	return bCanKill;
}

bool WeichiKnowledgeBase::isBitBoardInOwnCA( const WeichiBoard& board, const WeichiBlock* block, const WeichiBitBoard& bmLiberty ) const
{
	if( bmLiberty.empty() ) { return false; }

	for( uint iNumCA=0; iNumCA<block->getNumClosedArea(); iNumCA++ ) {
		const WeichiClosedArea* closedArea = block->getClosedArea(iNumCA,m_board.getCloseArea());
		if( closedArea->getNumBlock()>1 ) { continue; }

		if( (closedArea->getStoneMap()-board.getBitBoard()-bmLiberty).empty() ) { return true; }
	}

	return false;
}

bool WeichiKnowledgeBase::isSnapback( const WeichiMove& move ) const
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
	const WeichiGrid& snapBackGrid = m_board.getGrid(move);
	const int *iNeighbor = snapBackGrid.getStaticGrid().getAdjacentNeighbors();
	WeichiBitBoard bmStoneMap;
	WeichiBitBoard bmStoneNbrMap;
	WeichiBitBoard bmCheckIndex;

	// check rule 1
	for( ; *iNeighbor!=-1; iNeighbor++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNeighbor);
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
	bmStoneNbrMap -= m_board.getBitBoard();
	bmStoneNbrMap.SetBitOff(move.getPosition());

	if( bmStoneNbrMap.empty() ) {
		// check rule 3
		const WeichiGrid& opponentGrid = m_board.getGrid(opponentPos);
		bmStoneMap &= opponentGrid.getStaticGrid().getStoneNbrsMap();
		if( !bmStoneMap.empty() ) {
			// just for debug use
			assert( invariance_snapback(move) );
			return true;
		}
	}

	return false;
}

bool WeichiKnowledgeBase::invariance_snapback( WeichiMove move ) const
{
	WeichiBitBoard bmStoneAfterPlay;
	m_board.getLibertyBitBoardAfterPlay(move,bmStoneAfterPlay);
	assertToFile( bmStoneAfterPlay.bitCount()==1, const_cast<WeichiBoard*>(&m_board) );

	Color moveColor = move.getColor();
	uint libertyPos = move.getPosition();
	uint snapbackPos = bmStoneAfterPlay.bitScanForward();
	const WeichiGrid& libertyGrid = m_board.getGrid(libertyPos);
	const WeichiGrid& snapBackGrid = m_board.getGrid(snapbackPos);
	const int *iNeighbor = libertyGrid.getStaticGrid().getAdjacentNeighbors();

	for( ; *iNeighbor!=-1; iNeighbor++ ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNeighbor);
		if( nbrGrid.getColor()==moveColor && nbrGrid.getBlock()->getLiberty()==1 ) {
			assertToFile( nbrGrid.getBlock()->getLastLiberty(m_board.getBitBoard())==libertyPos, const_cast<WeichiBoard*>(&m_board) );
			if( snapBackGrid.getBlock()->isNeighbor(nbrGrid.getBlock()) ) {
				return true;
			}
		}
	}

	assertToFile( false, const_cast<WeichiBoard*>(&m_board) );
	return false;
}