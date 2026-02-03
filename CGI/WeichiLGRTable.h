#ifndef __WEICHILGRTABLE_H__
#define __WEICHILGRTABLE_H__

#include "Dual.h"
#include "WeichiMove.h"

class WeichiLGRTable
{
private:
	Dual<uint> m_reply1[MAX_NUM_GRIDS];
	Dual<uint> m_reply2[MAX_NUM_GRIDS][MAX_NUM_GRIDS];
public:
	WeichiLGRTable();

	void clear();
	void setLastGoodReply( const Vector<WeichiMove, MAX_GAME_LENGTH>& moves, Color winColor );
	WeichiMove getLastGoodReply( const WeichiMove& secondLastMove, const WeichiMove& lastMove, Color colorToPlay );

private:
	inline WeichiMove getLastGoodReply1( const WeichiMove& lastMove, Color colorToPlay )
		{ return WeichiMove(colorToPlay,m_reply1[lastMove.getPosition()].get(colorToPlay)); }
	inline WeichiMove getLastGoodReply2( const WeichiMove& secondLastMove, const WeichiMove& lastMove, Color colorToPlay )
		{ return WeichiMove(colorToPlay,m_reply2[secondLastMove.getPosition()][lastMove.getPosition()].get(colorToPlay)); }
};

#endif