#ifndef H_WEICHIKNOWLEDGE_RZONE
#define H_WEICHIKNOWLEDGE_RZONE

#include "WeichiBoard.h"
#include "BasicType.h"
#include "WeichiKnowledgeBase.h"

class WeichiLifeDeathKnowledge
{
public:
	WeichiLifeDeathKnowledge( WeichiBoard& board, OpenAddrHashTable& ht )
	: m_board(board)
	, m_ht(ht)	
	, m_kb(board,ht)
	{ } 

	// For R-zone
	void computeKoRZoneByPattern( const WeichiGrid& invadeGrid, Color color, WeichiBitBoard& bmRZone ) ;
	void compute1libBlockEscapeRZone( const WeichiBlock* block, WeichiBitBoard& bmRZone ) ;
	void compute2libBlockEscapeRZone( const WeichiBlock* block, WeichiBitBoard& bmRZone ) ;	

	//// For block utilities
	bool isBlockCanEscape(const WeichiBlock* block) ;
	bool is1libBlockCanEscape(const WeichiBlock* block) ;
	bool is2libBlockCanEscape(const WeichiBlock* block) ;

	bool isBlockDead(const WeichiBlock* block);
	bool is1LibertyBlockDead(const WeichiBlock* block);
	bool is2LibertyBlockDead(const WeichiBlock* block);

	//// Some usefuke knowledge
	bool isDoubleAtariSafe(const WeichiMove& atariMove) ;
	bool isKoMove(const WeichiMove& move) ;
	bool isConcretPointInRegion(WeichiGrid& grid, WeichiClosedArea* region);	
	int getKoBaseLocation(Color koColor, const WeichiGrid& lastLibertyGrid);

private:
	WeichiBoard& m_board ;	
	OpenAddrHashTable& m_ht ;
	WeichiKnowledgeBase m_kb ;
};

#endif