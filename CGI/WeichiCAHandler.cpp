#include "WeichiCAHandler.h"
#include "WeichiBoard.h"
#include "WeichiDynamicKomi.h"

WeichiCAHandler::WeichiCAHandler()
	: m_board(NULL)
{
	reset();
}

bool WeichiCAHandler::isGameEnd() const
{
	if (WeichiConfigure::dcnn_use_ftl) {
		return isLADEndGame();
	} else {
		return false;
		return m_numLifeStone.get(COLOR_BLACK) >= WeichiDynamicKomi::BlackWinStoneNum
			|| m_numLifeStone.get(COLOR_WHITE) >= WeichiDynamicKomi::WhiteWinStoneNum
			|| m_numLifeStone.get(COLOR_BLACK) + m_numLifeStone.get(COLOR_WHITE) == WeichiConfigure::TotalGrids;
	}
}

bool WeichiCAHandler::isEnoughTerritoryGameEnd() const
{
	int numBlackWinStones = (WeichiConfigure::TotalGrids+WeichiConfigure::komi)/2 + 1;
	int numWhiteWinStones = (WeichiConfigure::TotalGrids-WeichiConfigure::komi)/2 + 1;

	return m_numLifeStone.get(COLOR_BLACK)>=numBlackWinStones|| m_numLifeStone.get(COLOR_WHITE)>=numWhiteWinStones ;	
}

bool WeichiCAHandler::isEnoughTerritory( Color winColor ) const
{
	int numBlackWinStones = (WeichiConfigure::TotalGrids+WeichiConfigure::komi)/2 + 1;
	int numWhiteWinStones = (WeichiConfigure::TotalGrids-WeichiConfigure::komi)/2 + 1;

	if( winColor == COLOR_BLACK ) { return m_numLifeStone.get(COLOR_BLACK)>=numBlackWinStones ; }	
	else if( winColor == COLOR_WHITE ) { return m_numLifeStone.get(COLOR_WHITE)>=numWhiteWinStones ; }

	return false ;
}

void WeichiCAHandler::initialize( WeichiBoard* board )
{
	m_board = board ;
}

void WeichiCAHandler::reset()
{
	m_numLifeStone.reset();
	m_bmLife.reset();
	m_closedAreas.reset();
	m_nakadeLists.m_black.clear();
	m_nakadeLists.m_white.clear();
}

void WeichiCAHandler::setBackupFlag()
{
	m_backupFlag.m_numLifeStone = m_numLifeStone;
	m_backupFlag.m_bmLife = m_bmLife;
	m_backupFlag.m_nakadeLists = m_nakadeLists;
	m_backupFlag.m_closedAreas = m_closedAreas;
}

void WeichiCAHandler::restoreBackupFlag()
{
	m_numLifeStone = m_backupFlag.m_numLifeStone;
	m_bmLife = m_backupFlag.m_bmLife;
	m_nakadeLists = m_backupFlag.m_nakadeLists;
	m_closedAreas = m_backupFlag.m_closedAreas;
}

void WeichiCAHandler::storeMoveBackup() 
{
	BackupFlag backupFlag ;
	backupFlag.m_numLifeStone = m_numLifeStone;
	backupFlag.m_bmLife = m_bmLife;
	backupFlag.m_nakadeLists = m_nakadeLists;
	backupFlag.m_closedAreas = m_closedAreas; 
	
	m_backupStacks.push_back(backupFlag) ;
	
	return ;
}

void WeichiCAHandler::resumeMoveBackup()
{
	BackupFlag& backupFlag = m_backupStacks.back() ;
	m_numLifeStone = backupFlag.m_numLifeStone;
	m_bmLife = backupFlag.m_bmLife;
	m_nakadeLists = backupFlag.m_nakadeLists;
	m_closedAreas = backupFlag.m_closedAreas;

	m_backupStacks.pop_back() ;

	return ;
}

WeichiCAHandler::BackupFlag WeichiCAHandler::getCurrentBackupFlag()
{
	struct BackupFlag backupFlag;
	backupFlag.m_numLifeStone = m_numLifeStone;
	backupFlag.m_bmLife = m_bmLife;
	backupFlag.m_nakadeLists = m_nakadeLists;
	backupFlag.m_closedAreas = m_closedAreas;

	return backupFlag ;
}

float WeichiCAHandler::eval( Vector<Color,MAX_NUM_GRIDS>& territory ) const
{
	if( m_board->isEarlyEndGameStatus() ) { return earlyEndeval(territory); }
	else { return gameEndeval(territory); }
}

float WeichiCAHandler::gameEndeval( Vector<Color,MAX_NUM_GRIDS>& territory ) const
{
	Dual<float> numTerritory;
	numTerritory.get(COLOR_BLACK) = 0.0f;
	numTerritory.get(COLOR_WHITE) = WeichiDynamicKomi::Internal_komi;

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiGrid& grid = m_board->getGrid(*it);
		if( grid.isEmpty() ) {
			const WeichiClosedArea* blackCA = grid.getClosedArea(COLOR_BLACK);
			const WeichiClosedArea* whiteCA = grid.getClosedArea(COLOR_WHITE);
			if( blackCA && blackCA->getStatus()==LAD_LIFE ) {
				assertToFile( !whiteCA || whiteCA->getStatus()!=LAD_LIFE, m_board );
				territory[*it] = COLOR_BLACK;
				++numTerritory.get(COLOR_BLACK);
			} else if( whiteCA && whiteCA->getStatus()==LAD_LIFE ){
				assertToFile( !blackCA || blackCA->getStatus()!=LAD_LIFE, m_board );
				territory[*it] = COLOR_WHITE;
				++numTerritory.get(COLOR_WHITE);
			} else {
				if( grid.getPattern().getAdjGridCount(COLOR_BLACK)==0 ) {
					territory[*it] = COLOR_WHITE;
					++numTerritory.get(COLOR_WHITE);
				} else if( grid.getPattern().getAdjGridCount(COLOR_WHITE)==0 ) {
					territory[*it] = COLOR_BLACK;
					++numTerritory.get(COLOR_BLACK);
				} else {
					territory[*it] = COLOR_NONE;
				}
			}
		} else {
			Color oppColor = AgainstColor(grid.getColor());
			const WeichiBlock* block = grid.getBlock();
			const WeichiClosedArea* oppCA = grid.getClosedArea(oppColor);
			if( oppCA && oppCA->getStatus()==LAD_LIFE ) {
				assertToFile( block->getStatus()!=LAD_LIFE, m_board );
				territory[*it] = oppColor;
				++numTerritory.get(oppColor);
			} else {
				territory[*it] = block->getColor();
				++numTerritory.get(block->getColor());
			}
		}
	}

	return (numTerritory.get(COLOR_BLACK) - numTerritory.get(COLOR_WHITE));
}

