#ifndef H_WEICHICONNECOTR_HANDLER
#define H_WEICHICONNECOTR_HANDLER

#include "BasicType.h"
#include "WeichiBoard.h"
#include "WeichiConnector.h"
#include "WeichiKnowledgeBase.h"

#define DO_DEBUG_CONNECTOR 0

class WeichiConnectorHandler
{
public:
	WeichiConnectorHandler( WeichiBoard& board, OpenAddrHashTable& ht )
		: m_board(board)
		, m_ht(ht)
		, m_kb(board, ht) { }
	
	typedef WeichiConnectorType ConnectorType ;
	typedef WeichiConnector Connector ;

	void resetConnectors() ;	
	void findFullBoardConnectors( Color findColor ) ;
	string getConnectorThreatDrawingString() ;
	string getConnectorConnDrawingString() ;
	
	inline const FeatureList<WeichiConnector, MAX_NUM_CONNECTORS>& getConnectors() { return m_connectors ; }
	inline WeichiConnector* getConnector( uint index ) { return m_connectors.getAt(index) ; }
	inline vector<WeichiConnector*>& getBlockConnectors( const WeichiBlock* block ) { return m_blockConnectors[block->getiFirstGrid()] ; }

private:
	WeichiBitBoard getConnectorEmptyCandidates( Color findColor ) ;

	WeichiBitBoard findFullBoardBamboo( WeichiBitBoard bmCandidate, Color findColor ) ;
	WeichiBitBoard findFullBoardHalfBamboo( WeichiBitBoard bmCandidate, Color findColor ) ;
	WeichiBitBoard findFullBoardWallBamboo( WeichiBitBoard bmCandidate, Color findColor ) ;
	WeichiBitBoard findFullBoardFalseEye( WeichiBitBoard bmCandidate, Color findColor ) ;
	WeichiBitBoard findFullBoardTigerMouth( WeichiBitBoard bmCandidate, Color findColor ) ;
	WeichiBitBoard findFullBoardDiagonal( WeichiBitBoard bmCandidate, Color findColor ) ;
	WeichiBitBoard findFullBoardJump( WeichiBitBoard bmCandidate, Color findColor ) ;
	WeichiBitBoard findFullBoardWallJump( WeichiBitBoard bmCandidate, Color findColor ) ;	
	WeichiBitBoard findFullBoardTigerMouthOneLibDeadBlock( Color findColor );
	WeichiBitBoard findFullBoardBan( WeichiBitBoard bmCandidate, Color findColor );	
	WeichiBitBoard findFullBoardLibConnector( WeichiBitBoard bmCandidate, Color findColor );	

	bool isBambooPattern( Color findColor, const WeichiGrid& startGrid ) ;
	bool isHalfBambooPattern( Color findColor, const WeichiGrid& startGrid ) ;
	bool isWallBambooPattern( Color findColor, const WeichiGrid& startGrid ) ;
	bool isJumpPattern( Color findColor, const WeichiGrid& startGrid ) ;
	bool isWallJumpPattern( Color findColor, const WeichiGrid& startGrid ) ;
	bool isBanPattern( Color findColor, const WeichiGrid& startGrid ) ;

	bool isFalseEyeConnector(const WeichiMove& invadeMove, Vector<short, 4>& vBlocksIds);
	bool isTigerMouthConnecotr(const WeichiMove& invadeMove, Vector<short, 4>& vBlocksIds);	
	bool isDiagonalConnector(const WeichiMove& invadeMove, Vector<short, 4>& vBlocksIds);	
	bool isTigerMouth1libDeadBlockConnector(const WeichiGrid& grid, Vector<short, 4>& vBlocksIds);
	bool isMutualBlockLibConnector(const WeichiGrid& grid, Vector<short, 4>& vBlocksIds);

	void getBambooAttrs( Color findColor, const WeichiGrid& startGrid, int& endPos, Vector<short, 4>& vBlocksIds ) ;
	void getHalfBambooAttrs( Color findColor, const WeichiGrid& startGrid, int& endPos, Vector<short, 4>& vBlocksIds ) ;	
	void getWallBambooAttrs( Color findColor, const WeichiGrid& startGrid, int& endPos ) ;

	void collectNbrBlocks( Color collectColor, const WeichiGrid& centerGrid,  Vector<short, 4>& vBlocksIds ) ;	
	void setConnectorAttribute( ConnectorType type, Connector* conn, Color findColor, int miaiPos1, int miaiPos2, Vector<short, 4>& vBlocksIds);	
	void setBambooConnectorAttribute( WeichiConnector* conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds) ;
	void setHalfBambooConnectorAttribute( WeichiConnector* conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds) ;
	void setWallBambooConnectorAttribute( WeichiConnector* conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds ) ;
	void setTigerMouthConnectorAttribute( WeichiConnector* conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds ) ;
	void setTigetMouth1libDeadBlockConnectorAttribute(Connector* conn, Color findColor, int miaiPos1, int miaiPos2, WeichiBitBoard bmDeadStone, Vector<short, 4>& vBlocksIds);
	void setJumpConnectorAttribute( WeichiConnector* conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds ) ;
	void setWallJumpConnectorAttribute( WeichiConnector* conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds ) ;
	void setBanConnectorAttribute( WeichiConnector* conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds ) ;
	void setMutualBlockLibConnectorAttribute( WeichiConnector* conn, Color findColor, int firstPos, int secondPos, Vector<short, 4>& vBlocksIds ) ;

	void overrideDiagonalCheckBitmap( const WeichiGrid& grid, Color findColor, WeichiBitBoard& bmChecked ) ;
	void debugConnector( string comment, WeichiBitBoard bitboard ) ;

	bool isInterestedConnectorBlock(Color findColor, int blockID);

private:
	WeichiBoard& m_board ;	
	OpenAddrHashTable& m_ht ;	
	WeichiKnowledgeBase m_kb ;	
	FeatureList<WeichiConnector, MAX_NUM_CONNECTORS> m_connectors ;	
	vector<WeichiConnector*> m_blockConnectors[MAX_NUM_GRIDS] ;
};

#endif