#include "WeichiEyeHandler.h"
#include "WeichiLifeDeathHandler.h"

void WeichiEyeHandler::reset()
{
	FeatureList<WeichiClosedArea, MAX_NUM_CLOSEDAREA>& vClosedAreas = m_board.getCloseArea() ;
	for( int iCA=0 ; iCA < vClosedAreas.getCapacity() ; iCA++ ) {
		if( !vClosedAreas.isValidIdx(iCA) ) { continue ; }

		WeichiClosedArea* ca  = vClosedAreas.getAt(iCA) ;
		ca->setEyeStatus(EYE_UNKNOWN);
		ca->setRZone(WeichiBitBoard());
	} 

	m_sDebugInfo = "";

	return ;
}

void WeichiEyeHandler::findFullBoardEyes(Color ownColor)
{
	reset();	
	
	// 1. Judge Eyes for closed areas
	FeatureList<WeichiClosedArea, MAX_NUM_CLOSEDAREA>& vClosedAreas = m_board.getCloseArea();
	for (int iCA=0; iCA < vClosedAreas.getCapacity(); ++iCA) {
		if (!vClosedAreas.isValidIdx(iCA) ) { continue; }
		WeichiClosedArea* ca = vClosedAreas.getAt(iCA);
		if (ca->getStatus() == LAD_LIFE) { continue; }
		if (ca->getColor() != ownColor) { continue; }		

		judgeTrueEyeByRule(ca);
	} 

	if (!WeichiConfigure::use_pattern_eye) { return; }

	// 2. Judge Eyes for non-closed areas
	FeatureList<WeichiOpenedArea, MAX_NUM_OPENEDAREA>& vOpenedAreas = m_lifedeathHandler->getOpenedAreasHandler().getDragonOpenedAreas() ;
	for (int iOA=0 ; iOA < vOpenedAreas.getCapacity() ; ++iOA) {
		if( !vOpenedAreas.isValidIdx(iOA) ) { continue; }

		WeichiOpenedArea* oa = vOpenedAreas.getAt(iOA);
		judgeOpenedAreaTrueEyeByRule(oa);
	} 

	return ;
}

void WeichiEyeHandler::judgeTrueEyeByRule(WeichiClosedArea* closedarea) 
{
	if (!isTrueEyeByRule(closedarea)) { return; }
	
	postUpdateRZone(closedarea);

	return;
}

bool WeichiEyeHandler::isTrueEyeByRule(WeichiClosedArea * closedarea)
{
	// Rules: Check precondition
	if (!checkNbrBlockLiberty(closedarea)) { return false; }
	if (!checkInsideBlockTrapped(closedarea)) { return false; }

	// Rules: Check trues eyes
	if (judgeOneRegionLiveByRule(closedarea))				{ return true; }
	else if (judgeStaticTrueEyeByRule(closedarea))			{ return true; }		
	else if (judgeEmptyLargeArea(closedarea))				{ return true; }
	//else if ( judgeSluggishTrueEyeByRule(closedarea) ) { return true; }	
	else if (judgeSize1ProtectedTrueEyeByRule(closedarea))  { return true; }
	else if (judgeSize2ProtectedTrueEyeByRule(closedarea))  { return true; }
	else if (judge3StonesInPrisonTrueEyeByRule(closedarea)) { return true; }	
	else if (judgeMiaiPointsTrueEyeByRule(closedarea))		{ return true; }
	else if (judgeBigConnectorTrueEyeByRule(closedarea))	{ return true; }

	return false;
}

bool WeichiEyeHandler::checkNbrBlockLiberty( WeichiClosedArea* closedarea ) 
{
	// Check the following conditions		
	// Others are safe

	Color ownColor = closedarea->getColor() ;
	Color oppColor = AgainstColor(ownColor) ;

	// 1. Check 1-lib except snapback --> not safe
	WeichiBitBoard bmLibIntersection = StaticBoard::getMaskBorder() ;	
	int num2libBlocks = 0 ;
	for (uint iBlock=0 ; iBlock < closedarea->getNumBlock() ; ++iBlock) {
		WeichiBlock* nbrBlock = m_board.getBlock(closedarea->getBlockID(iBlock));
		if (nbrBlock->getLiberty() == 1) { 
			// Check 1-lib not snapback unsafe
			WeichiMove captureMove(oppColor, nbrBlock->getLastLiberty(m_board.getBitBoard()));
			if( !m_kb.isSnapback(captureMove) ) { return false; }
		} else if (nbrBlock->getLiberty() == 2) {			
			++num2libBlocks; 
			bmLibIntersection &= nbrBlock->getLibertyBitBoard(m_board.getBitBoard()); 
		}
	}

	// 2. Check double-atari --> not safe
	// Check double-atari unsafe
	if (num2libBlocks >= 2) {
		int pos = 0 ;
		while( (pos=bmLibIntersection.bitScanForward()) != -1 ) {
			WeichiMove atariMove(oppColor, pos) ;			
			if( !m_lifedeathHandler->getKnowledge().isDoubleAtariSafe(atariMove) ) { return false ; }
		}
	}	

	return true ;
}

bool WeichiEyeHandler::checkInsideBlockTrapped( WeichiClosedArea* closedarea ) 
{
	if (!checkInsideBlockDead(closedarea)) { return false; }
	if (!checkInsideBlockInert(closedarea)) { return false; }

	return true;
}

bool WeichiEyeHandler::checkInsideBlockDead(WeichiClosedArea * closedarea)
{
	WeichiBitBoard bmClosedArea = closedarea->getStoneMap() ;
	int pos = 0 ;
	while( (pos=bmClosedArea.bitScanForward()) != -1 ) {
		WeichiGrid& grid = m_board.getGrid(pos) ;
		if (grid.isEmpty()) { continue; }
		
		WeichiBlock* block = grid.getBlock() ;
		//if( m_lifedeathHandler->getKnowledge().isBlockCanEscape(block) ) { return false; }
		if (!m_lifedeathHandler->getKnowledge().isBlockDead(block)) { return false; }
		
		bmClosedArea -= block->getStoneMap() ;
	}

	return true ;
}

