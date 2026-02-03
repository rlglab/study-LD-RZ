#include "WeichiDragonHandler.h"
#include "WeichiLifeDeathHandler.h"

void WeichiDragonHandler::reset()
{
	m_vDragons.clear() ;
	m_vSnakes.clear() ;
	m_bmCheckDragonBlocks.Reset() ;		
	m_bmCheckSnakeBlocks.Reset() ;		

	FeatureList<WeichiBlock,MAX_GAME_LENGTH>& blockList = m_board.getBlockList() ;	
	for (int iBlock=0 ; iBlock < blockList.getCapacity() ; ++iBlock) {
		WeichiBlock* block = blockList.getAt(iBlock);
		if (!block->valid() || !block->isUsed()) { continue; }
		block->setDragonId(-1);
		block->setSnakeId(-1);
	}	

	return ;
}

void WeichiDragonHandler::findFullBoardBlockGroups()
{	
	reset();	
	findFullBoardDragons();
	findFullBoardSnakes();

	return ;		
}

void WeichiDragonHandler::findFullBoardDragons() 
{
	FeatureList<WeichiBlock,MAX_GAME_LENGTH>& blockList = m_board.getBlockList() ;	
	for( int iBlock=0 ; iBlock < blockList.getCapacity() ; ++iBlock ) {
		WeichiBlock* leaderBlock = blockList.getAt(iBlock) ;
		if( !isInterestedBlock(leaderBlock) ) { continue ; }
							
		WeichiDragon dragon ;
		dragon.setColor(leaderBlock->getColor()) ;
		Vector<WeichiBlock*, MAX_NUM_BLOCKS> vSearchedBlocks ;
		vSearchedBlocks.push_back(leaderBlock);		
		
		while( !vSearchedBlocks.empty() ) {
			WeichiBlock* currentBlock = vSearchedBlocks.back() ;
			vSearchedBlocks.pop_back() ;			
			dragon.addBlock(currentBlock) ;
			currentBlock->setDragonId(m_vDragons.size()) ;
			m_bmCheckDragonBlocks |= currentBlock->getStoneMap() ;	
			
			WeichiBitBoard bmConn ;
			connectByStrongConnectors(currentBlock, vSearchedBlocks, bmConn);
			dragon.addConnector(bmConn) ;
		}
		m_vDragons.push_back(dragon) ;
	}	

	setClosedAreasToDragons();		

	return ;
}

void WeichiDragonHandler::findFullBoardSnakes() 
{
	FeatureList<WeichiBlock,MAX_GAME_LENGTH>& blockList = m_board.getBlockList() ;	
	for( int iBlock=0 ; iBlock < blockList.getCapacity() ; ++iBlock ) {
		WeichiBlock* leaderBlock = blockList.getAt(iBlock) ;
		if( !isInterestedBlockForSnake(leaderBlock) ) { continue ; }
							
		WeichiDragon snake ;
		snake.setColor(leaderBlock->getColor()) ;
		Vector<WeichiBlock*, MAX_NUM_BLOCKS> vSearchedBlocks ;
		vSearchedBlocks.push_back(leaderBlock) ;		
		while( !vSearchedBlocks.empty() ) {
			WeichiBlock* currentBlock = vSearchedBlocks.back() ;
			vSearchedBlocks.pop_back() ;			
			snake.addBlock(currentBlock) ;
			currentBlock->setSnakeId(m_vSnakes.size()) ;
			m_bmCheckSnakeBlocks |= currentBlock->getStoneMap() ;	
			
			WeichiBitBoard bmConn ;
			connectByAllConnectors(currentBlock, vSearchedBlocks, bmConn) ;
			snake.addConnector(bmConn) ;
		}
		m_vSnakes.push_back(snake) ;
	}	

	setClosedAreasToSnakes();		

	return ;
}

void WeichiDragonHandler::findNbrBlocks( WeichiBlock* block, Vector<WeichiBlock*, MAX_NUM_BLOCKS>& vNbrBlocks, WeichiBitBoard& bmChecked )
{
	Color ownColor = block->getColor() ;
	Color oppColor = AgainstColor(ownColor) ;
	
	WeichiBitBoard bmNbrBlock = block->getStoneMap().dilate() & StaticBoard::getMaskBorder() ;
	bmNbrBlock = (bmNbrBlock.dilate() - bmNbrBlock ) & StaticBoard::getMaskBorder() ;

	int pos = 0 ;
	while( (pos=bmNbrBlock.bitScanForward()) != -1 ) {
		WeichiGrid& grid = m_board.getGrid(pos) ;
		if( grid.getBlock() == NULL ) { continue ; }
		if( grid.getBlock()->getColor() == oppColor ) { continue ; }
		
		WeichiBlock* nbrblock = grid.getBlock() ;
		bmNbrBlock -= nbrblock->getStoneMap() ;
		if( bmChecked.hasIntersection(nbrblock->getStoneMap())) { continue ; }

		vNbrBlocks.push_back(nbrblock) ;				
	}

	return ;
}

void WeichiDragonHandler::connectByStrongConnectors(const WeichiBlock * block, Vector<WeichiBlock*, MAX_NUM_BLOCKS>& searchedBlock, WeichiBitBoard& bmConn)
{
	vector<WeichiConnector*>& blockConnector = m_lifedeathHandler->getConnectorHandler().getBlockConnectors(block) ;

	for( int iConn=0 ; iConn < blockConnector.size() ; ++iConn ) {
		WeichiConnector* conn = blockConnector[iConn] ;
		if( conn->isWeak() ) { continue ; }
		bmConn |= conn->getConnectorBitmap() ;
		
		// Only connect by strong connector
		const Vector<short, 4>& vBlockIds = conn->getBlockIDs() ;
		for( int iBlock=0 ; iBlock < vBlockIds.size() ; ++iBlock ) {
			short blockId = vBlockIds[iBlock] ;
			WeichiBlock* nbrBlock = m_board.getBlock(blockId) ;
			if( m_bmCheckDragonBlocks.hasIntersection(nbrBlock->getStoneMap()) ) { continue ; }

			bmConn |= conn->getConnectorBitmap() ;
			searchedBlock.push_back(nbrBlock) ;				
			m_bmCheckDragonBlocks |= nbrBlock->getStoneMap() ;
		}
	}

	return ;
}

