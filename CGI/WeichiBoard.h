#ifndef WEICHIBOARD_H
#define WEICHIBOARD_H

#include "BasicType.h"
#include "BoardStatus.h"
#include "BoardMoveStack.h"
#include "WeichiGrid.h"
#include "WeichiBlock.h"
#include "WeichiClosedArea.h"
#include "WeichiEdge.h"
#include "FeatureList.h"
#include "FeaturePtrList.h"
#include "OpenAddrHashTable.h"
#include "PredictPlayInfo.h"
#include "CandidateList.h"
#include "WeichiKnowledgeBase.h"
#include "WeichiCAHandler.h"
#include "WeichiEdgeHandler.h"
#include "WeichiDynamicKomi.h"
#include "WeichiProbabilityPlayoutHandler.h"
#include "Dual.h"
#include "TimeSystem.h"
#include "WeichiPlayoutResult.h"
#include "SgfTag.h"

class WeichiBoard
{
    friend class WeichiPlayoutAgent;
    friend class WeichiUctAccessor;
	friend class WeichiCAHandler;
	friend class WeichiKnowledgeBase;
	friend class WeichiMoveFeatureHandler;
	friend class WeichiCNNFeatureGenerator;
	friend class WeichiMiniSearch;
	friend class WeichiGtpEngine;
	friend class WeichiBoardStructureGtpEngine;	// remove this!!!!!
	friend class Training;
	friend class TrainGenerator;
	friend class WeichiRootFilter;		// remove this!!!!!
	friend class WeichiThreadState;
	friend class WeichiDragonHandler; // remove this!!!!!
	friend class TMiniMCTS;				// remove this!!!!!
private:
	/*
		struct of WeichiBoard used
	*/
	struct BackupFlag {
		BoardStatus m_status;
		HashKey64 m_hash;
		WeichiBitBoard m_bmBoard;
		WeichiBitBoard m_bmDeadStones;
		WeichiBitBoard m_bmApproachLibBlockPos;
		Dual<int> m_capture;
		Dual<WeichiBitBoard> m_bmStone;
		Dual<BoundingBox> m_boundingBox;
		CandidateList m_candidates;
		WeichiBitBoard m_bmEverCaptured;
		uint m_moveSize;
		WeichiGrid m_grids[MAX_NUM_GRIDS];
		Dual<WeichiBitBoard> m_bmOneLibBlocks;
		Dual<WeichiBitBoard> m_bmTwoLibBlocks;
		FeatureList<WeichiBlock,MAX_GAME_LENGTH> m_blockList;
	};
	
	// full board data
	BoardStatus m_status;
	HashKey64 m_hash;
	WeichiBitBoard m_bmBoard;
	WeichiBitBoard m_bmDeadStones;
	WeichiBitBoard m_bmApproachLibBlockPos;
	Dual<WeichiBitBoard> m_bmStone;
	Dual<BoundingBox> m_boundingBox;
	Vector<WeichiMove,MAX_NUM_GRIDS> m_preset;
	Vector<WeichiMove,MAX_GAME_LENGTH> m_moves;
	Vector<Dual<WeichiBitBoard>, MAX_GAME_LENGTH> m_vbmStone;
	Vector<BoardMoveStack,MAX_GAME_LENGTH> m_moveStacks;
	mutable Vector<Color,MAX_NUM_GRIDS> m_territory;
	CandidateList m_candidates;
	WeichiBitBoard m_bmEverCaptured;

	// grids
	WeichiGrid m_grids[MAX_NUM_GRIDS];

	// blocks
	WeichiBitBoard m_bmUpdateGridLibPos;
	Dual<WeichiBitBoard> m_bmOneLibBlocks;
	Dual<WeichiBitBoard> m_bmTwoLibBlocks;
	FeatureList<WeichiBlock,MAX_GAME_LENGTH> m_blockList;

	// closed area
	WeichiCAHandler m_closedAreaHandler;

	// edges
	WeichiEdgeHandler m_edgeHandler;

	// probability playout table
	WeichiProbabilityPlayoutHandler m_probabilityHandler;

	// others
	BackupFlag m_backupFlag;	
	Vector<BackupFlag, MAX_NUM_GRIDS> m_backupStacks;