float WeichiCAHandler::earlyEndeval( Vector<Color,MAX_NUM_GRIDS>& territory ) const
{
	Dual<float> numTerritory;
	numTerritory.get(COLOR_BLACK) = 0.0f;
	numTerritory.get(COLOR_WHITE) = WeichiDynamicKomi::Internal_komi;

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiGrid& grid = m_board->getGrid(*it);
		if( grid.isEmpty() ) {
			const WeichiClosedArea* blackCA = grid.getClosedArea(COLOR_BLACK);
			const WeichiClosedArea* whiteCA = grid.getClosedArea(COLOR_WHITE);
			if( blackCA && blackCA->getStatus()==LAD_LIFE ) {
				assertToFile( !whiteCA || whiteCA->getStatus()!=LAD_LIFE, m_board );
				territory[*it] = COLOR_BLACK;
				++numTerritory.get(COLOR_BLACK);
			} else if( whiteCA && whiteCA->getStatus()==LAD_LIFE ){
				assertToFile( !blackCA || blackCA->getStatus()!=LAD_LIFE, m_board );
				territory[*it] = COLOR_WHITE;
				++numTerritory.get(COLOR_WHITE);
			} else {
				// no one owns this, do not record this position
				territory[*it] = COLOR_NONE;
			}
		} else {
			Color oppColor = AgainstColor(grid.getColor());
			const WeichiBlock* block = grid.getBlock();
			const WeichiClosedArea* oppCA = grid.getClosedArea(oppColor);
			if( oppCA && oppCA->getStatus()==LAD_LIFE ) {
				assertToFile( block->getStatus()!=LAD_LIFE, m_board );
				territory[*it] = oppColor;
				++numTerritory.get(oppColor);
			} else if( block->getStatus()==LAD_LIFE ) {
				territory[*it] = block->getColor();
				++numTerritory.get(block->getColor());
			} else {
				// no one owns this, do not record this position
				territory[*it] = COLOR_NONE;
			}
		}
	}

	assertToFile( (numTerritory.get(COLOR_BLACK)>=WeichiDynamicKomi::BlackWinStoneNum ||
				   numTerritory.get(COLOR_WHITE)>=WeichiDynamicKomi::WhiteWinStoneNum ||
				   numTerritory.get(COLOR_BLACK)+numTerritory.get(COLOR_WHITE)==WeichiConfigure::TotalGrids), m_board );

	return (numTerritory.get(COLOR_BLACK) - numTerritory.get(COLOR_WHITE));
}

void WeichiCAHandler::removeAllClosedArea()
{
	// remove all closed area
	for( uint iIndex=0; iIndex<m_closedAreas.getCapacity(); iIndex++ ) {
		if( !m_closedAreas.isValidIdx(iIndex) ) { continue; }
		WeichiClosedArea* closedArea = m_closedAreas.getAt(iIndex);
		removeClosedArea(closedArea);
	}
	for( uint iIndex=0; iIndex<m_board->m_blockList.getCapacity(); iIndex++ ) {
		if( !m_board->m_blockList.isValidIdx(iIndex) ) { continue; }
		if( !m_board->m_blockList.getAt(iIndex)->isUsed() ) { continue; }
		m_board->m_blockList.getAt(iIndex)->setStatus(LAD_NOT_EYE);
	}
	reset();
}

void WeichiCAHandler::updateClosedArea( WeichiGrid& grid, BoardMoveStack& moveStack )
{
	// when put one stone, the possible closed area will be:
	// 1. put stone in own CA, use pattern to check
	//		a. no split CA --> remove grid from CA
	//		b. split CA --> remove current CA & find CA again
	// 2. put stone not in own CA --> find CA again
	bool bNeedReLookCA = true;
	Color gridColor = grid.getColor();
	Color oppColor = AgainstColor(gridColor);
	WeichiClosedArea *myClosedArea = grid.getClosedArea(gridColor);
	WeichiClosedArea* oppClosedArea = grid.getClosedArea(oppColor);

	m_board->m_status.m_bLastHasModifyClosedArea = false;

	if( myClosedArea ) {
		if( !grid.getPattern().getIsSplitCA(gridColor,gridColor) ) {
			if( myClosedArea->getNumStone()==1 ) {
				removeClosedArea(myClosedArea);
			} else { 
				m_board->m_status.m_bLastHasModifyClosedArea = true;
				removeGridFromClosedArea(grid,myClosedArea);
			}
			bNeedReLookCA = false;
		} else {
			removeClosedArea(myClosedArea);
			m_board->m_status.m_bLastHasModifyClosedArea = true;
		}
	}

	if( oppClosedArea ) {
		if( oppClosedArea->getNakade()==grid.getPosition() ) {
			assertToFile( m_nakadeLists.get(oppColor).contains(oppClosedArea->getNakade()), m_board );
			m_nakadeLists.get(oppColor).removeFeature(oppClosedArea->getNakade());
			oppClosedArea->clearNakade();
		} else {
			reCheckClosedAreaNakade(oppClosedArea);
		}
	}

	if( bNeedReLookCA ) { findAllClosedArea(grid); }
	if( m_board->isLastMoveCombineBlock() ) { combineBlockClosedArea(moveStack.m_mainBlock,moveStack.m_vCombineBlocks); }
	else if( grid.getBlock()->getStatus()==LAD_LIFE ) { 
		++m_numLifeStone.get(grid.getColor()); 
		m_bmLife.get(grid.getColor()).SetBitOn(grid.getPosition());
	}
	if( m_board->isLastMoveCaptureBlock() ) { updateDeadBlockClosedArea(moveStack.m_vDeadBlocks); }
}

