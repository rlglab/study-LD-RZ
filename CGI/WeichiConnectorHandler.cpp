#include "WeichiConnectorHandler.h"

/*
	*** Author: rockmanray ***
	Connector definition:
	* Strong Connector: A connector of color C can connect the start block and the target(block or wall) in one move even when it is GOTE for C.
	* Weak Connector: A connector of color C can connect the start block and the target(block or wall) in one move only when it is SENTE for C.
*/

void WeichiConnectorHandler::resetConnectors()
{
	m_connectors.reset();
	for ( uint iBlockConnector = 0 ; iBlockConnector < MAX_NUM_GRIDS ; ++iBlockConnector ) { 
		m_blockConnectors[iBlockConnector].clear(); 
	}

	return ;
}

void WeichiConnectorHandler::findFullBoardConnectors( Color findColor )
{
	resetConnectors();	

	WeichiBitBoard bmEmptyCandidate = getConnectorEmptyCandidates(findColor);		
	WeichiBitBoard bmCheckBamboo = findFullBoardBamboo(bmEmptyCandidate, findColor);
	WeichiBitBoard bmCheckHalfBamboo = findFullBoardHalfBamboo(bmEmptyCandidate, findColor);
	WeichiBitBoard bmCheckedWallBamboo = findFullBoardWallBamboo(bmEmptyCandidate, findColor); 
	WeichiBitBoard bmCheckedFalseEye = findFullBoardFalseEye(bmEmptyCandidate, findColor);	
	WeichiBitBoard bmCheckedTigerMouth = findFullBoardTigerMouth(bmEmptyCandidate, findColor);		
	WeichiBitBoard bmCheckedDiagonal = findFullBoardDiagonal(bmEmptyCandidate-bmCheckedFalseEye-bmCheckedTigerMouth, findColor);
	WeichiBitBoard bmCheckedTigerMouthOneLibDeadBlock = findFullBoardTigerMouthOneLibDeadBlock(findColor);	
	WeichiBitBoard bmCheckedJump = findFullBoardJump(bmEmptyCandidate, findColor);
	WeichiBitBoard bmCheckedWallJump = findFullBoardWallJump(bmEmptyCandidate, findColor);
	WeichiBitBoard bmCheckedBan = findFullBoardBan(bmEmptyCandidate, findColor);

	return;
}