bool WeichiEyeHandler::checkInsideBlockInert( WeichiClosedArea* closedarea ) 
{
	WeichiBitBoard bmStone = closedarea->getStoneMap() ;
	int pos = 0;
	while( (pos=bmStone.bitScanForward()) != -1 ) {
		const WeichiGrid& grid = m_board.getGrid(pos) ;
		const WeichiBlock* oppBlock = grid.getBlock() ;
		if( oppBlock == NULL || oppBlock->getLiberty() == 1 ) { continue; }				
		if( oppBlock->getNumClosedArea() != 0 ) { 			
			// Check the situation that oppBlock has closed-area inside the region, if it is, then return false.
			for (uint iCa=0 ; iCa < oppBlock->getNumClosedArea() ; ++iCa) {
				const WeichiClosedArea* oppClosedArea = oppBlock->getClosedArea(iCa, m_board.getCloseArea()) ;								
				if( oppClosedArea->getStoneMap().isSubsetOf(closedarea->getStoneMap()) ) { return false ; }				
			}		
		}		
		bmStone -= oppBlock->getStoneMap() ;
	}

	return true ;
}


bool WeichiEyeHandler::checkSize1PreCondition( WeichiClosedArea* closedarea ) 
{
	if (closedarea->getNumStone() != 1) { return false; }
	//if( !checkNbrBlockLiberty(closedarea) ) { return false ; }	

	return true ;
}

bool WeichiEyeHandler::checkSize2PreCondition(WeichiClosedArea * closedarea)
{
	if (closedarea->getNumStone() != 2) { return false; }

	return true;
}


bool WeichiEyeHandler::judgeOneRegionLiveByRule( WeichiClosedArea* closedarea )
{	
	 //Use one region to judge live, use very strict rules.	
	 if (isLinearPattern4Live(closedarea)) { return true ; }
	 //else if ( isLinearPattern5Live(region) ) { return true ; }
	 //else ..

	return false ;
}

bool WeichiEyeHandler::judgeStaticTrueEyeByRule( WeichiClosedArea* closedarea ) 
{		
	if (isStaticTrueEye(closedarea)) { 		
		WeichiBitBoard bmRZone;
		bmRZone |= getSurroundedBlocksBitboard(closedarea);
		bmRZone |= closedarea->getStoneMap();
		closedarea->setRZone(bmRZone);
		closedarea->setEyeStatus(STATIC_TRUE_EYE); 				
		return true ;
	}	

	return false; 
}

bool WeichiEyeHandler::judgeSluggishTrueEyeByRule( WeichiClosedArea* closedarea )
{
	if( closedarea->getNumBlock() != 1 ) { return false ; }

	// Resolve the unhealthy property.
	Color liveColor = closedarea->getColor() ;
	Color oppColor = AgainstColor(liveColor) ;
	WeichiBlock* surOneBlock = m_board.getBlock(closedarea->getBlockID(0)) ;
	WeichiBitBoard bmUnTouchedEmptyPoints = closedarea->getStoneMap() - m_board.getStoneBitBoard(oppColor) - surOneBlock->getLibertyBitBoard(m_board.getBitBoard()) ;
	WeichiBitBoard bmRZone ;
	if ( bmUnTouchedEmptyPoints.bitCount() == 1 ) {
		bmRZone |= getSurroundedBlocksBitboard(closedarea) ;
		bmRZone |= closedarea->getStoneMap() ;
		closedarea->setRZone(bmRZone) ;
		closedarea->setEyeStatus(SLUGGISH_TRUE_EYE) ;
		return true;
	}		

	return false ;	
}

bool WeichiEyeHandler::judgeEmptyLargeArea(WeichiClosedArea* closedarea)
{
	if (closedarea->getNumStone() < 7) { return false; }
	WeichiBitBoard bmStone = closedarea->getStoneMap();
	if ((bmStone-m_board.getBitBoard()) != bmStone) { return false; }

	// 1. Should be all empty inside
	// 2. Every point in the area is static

	int nConcreteGrid = 0;
	int pos=0; 
	while ((pos=bmStone.bitScanForward()) != -1) {
		const WeichiGrid& grid = m_board.getGrid(pos);
		if (isConcreteGrid(grid,closedarea)) { ++nConcreteGrid; }
	}

	if (nConcreteGrid<7) { return false; }

	WeichiBitBoard bmRZone;	
	bmRZone |= getSurroundedBlocksBitboard(closedarea);
	bmRZone |= closedarea->getStoneMap();
	closedarea->setEyeStatus(ONE_REGION_ENOUGH_LIVE);		
	closedarea->setRZone(bmRZone);		

	return true;
}

bool WeichiEyeHandler::judgeSize1ProtectedTrueEyeByRule(WeichiClosedArea* closedarea) 
{
	if (!checkSize1PreCondition(closedarea)) { return false; }
		
	WeichiBitBoard bmStone = closedarea->getStoneMap();	
	const WeichiGrid& regionGrid = m_board.getGrid(bmStone.bitScanForward()) ;

	WeichiBitBoard bmRZone;
	uint numProtectedCorner = 0 ;	
	Vector<uint, 4> vEmptyNonProtectedCorners ;

	for (const int *iDia = regionGrid.getStaticGrid().getDiagonalNeighbors(); *iDia!=-1 ; ++iDia) {
		WeichiGrid& diaGrid = m_board.getGrid(*iDia);
		if (isProtectedCorner(closedarea, diaGrid, bmRZone)) { ++numProtectedCorner; }
		else if (diaGrid.getColor() == COLOR_NONE) { vEmptyNonProtectedCorners.push_back(*iDia); }
	}	

	if (isSize1RegionEnoughProtectedCorner(closedarea, numProtectedCorner, vEmptyNonProtectedCorners)) { 		
		bmRZone |= getSurroundedBlocksBitboard(closedarea) ;
		bmRZone |= closedarea->getStoneMap() ;
		closedarea->addRZone(bmRZone) ;		
		closedarea->setEyeStatus(PROTECTED_TRUE_EYE) ; 		
		return true ;
	} 

	closedarea->setEyeStatus(EYE_UNKNOWN); 
	return false;
}