void WeichiCAHandler::traversalBensonSet( WeichiBlock* startBlock, Benson& benson )
{
	assertToFile( !benson.m_blockIDs.contains(startBlock->GetID()), m_board );

	uint numHealthy = startBlock->getNumClosedArea();
	benson.m_blockIDs.addFeature(startBlock->GetID());
	for( uint iNumCA=0; iNumCA<startBlock->getNumClosedArea(); iNumCA++ ) {
		WeichiClosedArea* blockNbrCA = startBlock->getClosedArea(iNumCA,m_closedAreas);
		if( blockNbrCA->getStatus()==LAD_LIFE ) { continue; }
		if( benson.m_closedAreaIDs.contains(blockNbrCA->GetID()) ) { continue; }
		if( !isCAHealthy(blockNbrCA) ) { --numHealthy; continue; }
		benson.m_closedAreaIDs.addFeature(blockNbrCA->GetID());
		for( uint iNumBlock=0; iNumBlock<blockNbrCA->getNumBlock(); iNumBlock++ ) {
			WeichiBlock* caNbrBlock = m_board->getBlock(blockNbrCA->getBlockID(iNumBlock));
			if( caNbrBlock->getStatus()==LAD_LIFE ) { continue; }
			if( benson.m_blockIDs.contains(caNbrBlock->GetID()) ) { continue; }
			traversalBensonSet(caNbrBlock,benson);
		}
	}
	benson.m_blockNumHealthyCA[startBlock->GetID()] = numHealthy;
}

bool WeichiCAHandler::isCAHealthy( const WeichiClosedArea* closedArea ) const
{
	if( closedArea->getNumStone()==1 ) { return true; }
	if( closedArea->getNumBlock()==1 && closedArea->getNumStone()<3 ) { return true; }

	for( uint iNum=0; iNum<closedArea->getNumBlock(); iNum++ ) {
		WeichiBlock* caNbrBlock = m_board->getBlock(closedArea->getBlockID(iNum));
		assertToFile( caNbrBlock, m_board );
		if( !(closedArea->getStoneMap()&~caNbrBlock->getStonenNbrMap()&~m_board->getBitBoard()).empty() ) { return false; }
	}

	return true;
}

bool WeichiCAHandler::isLADEndGame() const
{
	// When training on 7x7 killall, the game is always not end for the first 3 moves.
	if (WeichiConfigure::zero_training_7x7_killall && m_board->getMoveList().size() <= 3) { return false; }

	// Life (OR)
	int pos;
	if (isAnyStoneLife(m_board->getLADToLifeStones().get(COLOR_BLACK), COLOR_BLACK)
		|| isAnyStoneLife(m_board->getLADToLifeStones().get(COLOR_WHITE), COLOR_WHITE)) {
		return true;
	}

	// Kill (AND)
	WeichiBitBoard bmBlackKill = m_board->getLADToKillStones().get(COLOR_BLACK);
	WeichiBitBoard bmWhiteKill = m_board->getLADToKillStones().get(COLOR_WHITE);
	if (!bmBlackKill.empty() && (bmBlackKill&m_board->getStoneBitBoard(COLOR_WHITE)).empty()) { return true; }
	if (!bmWhiteKill.empty() && (bmWhiteKill&m_board->getStoneBitBoard(COLOR_BLACK)).empty()) { return true; }

	// Kill & Life (AND)
	/*Dual<WeichiBitBoard> bmKill = m_board->getLADToKillStones();
	if (!bmKill.get(COLOR_BLACK).empty()) {
		while ((pos = bmKill.get(COLOR_BLACK).bitScanForward()) != -1) {
			if (m_board->getGrid(pos).getClosedArea(COLOR_BLACK)->getStatus() != LAD_LIFE) { return false; }
		}
		return true;
	}
	if (!bmKill.get(COLOR_WHITE).empty()) {
		while ((pos = bmKill.get(COLOR_WHITE).bitScanForward()) != -1) {
			if (m_board->getGrid(pos).getClosedArea(COLOR_WHITE)->getStatus() != LAD_LIFE) { return false; }
		}
		return true;
	}*/

	return false;
}

bool WeichiCAHandler::isAnyStoneLife(WeichiBitBoard bmStone, Color c) const
{
	int pos;
	while ((pos = bmStone.bitScanForward()) != -1) {
		const WeichiGrid& grid = m_board->getGrid(pos);
		if (grid.isEmpty()) {
			if (grid.getClosedArea(c) && grid.getClosedArea(c)->getStatus() == LAD_LIFE) { return true; }
		} else {
			const WeichiBlock* block = grid.getBlock();
			if (block->getColor() == c && block->getStatus() == LAD_LIFE) { return true; }
		}
	}

	return false;
}

uint WeichiCAHandler::findNakadePoint( const WeichiClosedArea* closedArea ) const
{
	if( !isNeedToCheckNakade(closedArea->getNumStone()) ) { return -1; }

	uint pos;
	uint nakadePos = -1;
	Color caColor = closedArea->getColor();
	WeichiBitBoard bmStone = closedArea->getStoneMap();
	while( (pos=bmStone.bitScanForward())!=-1 ) {
		const WeichiGrid& grid = m_board->getGrid(pos);
		if( !grid.isEmpty() ) { continue; }

		if( grid.getPattern().getIsSplitCA(caColor,caColor) ) {
			if( nakadePos!=-1 ) { return -1; }
			else { nakadePos = pos; }
		}
	}

	return nakadePos;
}

