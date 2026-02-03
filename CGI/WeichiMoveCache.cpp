#include "WeichiMoveCache.h"
#include "WeichiThreadState.h"

WeichiMoveCache::CacheHashKeyInfo WeichiMoveCache::m_replyHashKeyInfo;
WeichiMoveCache::CacheHashKeyInfo WeichiMoveCache::m_triggerHashKeyInfo;

void WeichiMoveCache::CacheHashKeyInfo::initialize()
{
	m_blackColor = rand64();
	for( int i=0; i<8; i++ ) {
		for( int color=0; color<COLOR_SIZE; color++ ) {
			m_3x3HashKey[i][color] = rand64();
		}
	}
	for( int i=0; i<RADIUS_NBR_LIB_SIZE; i++ ) {
		for( int lib=0; lib<5; lib++ ) {
			m_nbrLibHashKey[i][lib] = rand64();
			m_nbrStoneHashKey[i][lib] = rand64();
		}
	}
	for( int i=0; i<MAX_NUM_GRIDS; i++ ) {
		m_positionHashKey[i] = rand64();
	}
	for( int i=0; i<(1<<16); i++ ) {
		m_3x3HashTable[i] = 0;
		for( int j=0; j<8; j++ ) {
			int gridStatus = (i >> (j*2))& 0x3;
			if( gridStatus==0 ) { continue; }

			m_3x3HashTable[i] ^= m_3x3HashKey[j][gridStatus];
		}
	}
}

void WeichiMoveCache::initialize()
{
	m_replyHashKeyInfo.initialize();
	m_triggerHashKeyInfo.initialize();
}

void WeichiMoveCache::clear()
{
	m_triggerHashTable.clear();
}

void WeichiMoveCache::cleanHashTable()
{
	m_nTriggerEntryBeforeClean = m_triggerHashTable.getValidEntryCount();

	const int CLEAR_STEP = 10;
	short currentStep = WeichiGlobalInfo::getTreeInfo().m_nMove;

	// collect remove id
	vector<uint> vRemoveID;
	for (uint index = 0; index < m_triggerHashTable.getValidEntryCount(); index++) {
		uint entryID = m_triggerHashTable.getValidEntryID(index);
		TriggerMoveContext& triggerMoveContext = m_triggerHashTable.m_entry[entryID].m_data;

		if (currentStep - triggerMoveContext.getStep() <= CLEAR_STEP) { continue; }
		vRemoveID.push_back(entryID);
	}

	// remove entry
	for (uint i = 0; i < vRemoveID.size(); i++) {
		m_triggerHashTable.erase(vRemoveID[i]);
	}
}

void WeichiMoveCache::insert(const WeichiBoard& board, const WeichiMove& triggerMove, const WeichiMove& replyMove, short step)
{
	if (triggerMove.isPass() || replyMove.isPass()) { return; }

	WeichiMoveCacheContext moveCacheContext = getInsertData(board, triggerMove, replyMove);
	if (!moveCacheContext.m_bIsValid) { return; }
	if (m_triggerHashTable.isTableFull()) { return; }

	bool bHasInsert = false;
	uint triggerIndex = m_triggerHashTable.lookupWithInsert(moveCacheContext.m_triggerKey, bHasInsert);
	TriggerMoveContext& triggerMoveContext = m_triggerHashTable.m_entry[triggerIndex].m_data;

	// insert trigger entry
	triggerMoveContext.setStep(step);
	triggerMoveContext.setReplyHashKey(moveCacheContext.m_replyKey);
	triggerMoveContext.setReplyPosition(moveCacheContext.m_replyMove.getPosition());
}

WeichiMoveCacheContext WeichiMoveCache::getInsertData(const WeichiBoard& board, const WeichiMove& triggerMove, const WeichiMove& replyMove) const
{
	if( triggerMove.isPass() || replyMove.isPass() ) { return WeichiMoveCacheContext(); }

	// note: we use trigger grid pattern index after play
	const WeichiGrid& triggerGrid = board.getGrid(triggerMove);
	const WeichiGrid& replyGrid = board.getGrid(replyMove);

	// restriction: both empty 3x3 will not insert to cache
	if( triggerGrid.getRadiusPatternRealIndex(3)==0 && replyGrid.getRadiusPatternRealIndex(3)==0 ) { return WeichiMoveCacheContext(); }

	const HashKey64 triggerKey = calculateGridHashKey(board,triggerGrid,triggerMove.getColor(),true);
	const HashKey64 replyKey = triggerKey ^ calculateGridHashKey(board,replyGrid,replyMove.getColor(),false);

	return WeichiMoveCacheContext(true,WeichiGlobalInfo::getTreeInfo().m_nMove,replyMove,triggerKey,replyKey);
}

WeichiMove WeichiMoveCache::getReplyMove(const WeichiBoard& board, const WeichiMove& triggerMove) const
{
	Color turnColor = board.getToPlay();
	HashKey64 triggerKey = calculateGridHashKey(board, board.getGrid(triggerMove), triggerMove.getColor(), true);
	uint index = m_triggerHashTable.lookup(triggerKey);
	if (index == -1) { return WeichiMove(turnColor); }

	TriggerMoveContext& triggerMoveContext = m_triggerHashTable.m_entry[index].m_data;
	HashKey64 replyKey = triggerMoveContext.getReplyHashKey();
	uint replyPosition = triggerMoveContext.getReplyPosition();
	
	if ((triggerKey ^ calculateGridHashKey(board, board.getGrid(replyPosition), turnColor, false)) == replyKey) {
		return WeichiMove(turnColor, replyPosition);
	} else { return WeichiMove(turnColor); }
}

HashKey64 WeichiMoveCache::calculateGridHashKey( const WeichiBoard& board, const WeichiGrid& grid, Color centerColor, bool bIsTrigger ) const
{
	int patternIndex = grid.getRadiusPatternRealIndex(3);
	const CacheHashKeyInfo& hashKeyInfo = bIsTrigger ? m_triggerHashKeyInfo: m_replyHashKeyInfo;
	HashKey64 key = hashKeyInfo.m_3x3HashTable[patternIndex];

	for( int iNbr=0; iNbr<RADIUS_NBR_LIB_SIZE; iNbr++ ) {
		int dir = WeichiRadiusPatternTable::vRadiusNbrLibOrder[iNbr];
		if( grid.getStaticGrid().getNeighbor(dir)==-1 ) { continue; }

		const WeichiGrid& nbrGrid = board.getGrid(grid,dir);
		if( nbrGrid.isEmpty() ) { continue; }

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if( nbrBlock->getLiberty()!=1 ) { continue; }

		// lib: 1~5+, count: 1~5+
		int lib = nbrBlock->getLiberty();
		int count = nbrBlock->getNumStone();
		lib = (lib>=5) ? 5: lib;
		count = (count>=5) ? 5: count;
		if( lib>0 ) { key ^= hashKeyInfo.m_nbrLibHashKey[iNbr][lib]; }
		if( count>0 ) { key ^= hashKeyInfo.m_nbrStoneHashKey[iNbr][count]; }
	}

	// position key
	if( centerColor==COLOR_BLACK ) { key ^= hashKeyInfo.m_blackColor; }
	key ^= hashKeyInfo.m_positionHashKey[grid.getPosition()];

	return key;
}

string WeichiMoveCache::reportUseRateString( uint useEntry, uint totalSize ) const
{
	ostringstream oss;
	oss << setprecision(3) << (static_cast<double>(useEntry)*100.0/totalSize) << "% (" << useEntry << ")";

	return oss.str();
}