	// LAD data
	Dual<bool> m_LADKoAvailable;
	Dual<WeichiBitBoard> m_bmLADToLifeStones;
	Dual<WeichiBitBoard> m_bmLADToKillStones;

#if DO_WINDOWS_TIMER
	MicrosecondTimer m_microSecondTimer;
#endif

public:
	// public function
    static void initializeStatic();

    WeichiBoard();
	void reset();

	// play & undo
	bool preset( const WeichiMove& move );
	void recordPreviousMoveInfo( const WeichiMove& move );
	bool play( const WeichiMove& move );
	void undo();
	bool playLight( const WeichiMove& move );	
	void undoLight();		

	// grids
	inline WeichiGrid& getGrid ( WeichiMove move ) { assertToFile( move.isValidPosition(move.getPosition()), this ); return m_grids[move.getPosition()]; }
	inline const WeichiGrid& getGrid ( WeichiMove move ) const { assertToFile( move.isValidPosition(move.getPosition()), this ); return m_grids[move.getPosition()]; }
	inline WeichiGrid& getGrid ( uint pos ) { assertToFile( WeichiMove::isValidPosition(pos), this ); return m_grids[pos]; }
	inline const WeichiGrid& getGrid ( uint pos ) const { assertToFile( WeichiMove::isValidPosition(pos), this ); return m_grids[pos]; }
	inline WeichiGrid& getGrid ( uint x, uint y ) { assertToFile( WeichiMove::isValidPosition(WeichiMove::toPosition(x,y)), this ); return m_grids[WeichiMove::toPosition(x,y)]; }
	inline const WeichiGrid& getGrid ( uint x, uint y ) const { assertToFile( WeichiMove::isValidPosition(WeichiMove::toPosition(x,y)), this ); return m_grids[WeichiMove::toPosition(x,y)]; }
	inline WeichiGrid& getGrid ( const WeichiGrid& grid, uint direct ) { assert (grid.getStaticGrid().getNeighbor(direct)!=-1); return m_grids[grid.getStaticGrid().getNeighbor(direct)]; }
	inline const WeichiGrid& getGrid ( const WeichiGrid& grid, uint direct ) const { assert (grid.getStaticGrid().getNeighbor(direct)!=-1); return m_grids[grid.getStaticGrid().getNeighbor(direct)]; }

	// blocks
	inline FeatureList<WeichiBlock,MAX_GAME_LENGTH>& getBlockList() { return m_blockList; }
	inline const FeatureList<WeichiBlock,MAX_GAME_LENGTH>& getBlockList() const { return m_blockList; }
	uint getBlockCapacity() const { return m_blockList.getCapacity(); } ;
	const WeichiBlock* getBlock( uint idx ) const { if( m_blockList.isValidIdx(idx) ) return m_blockList.getAt(idx); else return NULL; }
	WeichiBlock* getBlock( uint idx ) { if( m_blockList.isValidIdx(idx) ) return m_blockList.getAt(idx); else return NULL; }

	// closed area
	inline FeatureList<WeichiClosedArea, MAX_NUM_CLOSEDAREA>& getCloseArea() { return m_closedAreaHandler.m_closedAreas; }
	inline const FeatureList<WeichiClosedArea, MAX_NUM_CLOSEDAREA>& getCloseArea() const { return m_closedAreaHandler.m_closedAreas; }
	inline bool isCAHealthy( WeichiClosedArea* closedarea ) { return m_closedAreaHandler.isCAHealthy(closedarea) ; }
	inline WeichiBitBoard getBensonBitboard(Color c) { return m_closedAreaHandler.getBensonBitboard(c); }

