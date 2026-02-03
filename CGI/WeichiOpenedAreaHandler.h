#ifndef H_WEICHI_OPENEDAREA_HANDLER
#define H_WEICHI_OPENEDAREA_HANDLER

#include "BasicType.h"
#include "WeichiBoard.h"
#include "WeichiConnectorHandler.h"
#include "WeichiDragon.h"
#include "WeichiDragonHandler.h"
#include "WeichiLifeDeathKnowledge.h"
#include "WeichiOpenedArea.h"

class WeichiLifeDeathHandler ;

class WeichiOpenedAreaHandler
{
	friend class WeichiLifeDeathHandler;

public:
	WeichiOpenedAreaHandler( WeichiBoard& board, OpenAddrHashTable& ht ) 	
		: m_board(board) , m_ht(ht) {  } 

	inline void initialize( WeichiLifeDeathHandler* lifedeathHandler ) { m_lifedeathHandler = lifedeathHandler ; }
	void findFullBoardOpenedAreasForDragon(Color color) ;		
	void findFullBoardOpenedAreasForSnake(Color color) ;		

	string getOpenedAreasRZoneDrawingString( ) const ;

	string getDragonOpenedAreasDrawingString( ) const ;
	string getSnakeOpenedAreasDrawingString( ) const ;

	inline FeatureList<WeichiOpenedArea, MAX_NUM_OPENEDAREA>& getDragonOpenedAreas() { return m_dragonOpenedareas; }	
	inline FeatureList<WeichiOpenedArea, MAX_NUM_OPENEDAREA>& getSnakeOpenedAreas() { return m_snakeOpenedareas; }

private:		
	WeichiBoard& m_board ;
	OpenAddrHashTable& m_ht ;		
	WeichiLifeDeathHandler* m_lifedeathHandler ;	
	FeatureList<WeichiOpenedArea, MAX_NUM_OPENEDAREA> m_dragonOpenedareas ;		
	FeatureList<WeichiOpenedArea, MAX_NUM_OPENEDAREA> m_snakeOpenedareas ;

	void setOpenedAreaAttributeForDragon( WeichiOpenedArea* eyearea, Color color, WeichiBitBoard bmRegion, WeichiBitBoard bmRZone=WeichiBitBoard() ) ;
	void setOpenedAreaAttributeForSnake( WeichiOpenedArea* eyearea, Color color, WeichiBitBoard bmRegion, WeichiBitBoard bmRZone=WeichiBitBoard() ) ;
};

#endif 
