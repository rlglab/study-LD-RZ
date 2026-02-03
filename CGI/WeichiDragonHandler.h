#ifndef H_WEICHIDRAGON_HANDLER
#define H_WEICHIDRAGON_HANDLER

#include "BasicType.h"
#include "WeichiBoard.h"
#include "OpenAddrHashTable.h"
#include "WeichiDragon.h"
#include "WeichiGlobalInfo.h"

class WeichiBoard ;
class WeichiLifeDeathHandler ;

class WeichiDragonHandler
{
	friend class WeichiBoard;

public:	
	WeichiDragonHandler( WeichiBoard& board, OpenAddrHashTable& ht ):
	m_board(board), m_ht(ht)
	{ }
	
	inline void initialize( WeichiLifeDeathHandler* lifedeathHandler ) { m_lifedeathHandler = lifedeathHandler ; }

	void reset() ;	
	void findFullBoardBlockGroups() ;		

	inline Vector<WeichiDragon, MAX_NUM_BLOCKS>& getDragons() { return m_vDragons ; }	
	inline Vector<WeichiDragon, MAX_NUM_BLOCKS>& getSnakes() { return m_vSnakes ; }	

private:	
	WeichiBoard& m_board ;			
	OpenAddrHashTable& m_ht ;
	WeichiLifeDeathHandler* m_lifedeathHandler ;
	WeichiBitBoard m_bmCheckDragonBlocks, m_bmCheckSnakeBlocks ;		
	Vector<WeichiDragon, MAX_NUM_BLOCKS> m_vDragons ;		
	Vector<WeichiDragon, MAX_NUM_BLOCKS> m_vSnakes ;		

	void findFullBoardDragons(); 
	void findFullBoardSnakes(); 
	void findNbrBlocks(WeichiBlock* block, Vector<WeichiBlock*, MAX_NUM_BLOCKS>& vNbrBlocks, WeichiBitBoard& bmChecked);		
	void connectByStrongConnectors(const WeichiBlock* block, Vector<WeichiBlock*, MAX_NUM_BLOCKS>& searchedBlock, WeichiBitBoard& bmConn);
	void connectByAllConnectors(const WeichiBlock* block, Vector<WeichiBlock*, MAX_NUM_BLOCKS>& searchedBlock, WeichiBitBoard& bmConn) ;
	void setClosedAreasToDragons() ;	
	void setClosedAreasToSnakes() ;	
	bool isInterestedBlock( WeichiBlock* block ) ;	
	bool isInterestedBlockForSnake( WeichiBlock* block ) ;
};

#endif