bool WeichiEyeHandler::judgeSize2ProtectedTrueEyeByRule( WeichiClosedArea* closedarea ) 
{
	if (!checkSize2PreCondition(closedarea)) { return false; }

	Color ownColor = closedarea->getColor() ;
	Color oppColor = AgainstColor(ownColor) ;

	WeichiBitBoard bmStone = closedarea->getStoneMap() ;
	Vector<uint, 2> vReiognPos ;
	bmStone.bitScanAll(vReiognPos) ;	
	WeichiBitBoard bmMinNbrBitBoard = closedarea->getMinimumSurroundedBitBoard() ;
	
	WeichiBitBoard bmRZone;
	uint numProtectedCorner = 0;	
	Vector<uint, 4> vNonProtected;
	for (uint iPos=0 ; iPos < vReiognPos.size() ; ++iPos) {
		uint regionPos = vReiognPos[iPos] ;
		const WeichiGrid& regionGrid = m_board.getGrid(regionPos) ;
		for( const int *iDia = regionGrid.getStaticGrid().getDiagonalNeighbors() ; *iDia!=-1 ; iDia++ ) {
			WeichiGrid& diaGrid = m_board.getGrid(*iDia) ;	
			if (bmMinNbrBitBoard.BitIsOn(*iDia)) { continue; }
			if (isProtectedCorner(closedarea, diaGrid, bmRZone)) { ++numProtectedCorner; }
			else { vNonProtected.push_back(*iDia); }
		}
	}

	if (isSize2RegionEnoughProtectedCorner(vReiognPos, numProtectedCorner, vNonProtected)) {			
		bmRZone |= getSurroundedBlocksBitboard(closedarea) ;
		bmRZone |= closedarea->getStoneMap();		
		closedarea->setRZone(bmRZone);		
		closedarea->setEyeStatus(PROTECTED_TRUE_EYE); 
		return true ;
	}	

	closedarea->setEyeStatus(EYE_UNKNOWN); 
	return false ;
}

bool WeichiEyeHandler::judge3StonesInPrisonTrueEyeByRule(WeichiClosedArea* closedarea) 
{
	if (closedarea->getNumStone() < 4) { return false ; }

	Color ownColor = closedarea->getColor() ;
	Color oppColor = AgainstColor(ownColor) ;
	WeichiBitBoard bmStone = closedarea->getStoneMap() ;
	WeichiBitBoard bmRZone ;	

	int pos = 0 ;	
	while( (pos=bmStone.bitScanForward()) != -1 ) {
		WeichiBlock* block = m_board.getGrid(pos).getBlock() ;
		if( block==NULL ) { continue ; }		

		int blockLib = block->getLiberty() ;
		int blockNumStone = block->getNumStone() ;			
		//if (blockNumStone >= 3 && !m_lifedeathHandler->getKnowledge().isBlockCanEscape(block)) {
		if (blockNumStone >= 3 && m_lifedeathHandler->getKnowledge().isBlockDead(block)) {
			bmRZone |= getSurroundedBlocksBitboard(closedarea);
			bmRZone |= closedarea->getStoneMap();			
			closedarea->setRZone(bmRZone);				
			closedarea->setEyeStatus(SIZE3STONE_IN_PRISON_TRUE_EYE);	
			return true;	
		}

		bmStone -= block->getStoneMap() ;			
	} 

	return false;
}

bool WeichiEyeHandler::judgeMiaiPointsTrueEyeByRule( WeichiClosedArea* closedarea ) 
{	
	if( closedarea->getNumBlock()!=2 ) { return false ; }

	// 看是否有兩個以上空點for見合	
	WeichiBitBoard bmRZone ;
	WeichiBitBoard bmConn = StaticBoard::getMaskBorder() ;	
	for( uint iBlock=0 ; iBlock < closedarea->getNumBlock() ; ++iBlock ) {
		WeichiBlock* block = m_board.getBlock(closedarea->getBlockID(iBlock)) ;
		bmConn &= (block->getNbrMap()-m_board.getBitBoard()) ; // 剩下空點
	}

	// Size 1的內部點不要參與見合
	if( closedarea->getNumStone() == 1 ) { bmConn -= closedarea->getStoneMap() ; }

	Color ownColor = closedarea->getColor() ;	
	// 有兩個以上見合點就是真眼
	if( bmConn.bitCount() >= 2 ) {			
		bmRZone |= bmConn ;		
		bmRZone |= getSurroundedBlocksBitboard(closedarea) ;
		bmRZone |= closedarea->getStoneMap() ;						
		closedarea->addRZone(bmRZone);
		closedarea->setRZone(bmRZone);
		closedarea->setEyeStatus(MIAI_TO_BE_TRUE_EYE);
		return true;
	}	

	return false ;
}

bool WeichiEyeHandler::judgeBigConnectorTrueEyeByRule( WeichiClosedArea* closedarea ) 
{		
	// Only judge eye that its size is equal to 3 or above 3 
	if (closedarea->getNumStone() < 3) { return false; } 

	Color liveColor = closedarea->getColor() ;
	Color oppColor = AgainstColor(liveColor) ;

	// Use connector to judge whether it is true eye or not.	
	WeichiBitBoard bmSurround = getSurroundedBlocksBitboard(closedarea) ;
	WeichiBlock* firstBlock = m_board.getBlock(closedarea->getBlockID(0)) ;	
	bmSurround -= firstBlock->getStoneMap() ;	
	vector<WeichiConnector*> vConnecotrs;
	vector<WeichiConnector*>& connectors = m_lifedeathHandler->getConnectorHandler().getBlockConnectors(firstBlock);

	bool checkConnectors[MAX_NUM_CONNECTORS] ;
	for (uint iResetCheck = 0 ; iResetCheck < MAX_NUM_CONNECTORS ; ++iResetCheck) { checkConnectors[iResetCheck] = false ; }
	
	WeichiBitBoard bmRZone ;
	WeichiBitBoard bmRelevant ;
	for( uint iConn=0 ; iConn < connectors.size() ; ++iConn ) { 
		checkConnectors[connectors[iConn]->GetID()] = true;
		if (connectors[iConn]->isWeak()) { continue; }
		bmRZone |= connectors[iConn]->getThreatBitmap();		
		vConnecotrs.push_back(connectors[iConn]); 
	}		

	while (vConnecotrs.size() != 0 && !bmSurround.empty()) {
		WeichiConnector* conn = vConnecotrs.back();		
		WeichiBitBoard bmThreat = conn->getThreatBitmap();
		int threatPos = 0 ;
		while ((threatPos=bmThreat.bitScanForward()) != -1) {
			WeichiMove connectMove(liveColor, threatPos) ;
			if (!m_board.isIllegalMove(connectMove, m_ht)) { bmRelevant |= m_board.getStoneBitBoardAfterPlay(connectMove) ; }
		}
		// To Do: 確認這個Connector的緊氣問題以及保護的R-zone
				
		// Use all connector to connect.
		vConnecotrs.pop_back();		
		Vector<short, 4> vBlocksIds = conn->getBlockIDs() ;
		for (uint iBlockId= 0 ; iBlockId < vBlocksIds.size() ; ++ iBlockId ) {
			WeichiBlock* connectBlock = m_board.getBlock(vBlocksIds[iBlockId]) ;					
			if (!bmSurround.hasIntersection(connectBlock->getStoneMap())) { continue; }

			bmRZone |= conn->getThreatBitmap() ;
			bmSurround -= connectBlock->getStoneMap() ; 				
			vector<WeichiConnector*>& connectors = m_lifedeathHandler->getConnectorHandler().getBlockConnectors(connectBlock) ;
			for (uint iConn=0 ; iConn < connectors.size() ; ++iConn) { 
				if (checkConnectors[connectors[iConn]->GetID()]) { continue; }			
				if (connectors[iConn]->isWeak()) { continue; }
				if (connectors[iConn]->isConnectedToWall()) { continue; }

				checkConnectors[connectors[iConn]->GetID()] = true ;					
				vConnecotrs.push_back(connectors[iConn]) ; 		
			}
		}
	}

	if (!bmSurround.empty()) { return false; }
	if (!checkSize3ConnectorEyeException(closedarea, bmRZone)) { return false; }		
	
	bmRZone |= getSurroundedBlocksBitboard(closedarea);
	bmRZone |= closedarea->getStoneMap();
	closedarea->setRZone(bmRZone);
	closedarea->setEyeStatus(CONNECTOR_TRUE_EYE);	

	return true; 
}

