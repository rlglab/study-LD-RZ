#include "TTutility.h"
#include "WeichiThreadState.h"

TTentry TtUtility::createTtEntry(const WeichiThreadState& state, const TTentry& ttEntry)
{
	const WeichiBoard& board = state.m_board;
	WeichiBitBoard bmTtRZone = ttEntry.m_bmRZone;

	TTentry queryEntry;
	queryEntry.m_turnColor = board.getToPlay();
	queryEntry.m_bmBlackPiece = board.getStoneBitBoard(COLOR_BLACK) & bmTtRZone;
	queryEntry.m_bmWhitePiece = board.getStoneBitBoard(COLOR_WHITE) & bmTtRZone;
	queryEntry.m_bmEmptyPiece = bmTtRZone - queryEntry.m_bmBlackPiece - queryEntry.m_bmWhitePiece;
	queryEntry.m_koPosition = getKoPosition(state, ttEntry);

	return queryEntry;
}

int TtUtility::getKoPosition(const WeichiThreadState& state, const TTentry& entry)
{
	const WeichiBoard& board = state.m_board;

	Color winColor = Color(WeichiConfigure::win_color);
	Color oppColor = AgainstColor(winColor);
	if (entry.m_turnColor == winColor) { return getEatKoPosition(state, entry); }
	else if (entry.m_turnColor == oppColor) { return getForbiddenKoPosition(state, entry); }

	return -1;
}

int TtUtility::getEatKoPosition(const WeichiThreadState& state, const TTentry& entry)
{
	const WeichiBoard& board = state.m_board;

	Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
	Color oppColor = AgainstColor(winColor);

	assert(entry.m_turnColor == winColor);
	if (entry.m_koPosition == -1) { return -1; }

	WeichiMove nextWinMove(winColor, entry.m_koPosition);
	if (board.isIllegalMove(nextWinMove, state.m_ht)) { return -1; }

	return entry.m_koPosition;
}

int TtUtility::getForbiddenKoPosition(const WeichiThreadState& state, const TTentry& entry)
{
	const WeichiBoard& board = state.m_board;
	if (!board.hasKo()) { return -1; }

	Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
	Color oppColor = AgainstColor(winColor);

	int koPosition = -1;
	if (entry.m_status == UCT_STATUS_WIN && entry.m_turnColor == oppColor) {
		WeichiBitBoard bmRZone = entry.m_bmRZone;
		if (bmRZone.BitIsOn(board.getKo())) { koPosition = board.getKo(); }
	}
	else if (entry.m_status == UCT_STATUS_LOSS && entry.m_turnColor == winColor) {
		WeichiBitBoard bmRZone = entry.m_bmRZone;
		if (bmRZone.BitIsOn(board.getKo())) { koPosition = board.getKo(); }
	}

	return koPosition;
} 
