#include "WeichiOpenedAreaHandler.h"
#include "WeichiLifeDeathHandler.h"

void WeichiOpenedAreaHandler::findFullBoardOpenedAreasForDragon( Color ownColor ) 
{
	if (!WeichiConfigure::use_pattern_eye) { return; }

	m_dragonOpenedareas.reset();

	Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = m_lifedeathHandler->getDragons() ;
	for( int iDragon=0 ; iDragon < vDragons.size() ; ++iDragon ) {		
		WeichiDragon& dragon = vDragons[iDragon] ;
		Color dragonColor = dragon.getColor() ;
		WeichiBitBoard bmClosedAreas = dragon.getClosedAreasMap() ;
		WeichiBitBoard bmOpenedAreas = dragon.getOpenedAreasMap() ;				
		WeichiBitBoard bmSearchAreas = ~((dragon.getConnectorMap()|m_board.getStoneBitBoard(dragonColor))&StaticBoard::getMaskBorder())-bmClosedAreas-bmOpenedAreas ;						
		WeichiBitBoard bmDragonAllStone = dragon.getStoneMap() | dragon.getConnectorMap() ;		
		WeichiBitBoard bmPossibleStart = (bmDragonAllStone.dilate()-bmDragonAllStone) & StaticBoard::getMaskBorder() ; 
		int startPos = 0 ;
		while( (startPos=bmPossibleStart.bitScanForward()) != -1 ) {
			WeichiBitBoard bmResult ;
			bmSearchAreas.floodfill(startPos, bmResult, WeichiBitBoard(), MAX_OPENEDAREA_SCAN_STOP_TIMES) ;
			if (bmResult.bitCount() > 0 && bmResult.bitCount() <= MAX_OPENEDAREA_SIZE) {				
				WeichiOpenedArea* eyeaArea = m_dragonOpenedareas.NewOne();
				//CERR() << "dragon opened area size:" << m_dragonOpenedareas.getSize() << endl;
				setOpenedAreaAttributeForDragon(eyeaArea, ownColor, bmResult);
			}									
			bmPossibleStart -= bmResult ;
		}
	}

	return ;
}

void WeichiOpenedAreaHandler::findFullBoardOpenedAreasForSnake( Color ownColor )
{
	if (!WeichiConfigure::use_pattern_eye) { return; }

	m_snakeOpenedareas.reset() ;

	Vector<WeichiDragon, MAX_NUM_BLOCKS>& vSnakes = m_lifedeathHandler->getSnakes() ;
	for( int iSnake=0 ; iSnake < vSnakes.size() ; ++iSnake ) {		
		WeichiDragon& snake = vSnakes[iSnake];
		Color dragonColor = snake.getColor();
		WeichiBitBoard bmOpenedAreas = snake.getOpenedAreasMap();						
		WeichiBitBoard bmSearchAreas = ~((snake.getConnectorMap()|m_board.getStoneBitBoard(dragonColor))&StaticBoard::getMaskBorder())-bmOpenedAreas;				
		WeichiBitBoard bmDragonAllStone = snake.getStoneMap() | snake.getConnectorMap();
		WeichiBitBoard bmPossibleStart = (bmDragonAllStone.dilate()-bmDragonAllStone) & StaticBoard::getMaskBorder() ; 
		int startPos = 0 ;
		while( (startPos=bmPossibleStart.bitScanForward()) != -1 ) {
			WeichiBitBoard bmResult ;
			bmSearchAreas.floodfill(startPos, bmResult, WeichiBitBoard(), MAX_OPENEDAREA_SCAN_STOP_TIMES) ;
			if( bmResult.bitCount() > 0 && bmResult.bitCount() <= MAX_OPENEDAREA_SIZE ) {				
				WeichiOpenedArea* eyeaArea = m_snakeOpenedareas.NewOne();
				//CERR() << "snake opened area size:" << m_dragonOpenedareas.getSize() << endl;
				setOpenedAreaAttributeForSnake(eyeaArea, ownColor, bmResult) ;
			}									
			bmPossibleStart -= bmResult ;
		}
	}

	return ;
}

string WeichiOpenedAreaHandler::getOpenedAreasRZoneDrawingString( ) const 
{	
	WeichiBitBoard bmAllRZone ;
	for( uint indexPatternEye=0 ; indexPatternEye < m_dragonOpenedareas.getCapacity() ; ++indexPatternEye ) {
		if( !m_dragonOpenedareas.isValidIdx(indexPatternEye) ) { continue ; }
		const WeichiOpenedArea* eyearea = m_dragonOpenedareas.getAt(indexPatternEye) ;
		if( eyearea->getEyeStatus() == PATTERN_TRUE_EYE ) { cerr << "PATTERN_TRUE_EYE" ; }
		WeichiBitBoard bmRZone = eyearea->getRZone() ;		
		bmAllRZone |= eyearea->getRZone() ;
	}

	ostringstream oss ;
	oss << endl ;

	int pos = 0 ;		
	for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
		if( bmAllRZone.BitIsOn(*it) ) { oss << "#8ef844 " ; }
		else { oss << "\"\" " ; }
		if( *it%MAX_BOARD_SIZE==WeichiConfigure::BoardSize-1 ) { oss << endl; }
	}

	return oss.str() ;
}