bool WeichiEyeHandler::checkSize3ConnectorEyeException( WeichiClosedArea* closedarea, WeichiBitBoard& bmRZone ) 
{	
	if( closedarea->getNumStone() != 3 ) { return true ; }

	Color liveColor = closedarea->getColor() ;
	Color oppColor = AgainstColor(liveColor) ;

	WeichiBitBoard bmInnerBlock = m_board.getStoneBitBoard(oppColor) & closedarea->getStoneMap() ;
	if( bmInnerBlock.bitCount() < 1 ) { return true ; }

	WeichiBitBoard bmStone = closedarea->getStoneMap() ;
	Vector<uint, 3> vSize3RegionPos ; 
	bmStone.bitScanAll(vSize3RegionPos) ;
	for( uint iPos = 0 ; iPos < vSize3RegionPos.size() ; ++iPos ) {
		int regionPos = vSize3RegionPos[iPos] ;
		WeichiGrid& grid = m_board.getGrid(regionPos) ;
		if( grid.getColor() == COLOR_NONE && grid.getPattern().getSize3FalseEye(liveColor, grid.getColor()) )  {
			WeichiMove invadeMove(oppColor, regionPos) ; 
			if( !m_board.isIllegalMove(invadeMove, m_ht) && m_board.getStoneAfterPlay(invadeMove) == 2 ) { return false ; }
		}
		else if( grid.getColor() == oppColor && grid.getPattern().getSize3FalseEye(liveColor, grid.getColor()) ) {
			WeichiBlock* oppBlock = grid.getBlock() ;
			if( oppBlock->getNumStone() == 2 ) { 
				if( grid.getPattern().getSize3FalseEyeCheckDir(liveColor, grid.getColor()) ) {
					uint patternDir8 = grid.getPattern().getSize3FalseEyeCheckDir(liveColor, grid.getColor()) ;
					uint realDir = StaticBoard::getPattern3x3Direction(patternDir8)[0] ;		
					int checkPos = m_board.getGrid(grid, realDir).getPosition() ;					
					WeichiGrid& checkGrid = m_board.getGrid(checkPos) ;
					if( checkGrid.getColor() == COLOR_NONE ) {
						WeichiMove invadeMove(oppColor, checkPos) ;
						if( m_board.getLibertyAfterPlay(invadeMove) > 2 ) { return false ; }
						else if( m_board.getLibertyAfterPlay(invadeMove) == 1 ) {
							if( m_board.isKoEatPlay(invadeMove) ) { return false ; }
							else if( m_kb.isMakeKo(m_board, invadeMove) ) { return false ; }									
						}
					} else if( checkGrid.getColor() == oppColor ) {
						WeichiBlock* suckBlock = checkGrid.getBlock() ;
						if( suckBlock->getLiberty() == 1 && !m_lifedeathHandler->getKnowledge().is1libBlockCanEscape(suckBlock) ) {
							//m_lifeDeathKB.compute1libBlockEscapeRZone(suckBlock, rzoneSet) ;
						} else if( suckBlock->getLiberty() == 2 && !m_lifedeathHandler->getKnowledge().is2libBlockCanEscape(suckBlock) )  { 
							//m_lifeDeathKB.compute2libBlockEscapeRZone(suckBlock, rzoneSet) ;
						} else { return false ;	}
					}
				}
				else { return false ; }
			}
			else if( oppBlock->getNumStone() == 1 && oppBlock->getLiberty() == 1 ) {
				int lastLib = oppBlock->getLastLiberty(m_board.getBitBoard()) ;
				WeichiMove invadeMove(oppColor, lastLib) ; 
				if( !m_board.isIllegalMove(invadeMove, m_ht) && m_board.getStoneAfterPlay(invadeMove) == 2 ) { return false ; }
			}
		}
	}

	return true ;
}

