#include "WeichiLGRTable.h"
#include "StaticBoard.h"

WeichiLGRTable::WeichiLGRTable()
{
}

void WeichiLGRTable::clear()
{
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		m_reply1[*it].get(COLOR_BLACK) = m_reply1[*it].get(COLOR_WHITE) = PASS_MOVE.getPosition();
		for( StaticBoard::iterator it2=StaticBoard::getIterator(); it2; ++it2 ) {
			m_reply2[*it][*it2].get(COLOR_BLACK) = m_reply2[*it][*it2].get(COLOR_WHITE) = PASS_MOVE.getPosition();
		}
	}
}

void WeichiLGRTable::setLastGoodReply( const Vector<WeichiMove, MAX_GAME_LENGTH>& moves, Color winColor )
{
	// draw or move size less than 2 moves do not update LGR table
	if( winColor==COLOR_NONE ) { return; }
	if( moves.size()<2 ) { return; }

	// update reply1 & reply2
	for( uint i=moves.size()-1; i>=2; i-- ) {
		WeichiMove currentMove = moves[i];
		WeichiMove lastMove = moves[i-1];		
		if( currentMove.isPass() || lastMove.isPass() ) { continue; }

		Color updateColor = currentMove.getColor();
		if( updateColor!=winColor ) { currentMove = PASS_MOVE; }
		m_reply1[lastMove.getPosition()].get(updateColor) = currentMove.getPosition();
		
		WeichiMove secondLastMove = moves[i-2];
		if( secondLastMove.isPass() ) { continue; }
		m_reply2[secondLastMove.getPosition()][lastMove.getPosition()].get(updateColor) = currentMove.getPosition();
	}

	// update first reply1
	WeichiMove currentMove = moves[1];
	WeichiMove lastMove = moves[0];
	Color updateColor = currentMove.getColor();
	if( !currentMove.isPass() && !lastMove.isPass() ) {
		if( updateColor!=winColor ) { currentMove = PASS_MOVE; }
		m_reply1[lastMove.getPosition()].get(updateColor) = currentMove.getPosition();
	}
}

WeichiMove WeichiLGRTable::getLastGoodReply( const WeichiMove& secondLastMove, const WeichiMove& lastMove, Color colorToPlay )
{
	WeichiMove replyMove = PASS_MOVE;
	if ( !secondLastMove.isPass() ) { getLastGoodReply2(secondLastMove,lastMove,colorToPlay); }
	if ( replyMove.isPass() && !lastMove.isPass() ) { replyMove = getLastGoodReply1(lastMove,colorToPlay); }

	return replyMove;
}