void WeichiCAHandler::updateClosedAreaLifeAndDeath( WeichiGrid& grid )
{
	updateOwnCALifeAndDeath(grid);
	updateOppCALifeAndDeath(grid);
}

void WeichiCAHandler::updateOwnCALifeAndDeath( WeichiGrid& grid )
{
	if( !m_board->isLastMoveModifyClosedArea() ) { return ; }
	if( grid.getBlock()->getNumClosedArea()==1 ) { return ; }

	Benson benson;
	WeichiBlock* startBlock = grid.getBlock();

	traversalBensonSet(startBlock,benson);
	if( startBlock->getStatus()==LAD_LIFE ) { benson.m_blockIDs.removeFeature(startBlock->GetID()); }
	findBensonLife(benson);
	setBensonLife(benson);
}

void WeichiCAHandler::updateOppCALifeAndDeath( WeichiGrid& grid )
{
	WeichiClosedArea* oppCA = grid.getClosedArea(AgainstColor(grid.getColor()));
	if( !oppCA || !isCAHealthy(oppCA) ) { return ; }

	Benson benson;
	assertToFile( oppCA->getNumBlock()!=0, m_board );
	WeichiBlock* startBlock = m_board->getBlock(oppCA->getBlockID(0));

	traversalBensonSet(startBlock,benson);
	if( startBlock->getStatus()==LAD_LIFE ) { benson.m_blockIDs.removeFeature(startBlock->GetID()); }
	findBensonLife(benson);
	setBensonLife(benson);
}

void WeichiCAHandler::combineBlockClosedArea( WeichiBlock* mainBlock, Vector<WeichiBlock*,4>& vCombineBlocks )
{
	// combine all block closed area to main block
	assertToFile( mainBlock, m_board );
	assertToFile( vCombineBlocks.size()>0, m_board );

	uint numPreStone = mainBlock->getNumStone() - 1;
	uint numLifeAdd = 1;
	bool bIsBlockLife = (mainBlock->getStatus()==LAD_LIFE);
	WeichiBitBoard bmAddLife;
	for( uint iIndex=0; iIndex<vCombineBlocks.size(); iIndex++ ) {
		WeichiBlock* combineBlock = vCombineBlocks[iIndex];

		if( combineBlock->getStatus()==LAD_LIFE ) { bIsBlockLife = true; }
		else { 
			numLifeAdd += combineBlock->getNumStone(); 
			bmAddLife |= combineBlock->getStoneMap();
		}
		numPreStone -= combineBlock->getNumStone();

		for( uint iNum=0; iNum<combineBlock->getNumClosedArea(); iNum++ ) {
			m_board->m_status.m_bLastHasModifyClosedArea = true;

			WeichiClosedArea* closedArea = combineBlock->getClosedArea(iNum,m_closedAreas);
			closedArea->deleteBlockID(combineBlock->GetID());
			if( mainBlock->hasClosedArea(closedArea) ) { continue; }

			mainBlock->addClosedArea(closedArea);
			closedArea->addBlockID(mainBlock->GetID());
		}
	}

	if( bIsBlockLife ) {
		if( mainBlock->getStatus()!=LAD_LIFE ) { 
			numLifeAdd += numPreStone; 
			m_bmLife.get(mainBlock->getColor()) |= mainBlock->getStoneMap();
		}
		m_numLifeStone.get(mainBlock->getColor()) += numLifeAdd;
		m_bmLife.get(mainBlock->getColor()) |= bmAddLife;
		mainBlock->setStatus(LAD_LIFE);
	}
}

void WeichiCAHandler::addDeadBlockToClosedArea( WeichiBlock* deadBlock, WeichiClosedArea *closedArea )
{
	// scan each stone & add to closed area
	uint pos;
	WeichiBitBoard bmStone = deadBlock->getStoneMap();
	while( (pos=bmStone.bitScanForward())!=-1 ) {
		addGridToClosedArea(m_board->getGrid(pos),closedArea);
	}

	// remove dead block from closed area
	assertToFile( closedArea->hasBlockID(deadBlock->GetID()), m_board );
	closedArea->deleteBlockID(deadBlock->GetID());
}

void WeichiCAHandler::mergeAndRemoveClosedArea( WeichiClosedArea* mainClosedArea, WeichiClosedArea* mergeClosedArea )
{
	uint pos;
	Color caColor = mainClosedArea->getColor();
	WeichiBitBoard bmStone = mergeClosedArea->getStoneMap();

	// merge grid
	while( (pos=bmStone.bitScanForward())!=-1 ) {
		assertToFile( m_board->getGrid(pos).getClosedArea(caColor), m_board );
		m_board->getGrid(pos).setClosedArea(mainClosedArea,caColor);
	}

	// merge block
	for( uint iIndex=0; iIndex<mergeClosedArea->getNumBlock(); iIndex++ ) {
		WeichiBlock* nbrBlock = m_board->getBlock(mergeClosedArea->getBlockID(iIndex));
		assertToFile( nbrBlock->getColor()==caColor && nbrBlock->hasClosedArea(mergeClosedArea), m_board );
		nbrBlock->deleteClosedArea(mergeClosedArea);
		if( !nbrBlock->hasClosedArea(mainClosedArea) ) { nbrBlock->addClosedArea(mainClosedArea); }
	}

	// merge nakade (if have two or more, remove all)
	if( mainClosedArea->hasNakade() ) {
		m_nakadeLists.get(caColor).removeFeature(mainClosedArea->getNakade());
		mainClosedArea->clearNakade();
	}
	if( mergeClosedArea->hasNakade() ) {
		m_nakadeLists.get(caColor).removeFeature(mergeClosedArea->getNakade());
		mergeClosedArea->clearNakade();
	}

	mainClosedArea->merge(mergeClosedArea);
	m_closedAreas.FreeOne(mergeClosedArea);
}