	// full board data
	bool hasTwoPass () const { return m_status.m_bIsTwoPass; }
	inline const WeichiBitBoard& getBitBoard() const { return m_bmBoard; }
	inline const WeichiBitBoard& getStoneBitBoard(Color c) const { return m_bmStone.get(c) ; }
	inline const WeichiBitBoard& getApproachLibBlockPos() const { return m_bmApproachLibBlockPos; }
	inline bool isPlayoutStatus() const { return m_status.m_bIsPlayoutStatus; }
	inline void setPlayoutStatus( bool status ) { m_status.m_bIsPlayoutStatus = status; }
	inline bool isExpansionStatus() const { return m_status.m_bIsExpansionStatus; }
	inline void setExpansionSataus( bool status ) { m_status.m_bIsExpansionStatus = status; }
	inline bool isPreSimulationStatus() const { return m_status.m_bIsPreSimulationStatus; }
	inline void setPreSimulationSataus( bool status ) { m_status.m_bIsPreSimulationStatus = status; }
	inline bool isPlayLightStatus() const { return m_status.m_bIsPlayLightStatus; }
	inline void setPlayLightSataus( bool status ) { m_status.m_bIsPlayLightStatus = status; }
	inline bool isEarlyEndGameStatus() const { return m_status.m_bIsEarlyEndGame; }
	inline void setEarlyEndGameStatus( bool status ) { m_status.m_bIsEarlyEndGame = status; }
	const Vector<WeichiMove, MAX_GAME_LENGTH>& getMoveList() const { return m_moves; }
	const Vector<WeichiMove, MAX_NUM_GRIDS>& getPresetList() const { return m_preset; }
	const Vector<Dual<WeichiBitBoard>, MAX_GAME_LENGTH>& getBitBoardList() const { return m_vbmStone; }
	inline const bool isLastMoveCombineBlock() const { return m_status.m_bLastHasCombineBlocks; }
	inline const bool isLastMoveCaptureBlock() const { 
		if ( getPrevMove().getPosition() == WeichiMove::PASS_POSITION ) { return false ; }
		return m_status.m_bLastHasDeadBlocks; 
	}
	inline const bool isLastMoveModifyClosedArea() const { return m_status.m_bLastHasModifyClosedArea; }
	inline bool hasPrevMove() const { return (m_moves.size()>=1) ? true : false; }
	inline bool hasPrevOwnMove() const { return (m_moves.size()>=2) ? true : false; }
	inline WeichiMove getPrevMove() { assertToFile( hasPrevMove(), this ); return m_moves[m_moves.size()-1]; }
	inline const WeichiMove& getPrevMove() const { assertToFile( hasPrevMove(), this ); return m_moves[m_moves.size()-1]; }
	inline WeichiMove getPrevOwnMove() { assertToFile( hasPrevOwnMove(), this ); return m_moves[m_moves.size()-2]; }
	inline const WeichiMove& getPrevOwnMove() const { assertToFile( hasPrevOwnMove(), this ); return m_moves[m_moves.size()-2]; }
	inline bool hasKo() const { return (m_status.m_ko!=-1); }
	inline const short getKo() const { return m_status.m_ko; }
	inline const short getKoEater() const { return m_status.m_koEater; }
	inline const short getLastKo() const { return m_status.m_lastKo; }
	inline WeichiBitBoard getOneLibBlocksBitBoard( Color color ) const { return m_bmOneLibBlocks.get(color); }
	inline WeichiBitBoard getTwoLibBlocksBitBoard( Color color ) const { return m_bmTwoLibBlocks.get(color); }
	inline WeichiBitBoard getDeadStoneBitBoard() const { return m_bmDeadStones; }
	inline Vector<Color, MAX_NUM_GRIDS>& getTerritory() { return m_territory; }
	inline const Vector<Color, MAX_NUM_GRIDS>& getTerritory() const { return m_territory; }
	inline const WeichiBitBoard getEverCaptured() const { return m_bmEverCaptured; }
	inline const WeichiProbabilityPlayoutHandler& getProbabilityPlayoutHandler() const { return m_probabilityHandler; }
	inline Dual<bool>& getLADKoAvailable() { return m_LADKoAvailable; }
	inline const Dual<bool>& getLADKoAvailable() const { return m_LADKoAvailable; }
	inline Dual<WeichiBitBoard>& getLADToLifeStones() { return m_bmLADToLifeStones; }
	inline const Dual<WeichiBitBoard>& getLADToLifeStones() const { return m_bmLADToLifeStones; }
	inline Dual<WeichiBitBoard>& getLADToKillStones() { return m_bmLADToKillStones; }
	inline const Dual<WeichiBitBoard>& getLADToKillStones() const { return m_bmLADToKillStones; }
	
	/*!
        @brief  get HashKey for current board
        @author T.F. Liao
        @return 64bits hash key of current board
    */
    inline HashKey64 getHash() const { return m_hash; }
	inline HashKey64 getKoHashKey() const {
		HashKey64 hashkey = 0;
		if (m_status.m_ko != -1) { hashkey ^= StaticBoard::getHashGenerator().getKoKeys(m_status.m_ko); }
		return hashkey;
	}
	inline HashKey64 getTThashkey() const { return getHash() ^ getKoHashKey(); }
	/*!
        @brief  get mininum HashKey for current board for 8 direction(not incremental)
        @author C.C. Shih
        @param  move    [in]    move to be put
        @return 64bits hash key of current board with a move
    */
	HashKey64 getMinimumHash() const ;