bool WeichiEyeHandler::judgeOpenedAreaTrueEyeByRule( WeichiOpenedArea * openedarea )
{
	Color liveColor = openedarea->getColor() ;
	Color oppColor = AgainstColor(liveColor) ;

	// Use connector to judge it true eye or not.	
	WeichiBitBoard bmSurround = getSurroundedBlocksBitboard(openedarea) ;
	WeichiBlock* firstBlock = m_board.getBlock(openedarea->getBlockID(0)) ;	
	bmSurround -= firstBlock->getStoneMap() ;	
	vector<WeichiConnector*> vConnecotrs ;
	vector<WeichiConnector*>& connectors = m_lifedeathHandler->getConnectorHandler().getBlockConnectors(firstBlock) ;

	bool checkConnectors[MAX_NUM_CONNECTORS] ;
	for( uint iResetCheck = 0 ; iResetCheck < MAX_NUM_CONNECTORS ; ++iResetCheck ) { checkConnectors[iResetCheck] = false ; }
	
	WeichiBitBoard bmRZone ;
	WeichiBitBoard bmRelevant ;
	for( uint iConn=0 ; iConn < connectors.size() ; ++iConn ) { 
		checkConnectors[connectors[iConn]->GetID()] = true ;
		bmRZone |= connectors[iConn]->getThreatBitmap() ;
		//bmRZone.addLevelRZone(2, connectors[iConn]->getThreatBitmap()) ;		
		vConnecotrs.push_back(connectors[iConn]) ; 
	}	
	
	while( vConnecotrs.size() != 0 && !bmSurround.empty() ) {
		WeichiConnector* conn = vConnecotrs.back() ;
		WeichiBitBoard bmThreat = conn->getThreatBitmap() ;
		int threatPos = 0 ;
		while( (threatPos=bmThreat.bitScanForward()) != -1 ) {
			WeichiMove connectMove(liveColor, threatPos) ;
			if( !m_board.isIllegalMove(connectMove, m_ht) ) { bmRelevant |= m_board.getStoneBitBoardAfterPlay(connectMove) ; }
		}		
				
		// Use all connector to connect.
		vConnecotrs.pop_back() ;		
		Vector<short, 4> vBlocksIds = conn->getBlockIDs() ;
		for( uint iBlockId=0 ; iBlockId < vBlocksIds.size() ; ++iBlockId ) {
			WeichiBlock* connectBlock = m_board.getBlock(vBlocksIds[iBlockId]) ;					
			if( !bmSurround.hasIntersection(connectBlock->getStoneMap()) ) { continue ; }

			bmSurround -= connectBlock->getStoneMap() ; 				
			vector<WeichiConnector*>& connectors = m_lifedeathHandler->getConnectorHandler().getBlockConnectors(connectBlock) ;
			for( uint iConn=0 ; iConn < connectors.size() ; ++iConn ) { 
				if( checkConnectors[connectors[iConn]->GetID()] ) { continue ; }			
				if( connectors[iConn]->isWeak() ) { continue ; }

				checkConnectors[connectors[iConn]->GetID()] = true ;
				bmRZone |= connectors[iConn]->getThreatBitmap() ;
				// bmRZone.addLevelRZone(2, connectors[iConn]->getThreatBitmap()) ;		
				// 針對TigerMouth加入KoRZone
				if( connectors[iConn]->getType() == TYPE_TIGERMOUTH ) {
					int tigerMouthPos = connectors[iConn]->getThreatBitmap().bitScanForward() ;						
					const WeichiGrid& tigetMouthGrid = m_board.getGrid(tigerMouthPos) ;						
					//m_lifeDeathKB.computeKoRZoneByPattern(tigetMouthGrid, liveColor, rzoneSet) ;
				}
				vConnecotrs.push_back(connectors[iConn]) ; 		
			}
		}
	}

	if( !bmSurround.empty() ) { return false ; }

	bmRZone |= getSurroundedBlocksBitboard(openedarea) ;
	bmRZone |= openedarea->getStoneMap() ;
	openedarea->setRZone(bmRZone) ;
	openedarea->setEyeStatus(CONNECTOR_TRUE_EYE) ;
	//closedarea->addRelevantArea(bmRelevant) ;

	return true ; 	
}

bool WeichiEyeHandler::isSize1RegionEnoughProtectedCorner( WeichiClosedArea* closedarea, uint numProtected, Vector<uint, 4>& vEmptyNonProtectedCorners ) 
{
	WeichiBitBoard bmStone = closedarea->getStoneMap() ;
	int regionPos = bmStone.bitScanForward() ;
	const WeichiGrid& grid = m_board.getGrid(regionPos) ;	

	if( grid.getStaticGrid().getLineNo() >= 2 )	{
		if( numProtected >= 3 ) { return true ; }
		else if( numProtected == 2 && !vEmptyNonProtectedCorners.empty() ) {
			int firstFixPos = vEmptyNonProtectedCorners[0];
			//closedarea->setFormEyePos(firstFixPos) ;
		}
	}
	else if( grid.getStaticGrid().isEdge() ) { 
		if( numProtected == 2 ) { return true ; }
		else if( numProtected == 1 && !vEmptyNonProtectedCorners.empty() ) {
			int firstFixPos = vEmptyNonProtectedCorners[0];
			//closedarea->setFormEyePos(firstFixPos) ;
		}
	}
	else if( grid.getStaticGrid().isCorner() )	{ 
		if( numProtected == 1 ) { return true ; }
		else if( numProtected == 0 && !vEmptyNonProtectedCorners.empty() ) {
			int firstFixPos = vEmptyNonProtectedCorners[0] ;
			//closedarea->setFormEyePos(firstFixPos) ;
		}		
	}

	return false ;
}

bool WeichiEyeHandler::isSize2RegionEnoughProtectedCorner( Vector<uint, 2>& vRegionPos, uint numProtected, Vector<uint, 4>& vNonProtected )
{
	const StaticGrid& firstGrid = m_board.getGrid(vRegionPos[0]).getStaticGrid() ;
	const StaticGrid& secondGrid = m_board.getGrid(vRegionPos[1]).getStaticGrid() ;

	if( firstGrid.getLineNo() >= 2 && secondGrid.getLineNo() >= 2 && numProtected == 3 ) { return true ; }
	else if (firstGrid.getLineNo() >= 2 && secondGrid.getLineNo() >= 2 && numProtected == 2) {
		// If the two non-protected grids are in the same short side, then it is not safe.		
		// OOO*
		// O..O
		// OOO*
			
		// Dirty code, figure out how to fix it.
		WeichiGrid& nonProtectedGrid1 = m_board.getGrid(vNonProtected[0]) ;
		WeichiGrid& nonProtectedGrid2 = m_board.getGrid(vNonProtected[1]);
		int numDiff = 0 ;
		if (nonProtectedGrid1.getStaticGrid().x() == nonProtectedGrid2.getStaticGrid().x() ) {
			numDiff = nonProtectedGrid1.getStaticGrid().y() - nonProtectedGrid2.getStaticGrid().y();
			numDiff = numDiff < 0 ? numDiff = 0 - numDiff : numDiff ;
		}
		else if (nonProtectedGrid1.getStaticGrid().y() == nonProtectedGrid2.getStaticGrid().y() ) {
			numDiff = nonProtectedGrid1.getStaticGrid().x() - nonProtectedGrid2.getStaticGrid().x();
			numDiff = numDiff < 0 ? numDiff = 0 - numDiff : numDiff;
		}
		// Only diff=2 is not safe.
		if (numDiff == 2) { return false; }
		return true ;
	}
	else if (firstGrid.isEdge() && secondGrid.getLineNo() >= 2 && numProtected >= 1 ) { return true; }
	else if (firstGrid.getLineNo() >= 2 && secondGrid.isEdge() && numProtected >= 1 ) { return true; }
	else if (firstGrid.isEdge() && secondGrid.isEdge() && numProtected == 2 ) { return true; }
	else if ((firstGrid.isCorner()||secondGrid.isCorner()) && numProtected == 1 ) { return true; }

	return false ;
}