string WeichiOpenedAreaHandler::getDragonOpenedAreasDrawingString ( ) const 
{
	WeichiBitBoard bmAllStone ;
	CERR() << "OpenedArea Size: " << m_dragonOpenedareas.getSize() << endl ;

	for( uint iOpenedArea=0 ; iOpenedArea < m_dragonOpenedareas.getCapacity() ; ++iOpenedArea ) {
		if( !m_dragonOpenedareas.isValidIdx(iOpenedArea) ) { continue ; }
		const WeichiOpenedArea* openedArea = m_dragonOpenedareas.getAt(iOpenedArea) ;		
		WeichiBitBoard bmStone = openedArea->getStoneMap() ;
		bmAllStone |= openedArea->getStoneMap() ;
	}

	ostringstream oss ;
	oss << endl ;

	int pos = 0 ;		
	for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
		if( bmAllStone.BitIsOn(*it) ) { oss << "#2b3cfc " ; }
		else { oss << "\"\" " ; }
		if( *it%MAX_BOARD_SIZE==WeichiConfigure::BoardSize-1 ) { oss << endl; }
	}

	return oss.str() ;
}

string WeichiOpenedAreaHandler::getSnakeOpenedAreasDrawingString() const
{
	WeichiBitBoard bmAllStone ;
	CERR() << "Snake OpenedArea Size: " << m_snakeOpenedareas.getSize() << endl ;

	for( uint iOpenedArea=0 ; iOpenedArea < m_snakeOpenedareas.getCapacity() ; ++iOpenedArea ) {
		if( !m_snakeOpenedareas.isValidIdx(iOpenedArea) ) { continue ; }
		const WeichiOpenedArea* openedArea = m_snakeOpenedareas.getAt(iOpenedArea) ;		
		WeichiBitBoard bmStone = openedArea->getStoneMap() ;
		bmAllStone |= openedArea->getStoneMap() ;
	}

	ostringstream oss ;
	oss << endl ;

	int pos = 0 ;		
	for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
		if( bmAllStone.BitIsOn(*it) ) { oss << "#2b3cfc " ; }
		else { oss << "\"\" " ; }
		if( *it%MAX_BOARD_SIZE==WeichiConfigure::BoardSize-1 ) { oss << endl; }
	}

	return oss.str() ;	
}

void WeichiOpenedAreaHandler::setOpenedAreaAttributeForDragon( WeichiOpenedArea* eyearea, Color ownColor, WeichiBitBoard bmRegion, WeichiBitBoard bmRZone ) 
{
	eyearea->init() ;
	eyearea->setColor(ownColor) ;	
	eyearea->setStoneMap(bmRegion) ;
	eyearea->addRZone(bmRZone) ;
	eyearea->setNumStone(bmRegion.bitCount()) ;	
	eyearea->setEyeStatus(EYE_UNKNOWN) ;	

	// link neighbor block and to the dragon
	WeichiBitBoard bmSurroundStone = bmRegion.dilate() & ~bmRegion & StaticBoard::getMaskBorder() & m_board.getStoneBitBoard(ownColor);
	int pos = 0 ;
	while( (pos=bmSurroundStone.bitScanForward()) != -1 ) {
		WeichiBlock *nbrBlock = m_board.getGrid(pos).getBlock();
		if (nbrBlock == NULL) { continue ; }		
		assert(nbrBlock->getColor() == ownColor) ;	

		eyearea->addBlockID(nbrBlock->GetID()) ;
		Vector<WeichiDragon, MAX_NUM_BLOCKS>& dragons = m_lifedeathHandler->getDragons() ;
		WeichiDragon& dragon = dragons[nbrBlock->getDragonId()] ;				
		if( !dragon.hasOpenedArea(eyearea) ) { dragon.addOpenedArea(eyearea) ; }
		bmSurroundStone -= nbrBlock->getStoneMap() ;
	}	

	return ;
}

void WeichiOpenedAreaHandler::setOpenedAreaAttributeForSnake(WeichiOpenedArea * eyearea, Color ownColor, WeichiBitBoard bmRegion, WeichiBitBoard bmRZone)
{
	eyearea->init() ;
	eyearea->setColor(ownColor) ;	
	eyearea->setStoneMap(bmRegion) ;
	eyearea->addRZone(bmRZone) ;
	eyearea->setNumStone(bmRegion.bitCount()) ;	
	eyearea->setEyeStatus(EYE_UNKNOWN) ;	

	// link neighbor block and to the dragon
	WeichiBitBoard bmSurroundStone = bmRegion.dilate() & ~bmRegion & StaticBoard::getMaskBorder() & m_board.getStoneBitBoard(ownColor);
	int pos = 0 ;
	while( (pos=bmSurroundStone.bitScanForward()) != -1 ) {
		WeichiBlock *nbrBlock = m_board.getGrid(pos).getBlock();
		if( nbrBlock == NULL ) { continue ; }		
		assert( nbrBlock->getColor()==ownColor ) ;	

		eyearea->addBlockID(nbrBlock->GetID()) ;
		Vector<WeichiDragon, MAX_NUM_BLOCKS>& vSnakes = m_lifedeathHandler->getSnakes() ;		
		WeichiDragon& snake = vSnakes[nbrBlock->getSnakeId()] ;				
		if( !snake.hasOpenedArea(eyearea) ) { snake.addOpenedArea(eyearea) ; }
		bmSurroundStone -= nbrBlock->getStoneMap() ;
	}	

	return ;
}