void WeichiCAHandler::updateDeadBlockClosedArea( const Vector<WeichiBlock*,4>& vDeadBlocks )
{
	// Three possible case:
	//	1. dead block doesn't has closed area --> do nothing
	//	2. dead block has closed area
	//		a. dead block is inner block --> remove & merge close area
	//			PS: new closed area num should be less than MAX
	//		b. dead block is not inner block --> remove closed area
	for( uint iIndex=0; iIndex<vDeadBlocks.size(); iIndex++ ) {
		WeichiBlock* deadBlock = vDeadBlocks[iIndex];

		if( deadBlock->getNumClosedArea()==0 ) { continue; }

		WeichiClosedArea *mainClosedArea = deadBlock->getClosedArea(0,m_closedAreas);
		WeichiBitBoard bmNbrStone = deadBlock->getStonenNbrMap();

		bmNbrStone -= mainClosedArea->getStoneMap();
		for( uint iNum=deadBlock->getNumClosedArea()-1; iNum>=1; iNum-- ) {	// use backward to avoid missing something
			assertToFile( iNum<deadBlock->getNumClosedArea(),m_board );
			WeichiClosedArea* mergeClosedArea = deadBlock->getClosedArea(iNum,m_closedAreas);
			mergeAndRemoveClosedArea(mainClosedArea,mergeClosedArea);
			bmNbrStone -= mergeClosedArea->getStoneMap();
		}

		if( (mainClosedArea->getNumStone()+deadBlock->getNumStone())<=MAX_CLOSEDAREA_SIZE && 
			(bmNbrStone==deadBlock->getStoneMap()) )
		{
			addDeadBlockToClosedArea(deadBlock,mainClosedArea);
			reCheckClosedAreaNakade(mainClosedArea);
		} else {
			removeClosedArea(mainClosedArea);
		}
	}
}

void WeichiCAHandler::findAllClosedArea( WeichiGrid& grid )
{
	// use split direction to find all closed area
	Color gridColor = grid.getColor();
	const uint splitDir = grid.getPattern().getStartCAPosition(gridColor,gridColor);

	const Vector<uint,8>& vSplitDir = StaticBoard::getPattern3x3Direction(splitDir);
	for( uint i=0; i<vSplitDir.size(); i++ ) {
		const uint startPos = grid.getStaticGrid().getNeighbor(vSplitDir.at(i));
		const WeichiGrid& startGrid = m_board->getGrid(startPos);

		assertToFile( startGrid.getColor()!=gridColor, m_board );	//avoid by pattern

		if( startGrid.getClosedArea(gridColor) ) { continue; }
		findOneClosedArea(startPos,gridColor,m_board->m_boundingBox.get(gridColor),MAX_CLOSEDAREA_SCAN_STOP_TIMES);
	}
}

void WeichiCAHandler::removeClosedArea( WeichiClosedArea* closedArea )
{
	// remove closed area
	//	1. reset all grid pointer to CA
	//	2. reset all neighbor block link to CA
	uint pos;
	Color caColor = closedArea->getColor();
	WeichiBitBoard bmStone = closedArea->getStoneMap();

	// reset all grid pointer to CA
	while( (pos=bmStone.bitScanForward())!=-1 ) {
		assertToFile( m_board->getGrid(pos).getClosedArea(caColor), m_board );
		m_board->getGrid(pos).setClosedArea(NULL,caColor);
	}

	// reset all neighbor block link to CA
	for( uint iIndex=0; iIndex<closedArea->getNumBlock(); iIndex++ ) {
		WeichiBlock* nbrBlock = m_board->getBlock(closedArea->getBlockID(iIndex));
		assertToFile( nbrBlock->getColor()==caColor && nbrBlock->hasClosedArea(closedArea), m_board );
		nbrBlock->deleteClosedArea(closedArea);
	}

	// remove nakade point
	if( closedArea->hasNakade() ) {
		assertToFile( m_nakadeLists.get(caColor).contains(closedArea->getNakade()), m_board );
		m_nakadeLists.get(caColor).removeFeature(closedArea->getNakade());
	}

	// remove CA
	m_closedAreas.FreeOne(closedArea);
}

void WeichiCAHandler::setClosedAreaAttribute( WeichiClosedArea* closedArea, Color caColor, WeichiBitBoard& bmStone, uint numStone )
{
	// set a new closed area attribute
	//	1. set closed area type
	//	2. link all grid to closed area
	//	3. link neighbor block to closed area & add block to closed area
	closedArea->init();
	closedArea->setColor(caColor);
	closedArea->setStoneMap(bmStone);
	closedArea->setNumStone(numStone);

	// set closed area type
	closedArea->calculateCAType();

	// link all grid to closed area & set nakade
	uint pos;
	while( (pos=bmStone.bitScanForward())!=-1 ) {
		WeichiGrid& grid = m_board->getGrid(pos);
		
		assertToFile( grid.getColor()!=caColor, m_board );
		assertToFile( grid.getClosedArea(caColor)==NULL, m_board );
		
		grid.setClosedArea(closedArea,caColor);
	}

	// set nakade
	uint nakadePos;
	if( (nakadePos=findNakadePoint(closedArea))!=-1 ) {
		closedArea->setNakade(nakadePos);
		m_nakadeLists.get(caColor).addFeature(nakadePos);
	}

	// link neighbor block to closed area
	WeichiBitBoard bmSurroundStone = closedArea->getSurroundBitBoard();
	while( (pos=bmSurroundStone.bitScanForward())!=-1 ) {
		WeichiBlock *nbrBlock = m_board->getGrid(pos).getBlock();

		assertToFile( nbrBlock, m_board );
		assertToFile( nbrBlock->getColor()==caColor, m_board );
		assertToFile( !nbrBlock->hasClosedArea(closedArea), m_board );

		nbrBlock->addClosedArea(closedArea);
		closedArea->addBlockID(nbrBlock->GetID());
		bmSurroundStone -= nbrBlock->getStoneMap();
	}
}