bool WeichiEyeHandler::isProtectedCorner( WeichiClosedArea* checkedCA, WeichiGrid& diagGrid, WeichiBitBoard& bmRZone ) 
{
	Color ownColor = checkedCA->getColor() ;
	Color oppColor = AgainstColor(checkedCA->getColor()) ;

	if (diagGrid.getColor() == ownColor) { return true; }
	else if (diagGrid.getColor() == COLOR_NONE) {	
		if (!isSafeEmptyCorner(checkedCA, WeichiMove(oppColor, diagGrid.getPosition()))) { return false; }
		// Next compute the R-zone.
		WeichiMove invadeMove(oppColor, diagGrid.getPosition());
		WeichiMove connectMove(ownColor, diagGrid.getPosition());		
		if (m_board.getLibertyAfterPlay(invadeMove) == 0) {			
			bmRZone.SetBitOn(diagGrid.getPosition());			
		} else if (m_board.getLibertyAfterPlay(invadeMove) == 1) {			
			bmRZone.SetBitOn(diagGrid.getPosition());
			bmRZone |= m_board.getStoneBitBoardAfterPlay(connectMove);
			bmRZone |= m_board.getStoneBitBoardAfterPlay(invadeMove);						
			bmRZone |= m_board.getLibertyBitBoardAfterPlay(invadeMove);						
			int lastLib = m_board.getLastLibertyPositionAfterPlay(invadeMove);
			WeichiMove captureMove(ownColor, lastLib);
			if (m_board.getGrid(lastLib).getColor() == COLOR_NONE) { bmRZone |= m_board.getStoneBitBoardAfterPlay(captureMove); }
			m_lifedeathHandler->getKnowledge().computeKoRZoneByPattern(diagGrid, ownColor, bmRZone);			
		}			
		bmRZone |= m_board.getStoneBitBoardAfterPlay(connectMove);
		return true;
	} else if (diagGrid.getColor() == oppColor) {
		const WeichiBlock* oppBlock = diagGrid.getBlock() ;		
		if (oppBlock->getLiberty() == 1) {
			if (m_lifedeathHandler->getKnowledge().is1libBlockCanEscape(oppBlock)) { return false; }
			WeichiMove escapeMove(oppColor, oppBlock->getLastLiberty(m_board.getBitBoard()));
			bmRZone |= m_board.getStoneBitBoardAfterPlay(escapeMove) ;
			m_lifedeathHandler->getKnowledge().compute1libBlockEscapeRZone(oppBlock, bmRZone);						
			return true ;
		} else if (oppBlock->getLiberty() == 2) {			
			if (m_lifedeathHandler->getKnowledge().is2libBlockCanEscape(oppBlock)) { return false; }						
			m_lifedeathHandler->getKnowledge().compute2libBlockEscapeRZone(oppBlock, bmRZone);			
			return true ;
		}
	}

	return false ;
}

bool WeichiEyeHandler::isSafeEmptyCorner( WeichiClosedArea* checkedCA, WeichiMove killMove)
{
	assert(m_board.getGrid(killMove).getColor() == COLOR_NONE);

	// Four situation not safe.
	// 1. Two-lib above after invading
	// 2. Two-lib below after connecting
	// 3. one-lib invade and eat ko.
	// 4. one-lib invade and make ko.

	Color killColor = killMove.getColor();
	Color liveColor = AgainstColor(killMove.getColor());
	WeichiMove invadeMove(killColor, killMove.getPosition());
	WeichiMove connectMove(liveColor, killMove.getPosition());

	// Precondtion
	if (m_board.getLibertyAfterPlay(invadeMove) >= 2) { return false; }

	// Check Ko Move
	if (m_board.getLibertyAfterPlay(invadeMove) == 1) {
		// 1. 檢查是否為打劫。
		if ( m_board.isKoEatPlay(invadeMove) || m_kb.isMakeKo(m_board, invadeMove) ) {  
			// 檢查是否可以blocking			
			int lastLib = m_board.getLastLibertyPositionAfterPlay(invadeMove) ;
			if (m_board.isIllegalMove(WeichiMove(killColor, lastLib), m_ht)) { return false; }
			WeichiBitBoard bmEscapeLib = m_board.getLibertyBitBoardAfterPlay(WeichiMove(killColor, lastLib)) ;
			if( bmEscapeLib.bitCount() == 2 ) { 
				bmEscapeLib.SetBitOff(killMove.getPosition()) ;
				int blockingPos = bmEscapeLib.bitScanForward() ;
				WeichiMove blockingMove(liveColor, blockingPos);
				if ( m_board.getLibertyAfterPlay(blockingMove) == 2 ) { return false ; }
				else { return true ; }
			}
			return false ;  
		}
		
		// 2. 檢查是否為可以滾打的Pattern		
		// XXXXXXX
		// ..XOOXX
		// ..O.OOO
		// X.AOXX.
		//if ( invadeGrid.getPattern().getCheckProectedConnected2lib(liveColor, COLOR_NONE) ) {
		//	WeichiBitBoard bmLibAfterPlay = m_board.getLibertyBitBoardAfterPlay(invadeMove) ;
		//	int lastLib = bmLibAfterPlay.bitScanForward();
		//	WeichiMove captureMove(liveColor, lastLib);
		//	m_board.setPlayMediumStatus(true);
		//	m_board.play(captureMove);
		//	bool lib2AfterConnectSafe = true ;
		//	WeichiBitBoard bm2libAfterConnect = m_board.getLibertyBitBoardAfterPlay(connectMove) ;		
		//	if( bm2libAfterConnect.bitCount() == 2 ) {
		//		Vector<uint, 2> v2Libs ;
		//		bm2libAfterConnect.bitScanAll(v2Libs) ;
		//		for( uint iLib=0 ; iLib < 2 ; ++iLib ) {
		//			int libPos = v2Libs[iLib] ;
		//			WeichiMove atkMove(invadeColor, libPos) ;	
		//			if( m_board.getLibertyAfterPlay(atkMove) >= 2 ) {
		//				lib2AfterConnectSafe = false ;
		//				break ;
		//			}
		//		}
		//	}
		//	m_board.undoMiniSearch();
		//	m_board.setPlayMediumStatus(false) ;
		//	if ( !lib2AfterConnectSafe ) { return false ; }
		//}
	}	

	// Check Peep Move
	if (m_board.getLibertyAfterPlay(connectMove) == 1) { return false; }
	else if (m_board.getLibertyAfterPlay(connectMove) == 2 ) { 
		WeichiBitBoard bmLibs = m_board.getLibertyBitBoardAfterPlay(connectMove) ;
		Vector<uint, 2> vlibertyPosistions;
		bmLibs.bitScanAll(vlibertyPosistions);
		bool bSuccessOpponentPeep = false;
		for (uint iPos=0 ; iPos < vlibertyPosistions.size() ; ++iPos) {
			WeichiMove peepMove(killColor, vlibertyPosistions[iPos]);
			if (!m_board.isIllegalMove(peepMove, m_ht) && m_board.getLibertyAfterPlay(peepMove) >= 2) { 
				bSuccessOpponentPeep = true;
				break;
			}					
		}
		if (bSuccessOpponentPeep) { return false; }
		else { return true; }
	}

	return true ;
}