void WeichiDragonHandler::connectByAllConnectors(const WeichiBlock * block, Vector<WeichiBlock*, MAX_NUM_BLOCKS>& searchedBlock, WeichiBitBoard& bmConn)
{
	vector<WeichiConnector*>& blockConnector = m_lifedeathHandler->getConnectorHandler().getBlockConnectors(block) ;
	for( int iConn=0 ; iConn < blockConnector.size() ; ++iConn ) {
		WeichiConnector* conn = blockConnector[iConn] ;
		if( conn->isConnectedToWall() ) { bmConn |= conn->getConnectorBitmap() ; }
		bmConn |= conn->getConnectorBitmap() ;

		// Connect by all connector
		const Vector<short, 4>& vBlockIds = conn->getBlockIDs() ;		
		for( int iBlock=0 ; iBlock < vBlockIds.size() ; ++iBlock ) {
			short blockId = vBlockIds[iBlock] ;
			WeichiBlock* nbrBlock = m_board.getBlock(blockId) ;			
			if( m_bmCheckSnakeBlocks.hasIntersection(nbrBlock->getStoneMap()) ) { continue ; }
			
			searchedBlock.push_back(nbrBlock) ;				
			m_bmCheckSnakeBlocks |= nbrBlock->getStoneMap() ;
		}
	}

	return ;
}

void WeichiDragonHandler::setClosedAreasToDragons() 
{
	FeatureList<WeichiClosedArea, MAX_NUM_CLOSEDAREA>& closedareas = m_board.getCloseArea();
	for (uint i=0; i < closedareas.getCapacity(); i++) {
		if (!closedareas.isValidIdx(i)) { continue; }			
		WeichiClosedArea* closedarea = closedareas.getAt(i);		
		if (closedarea->getColor()!=WeichiGlobalInfo::getTreeInfo().m_winColor) { continue; }	
		if (closedarea->getStatus()==LAD_LIFE) { continue; }		

		// each closed area should only belong to only one dragon			
		bool bTheSameDragon = true;			
		WeichiDragon* identicalDragon = NULL;		
		for (uint iNumBlock=0; iNumBlock < closedarea->getNumBlock(); iNumBlock++) {					
			WeichiBlock* nbrBlock = m_board.getBlock(closedarea->getBlockID(iNumBlock));
			if (!nbrBlock->isUsed() || !nbrBlock->valid()) { continue; }
			if (nbrBlock->getDragonId() == -1) { 
				bTheSameDragon = false;
				break;
			}		

			if (identicalDragon == NULL) { identicalDragon = &m_vDragons[nbrBlock->getDragonId()]; }
			else if (identicalDragon != &m_vDragons[nbrBlock->getDragonId()]) { 
				bTheSameDragon = false;
				break; 
			}
		}
		
		if (bTheSameDragon) { identicalDragon->addClosedArea(closedarea); }		
	}

	return ;
}

void WeichiDragonHandler::setClosedAreasToSnakes() 
{
	FeatureList<WeichiClosedArea, MAX_NUM_CLOSEDAREA>& closedareas = m_board.getCloseArea();
	for (uint i=0; i < closedareas.getCapacity(); i++) {		
		if (!closedareas.isValidIdx(i)) { continue; }					
		WeichiClosedArea* closedarea = closedareas.getAt(i);		
		if (closedarea->getColor()!=WeichiGlobalInfo::getTreeInfo().m_winColor) { continue; }	
		if (closedarea->getStatus()==LAD_LIFE) { continue; }				
		
		for( uint iNumBlock=0; iNumBlock < closedarea->getNumBlock(); iNumBlock++ ) {					
			WeichiBlock* nbrBlock = m_board.getBlock(closedarea->getBlockID(iNumBlock));
			if (!nbrBlock->isUsed() || !nbrBlock->valid()) { continue; }
			if (nbrBlock->getSnakeId() == -1) { continue; }		

			WeichiDragon& snake = m_vSnakes[nbrBlock->getSnakeId()] ;					
			if (!snake.hasClosedArea(closedarea)) { snake.addClosedArea(closedarea); }
		}
	}

	return ;
}

bool WeichiDragonHandler::isInterestedBlock( WeichiBlock* block ) 
{
	Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
	
	if (!block->valid() || !block->isUsed()) { return false; }
	if (WeichiConfigure::only_find_winner && block->getColor() != winColor) { return false; }
	if (WeichiConfigure::ignore_benson && block->getStatus() == LAD_LIFE) { return false; }
	if (m_bmCheckDragonBlocks.hasIntersection(block->getStoneMap())) { return false; }

	return true ;
}

bool WeichiDragonHandler::isInterestedBlockForSnake(WeichiBlock * block)
{
	Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
	
	if( !block->valid() || !block->isUsed() ) { return false ; }
	if( WeichiConfigure::only_find_winner && block->getColor() != winColor ) { return false ; }
	if( WeichiConfigure::ignore_benson && block->getStatus() == LAD_LIFE ) { return false ; }
	if( m_bmCheckSnakeBlocks.hasIntersection(block->getStoneMap()) ) { return false ; }

	return true ;
} 