void WeichiCAHandler::reCheckClosedAreaNakade( WeichiClosedArea* closedArea )
{
	Color myColor = closedArea->getColor();

	if( closedArea->hasNakade() ) {
		assertToFile( m_nakadeLists.get(myColor).contains(closedArea->getNakade()), m_board );
		m_nakadeLists.get(myColor).removeFeature(closedArea->getNakade());
		closedArea->clearNakade();
	}

	if( !isNeedToCheckNakade(closedArea->getNumStone()) ) { return; }

	uint nakadePos;
	if( (nakadePos=findNakadePoint(closedArea))!=-1 ) {
		closedArea->setNakade(nakadePos);
		m_nakadeLists.get(myColor).addFeature(nakadePos);
	}
}

void WeichiCAHandler::findOneClosedArea( uint startPos, Color findColor, const BoundingBox& boundingBox, uint maxSize )
{
	// use floodfill to find one closed area & set closed area attribute
	assertToFile( m_board->getGrid(startPos).getClosedArea(findColor)==NULL, m_board );

	WeichiBitBoard bmCAStone;
	WeichiBitBoard bmBorder = ~m_board->getStoneBitBoard(findColor) & StaticBoard::getMaskBorder();

	if( !boundingBox.isInBoundingBox(startPos) ) { return ; }
	if( !bmBorder.floodfill(startPos,bmCAStone,boundingBox.getBitBoard(),maxSize) ) { return ; }

	uint numStone = bmCAStone.bitCount();

	m_board->m_status.m_bLastHasModifyClosedArea = true;

	WeichiClosedArea* newClosedArea = m_closedAreas.NewOne();
	setClosedAreaAttribute(newClosedArea,findColor,bmCAStone,numStone);
}

void WeichiCAHandler::setFullBoardClosedArea( Color findColor )
{
	uint pos,numStone;
	WeichiBitBoard bmFindCAStone = ~m_board->m_bmStone.get(findColor) & StaticBoard::getMaskBorder();

	//find full board CA
	while( (pos=bmFindCAStone.bitScanForward())!=-1 ) {
		bmFindCAStone.SetBitOn(pos);

		WeichiBitBoard bmStone;

		bmFindCAStone.floodfill(pos,bmStone);
		bmFindCAStone -= bmStone;
		numStone = bmStone.bitCount();

		assertToFile( numStone!=0, m_board );

		if( numStone>MAX_UCT_CLOSEDAREA_SIZE ) { continue; }

		WeichiClosedArea* closedArea = m_closedAreas.NewOne();
		setClosedAreaAttribute(closedArea,findColor,bmStone,numStone);
	}
}

void WeichiCAHandler::findBensonSet( Benson& benson ) const
{
	// find closed area
	for( uint iIndex=0; iIndex<m_closedAreas.getCapacity(); iIndex++ ) {
		if( !m_closedAreas.isValidIdx(iIndex) ) { continue; }
		const WeichiClosedArea* closedArea = m_closedAreas.getAt(iIndex);
		if( !isCAHealthy(closedArea) ) { continue; }
		benson.m_closedAreaIDs.addFeature(closedArea->GetID());
	}

	// find block
	FeatureList<WeichiBlock,MAX_GAME_LENGTH>& blockList = m_board->m_blockList;
	for( uint iIndex=0; iIndex<blockList.getCapacity(); iIndex++ ) {
		if( !blockList.isValidIdx(iIndex) ) { continue; }
		const WeichiBlock* block = blockList.getAt(iIndex);
		if( !block->isUsed() ) { continue; }

		uint numHealthyCA = block->getNumClosedArea();
		for( uint iNum=0; iNum<block->getNumClosedArea(); iNum++ ) {
			uint closedAreaID = block->getClosedAreaID(iNum);
			if( benson.m_closedAreaIDs.contains(closedAreaID) ) { continue; }
			--numHealthyCA;
		}
		benson.m_blockIDs.addFeature(block->GetID());
		benson.m_blockNumHealthyCA[block->GetID()] = numHealthyCA;
	}
}

void WeichiCAHandler::findBensonLife( Benson& benson ) const
{
	// do benson algorithm
	bool bIsOver = false;

	while( !bIsOver ) {
		bIsOver = true;

		for( uint iIndex=0; iIndex<benson.m_blockIDs.size(); iIndex++ ) {
			const WeichiBlock* block = m_board->getBlock(benson.m_blockIDs[iIndex]);
			if( benson.m_blockNumHealthyCA[block->GetID()]>=2 ) { continue; }

			bIsOver = false;
			benson.m_blockIDs.removeFeature(block->GetID());
			for( uint iNumCA=0; iNumCA<block->getNumClosedArea(); iNumCA++ ) {
				uint blockNbrCAID = block->getClosedAreaID(iNumCA);
				const WeichiClosedArea* blockNbrCA = m_closedAreas.getAt(blockNbrCAID);
				if( !benson.m_closedAreaIDs.contains(blockNbrCAID) ) { continue; }
				benson.m_closedAreaIDs.removeFeature(blockNbrCAID);
				for( uint iNumBlock=0; iNumBlock<blockNbrCA->getNumBlock(); iNumBlock++ ) {
					--benson.m_blockNumHealthyCA[blockNbrCA->getBlockID(iNumBlock)];
				}
			}
		}
	}
}

void WeichiCAHandler::setBensonLife( Benson& benson )
{
	// set block life
	for( uint iNum=0; iNum<benson.m_blockIDs.size(); iNum++ ) { 
		WeichiBlock* block = m_board->getBlock(benson.m_blockIDs[iNum]);
		assertToFile( block->getStatus()!=LAD_LIFE, m_board );
		block->setStatus(LAD_LIFE);
		m_numLifeStone.get(block->getColor()) += block->getNumStone();
		m_bmLife.get(block->getColor()) |= block->getStoneMap();
	}

	// set closed area life
	for( uint iNum=0; iNum<benson.m_closedAreaIDs.size(); iNum++ ) {
		WeichiClosedArea* closedArea = m_closedAreas.getAt(benson.m_closedAreaIDs[iNum]);
		assertToFile( closedArea->getStatus()!=LAD_LIFE , m_board );
		closedArea->setStatus(LAD_LIFE);
		if (WeichiConfigure::use_probability_playout) { m_board->m_probabilityHandler.removeLifeCandidates(closedArea); }
		m_numLifeStone.get(closedArea->getColor()) += closedArea->getNumStone();
		m_bmLife.get(closedArea->getColor()) |= closedArea->getStoneMap();
	}
}