	inline void setToPlay(Color c) { m_status.m_colorToPlay = c; }
    inline Color getToPlay() const { return m_status.m_colorToPlay; }

	// backup flag
	void setBackupFlag();
	void restoreBackupFlag();
	BackupFlag getCurrentBoardBackupFlag() ;

	void storeMoveBackup() ;
	void resumeMoveBackup() ;
	void clearBackupStacks() ;

	// neighbor radius2 liberty pattern index
	uint calculateRadiusLibertyPatternIndex( const WeichiGrid& center_grid ) const;
	uint getOneRadiusLibertyIndex( const WeichiGrid& grid ) const;

	// neighbor
	inline bool isNeighborHasLibertyBlock( const WeichiBlock* block, uint liberty ) const
	{
		WeichiBitBoard bmNbrBlocks;
		Color oppColor = AgainstColor(block->getColor());
		findNeighborBlockBitBoard(block,m_bmStone.get(oppColor),bmNbrBlocks);

		uint pos;
		while( (pos=bmNbrBlocks.bitScanForward())!=-1 ) {
			const WeichiBlock* nbrBlock = getGrid(pos).getBlock();
			bmNbrBlocks -= nbrBlock->getStoneMap();

			if( nbrBlock->getLiberty()==liberty ) { return true; }
		}

		return false;
	}
	inline void findNeighborBlocks( const WeichiBlock* block, WeichiBitBoard bmNbrBlocks, Vector<const WeichiBlock*,MAX_NUM_BLOCKS>& vNbrBlocks ) const
	{
		uint pos;
		while( (pos=bmNbrBlocks.bitScanForward())!=-1 ) {
			const WeichiBlock* block = getGrid(pos).getBlock();
			vNbrBlocks.push_back(block);
			bmNbrBlocks -= block->getStoneMap();
		}
	}
	inline void findNeighborOneLibBlocks( const WeichiBlock* block, Vector<const WeichiBlock*,MAX_NUM_BLOCKS>& vNbrBlocks ) const
	{
		WeichiBitBoard bmNbrBlocks;
		findNeighborOneLibBlocksBitBoard(block,bmNbrBlocks);
		findNeighborBlocks(block,bmNbrBlocks,vNbrBlocks);
	}
	inline void findNeighborTwoLibBlocks( const WeichiBlock* block, Vector<const WeichiBlock*,MAX_NUM_BLOCKS>& vNbrBlocks ) const
	{
		WeichiBitBoard bmNbrBlocks;
		findNeighborTwoLibBlocksBitBoard(block,bmNbrBlocks);
		findNeighborBlocks(block,bmNbrBlocks,vNbrBlocks);
	}
	inline void findNeighborBlockBitBoard( const WeichiBlock* block, const WeichiBitBoard& bmFindBlocks, WeichiBitBoard& bmNbrBlocks ) const
		{ bmNbrBlocks = (bmFindBlocks&block->getStonenNbrMap()); }
	inline void findNeighborOneLibBlocksBitBoard( const WeichiBlock* block, WeichiBitBoard& bmNbrOneLibBlocks ) const
		{ findNeighborBlockBitBoard(block,m_bmOneLibBlocks.get(AgainstColor(block->getColor())),bmNbrOneLibBlocks); }
	inline void findNeighborTwoLibBlocksBitBoard( const WeichiBlock* block, WeichiBitBoard& bmNbrTwoLibBlocks ) const
		{ findNeighborBlockBitBoard(block,m_bmTwoLibBlocks.get(AgainstColor(block->getColor())),bmNbrTwoLibBlocks); }