string WeichiConnectorHandler::getConnectorThreatDrawingString() 
{
	WeichiBitBoard bmAllThreats ;
	for( uint iConn=0 ; iConn < m_connectors.getCapacity() ; ++iConn ) {
		if( !m_connectors.isValidIdx(iConn) ) { continue ; }
		const WeichiConnector* conn = m_connectors.getAt(iConn) ;	
		if( conn->isStrong() && !WeichiConfigure::DrawStrong ) { continue ; }
		if( conn->isWeak() && !WeichiConfigure::DrawWeak ) { continue ; }

		CERR() << conn->toTypeString()  ;
		WeichiBitBoard bmThreat = conn->getThreatBitmap() ;
		bmAllThreats |= bmThreat ;
		int pos = 0 ;		
		CERR() << " at " ;
		while( (pos=bmThreat.bitScanForward()) != -1 ) { CERR() << WeichiMove(COLOR_NONE, pos).toGtpString() << " "  ; }		
		CERR() << endl ;
	}

	if( !WeichiConfigure::DrawConnector ) { bmAllThreats.Reset() ; }

	ostringstream oss ;
	oss << endl ;
    for ( StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it ) {
        if ( bmAllThreats.BitIsOn(*it) ) { oss << "#FF0000 " ; }
        else { oss << "\"\" "; }

        if (*it%MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

	return oss.str() ;
}

string WeichiConnectorHandler::getConnectorConnDrawingString()
{
	WeichiBitBoard bmAllConns ;
	for( uint iConn=0 ; iConn < m_connectors.getCapacity() ; ++iConn ) {
		if( !m_connectors.isValidIdx(iConn) ) { continue ; }
		const WeichiConnector* conn = m_connectors.getAt(iConn) ;	
		if( conn->isStrong() && !WeichiConfigure::DrawStrong ) { continue ; }
		if( conn->isWeak() && !WeichiConfigure::DrawWeak ) { continue ; }

		CERR() << conn->toTypeString()  ;
		WeichiBitBoard bmConn = conn->getConnectorBitmap() ;
		bmAllConns |= bmConn ;
		int pos = 0 ;		
		CERR() << " at " ;
		while( (pos=bmConn.bitScanForward()) != -1 ) { CERR() << WeichiMove(COLOR_NONE, pos).toGtpString() << " "  ; }		
		CERR() << endl ;
	}

	if( !WeichiConfigure::DrawConnector ) { bmAllConns.Reset() ; }

	ostringstream oss ;
	oss << endl ;
    for ( StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it ) {
        if ( bmAllConns.BitIsOn(*it) ) { oss << "#00FF00 " ; }
        else { oss << "\"\" "; }

        if (*it%MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

	return oss.str() ;
}

WeichiBitBoard WeichiConnectorHandler::getConnectorEmptyCandidates(Color findColor) 
{
	WeichiBitBoard bmBlockNbr;	

	FeatureList<WeichiBlock,MAX_GAME_LENGTH>& blockList = m_board.getBlockList();	
	for (int iBlock = 0; iBlock < blockList.getCapacity(); ++iBlock) {
		if (!isInterestedConnectorBlock(findColor, iBlock)) { continue; }

		WeichiBlock* block = blockList.getAt(iBlock);
		bmBlockNbr |= block->getNbrMap();
	}	

	return (bmBlockNbr - m_board.getBitBoard()); 
}

WeichiBitBoard WeichiConnectorHandler::findFullBoardBamboo(WeichiBitBoard bmCandidate, Color findColor)
{
	Color oppColor = AgainstColor(findColor) ;
	WeichiBitBoard bmFindPos = bmCandidate ;
	WeichiBitBoard bmBamboo ;
	int startPos = 0;
	while ( (startPos=bmFindPos.bitScanForward()) != -1 ) {
		const WeichiGrid& startGrid = m_board.getGrid(startPos);
		assert( startGrid.getColor() == COLOR_NONE );
		if( !isBambooPattern(findColor, startGrid) ) { continue ; }

		Vector<short, 4> vBlocksIds;
		int endPos ;
		getBambooAttrs(findColor, startGrid, endPos, vBlocksIds) ;		

		WeichiConnector* connector = m_connectors.NewOne();
		setBambooConnectorAttribute(connector, findColor, startPos, endPos, vBlocksIds) ;

		bmFindPos.SetBitOff(endPos) ;
		bmBamboo.SetBitOn(startPos) ;
		bmBamboo.SetBitOn(endPos) ;
	}

	return bmBamboo;
}

WeichiBitBoard WeichiConnectorHandler::findFullBoardHalfBamboo(WeichiBitBoard bmCandidate, Color findColor)
{
	Color oppColor = AgainstColor(findColor) ;
	WeichiBitBoard bmFindPos = bmCandidate ;
	WeichiBitBoard bmHalfBamboo ;
	int startPos = 0;
	while ( (startPos=bmFindPos.bitScanForward()) != -1 ) {
		const WeichiGrid& startGrid = m_board.getGrid(startPos);
		assert( startGrid.getColor() == COLOR_NONE );
		if( !isHalfBambooPattern(findColor, startGrid) ) { continue ; }		

		Vector<short, 4> vBlocksIds;
		int endPos ;
		getHalfBambooAttrs(findColor, startGrid, endPos, vBlocksIds) ;		

		WeichiConnector* connector = m_connectors.NewOne();
		setHalfBambooConnectorAttribute(connector, findColor, startPos, endPos, vBlocksIds) ;

		bmFindPos.SetBitOff(endPos) ;
		bmHalfBamboo.SetBitOn(startPos) ;
		bmHalfBamboo.SetBitOn(endPos) ;
	}

	return bmHalfBamboo;
}

WeichiBitBoard WeichiConnectorHandler::findFullBoardWallBamboo( WeichiBitBoard bmCandidate, Color findColor )
{	
	debugConnector("WalBamboo Candidate:", bmCandidate) ;

	Color oppColor = AgainstColor(findColor) ;
	WeichiBitBoard bmFindPos = bmCandidate ;
	WeichiBitBoard bmWallBamboo ;
	int startPos = 0;
	while ( (startPos=bmFindPos.bitScanForward()) != -1 ) {
		const WeichiGrid& startGrid = m_board.getGrid(startPos);
		assert( startGrid.getColor() == COLOR_NONE );
		if( !isWallBambooPattern(findColor, startGrid) ) { continue ; }
		
		int endDir, endPos ;
		getWallBambooAttrs(findColor, startGrid, endPos) ;		

		Vector<short, 4> vBlocksIds;
		collectNbrBlocks(findColor, startGrid, vBlocksIds) ;

		WeichiConnector* connector = m_connectors.NewOne();
		setWallBambooConnectorAttribute(connector, findColor, startPos, endPos, vBlocksIds) ;

		bmFindPos.SetBitOff(endPos) ;
		bmWallBamboo.SetBitOn(startPos) ;
		bmWallBamboo.SetBitOn(endPos) ;
	}

	debugConnector("WallBamboo:", bmWallBamboo) ;	

	return bmWallBamboo ;
}

WeichiBitBoard WeichiConnectorHandler::findFullBoardFalseEye( WeichiBitBoard bmCandidate, Color findColor ) 
{	
	debugConnector("FalseEye Candidate:", bmCandidate) ;	

	Color oppColor = AgainstColor(findColor) ;
	WeichiBitBoard bmFindPos = bmCandidate ;
	WeichiBitBoard bmCheckedByFalseEye;
	int startPos = 0 ;
	while ( (startPos=bmFindPos.bitScanForward()) != -1 ) {
		const WeichiGrid& findGrid = m_board.getGrid(startPos) ;
		assert( findGrid.getColor() == COLOR_NONE ) ;
		WeichiMove invadeMove(oppColor, startPos) ;
		Vector<short, 4> vBlocksIds;
		if ( !isFalseEyeConnector(invadeMove, vBlocksIds) ) { continue ; }

		WeichiConnector* connector = m_connectors.NewOne() ;
		setConnectorAttribute(TYPE_FALSE_EYE, connector, findColor, startPos, startPos, vBlocksIds) ;
		overrideDiagonalCheckBitmap(findGrid, findColor, bmCheckedByFalseEye) ;
		bmCheckedByFalseEye.SetBitOn(startPos) ;
	}
	
	debugConnector("FalseEye:", bmCheckedByFalseEye) ;	

	return bmCheckedByFalseEye ;
}

WeichiBitBoard WeichiConnectorHandler::findFullBoardTigerMouth( WeichiBitBoard bmCandidate, Color findColor ) 
{	
	debugConnector("TigerMouth Candidate:", bmCandidate) ;	

	Color oppColor = AgainstColor(findColor) ;
	WeichiBitBoard bmFindPos = bmCandidate ;
	WeichiBitBoard bmCheckedTigerMouth ;
	int startPos = 0 ;
	while ( (startPos = bmFindPos.bitScanForward()) != -1 ) {
		const WeichiGrid& findGrid = m_board.getGrid(startPos);
		assert(findGrid.getColor() == COLOR_NONE);
		WeichiMove invadeMove(oppColor, startPos);
		Vector<short, 4> vBlocksIds;
		// for TigerMouth		
		if (isTigerMouthConnecotr(invadeMove, vBlocksIds))
		{			
			WeichiConnector* connector = m_connectors.NewOne();
			int lastLib = m_board.getLibertyAfterPlay(invadeMove)==1 ? m_board.getLastLibertyPositionAfterPlay(invadeMove) : startPos ;			
			//setConnectorAttribute(TYPE_TIGERMOUTH, connector, findColor, startPos, lastLib, vBlocksIds) ;
			setTigerMouthConnectorAttribute(connector, findColor,startPos, lastLib, vBlocksIds) ;
			overrideDiagonalCheckBitmap(findGrid, findColor, bmCheckedTigerMouth) ;
			bmCheckedTigerMouth.SetBitOn(startPos) ;
		}
		else if ( vBlocksIds.size() == 3 ) {
			// Not tiger mouth, and #nbrblock =3, then the two diagonals can not connect at the same time.
			// So we does not check the bits in diagonal 
			overrideDiagonalCheckBitmap(findGrid, findColor, bmCheckedTigerMouth) ;
			bmCheckedTigerMouth.SetBitOn(startPos) ;
		}
	}

	debugConnector("TigerMouth:", bmCheckedTigerMouth) ;	

	return bmCheckedTigerMouth ;
}

WeichiBitBoard WeichiConnectorHandler::findFullBoardDiagonal( WeichiBitBoard bmCandidate, Color findColor )
{
	debugConnector("Diagonal Candidate:", bmCandidate) ;

	Color oppColor = AgainstColor(findColor);
	WeichiBitBoard bmFindPos = bmCandidate;
	WeichiBitBoard bmCheckedDiagonal ;
	int startPos = 0;
	while( (startPos = bmFindPos.bitScanForward()) != -1 ) {
		const WeichiGrid& findGrid = m_board.getGrid(startPos);
		assert( findGrid.getColor() == COLOR_NONE );
		WeichiMove invadeMove(oppColor, startPos) ;
		// For diagonal.
		Vector<short, 4> vBlocksIds;
		if( isDiagonalConnector(invadeMove, vBlocksIds) ) {
			WeichiConnector* connector = m_connectors.NewOne();
			uint patternDir = findGrid.getPattern().getEyeConnectorDiagonal(findColor);
			uint diagonalDir = StaticBoard::getPattern3x3Direction(patternDir)[0];
			int diagonalPos = m_board.getGrid(findGrid, diagonalDir).getPosition();			
			setConnectorAttribute(TYPE_DIAGONAL, connector, findColor, startPos, diagonalPos, vBlocksIds) ;
			// PostUpdate: set bit off the diagonal
			bmFindPos.SetBitOff(diagonalPos);
			bmCheckedDiagonal.SetBitOn(diagonalPos);
			bmCheckedDiagonal.SetBitOn(startPos);
		} //end if
	}

	debugConnector("bmCheckedDiagonal:", bmCheckedDiagonal) ;	

	return bmCheckedDiagonal ;
}

WeichiBitBoard WeichiConnectorHandler::findFullBoardJump(WeichiBitBoard bmCandidate, Color findColor)
{
	Color oppColor = AgainstColor(findColor) ;
	WeichiBitBoard bmFindPos = bmCandidate ;
	WeichiBitBoard bmCheckedJump ;
	int startPos = 0;
	while ( (startPos = bmFindPos.bitScanForward()) != -1 ) {
		const WeichiGrid& startGrid = m_board.getGrid(startPos) ;
		assert(startGrid.getColor() == COLOR_NONE) ;
		WeichiMove invadeMove(oppColor, startPos) ;				
		if ( isJumpPattern(findColor, startGrid) ) {
			int endDir = m_board.realDirectionDIR4of(startGrid.getPattern().getEdgeJumpDirAnother(findColor, startGrid.getColor()));
			uint endPos = startGrid.getStaticGrid().getNeighbor(endDir) ;
			const WeichiGrid& endGrid = m_board.getGrid(endPos);			
			Vector<short, 4> vBlocksIds;
			collectNbrBlocks(findColor, startGrid, vBlocksIds) ;
			WeichiConnector* connector = m_connectors.NewOne();
			setJumpConnectorAttribute(connector, findColor, startPos, endPos, vBlocksIds);
			bmFindPos.SetBitOff(endPos) ;
			bmCheckedJump.SetBitOn(startPos) ;
			bmCheckedJump.SetBitOn(endPos) ;			
		}
	}

	return bmCheckedJump ;	
}

WeichiBitBoard WeichiConnectorHandler::findFullBoardWallJump( WeichiBitBoard bmCandidate, Color findColor ) 
{
	Color oppColor = AgainstColor(findColor) ;
	WeichiBitBoard bmFindPos = bmCandidate ;
	WeichiBitBoard bmCheckedWallJump ;
	int startPos = 0;
	while ( (startPos = bmFindPos.bitScanForward()) != -1 ) {
		const WeichiGrid& startGrid = m_board.getGrid(startPos) ;
		assert(startGrid.getColor() == COLOR_NONE) ;
		WeichiMove invadeMove(oppColor, startPos) ;				
		if ( isWallJumpPattern(findColor, startGrid) ) {
			int endDir = m_board.realDirectionDIR4of(startGrid.getPattern().getEdgeWallJumpDirAnother(findColor, startGrid.getColor()));
			uint endPos = startGrid.getStaticGrid().getNeighbor(endDir) ;
			const WeichiGrid& endGrid = m_board.getGrid(endPos);			
			Vector<short, 4> vBlocksIds;
			collectNbrBlocks(findColor, startGrid, vBlocksIds) ;
			WeichiConnector* connector = m_connectors.NewOne();
			setWallJumpConnectorAttribute(connector, findColor, startPos, endPos, vBlocksIds);
			bmFindPos.SetBitOff(endPos) ;
			bmCheckedWallJump.SetBitOn(startPos) ;
			bmCheckedWallJump.SetBitOn(endPos) ;			
		}
	}

	return bmCheckedWallJump ;
}

WeichiBitBoard WeichiConnectorHandler::findFullBoardTigerMouthOneLibDeadBlock( Color findColor ) 
{	
	Color oppColor = AgainstColor(findColor) ;
	WeichiBitBoard bmOneLibBlocksBitboard = m_board.getOneLibBlocksBitBoard(oppColor) ;
	WeichiBitBoard bmCheckedOneLibDeadBlock ;
	int oneLibBlockPos = 0 ;
	while( (oneLibBlockPos=bmOneLibBlocksBitboard.bitScanForward()) != -1 ) {
		const WeichiBlock* onelibBlock = m_board.getGrid(oneLibBlockPos).getBlock() ;
		assert( onelibBlock!=NULL ) ;

		// 1. check neighboring 1-lib blocks.
		WeichiBitBoard bmNbr = onelibBlock->getNbrMap() & m_board.getStoneBitBoard(findColor) ;
		int nbrPos = 0 ;
		bool bCanEatNbrBlock = false ;
		while( (nbrPos=bmNbr.bitScanForward()) != -1 ) {
			const WeichiBlock* nbrBlock = m_board.getGrid(nbrPos).getBlock() ;
			// 可以吃掉附近子，而且不會被反提。
			if( nbrBlock->getLiberty() == 1 && !m_kb.isSnapback(WeichiMove(oppColor, nbrBlock->getLastLiberty(m_board.getBitBoard()))) ) { 
				bCanEatNbrBlock = true ; 
				break ; 
			}
			bmNbr -= nbrBlock->getStoneMap() ;
		}
		bmOneLibBlocksBitboard -= onelibBlock->getStoneMap() ;
		if( bCanEatNbrBlock ) {	continue ; }

		// 2. Check it can escape or not.
		int lastLib = onelibBlock->getLastLiberty(m_board.getBitBoard()) ;
		WeichiMove escapeMove(oppColor, lastLib) ;
		if( m_board.getLibertyAfterPlay(escapeMove) >= 2 ) { continue ; }

		// 3. Check the pattern having tiger mouth (Check the pattern with COLOR_NONE at the center).
		WeichiBitBoard bmStoneMap = onelibBlock->getStoneMap();

		// 4. 蒐集周圍Block
		int stonePos = 0 ;
		while( (stonePos=bmStoneMap.bitScanForward()) != -1 ) {
			WeichiGrid& stoneGrid = m_board.getGrid(stonePos) ;
			Vector<short, 4> vBlocksIds;
			if( isTigerMouth1libDeadBlockConnector(stoneGrid, vBlocksIds) ) {
				int threatPos1=0, threatPos2=0 ;
				if (m_board.getLibertyAfterPlay(escapeMove) == 0) { 
					threatPos1 = threatPos2 = lastLib; 
				}
				else if (m_board.getLibertyAfterPlay(escapeMove) == 1) { 
					threatPos1 = lastLib ;
					threatPos2 = m_board.getLastLibertyPositionAfterPlay(escapeMove) ;
					if( m_board.getGrid(threatPos2).getColor() != COLOR_NONE ) { threatPos2=threatPos1; }
				}				
				WeichiConnector* connector = m_connectors.NewOne();								
				setTigetMouth1libDeadBlockConnectorAttribute(connector, findColor, threatPos1, threatPos2, onelibBlock->getStoneMap(), vBlocksIds) ;
				bmCheckedOneLibDeadBlock.SetBitOn(threatPos1);
				bmCheckedOneLibDeadBlock.SetBitOn(threatPos2);
			}
		}
	}

	debugConnector("TigerMouthOneLibDeadBlock:", bmCheckedOneLibDeadBlock) ;	

	return bmCheckedOneLibDeadBlock ;
}

WeichiBitBoard WeichiConnectorHandler::findFullBoardBan(WeichiBitBoard bmCandidate, Color findColor)
{
	Color oppColor = AgainstColor(findColor) ;
	WeichiBitBoard bmFindPos = bmCandidate ;
	WeichiBitBoard bmCheckedBan ;
	int startPos = 0;
	while ( (startPos = bmFindPos.bitScanForward()) != -1 ) {
		const WeichiGrid& startGrid = m_board.getGrid(startPos) ;
		assert(startGrid.getColor() == COLOR_NONE) ;
		WeichiMove invadeMove(oppColor, startPos) ;		
		if ( isBanPattern(findColor, startGrid) ) {						
			Vector<short, 4> vBlocksIds;
			collectNbrBlocks(findColor, startGrid, vBlocksIds) ;
			WeichiConnector* connector = m_connectors.NewOne();
			setBanConnectorAttribute(connector, findColor, startPos, startPos, vBlocksIds);
			bmFindPos.SetBitOff(startPos) ;
			bmCheckedBan.SetBitOn(startPos) ;	
		}
	}

	return bmCheckedBan ;
}

WeichiBitBoard WeichiConnectorHandler::findFullBoardLibConnector(WeichiBitBoard bmCandidate, Color findColor)
{
	Color oppColor = AgainstColor(findColor) ;
	WeichiBitBoard bmFindPos = bmCandidate ;
	WeichiBitBoard bmMutualBlockLibConnection ;
	int startPos = 0;
	while ( (startPos = bmFindPos.bitScanForward()) != -1 ) {
		const WeichiGrid& startGrid = m_board.getGrid(startPos) ;
		assert(startGrid.getColor() == COLOR_NONE) ;
		WeichiMove invadeMove(oppColor, startPos) ;		
		Vector<short, 4> vBlocksIds;		
		if ( isMutualBlockLibConnector(startGrid, vBlocksIds) ) {						
			WeichiConnector* connector = m_connectors.NewOne();
			setMutualBlockLibConnectorAttribute(connector, findColor, startPos, startPos, vBlocksIds);
			bmFindPos.SetBitOff(startPos) ;
			bmMutualBlockLibConnection.SetBitOn(startPos) ;	
		}
	}

	return bmMutualBlockLibConnection ;	
}

bool WeichiConnectorHandler::isBambooPattern( Color findColor, const WeichiGrid & startGrid )
{
	// [Bamboo]
	// @  X 
	// @ *WW 
	// @Xb..X
	// @ bWW 
	// @  X

	if ( !startGrid.getPattern().getEdgeBamboo(findColor) ) { return false ; }		
	uint endDir = m_board.realDirectionDIR4of(startGrid.getPattern().getEdgeBambooDirAnother(findColor, startGrid.getColor()));
	uint endPos = startGrid.getStaticGrid().getNeighbor(endDir) ;

	return true;
}

bool WeichiConnectorHandler::isHalfBambooPattern(Color findColor, const WeichiGrid & startGrid)
{
	// [HalfBamboo]
	// @  X 
	// @ .W* 
	// @X..bX
	// @ WW* 
	// @  X

	if ( !startGrid.getPattern().getEdgeHalfBamboo(findColor) ) { return false ; }		

	return true;
}

bool WeichiConnectorHandler::isWallBambooPattern( Color findColor, const WeichiGrid& startGrid )
{
	// [WallBamboo]
	// @  X 
	// @ WW* 
	// @X..bX
	// @     
	// @   
	// The start grid should satisft the above pattern.

	if ( !startGrid.getPattern().getEdgeWallBamboo(findColor) ) { return false ; }		
	uint endDir = m_board.realDirectionDIR4of(startGrid.getPattern().getEdgeWallBambooDirAnother(findColor, startGrid.getColor()));
	uint endPos = startGrid.getStaticGrid().getNeighbor(endDir) ;	

	return true ;
}

bool WeichiConnectorHandler::isJumpPattern(Color findColor, const WeichiGrid & startGrid)
{
	if ( !startGrid.getPattern().getEdgeJump(findColor) ) { return false ; }		

	return true ;
}

bool WeichiConnectorHandler::isWallJumpPattern(Color findColor, const WeichiGrid & startGrid)
{
	if ( !startGrid.getPattern().getEdgeWallJump(findColor) ) { return false ; }		

	return true ;
}

bool WeichiConnectorHandler::isBanPattern(Color findColor, const WeichiGrid &startGrid)
{
	if ( !startGrid.getPattern().getEdgeBan(findColor) ) { return false ; }		

	return true;
}

bool WeichiConnectorHandler::isFalseEyeConnector( const WeichiMove& invadeMove, Vector<short, 4>& vBlocksIds)
{
	const WeichiGrid& findGrid = m_board.getGrid(invadeMove);
	Color findColor = AgainstColor(invadeMove.getColor());
	if ( !findGrid.getPattern().getEyeConnectorFalseEye(findColor) ) { return false; }
	if ( m_board.getLibertyAfterPlay(invadeMove) != 0 ) { return false ; }

	collectNbrBlocks(findColor, m_board.getGrid(invadeMove), vBlocksIds) ;
	if ( vBlocksIds.size() == 1 ) { return false; }

	return true ;
}

bool WeichiConnectorHandler::isTigerMouthConnecotr( const WeichiMove& invadeMove, Vector<short, 4>& vBlocksIds ) 
{
	// 1. Corresponding Tiger-Mouth pattern
	// 2. 0-lib or 1-lib after play.
	// 3. 2-lib after capture.
	// 4. Not eat ko.
	// 5. Not make ko.		
	Color ownColor = AgainstColor(invadeMove.getColor()) ;	
	if (!m_board.getGrid(invadeMove).getPattern().getEyeConnectorTigerMouth(ownColor)) { return false ; }

	// Collect nbr blocks should be done first.
	collectNbrBlocks(ownColor, m_board.getGrid(invadeMove), vBlocksIds) ;
	if( vBlocksIds.size() == 1 ) { return false ; }

	if( m_board.isKoEatPlay(invadeMove) ) { return false ; }
	// if( m_kb.isMakeKo(m_board,invadeMove) ) { return false ; }	
	if( !m_board.isIllegalMove(invadeMove, m_ht) && m_board.getLibertyAfterPlay(invadeMove) >= 2 ) { return false ; }			
	//if( !m_board.isIllegalMove(invadeMove, m_ht) && m_board.getLibertyAfterPlay(invadeMove) == 1 && m_board.getStoneAfterPlay(invadeMove) == 1 ) {
	//	int lastLib = m_board.getLastLibertyPositionAfterPlay(invadeMove) ;
	//	WeichiMove captureMove(ownColor, lastLib) ;
	//	if( !m_board.isIllegalMove(captureMove, m_ht) && m_board.getLibertyAfterPlay(captureMove) == 1 ) { 
	//		// Dirty code, only want to prevent Size3_9.sgf
	//		// Can not just use snapback function.
	//		return false ; 
	//	}
	//}

	return true ;
}

bool WeichiConnectorHandler::isDiagonalConnector( const WeichiMove& invadeMove, Vector<short, 4>& vBlocksIds ) 
{
	const WeichiGrid& findGrid = m_board.getGrid(invadeMove) ;
	Color findColor = AgainstColor(invadeMove.getColor()) ;
	if( !findGrid.getPattern().getEyeConnectorDiagonal(findColor) ) { return false ; }
	
	uint patternDir = findGrid.getPattern().getEyeConnectorDiagonal(findColor) ;
	uint realDir = StaticBoard::getPattern3x3Direction(patternDir)[0] ;		
	int diagonalPos = m_board.getGrid(findGrid, realDir).getPosition() ;

	WeichiBitBoard bmA ; bmA.SetBitOn(invadeMove.getPosition()) ;
	WeichiBitBoard bmB ; bmB.SetBitOn(diagonalPos) ;

	WeichiBitBoard bmDiagonalBlock = ((bmA.dilate()-bmA)&(bmB.dilate()-bmB)) & StaticBoard::getMaskBorder() ;

	WeichiBlock* blockA = m_board.getGrid(bmDiagonalBlock.bitScanForward()).getBlock() ;
	WeichiBlock* blockB = m_board.getGrid(bmDiagonalBlock.bitScanForward()).getBlock() ;

	if( blockA == blockB ) { return false ; }

	vBlocksIds.push_back(blockA->GetID()) ;
	vBlocksIds.push_back(blockB->GetID()) ;

	return true ;
}

bool WeichiConnectorHandler::isTigerMouth1libDeadBlockConnector( const WeichiGrid& stoneGrid, Vector<short, 4>& vBlocksIds ) 
{
	Color ownColor = AgainstColor(stoneGrid.getColor()) ;
	if (!stoneGrid.getPattern().getEyeConnectorTigerMounthOneLibDeadBlock(ownColor, stoneGrid.getColor())) { return false; }
	
	collectNbrBlocks(ownColor, stoneGrid, vBlocksIds) ;
	if (vBlocksIds.size() == 1) { return false ; }

	return true ;
}

bool WeichiConnectorHandler::isMutualBlockLibConnector(const WeichiGrid& grid, Vector<short, 4>& vBlocksIds)
{
	Color ownColor = AgainstColor(grid.getColor());
	//if( !grid.getPattern().getEyeConnectorTigerMounthOneLibDeadBlock(ownColor, grid.getColor()) ) { return false ; }

	collectNbrBlocks(ownColor, grid, vBlocksIds);
	if (vBlocksIds.size() == 1) { return false; }	
	
	return true;
}

void WeichiConnectorHandler::getBambooAttrs( Color findColor, const WeichiGrid & startGrid, int & endPos, Vector<short, 4>& vBlocksIds )
{
	uint endDir = m_board.realDirectionDIR4of(startGrid.getPattern().getEdgeBambooDirAnother(findColor, startGrid.getColor()));
	endPos = startGrid.getStaticGrid().getNeighbor(endDir) ;

	collectNbrBlocks(findColor, startGrid, vBlocksIds) ;

	return ;
}

void WeichiConnectorHandler::getHalfBambooAttrs(Color findColor, const WeichiGrid & startGrid, int & endPos, Vector<short, 4>& vBlocksIds)
{
	uint endDir = m_board.realDirectionDIR4of(startGrid.getPattern().getEdgeHalfBambooDirAnother(findColor, startGrid.getColor()));
	endPos = startGrid.getStaticGrid().getNeighbor(endDir) ;

	collectNbrBlocks(findColor, startGrid, vBlocksIds) ;

	return ;
}

void WeichiConnectorHandler::getWallBambooAttrs( Color findColor, const WeichiGrid& startGrid, int& endPos ) 
{
	int endDir = m_board.realDirectionDIR4of(startGrid.getPattern().getEdgeWallBambooDirAnother(findColor, startGrid.getColor()));
	endPos = startGrid.getStaticGrid().getNeighbor(endDir) ;

	return ;
}

void WeichiConnectorHandler::collectNbrBlocks( Color collectColor, const WeichiGrid& centerGrid, Vector<short, 4>& vBlocksIds ) 
{
	WeichiBitBoard bmNbrBlocks;		
	for( const int *iNbr = centerGrid.getStaticGrid().getAdjacentNeighbors() ; *iNbr!=-1 ; ++iNbr ) {
		const WeichiGrid& nbrGrid = m_board.getGrid(*iNbr) ;				
		const WeichiBlock* nbrBlock = nbrGrid.getBlock() ;
		if( nbrGrid.getBlock()==NULL || nbrBlock->getColor()!=collectColor ) { continue ; }
		
		if( bmNbrBlocks.empty() ) { 
			bmNbrBlocks = nbrBlock->getStoneMap() ; 
			vBlocksIds.push_back(nbrBlock->GetID()) ;
		} else if( !bmNbrBlocks.hasIntersection(nbrBlock->getStoneMap()) ) {
			bmNbrBlocks |= nbrBlock->getStoneMap() ;
			vBlocksIds.push_back(nbrBlock->GetID()) ;
		}
	}

	return ;
}

void WeichiConnectorHandler::setConnectorAttribute( ConnectorType type, Connector* conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds ) 
{
	conn->init() ;
	conn->setType(type) ;
	conn->setColor(findColor);
	conn->addConnecPoint(firstPos); 
	conn->addThreatLocation(firstPos, secondPos) ;		

	for( uint iBlock=0 ; iBlock < vBlocksIds.size() ; ++iBlock ) {
		conn->addBlockId(vBlocksIds[iBlock]) ;
		WeichiBlock* block = m_board.getBlock(vBlocksIds[iBlock]) ;
		m_blockConnectors[block->getiFirstGrid()].push_back(conn) ;		
	}

	return ;
}

void WeichiConnectorHandler::setBambooConnectorAttribute(WeichiConnector* conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds)
{
	conn->init();
	conn->setType(TYPE_BAMBOO);
	conn->setColor(findColor);
	conn->addConnecPoint(firstPos);
	conn->addConnecPoint(secondPos);
	conn->addThreatLocation(firstPos, secondPos);	

	for( uint iBlock=0 ; iBlock < vBlocksIds.size() ; ++iBlock ) {
		conn->addBlockId(vBlocksIds[iBlock]) ;
		WeichiBlock* block = m_board.getBlock(vBlocksIds[iBlock]) ;
		m_blockConnectors[block->getiFirstGrid()].push_back(conn) ;
	}

	return ;		
}

void WeichiConnectorHandler::setHalfBambooConnectorAttribute(WeichiConnector* conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds)
{
	conn->init() ;
	conn->setType(TYPE_HALFBAMBOO) ;
	conn->setColor(findColor) ;
	conn->addConnecPoint(firstPos) ; // only add center point
	conn->addThreatLocation(firstPos, secondPos) ;		

	for( uint iBlock=0 ; iBlock < vBlocksIds.size() ; ++iBlock ) {
		conn->addBlockId(vBlocksIds[iBlock]) ;
		WeichiBlock* block = m_board.getBlock(vBlocksIds[iBlock]) ;
		m_blockConnectors[block->getiFirstGrid()].push_back(conn) ;
	}

	return ;
}

void WeichiConnectorHandler::setWallBambooConnectorAttribute( WeichiConnector* conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds) 
{
	conn->init() ;
	conn->setType(TYPE_WALLBAMBOO) ;
	conn->setColor(findColor) ;
	conn->addConnecPoint(firstPos) ;
	conn->addConnecPoint(secondPos) ;
	conn->addThreatLocation(firstPos, secondPos) ;		

	for( uint iBlock=0 ; iBlock < vBlocksIds.size() ; ++iBlock ) {
		conn->addBlockId(vBlocksIds[iBlock]) ;
		WeichiBlock* block = m_board.getBlock(vBlocksIds[iBlock]) ;
		m_blockConnectors[block->getiFirstGrid()].push_back(conn) ;
	}						

	return ;
}

void WeichiConnectorHandler::setTigerMouthConnectorAttribute(WeichiConnector * conn, Color findColor, int centerPos, int anotherPos, Vector<short, 4>& vBlocksIds)
{
	conn->init() ;
	conn->setType(TYPE_TIGERMOUTH) ;
	conn->setColor(findColor) ;
	conn->addConnecPoint(centerPos) ;	
	conn->addThreatLocation(centerPos, anotherPos) ;		

	for( uint iBlock=0 ; iBlock < vBlocksIds.size() ; ++iBlock ) {
		conn->addBlockId(vBlocksIds[iBlock]) ;
		WeichiBlock* block = m_board.getBlock(vBlocksIds[iBlock]) ;
		m_blockConnectors[block->getiFirstGrid()].push_back(conn) ;		
	}

	return ;
}

void WeichiConnectorHandler::setTigetMouth1libDeadBlockConnectorAttribute(Connector * conn, Color findColor, int miaiPos1, int miaiPos2, WeichiBitBoard bmDeadStone, Vector<short, 4>& vBlocksIds)
{
	conn->init() ;
	conn->setType(TYPE_TIGERMOUTH_1LIB_DEADBLOCK) ;
	conn->setColor(findColor);
	conn->addConnecPoint(miaiPos1); 
	conn->addThreatLocation(miaiPos1, miaiPos2);		
	conn->addThreatBitmap(bmDeadStone);
	
	for( uint iBlock=0 ; iBlock < vBlocksIds.size() ; ++iBlock ) {
		conn->addBlockId(vBlocksIds[iBlock]) ;
		WeichiBlock* block = m_board.getBlock(vBlocksIds[iBlock]) ;
		m_blockConnectors[block->getiFirstGrid()].push_back(conn) ;		
	}

	return ;
}

void WeichiConnectorHandler::setJumpConnectorAttribute(WeichiConnector * conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds)
{
	conn->init() ;
	conn->setType(TYPE_JUMP) ;
	conn->setColor(findColor) ;
	conn->addConnecPoint(firstPos) ;
	conn->addThreatLocation(firstPos, secondPos) ;								

	for( uint iBlock=0 ; iBlock < vBlocksIds.size() ; ++iBlock ) {
		conn->addBlockId(vBlocksIds[iBlock]) ;
		WeichiBlock* block = m_board.getBlock(vBlocksIds[iBlock]) ;
		m_blockConnectors[block->getiFirstGrid()].push_back(conn) ;
	}

	return ;
}

void WeichiConnectorHandler::setWallJumpConnectorAttribute(WeichiConnector * conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds)
{
	conn->init() ;
	conn->setType(TYPE_WALLJUMP) ;
	conn->setColor(findColor) ;
	conn->addConnecPoint(firstPos) ;
	conn->addThreatLocation(firstPos, secondPos) ;								

	for( uint iBlock=0 ; iBlock < vBlocksIds.size() ; ++iBlock ) {
		conn->addBlockId(vBlocksIds[iBlock]) ;
		WeichiBlock* block = m_board.getBlock(vBlocksIds[iBlock]) ;
		m_blockConnectors[block->getiFirstGrid()].push_back(conn) ;
	}

	return ;
}

void WeichiConnectorHandler::setBanConnectorAttribute(WeichiConnector * conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds)
{
	conn->init() ;
	conn->setType(TYPE_BAN) ;
	conn->setColor(findColor) ;
	conn->addConnecPoint(firstPos) ;
	conn->addThreatLocation(firstPos, secondPos) ;								

	for( uint iBlock=0 ; iBlock < vBlocksIds.size() ; ++iBlock ) {
		conn->addBlockId(vBlocksIds[iBlock]) ;
		WeichiBlock* block = m_board.getBlock(vBlocksIds[iBlock]) ;
		m_blockConnectors[block->getiFirstGrid()].push_back(conn) ;
	}

	return ;
}

void WeichiConnectorHandler::setMutualBlockLibConnectorAttribute(WeichiConnector * conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds)
{
	conn->init() ;
	conn->setType(TYPE_MUTUAL_BLOCK_LIB) ;
	conn->setColor(findColor) ;
	conn->addConnecPoint(firstPos) ;
	conn->addThreatLocation(firstPos, firstPos) ;								

	for( uint iBlock=0 ; iBlock < vBlocksIds.size() ; ++iBlock ) {
		conn->addBlockId(vBlocksIds[iBlock]) ;
		WeichiBlock* block = m_board.getBlock(vBlocksIds[iBlock]) ;
		m_blockConnectors[block->getiFirstGrid()].push_back(conn) ;
	}

	return ;
}

void WeichiConnectorHandler::overrideDiagonalCheckBitmap( const WeichiGrid& grid, Color findColor, WeichiBitBoard& bmChecked ) 
{
	uint patternDir = grid.getPattern().getEyeConnectorDiagonal(findColor);
	const Vector<uint, 8>& vDiagonalDir = StaticBoard::getPattern3x3Direction(patternDir) ;
	for ( uint iDir = 0 ; iDir < vDiagonalDir.size() ; ++iDir ) {
		int diagonalDir = vDiagonalDir[iDir];
		int diagonalPos = m_board.getGrid(grid, diagonalDir).getPosition() ;
		bmChecked.SetBitOn(diagonalPos) ;
	}
	
	return ;
}

void WeichiConnectorHandler::debugConnector( string comment, WeichiBitBoard bitboard ) 
{
	//if( !DO_DEBUG_CONNECTOR ) return ;

	//cerr << comment << bitboard.toString(7,7) << endl ;
	return ;
}

bool WeichiConnectorHandler::isInterestedConnectorBlock(Color findColor, int blockID)
{
	FeatureList<WeichiBlock,MAX_GAME_LENGTH>& blockList = m_board.getBlockList();	
	if (!blockList.isValidIdx(blockID)) { return false; }		
	WeichiBlock* block = blockList.getAt(blockID);
	if (!block->isUsed()) { return false; }
	if (block->getColor() != findColor) { return false; }
	if (block->getStatus() == LAD_LIFE) { return false; }

	return true;
} 