std::string WeichiCAHandler::getClosedAreaDrawingString() const
{
	WeichiBitBoard bmBothClosedArea;
	Dual<WeichiBitBoard> bmClosedArea;
	
	// initialize
	for( uint i=0; i<m_closedAreas.getCapacity(); i++ ) {
		if( !m_closedAreas.isValidIdx(i) ) { continue; }
		const WeichiClosedArea *closedArea = m_closedAreas.getAt(i);
		bmClosedArea.get(closedArea->getColor()) |= closedArea->getStoneMap();
	}
	bmBothClosedArea = bmClosedArea.get(COLOR_BLACK) & bmClosedArea.get(COLOR_WHITE);

	ostringstream oss;
	if( WeichiConfigure::DrawClosedAreaBlack ) { oss << "BLACK" << getBitBoardGtpFormatString(bmClosedArea.get(COLOR_BLACK)) << endl; }
	if( WeichiConfigure::DrawClosedAreaBlack ) { oss << "WHITE" << getBitBoardGtpFormatString(bmClosedArea.get(COLOR_WHITE)) << endl; }
	if( WeichiConfigure::DrawClosedAreaBlack && WeichiConfigure::DrawClosedAreaWhite ) {
		oss << "GRAY" << getBitBoardGtpFormatString(bmBothClosedArea) << endl;
	}

	return oss.str();
}

std::string WeichiCAHandler::getBitBoardGtpFormatString( WeichiBitBoard bmBoard ) const
{
	uint pos;
	ostringstream oss;

	while( (pos=bmBoard.bitScanForward())!=-1 ) {
		oss << ' ' << WeichiMove(pos).toGtpString();
	}

	return oss.str();
}

std::string WeichiCAHandler::getClosedAreaLifeAndDeathString() const
{
	Vector<Vector<uint,2*MAX_NUM_GRIDS>,LAD_SIZE> vTypes;
	std::string sTypeSymbol[LAD_SIZE] = {"MARK","CIRCLE","SQUARE"};

	CERR() << "black life stone num: " << m_numLifeStone.get(COLOR_BLACK) 
		   << " (" << m_numLifeStone.get(COLOR_BLACK)*100.0/WeichiConfigure::TotalGrids << "%)" << endl;
	CERR() << "white life stone num: " << m_numLifeStone.get(COLOR_WHITE)
		   << " (" << m_numLifeStone.get(COLOR_WHITE)*100.0/WeichiConfigure::TotalGrids << "%)" << endl;
	CERR() << "Total life stone num: " << m_numLifeStone.get(COLOR_BLACK) + m_numLifeStone.get(COLOR_WHITE)
		   << " (" << (m_numLifeStone.get(COLOR_BLACK) + m_numLifeStone.get(COLOR_WHITE))*100.0/WeichiConfigure::TotalGrids << "%)" << endl;

	// introduce each type symbol for user
	for( uint type=0; type<LAD_SIZE; type++ ) {
		CERR() << getWeichiLifeAndDeathString(static_cast<WeichiLifeAndDeathStatus>(type)) << ": " << sTypeSymbol[type] << endl; 
	}

	vTypes.resize(LAD_SIZE);
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiGrid& grid = m_board->getGrid(*it);
		if( grid.isEmpty() ) {
			const WeichiClosedArea* blackClosedArea = grid.getClosedArea(COLOR_BLACK);
			const WeichiClosedArea* whiteClosedArea = grid.getClosedArea(COLOR_WHITE);
			if( blackClosedArea ) { vTypes[blackClosedArea->getStatus()].push_back(*it); }
			if( whiteClosedArea ) { vTypes[whiteClosedArea->getStatus()].push_back(*it); }
		} else {
			const WeichiBlock* block = grid.getBlock();
			vTypes[block->getStatus()].push_back(*it);
		}	
	}

	ostringstream oss;
	for( uint type=0; type<LAD_SIZE; type++ ) {
		oss << sTypeSymbol[type] << ' ';
		for( uint iNum=0; iNum<vTypes[type].size(); iNum++ ) {
			oss << ' ' << WeichiMove(vTypes[type][iNum]).toGtpString();
		}
		oss << '\n';
	}

	return oss.str();
}

bool WeichiCAHandler::invariance() const
{
	assert( m_board!=NULL );

	if( m_board->isPlayLightStatus() ) { return true; }

	assertToFile( invariance_checkGridCAExist(), m_board );
	assertToFile( invariance_checkFullBoardCA(COLOR_BLACK), m_board );
	assertToFile( invariance_checkFullBoardCA(COLOR_WHITE), m_board );
	assertToFile( invariance_checkBensonLife(), m_board );
	assertToFile( invariance_checkNumLifeStone(), m_board );

	return true;
}

bool WeichiCAHandler::invariance_checkGridCAExist() const
{
	uint pos;
	WeichiBitBoard bmStone = m_board->getBitBoard();

	while( (pos=bmStone.bitScanForward())!=-1 ) {
		WeichiGrid& stoneGrid = m_board->getGrid(pos);
		if( stoneGrid.getColor()==COLOR_BLACK ) { assertToFile( m_board->getGrid(pos).getClosedArea(COLOR_BLACK)==NULL, m_board ); }
		if( stoneGrid.getColor()==COLOR_WHITE ) { assertToFile( m_board->getGrid(pos).getClosedArea(COLOR_WHITE)==NULL, m_board ); }
	}

	return true;
}