	// end game
    /*!
        @brief  eval score in end-game
        @author T.F. Liao
        @return score in aspect of black
    */
    float eval(float komi = WeichiDynamicKomi::Internal_komi) const;
	inline void preCalculateForEval()
	{
		if( WeichiConfigure::use_closed_area ) { m_closedAreaHandler.preCalculateForEval(); }
	}
	inline bool isEarlyEndGame() const { return m_closedAreaHandler.isGameEnd(); }
	inline bool isTwoPassEndGame() const { return (hasPrevMove()&&getPrevMove().isPass()&&hasPrevOwnMove()&&getPrevOwnMove().isPass()); }
	inline bool isEnoughTerritoryGameEnd() const { return m_closedAreaHandler.isEnoughTerritoryGameEnd() ; }
	inline bool isEnoughTerritory( Color color ) const { return m_closedAreaHandler.isEnoughTerritory(color) ; }
	inline WeichiBitBoard getBensonBitboard(Color color) const { return m_closedAreaHandler.getBensonBitboard(color); }

	// some knowledge
	bool isAtariMove( WeichiMove move ) const;	
	bool isCaptureMove( WeichiMove move ) const;	
	bool is2libMoveImmediateMoveForbidden( const WeichiMove& move ) const;

	WeichiBitBoard getCapturedStone(const WeichiMove move) const;

	bool isKoEatPlay( WeichiMove move ) const;
	bool isKoMakePlay( WeichiMove move ) const;
	inline void getPredictPlayInfoByPlay( const WeichiMove& move, PredictPlayInfo& moveInfo ) const
	{
		assertToFile( getGrid(move.getPosition()).getColor()==COLOR_NONE, this );
		assertToFile( !move.isPass(), this );

		Color myColor = move.getColor();
		Color oppColor = AgainstColor(myColor);
		const WeichiGrid& grid = getGrid(move);
		WeichiBitBoard bmExclude = getBitBoard();
		WeichiBitBoard bmCheckIndex;

		moveInfo.m_blockNumStone = 1;	// add one more for put stone
		moveInfo.m_bHasDeadBlocks = false;
		moveInfo.m_bHasCombineBlock = false;
		moveInfo.m_bmNewNbrStoneMap = grid.getStonenNbrMap();
		for( const int *iNbr=grid.getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1; iNbr++ ) {
			const WeichiGrid& nbrGrid = getGrid(*iNbr);
			if( nbrGrid.getColor()==COLOR_NONE ) { continue; }

			const WeichiBlock* nbrBlock = nbrGrid.getBlock();
			if( bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) ) { continue; }
			bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());