WeichiBitBoard WeichiEyeHandler::getSurroundedBlocksBitboard( WeichiClosedArea* closedarea ) const
{
	WeichiBitBoard bmSurroundBlocks ;
	for (uint iBlock = 0 ; iBlock < closedarea->getNumBlock() ; ++iBlock) {
		bmSurroundBlocks |= m_board.getBlock(closedarea->getBlockID(iBlock))->getStoneMap() ;
	}

	return bmSurroundBlocks ;
}

WeichiBitBoard WeichiEyeHandler::getSurroundedBlocksBitboard( WeichiOpenedArea* openedarea ) const
{
	WeichiBitBoard bmSurroundBlocks ;
	for (uint iBlock = 0 ; iBlock < openedarea->getNumBlock() ; ++iBlock) {
		bmSurroundBlocks |= m_board.getBlock(openedarea->getBlockID(iBlock))->getStoneMap() ;
	}

	return bmSurroundBlocks ;
}

WeichiBitBoard WeichiEyeHandler::getClosedAreaNeighborBlockRZone(const WeichiClosedArea* closedarea) const
{
	WeichiBitBoard bmUpdateRZone;
	for (uint iBlock = 0 ; iBlock < closedarea->getNumBlock() ; ++iBlock) {
		const WeichiBlock* block = m_board.getBlock(closedarea->getBlockID(iBlock));		
		WeichiBitBoard bmLiberty = block->getLibertyBitBoard(m_board.getBitBoard());
		// If already 2-lib in R-zone, don't add
		int nIntersection = (bmLiberty & closedarea->getRZone()).bitCount();		
		if (nIntersection < 2) {
			WeichiBitBoard bmRemaining = bmLiberty - closedarea->getRZone();
			if (nIntersection == 1 && bmRemaining.bitCount() >= 1) { 
				bmUpdateRZone.SetBitOn(bmRemaining.bitScanForward());
			} else if (nIntersection == 0) {
				if (bmRemaining.bitCount() == 1) {
					bmUpdateRZone.SetBitOn(bmRemaining.bitScanForward());
				} else if (bmRemaining.bitCount() >= 2) {
					bmUpdateRZone.SetBitOn(bmRemaining.bitScanForward());
					bmUpdateRZone.SetBitOn(bmRemaining.bitScanForward());
				}
			}
		}
	}	

	return bmUpdateRZone;
}

WeichiBitBoard WeichiEyeHandler::getClosedAreaSuicideBlockRZone(const WeichiClosedArea * closedarea) const
{
	Color ownColor = closedarea->getColor();
	Color oppColor = AgainstColor(closedarea->getColor());

	WeichiBitBoard bmOriginalRZone = closedarea->getRZone();
	WeichiBitBoard bmUpdateRZone;
	int pos = 0;
	while ( (pos=bmOriginalRZone.bitScanForward()) != -1) {
		const WeichiGrid& grid = m_board.getGrid(pos);		
		const WeichiBlock* block = grid.getBlock();		
		if (block) { 
			bmOriginalRZone -= block->getStoneMap(); 
			continue;
		}
		
		WeichiMove suicideMove(oppColor, pos);
		if (m_board.getLibertyAfterPlay(suicideMove) >= 1) { continue; }
				
		WeichiBitBoard bmDeadStone = m_board.getStoneBitBoardAfterPlay(suicideMove) ;
		WeichiBitBoard bmNbrBlock = bmDeadStone.dilate() & StaticBoard::getMaskBorder() & m_board.getStoneBitBoard(ownColor) ;		
		bmUpdateRZone |= bmNbrBlock ;
		bmUpdateRZone |= bmDeadStone ;
	}

	return bmUpdateRZone;
}