bool WeichiCAHandler::invariance_checkFullBoardCA( Color findColor ) const
{
	uint pos,startPos;
	uint numTotalNakade = 0;
	WeichiBitBoard bmFindCAStone = ~m_board->m_bmStone.get(findColor) & StaticBoard::getMaskBorder();
	WeichiClosedArea *closedArea = new WeichiClosedArea;

	//find full board CA
	while( (startPos=bmFindCAStone.bitScanForward())!=-1 ) {
		bmFindCAStone.SetBitOn(startPos);

		uint numStone,numBlock;
		bool bIsFindCA = false;
		WeichiBitBoard bmStone;
		const WeichiClosedArea* preClosedArea = m_board->getGrid(startPos).getClosedArea(findColor);

		bmFindCAStone.floodfill(startPos,bmStone);
		bmFindCAStone -= bmStone;
		numStone = bmStone.bitCount();

		assertToFile( numStone!=0 , m_board );

		// check is find closed area
		if( preClosedArea || (m_board->isPlayoutStatus() && numStone<=MAX_CLOSEDAREA_SIZE) ) {
			closedArea->init();
			closedArea->setColor(findColor);
			closedArea->setStoneMap(bmStone);
			closedArea->setNumStone(numStone);
			assertToFile( numStone!=WeichiConfigure::TotalGrids, m_board );
			bIsFindCA = true;
		}

		//check block link to CA & calculate num block
		if( bIsFindCA ) {
			numBlock = 0;
			WeichiBitBoard bmSurroundStone = bmStone.dilate() & ~bmStone & StaticBoard::getMaskBorder();
			while( (pos=bmSurroundStone.bitScanForward())!=-1 ) {
				const WeichiBlock* block = m_board->getGrid(pos).getBlock();
				assertToFile( preClosedArea, m_board );
				assertToFile( block->hasClosedArea(preClosedArea), m_board );
				assertToFile( preClosedArea->hasBlockID(block->GetID()), m_board );
				numBlock++;
				bmSurroundStone -= block->getStoneMap();
			}
			assertToFile( numBlock>0, m_board );
		}

		//check grid link to CA
		while( (pos=bmStone.bitScanForward())!=-1 ) {
			assertToFile( preClosedArea==m_board->getGrid(pos).getClosedArea(findColor), m_board );

			if( bIsFindCA ) {
				assertToFile( preClosedArea && preClosedArea->GetID()!=-1, m_board );
				assertToFile( preClosedArea->getColor()==closedArea->getColor(), m_board );
				assertToFile( preClosedArea->getStoneMap()==closedArea->getStoneMap(), m_board );
				assertToFile( preClosedArea->getNumStone()==closedArea->getNumStone(), m_board );
				assertToFile( preClosedArea->getNumBlock()==numBlock, m_board );	
			} else {
				assertToFile( preClosedArea==NULL, m_board );
			}
		}

		// nakade point
		if( bIsFindCA ) {
			uint nakadePos;
			if( (nakadePos=findNakadePoint(closedArea))!=-1 ) {
				++numTotalNakade;
				assertToFile( preClosedArea->getNakade()==nakadePos, m_board );
				assertToFile( m_nakadeLists.get(findColor).contains(nakadePos), m_board );
			}
		}
	}

	// check nakade num
	assertToFile( m_nakadeLists.get(findColor).size()==numTotalNakade, m_board );

	delete closedArea;

	return true;
}

bool WeichiCAHandler::invariance_checkBensonLife() const
{
	Benson benson;
	findBensonSet(benson);
	findBensonLife(benson);

	// check benson close area life status invariance
	for( uint iIndex=0; iIndex<m_closedAreas.getCapacity(); iIndex++ ) {
		if( !m_closedAreas.isValidIdx(iIndex) ) { continue; }
		const WeichiClosedArea* closedArea = m_closedAreas.getAt(iIndex);
		if( benson.m_closedAreaIDs.contains(closedArea->GetID()) ) { assertToFile( closedArea->getStatus()==LAD_LIFE, m_board ); }
	}

	// check benson block life status invariance
	for( uint iIndex=0; iIndex<m_board->m_blockList.getCapacity(); iIndex++ ) {
		if( !m_board->m_blockList.isValidIdx(iIndex) ) { continue; }
		const WeichiBlock* block = m_board->m_blockList.getAt(iIndex);
		if( !block->isUsed() ) { continue; }
		if( benson.m_blockIDs.contains(block->GetID()) ) { assertToFile( block->getStatus()==LAD_LIFE, m_board ); }
	}

	return true;
}

bool WeichiCAHandler::invariance_checkNumLifeStone() const
{
	Dual<WeichiBitBoard> bmLife;
	for( uint iIndex=0; iIndex<m_board->m_blockList.getCapacity(); iIndex++ ) {
		if( !m_board->m_blockList.isValidIdx(iIndex) ) { continue; }
		const WeichiBlock *blk = m_board->m_blockList.getAt(iIndex);
		if( !blk->isUsed() ) { continue; }
		if( blk->getStatus()==LAD_LIFE ) { bmLife.get(blk->getColor()) |= blk->getStoneMap(); }
	}
	for( uint iIndex=0; iIndex<m_closedAreas.getCapacity(); iIndex++ ) {
		if( !m_closedAreas.isValidIdx(iIndex) ) { continue; }
		const WeichiClosedArea *ca = m_closedAreas.getAt(iIndex);
		if( ca->getStatus()==LAD_LIFE ) { bmLife.get(ca->getColor()) |= ca->getStoneMap(); }
	}

	uint bLife = bmLife.get(COLOR_BLACK).bitCount();
	uint wLife = bmLife.get(COLOR_WHITE).bitCount();
	assertToFile( m_numLifeStone.get(COLOR_BLACK)==bLife, m_board );
	assertToFile( m_numLifeStone.get(COLOR_WHITE)==wLife, m_board );

	return true;
}