			if( nbrGrid.getColor()==myColor ) {
				if( moveInfo.m_blockNumStone!=1 ) { moveInfo.m_bHasCombineBlock = true; }
				moveInfo.m_bmPreNbrStoneMap |= nbrBlock->getStonenNbrMap();
				moveInfo.m_bmNewNbrStoneMap |= nbrBlock->getStonenNbrMap();
				moveInfo.m_blockNumStone += nbrBlock->getNumStone();
				moveInfo.m_blockBoundingBox.combine(nbrBlock->getBoundingBox());
			} else if( nbrGrid.getColor()==oppColor ) {
				if( nbrBlock->getLiberty()!=1 ) { continue; }
				bmExclude -= nbrBlock->getStoneMap();
				moveInfo.m_bmDeadStone |= nbrBlock->getStoneMap();
				moveInfo.m_bHasDeadBlocks = true;
			}
		}

		moveInfo.m_bmNewLiberty = moveInfo.m_bmNewNbrStoneMap - bmExclude;
		moveInfo.m_bmNewLiberty.SetBitOff(move.getPosition());
		moveInfo.m_bmNewNbrStoneMap &= getStoneBitBoard(oppColor);
		moveInfo.m_liberty = moveInfo.m_bmNewLiberty.bitCount();
		moveInfo.m_blockBoundingBox.combine(move.getPosition());
	}
	inline HashKey64 getHashKeyAfterPlay(const WeichiMove& move) const
	{
		const WeichiGrid& grid = getGrid(move);
		const StaticGrid& sgrid = grid.getStaticGrid();
		WeichiBitBoard bmCheckIndex;
		Color color = move.getColor();
		HashKey64 hashkey = 0;
		const int* nbr = sgrid.getAdjacentNeighbors();
		for (; *nbr != -1; ++nbr) {
			const WeichiGrid& nbrGrid = m_grids[*nbr];
			if (nbrGrid.isEmpty()) continue;

			const WeichiBlock* nbrBlock = nbrGrid.getBlock();
			if (nbrBlock->getColor() == AgainstColor(color)) {
				if (nbrBlock->getLiberty() == 1 && !bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid())) {
					// capture opponent
					bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
					hashkey ^= nbrBlock->getHash();
				}
			}
		}

		HashKey64 turnKey = StaticBoard::getHashGenerator().getTurnKey();
		return hashkey^m_hash^sgrid.getHash(color) ^ turnKey;
	}
	inline uint getLibertyAfterPlay( const WeichiMove& move ) const
	{
		WeichiBitBoard bmNewLib;
		return getLibertyBitBoardAndLibertyAfterPlay(move,bmNewLib);
	}
	inline uint getLibertyBitBoardAndLibertyAfterPlay( const WeichiMove& move, WeichiBitBoard& bmNewLib ) const
	{
		getLibertyBitBoardAfterPlay(move,bmNewLib);
		return bmNewLib.bitCount();
	}
	inline void getLibertyBitBoardAfterPlay( const WeichiMove& move, WeichiBitBoard& bmNewLib ) const
	{
		assertToFile( getGrid(move.getPosition()).getColor()==COLOR_NONE, this );
		assertToFile( !move.isPass(), this );
		Color myColor = move.getColor();
		Color oppColor = AgainstColor(myColor);
		const WeichiGrid& grid = getGrid(move);
		WeichiBitBoard bmExclude = m_bmBoard;

		bmNewLib = grid.getStonenNbrMap();
		for( const int *iNbr=grid.getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1 ; ++iNbr ) {
			const WeichiGrid& nbrGrid = getGrid(*iNbr);
			if ( nbrGrid.getColor()==myColor ) {
				bmNewLib |= nbrGrid.getBlock()->getStonenNbrMap();
			} else if( nbrGrid.getColor()==oppColor ) {
				if( nbrGrid.getBlock()->getLiberty()==1 ) {
					bmExclude -= nbrGrid.getBlock()->getStoneMap();
				}
			}
		}

		bmNewLib -= bmExclude;
		bmNewLib.SetBitOff(move.getPosition());
	}

	inline uint getLastLibertyPositionAfterPlay( const WeichiMove& move ) const 
	{
		WeichiBitBoard bmNewLib ;
		getLibertyBitBoardAfterPlay(move, bmNewLib);
		assert( bmNewLib.bitCount() == 1 ) ;
		return bmNewLib.bitScanForward() ;
	}

	inline WeichiBitBoard getLibertyBitBoardAfterPlay( const WeichiMove& move ) const
	{
		WeichiBitBoard bmNewLib ;
		getLibertyBitBoardAfterPlay(move, bmNewLib) ;
		return bmNewLib ;
	}

	inline uint getStoneAfterPlay( const WeichiMove& move ) const
	{
		WeichiBitBoard bmNewStones ;
		getStoneBitBoardAfterPlay(move,bmNewStones) ;
		return bmNewStones.bitCount() ;
	}

	inline void getStoneBitBoardAfterPlay( const WeichiMove& move, WeichiBitBoard& bmNewStone ) const
	{
		assertToFile( getGrid(move.getPosition()).getColor()==COLOR_NONE, this );
		assertToFile( !move.isPass(), this );
		Color myColor = move.getColor();
		Color oppColor = AgainstColor(myColor);
		const WeichiGrid& grid = getGrid(move);		

		for( const int *iNbr=grid.getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1 ; ++iNbr ) {
			const WeichiGrid& nbrGrid = getGrid(*iNbr);
			if ( nbrGrid.getColor()==myColor ) {
				bmNewStone |= nbrGrid.getBlock()->getStoneMap();
			} 
		}
		bmNewStone.SetBitOn(move.getPosition()) ;
	}

	inline WeichiBitBoard getStoneBitBoardAfterPlay( const WeichiMove& move ) const
	{
		WeichiBitBoard bmNewStone ;
		getStoneBitBoardAfterPlay(move, bmNewStone) ;
		return bmNewStone ;
	}

	inline bool isEmptyBoard() const { return (m_hash == HashKey64(0)) && (m_moves.size() == 0) ; }

	WeichiBitBoard getCaptureRZone(WeichiMove captureMove) const;
	WeichiBitBoard getUpdatedRZone(Color color, WeichiBitBoard bmStart) const;
	WeichiBitBoard getUpdatedRZoneFromOurTurn(WeichiMove ourMove, WeichiBitBoard bmNextMoveRZone) const;
	WeichiBitBoard getInfluenceBlocks(WeichiMove move) const;
	WeichiBitBoard getInfluenceBitBoard(WeichiMove move) const;
	WeichiBitBoard getAndPlayerRZone(WeichiBitBoard bmRZone, Color ownColor) const;
	WeichiBitBoard getLegalizeRZone(WeichiBitBoard bm, Color ownColor) const;
	WeichiBitBoard getSuicideRZone(WeichiBitBoard bm, Color ownColor) const;
	WeichiBitBoard getOpponentBorderRZone(WeichiBitBoard bm, Color ownColor) const;
	void getMoveInfluence(const WeichiMove& move, WeichiBitBoard& bmInfluence, WeichiBitBoard& bmNbrOwn);
	WeichiBitBoard getMoveRZone(WeichiBitBoard bmRZone, WeichiBitBoard bmOwnInfluence) const;

	WeichiBitBoard getEscapedBlockNbrBlocks( WeichiBitBoard bmFindStone, Color blockColor ) const
	{		
		Color enemyColor = AgainstColor(blockColor) ;		
		WeichiBitBoard bmNbrBlocks = (bmFindStone.dilate()-bmFindStone) & StaticBoard::getMaskBorder() & getStoneBitBoard(enemyColor) ;
		return bmNbrBlocks;
	}
	// go rule
    /*!
        @brief  check if move is illegal (including ko-rule & suicide rule)
        @author T.F. Liao
        @return true if rule pass
    */
    bool isIllegalMove ( WeichiMove move, const OpenAddrHashTable& ht ) const ;
	bool isTTIllegalMove(const WeichiMove& move, const OpenAddrHashTable& ht) const;
    /*!
        @brief  check suicide rule
        @author T.F. Liao
        @return true if rule pass
        pass if
            1. no put in position with stone
            2. after put, must has at least one liberty
    */
    bool checkSuicideRule ( WeichiMove move ) const ;
    /*!
        @brief  check ko rule
        @author T.F. Liao
        @return true if rule pass
        pass if
            1. pass simple ko check
            2. pass super ko check
    */
    bool checkKoRule ( WeichiMove move, const OpenAddrHashTable& ht ) const ;

	// go board visible information
	void showColorBoard() const;
	string toColorBoardString() const;
	string toBoardCoordinateString( bool bShowWithColor=false ) const;
	string toOneRowBoardString( int rowNumber, bool bShowWithColor=false ) const
	{
		Vector<uint,MAX_NUM_GRIDS> vNumberSequence;
		vNumberSequence.setAllAs(-1,MAX_NUM_GRIDS);
		return toOneRowBoardString(rowNumber,vNumberSequence,COLOR_NONE,false,bShowWithColor);
	}
	string toOneRowBoardString( int rowNumber, Vector<uint,MAX_NUM_GRIDS> vNumberSequecne, Color numberColor, bool bChangeColorWithEvenNumber, bool bShowWithColor=false ) const;
	string toOneRowTerritoryString( int rowNumber, bool bShowWithColor=false, Color onlyShowColor=COLOR_NONE ) const;
	std::string toString() const;
	std::string toString( const Vector<uint,MAX_NUM_GRIDS>& vSpecial ) const;
	std::string toSgfFilePrefix() const;
	std::string toSgfFileString(SgfTag sgfTag = SgfTag(), vector<string> vMoveComments = vector<string>()) const;
    std::string getBoardStatus() const;
	std::string toMoveString(bool with_semicolon = false, vector<string> vMoveComments = vector<string>()) const;
	/*!
		@brief  getCloseArea status for gogui drawing
		@author C.C. Shih
	*/
	inline std::string getClosedAreaDrawingString() const { return m_closedAreaHandler.getClosedAreaDrawingString(); }
	inline std::string getClosedAreaLifeAndDeathString() const { return m_closedAreaHandler.getClosedAreaLifeAndDeathString(); }

	// edge
	std::string getEdgeDrawingString() const;

	/*!
        @brief  get real direction value from native value obtained from pattern
        @param  nativeDirection the native value obtained from pattern
        @return                 the real value
        @author T.C. Ho
        @date   2013/7/5
    */
    inline uint realDirectionDIR4of( uint nativeDirection ) const {
        assert( nativeDirection<=7 );
        return nativeDirection & 3;
    }
    inline uint realDirectionDIR4Dof( uint nativeDirection ) const {
        assert( nativeDirection<=7 );
        return nativeDirection;
    }

	WeichiBitBoard getAliveArea(Color cololr) const;
	WeichiBitBoard getDeadArea(Color color) const;

