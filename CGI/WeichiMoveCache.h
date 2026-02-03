#ifndef WEICHIUCTMOVECACHE_H
#define WEICHIUCTMOVECACHE_H

#include <map>
#include <boost/atomic.hpp>
#include "WeichiGrid.h"
#include "WeichiBoard.h"
#include "LinkListHashTable.h"
#include "CandidateEntry.h"

class WeichiMoveCacheContext {
public:
	bool m_bIsValid;
	short m_step;
	WeichiMove m_replyMove;
	HashKey64 m_triggerKey;
	HashKey64 m_replyKey;

	WeichiMoveCacheContext( istringstream& stream )
	{
		char c;
		uint pos;
		stream >> hex >> m_triggerKey >> m_replyKey >> dec >> m_step >> c >> pos >> m_bIsValid;
		m_bIsValid = true;
		m_replyMove = WeichiMove(toColor(c),pos);
	}
	WeichiMoveCacheContext( bool bIsValid=false, short step=-1, WeichiMove replyMove=PASS_MOVE, HashKey64 triggerKey=0, HashKey64 replyKey=0 )
		: m_bIsValid(bIsValid), m_step(step), m_replyMove(replyMove), m_triggerKey(triggerKey), m_replyKey(replyKey) {}
	string toString()
	{
		ostringstream oss;
		oss << hex << m_triggerKey << ' ' << m_replyKey << ' ' << dec << m_step
			<< ' ' << toChar(m_replyMove.getColor()) << ' ' << m_replyMove.getPosition() << ' '
			<< m_bIsValid;
		return oss.str();
	}
};

class WeichiMoveCache {
private:
	class CacheHashKeyInfo {
	public:
		HashKey64 m_blackColor;
		HashKey64 m_3x3HashKey[8][COLOR_SIZE];
		HashKey64 m_nbrLibHashKey[RADIUS_NBR_LIB_SIZE][5];		// 1,2,3,4,5+
		HashKey64 m_nbrStoneHashKey[RADIUS_NBR_LIB_SIZE][5];	// 1,2,3,4,5+
		HashKey64 m_positionHashKey[MAX_NUM_GRIDS];
		HashKey64 m_3x3HashTable[1 << 16];

		CacheHashKeyInfo() {}
		void initialize();
	};
	class TriggerMoveContext {
	private:
		short m_step;
		HashKey64 m_replyKey;
		uint m_replyPosition;

	public:
		TriggerMoveContext() {}
		void clear()
		{
			m_step = 0;
			m_replyKey = 0;
			m_replyPosition = PASS_MOVE.getPosition();
		}
		uint getStep() const { return m_step; }
		HashKey64 getReplyHashKey() const { return m_replyKey; }
		uint getReplyPosition() const { return m_replyPosition; }
		void setStep(short step) { m_step = step; }
		void setReplyHashKey(HashKey64 replyKey) { m_replyKey = replyKey; }
		void setReplyPosition(uint replyPosition) { m_replyPosition = replyPosition; }
	};
	typedef LinkListHashTable<TriggerMoveContext> TriggerHashTable;

	uint m_nTriggerEntryBeforeClean;
	TriggerHashTable m_triggerHashTable;
	static CacheHashKeyInfo m_replyHashKeyInfo;
	static CacheHashKeyInfo m_triggerHashKeyInfo;

public:
	WeichiMoveCache()
		: m_triggerHashTable(16,16)
		, m_nTriggerEntryBeforeClean(0)
	{}

	static void initialize();

	void clear();
	void cleanHashTable();
	void insert(const WeichiBoard& board, const WeichiMove& triggerMove, const WeichiMove& replyMove, short step);
	WeichiMoveCacheContext getInsertData(const WeichiBoard& board, const WeichiMove& triggerMove, const WeichiMove& replyMove) const;
	WeichiMove getReplyMove(const WeichiBoard& board, const WeichiMove& triggerMove) const;

	inline string getTriggerUseRateString() const { return reportUseRateString(m_triggerHashTable.getValidEntryCount(), m_triggerHashTable.getEntrySize()); }
	inline string getTriggerUseRateStringBeforeClean() const { return reportUseRateString(m_nTriggerEntryBeforeClean, m_triggerHashTable.getEntrySize()); }

private:
	HashKey64 calculateGridHashKey( const WeichiBoard& board, const WeichiGrid& grid, Color centerColor, bool bIsTrigger ) const;
	string reportUseRateString( uint useEntry, uint totalSize ) const;
};

#endif