WeichiBitBoard WeichiEyeHandler::getClosedAreaInvolvedBlockRZone(const WeichiClosedArea * closedarea) const
{
	WeichiBitBoard bmOriginalRZone = closedarea->getRZone();
	WeichiBitBoard bmUpdateRZone;
	int pos = 0;
	while ( (pos=bmOriginalRZone.bitScanForward()) != -1) {
		const WeichiGrid& grid = m_board.getGrid(pos);		
		const WeichiBlock* block = grid.getBlock();		
		if (block) {
			if (block->getColor() == closedarea->getColor()) {
				WeichiBitBoard bmLiberty = block->getLibertyBitBoard(m_board.getBitBoard());
				// If already 2-lib in R-zone, don't add
				int nIntersection = (bmLiberty & closedarea->getRZone()).bitCount();		
				if (nIntersection < 2) {
					WeichiBitBoard bmRemaining = bmLiberty - closedarea->getRZone();
					if (nIntersection == 1 && bmRemaining.bitCount() >= 1) { 
						bmUpdateRZone.SetBitOn(bmRemaining.bitScanForward());
					} else if (nIntersection == 0) {
						if (bmRemaining.bitCount() == 1) {
							bmUpdateRZone.SetBitOn(bmRemaining.bitScanForward());
						} else if (bmRemaining.bitCount() >= 2) {
							bmUpdateRZone.SetBitOn(bmRemaining.bitScanForward());
							bmUpdateRZone.SetBitOn(bmRemaining.bitScanForward());
						}
					}
				}
			}
			bmOriginalRZone -= block->getStoneMap();
		}
	}

	return bmUpdateRZone;
}

bool WeichiEyeHandler::isLinearPattern4Live( WeichiClosedArea* closedarea ) 
{
	if (closedarea->getNumStone() != 4) { return false ; }

	Color ownColor = closedarea->getColor() ;
	Color invadeColor = AgainstColor(ownColor) ;

	WeichiBitBoard bmStone = closedarea->getStoneMap() ;
	int pos = 0 ;
	uint numSplit = 0 ;
	bool bAllConcret = true ;
	while( (pos=bmStone.bitScanForward()) != -1 ) {
		WeichiGrid& grid = m_board.getGrid(pos);
		if ( grid.getColor() != COLOR_NONE ) { return false; }
		WeichiMove invadeMove(invadeColor, pos) ;
		// if the atari move is safe (>= 2lib), then return false.
		if (m_board.isAtariMove(invadeMove) && m_board.getLibertyAfterPlay(invadeMove) >= 2) { return false; }
		if (grid.getPattern().getIsSplitCA(ownColor, ownColor)) { numSplit++; }
		if (!m_lifedeathHandler->getKnowledge().isConcretPointInRegion(grid, closedarea)) { bAllConcret = false; }
	}
	
	if (numSplit != 2) { return false ; } // Can avoid square-4 dead pattern.

	// Check the CA is healthy to the neighboring blocks, to avoid Corner-Bent-Four.
	WeichiBitBoard bmNonNbr = closedarea->getStoneMap() ;	
	for ( uint iBlock = 0; iBlock < closedarea->getNumBlock(); ++iBlock ) 
		bmNonNbr -= m_board.getBlock(closedarea->getBlockID(iBlock))->getNbrMap() ;
	if ( !bmNonNbr.empty() ) { return false ; }

	// Further check the neighboring block can connect or not.
	// Only check the region has only 2 blocks at it neighboring.
	if ( closedarea->getNumBlock() > 2 ) { return false; }
	else if ( closedarea->getNumBlock() == 2 ) {
		WeichiBitBoard bmConn = StaticBoard::getMaskBorder();
		for ( uint iBlock = 0; iBlock < closedarea->getNumBlock(); ++iBlock ) {
			WeichiBlock* block = m_board.getBlock(closedarea->getBlockID(iBlock));
			// Only collect liberties.
			bmConn &= (block->getNbrMap() - m_board.getBitBoard()); 
		}
		WeichiBitBoard bmOuterLibConn = bmConn - closedarea->getStoneMap();
		if( bmOuterLibConn.bitCount() >= 2 ) { return false ; }
		else if ( bmOuterLibConn.bitCount() == 1 ) { 
			// If outer-lib check has only 1 point, then futher check it.
			int checkPos = bmOuterLibConn.bitScanForward();
			WeichiMove invadeMove(invadeColor, checkPos);
			if (m_board.getLibertyAfterPlay(invadeMove) >= 2) { return false; }
			if (m_lifedeathHandler->getKnowledge().isKoMove(invadeMove)) { return false ; }
			// liberty <= 1, add the rzone and the relevant block.
			WeichiBitBoard bmRZone;
			if ( m_board.getLibertyAfterPlay(invadeMove) == 1 ) { bmRZone |= m_board.getLibertyBitBoardAfterPlay(invadeMove); }
			bmRZone.SetBitOn(checkPos);
			closedarea->setRZone(bmRZone);
			//closedarea->addRelevantArea(m_board.getStoneBitBoardAfterPlay(WeichiMove(ownColor, checkPos)));
		} 
		else if( bmOuterLibConn.bitCount() == 0 ) {
			// It means it can only attack inside, make sure it is all concret.
			if ( !bAllConcret ) { return false ; }
		}		
	}

	WeichiBitBoard bmExtraZone ;
	bmExtraZone |= getSurroundedBlocksBitboard(closedarea) ;
	bmExtraZone |= closedarea->getStoneMap() ;
	closedarea->addRZone(bmExtraZone) ;
	closedarea->setEyeStatus(ONE_REGION_ENOUGH_LIVE) ;	
	return true ;
}

void WeichiEyeHandler::postUpdateRZone(WeichiClosedArea* closedarea)
{	
	closedarea->addRZone(getClosedAreaNeighborBlockRZone(closedarea));
	closedarea->addRZone(getClosedAreaSuicideBlockRZone(closedarea));
	closedarea->addRZone(getClosedAreaInvolvedBlockRZone(closedarea));

	return;
}

bool WeichiEyeHandler::isConcreteGrid(const WeichiGrid& grid, const WeichiClosedArea* closedarea) const
{
	const int* diag = grid.getStaticGrid().getDiagonalNeighbors();
	WeichiBitBoard bmRegion = closedarea->getStoneMap();

	int nProteced = 0;
	for ( ; *diag != -1 ; ++diag ) {
		WeichiGrid& diagGrid = m_board.getGrid(*diag);
		if (bmRegion.BitIsOn(*diag)) { nProteced++; }
		else if (diagGrid.getColor() == closedarea->getColor()) { nProteced++; }
	}

	if (grid.getStaticGrid().getLineNo() >= 2 && nProteced >= 3) { return true; }
	else if (grid.getStaticGrid().isEdge() && nProteced >= 2) { return true; }
	else if (grid.getStaticGrid().isCorner() && nProteced >= 1) { return true; }

	return false;
} 