#if DO_WINDOWS_TIMER
    inline MicrosecondTimer& getMicrosecondTimer () { return m_microSecondTimer; }
#endif

private:

	// private function
	void initializeState();
	void initializeGrids();
	void initializeMoveStack();
    void setStone( const WeichiMove& move, BoardMoveStack& moveStack );
	void undoStone( const WeichiMove& lastMove, BoardMoveStack& moveStack );
	void updateFullBoardData( const WeichiMove& move, BoardMoveStack& moveStack );
	void updateSiblings( const WeichiMove& move, BoardMoveStack& moveStack );
	void updateBlocks( WeichiGrid& grid, BoardMoveStack& moveStack );
	void createNewBlock( WeichiGrid& grid );
	void addGridToBlock( WeichiGrid& grid, BoardMoveStack& moveStack );
	void combineBlocks( WeichiGrid& grid, BoardMoveStack& moveStack );
	void updateDeadBlocks( Vector<WeichiBlock*,4>& vDeadBlocks );
	void updateClosedArea( WeichiGrid& grid, BoardMoveStack& moveStack );
	void updateBlockModifyNumber( WeichiGrid& grid );
	void updateCandidateRange( uint pos );	
	void undoClosedArea(WeichiGrid& grid, BoardMoveStack& moveStack);
	void undoBlockModifyNumber(WeichiGrid& grid);
	void undoCandidateRange(uint pos);

	inline BoardMoveStack& getNextMoveStack()
	{
		m_moveStacks.resize(m_moveStacks.size()+1);
		BoardMoveStack& moveStack = m_moveStacks.back();
		moveStack.m_mainBlock = NULL;
		moveStack.m_vDeadBlocks.clear();
		moveStack.m_vCombineBlocks.clear();
		return moveStack;
	}
	inline BoardMoveStack& getCurrentMoveStack() { return m_moveStacks.back(); }
	inline const BoardMoveStack& getCurrentMoveStack() const { return m_moveStacks.back(); }
	inline void updateBoardStatus( BoardMoveStack& moveStack ) { moveStack.m_status = m_status; }
	inline void undoBoardStatus( BoardMoveStack& moveStack ) { m_status = moveStack.m_status; }
	void undoFullBoardData( WeichiMove move, BoardMoveStack& moveStack );
	void undoSiblings( WeichiMove move, BoardMoveStack& moveStack );
	void undoCombineBlocks( WeichiGrid& grid, BoardMoveStack& moveStack );
	void undoDeadBlocks( Vector<WeichiBlock*,4>& vDeadBlocks );
	void setGridToBlock( WeichiBitBoard bmStone, WeichiBlock* block );
	
	inline void setBlockModifyMoveNumber( WeichiBlock* block ) { block->setModifyMoveNumber(m_moves.size()); }
	void addAndRemoveLibertyBitBoard( WeichiBlock* block, uint org_lib );
	void removeLibertyBlock( WeichiBlock* block, uint liberty );
	void addLibertyBlock( WeichiBlock* block, uint liberty );
	void addLibertyStone( WeichiBlock* block, uint stonePos, uint org_lib );
	void removeLibertyStone( WeichiBlock* block, uint stonePos, uint org_lib );

	void updatePatternLibIndex( WeichiGrid& grid );
	void updateNeighborPatternIndex( const WeichiGrid& grid, Color color );
	void updateDeadGridPatternIndex( const WeichiGrid& grid, Color color );
	void updateRadiusPatternIndex( const WeichiGrid& grid, Color color );
	void updateDeadRadiusPatternIndex( const WeichiGrid& grid, Color color );

	string getMemberSizeString();

	// invariance
	bool invariance() const;
	bool invariance_checkFullBoard() const;
	bool invariance_checkGrid() const;
	bool invariance_checkBlock() const;
	bool invariance_checkFullBoardRadiusPattern() const;
	bool invariance_checkEachGridLibertyPattern( const WeichiGrid& grid ) const;
	bool invariance_checkEachGridRadiusPattern( const WeichiGrid& grid ) const;
	bool invariance_checkLibertyList( const FeaturePtrList<WeichiBlock*, MAX_GAME_LENGTH>& libBlockList, Color color, uint liberty ) const;
};

#endif 
