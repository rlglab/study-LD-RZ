#ifndef H_WEICHIEYE_HANDLER
#define H_WEICHIEYE_HANDLER

#include "BasicType.h"
#include "WeichiClosedArea.h"
#include "WeichiOpenedArea.h"
#include "WeichiBoard.h"
#include "WeichiLifeDeathKnowledge.h"
#include "WeichiKnowledgeBase.h"
#include "WeichiConnector.h"
#include "WeichiConnectorHandler.h"

class WeichiLifeDeathHandler ;

class WeichiEyeHandler
{
public:
	WeichiEyeHandler( WeichiBoard& board, OpenAddrHashTable& ht)
	: m_board(board), m_ht(ht), m_kb(board,ht)
	{ }

	inline void initialize( WeichiLifeDeathHandler* lifedeathHandler ) { m_lifedeathHandler = lifedeathHandler ; }

	void reset() ;
	void findFullBoardEyes(Color ownColor) ;	
	string getDebugInfo() const { return m_sDebugInfo; }

private:
	void judgeTrueEyeByRule(WeichiClosedArea* closedarea);	
	bool isTrueEyeByRule(WeichiClosedArea* closedarea);
		
	bool checkNbrBlockLiberty(WeichiClosedArea* closedarea);
	bool checkInsideBlockTrapped(WeichiClosedArea* closedarea);		
	bool checkInsideBlockDead(WeichiClosedArea* closedarea);
	bool checkInsideBlockInert(WeichiClosedArea* closedarea);	
	bool checkSize1PreCondition(WeichiClosedArea* closedarea);
	bool checkSize2PreCondition(WeichiClosedArea* closedarea);

	bool judgeOneRegionLiveByRule(WeichiClosedArea* closedarea);
	bool judgeStaticTrueEyeByRule(WeichiClosedArea* closedarea);
	bool judgeSluggishTrueEyeByRule(WeichiClosedArea* closedarea);
	bool judgeEmptyLargeArea(WeichiClosedArea* closedarea);
	
	bool judgeSize1ProtectedTrueEyeByRule( WeichiClosedArea* closedarea ) ;
	bool judgeSize2ProtectedTrueEyeByRule( WeichiClosedArea* closedarea ) ;
	bool judge3StonesInPrisonTrueEyeByRule( WeichiClosedArea* closedarea ) ;
	bool judgeMiaiPointsTrueEyeByRule( WeichiClosedArea* closedarea ) ;
	bool judgeBigConnectorTrueEyeByRule( WeichiClosedArea* closedarea ) ;	
	bool checkSize3ConnectorEyeException( WeichiClosedArea* closedarea, WeichiBitBoard& bmRZone ) ;
	bool judgeOpenedAreaTrueEyeByRule( WeichiOpenedArea* openedarea ) ;

	bool isSize1RegionEnoughProtectedCorner( WeichiClosedArea* region, uint numProtected, Vector<uint, 4>& vEmptyNonProtectedCorners ) ;
	bool isSize2RegionEnoughProtectedCorner( Vector<uint, 2>& vRegionPos, uint numProtected, Vector<uint, 4>& vNonProtected) ;

	bool isProtectedCorner(WeichiClosedArea* checkedCA, WeichiGrid& diagGrid, WeichiBitBoard& bmRZone);
	bool isSafeEmptyCorner(WeichiClosedArea* checkedCA, WeichiMove killMove);

	inline bool checkRegionHealthy( WeichiClosedArea* closedarea ) { return m_board.isCAHealthy(closedarea) ; }
	inline bool isStaticTrueEye ( WeichiClosedArea* closedarea ) const { return (closedarea->getNumBlock()==1 && m_board.isCAHealthy(closedarea)) ; } 

	WeichiBitBoard getSurroundedBlocksBitboard( WeichiClosedArea* closedarea ) const ;
	WeichiBitBoard getSurroundedBlocksBitboard( WeichiOpenedArea* openedarea ) const ;

	WeichiBitBoard getClosedAreaNeighborBlockRZone(const WeichiClosedArea* closedarea) const;
	WeichiBitBoard getClosedAreaSuicideBlockRZone(const WeichiClosedArea* closedarea) const;
	WeichiBitBoard getClosedAreaInvolvedBlockRZone(const WeichiClosedArea* closedarea) const;

	// One Region live related
	bool isLinearPattern4Live( WeichiClosedArea* closedarea ) ;	
	//bool isLinearPattern5Live(WeichiClosedArea* region) ;	
	void postUpdateRZone(WeichiClosedArea* closedarea);
	bool isConcreteGrid(const WeichiGrid& grid, const WeichiClosedArea* closedarea) const;

private:
	WeichiBoard& m_board;	
	OpenAddrHashTable& m_ht;
	WeichiKnowledgeBase m_kb;
	WeichiLifeDeathHandler* m_lifedeathHandler;
	string m_sDebugInfo;
};

#endif