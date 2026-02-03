#include "WeichiGtpUtil.h"

vector<WeichiMove> WeichiGtpUtil::getHandiCapStones(uint size, uint n) {
	vector<WeichiMove> handicapStones ;

	int line1 = -1;
	int line2 = -1;
	int line3 = -1;
	if (size >= 13)
	{
		line1 = 4;
		line3 = size - 3;
	}
	else if (size >= 7)
	{
		line1 = 3;
		line3 = size - 2;
	}
	if (size >= 9 && size % 2 != 0)
		line2 = size / 2 + 1;
	
	if (line1 < 0 || n == 1 || n > 9 || (n > 4 && line2 < 0))
		throw std::string("no standard handicap locations defined");

	if (n >= 1)
		handicapStones.push_back(WeichiMove(COLOR_BLACK, WeichiMove::toPosition(line1-1, line1-1)));
	if (n >= 2)
		handicapStones.push_back(WeichiMove(COLOR_BLACK, WeichiMove::toPosition(line3-1, line3-1)));
	if (n >= 3)
		handicapStones.push_back(WeichiMove(COLOR_BLACK, WeichiMove::toPosition(line1-1, line3-1)));
	if (n >= 4)
		handicapStones.push_back(WeichiMove(COLOR_BLACK, WeichiMove::toPosition(line3-1, line1-1)));
	if (n >= 5 && n % 2 != 0)
	{
		handicapStones.push_back(WeichiMove(COLOR_BLACK, WeichiMove::toPosition(line2-1, line2-1)));
		--n;
	}
	if (n >= 5)
		handicapStones.push_back(WeichiMove(COLOR_BLACK, WeichiMove::toPosition(line1-1, line2-1)));
	if (n >= 6)
		handicapStones.push_back(WeichiMove(COLOR_BLACK, WeichiMove::toPosition(line3-1, line2-1)));
	if (n >= 7)
		handicapStones.push_back(WeichiMove(COLOR_BLACK, WeichiMove::toPosition(line2-1, line1-1)));
	if (n >= 8)
		handicapStones.push_back(WeichiMove(COLOR_BLACK, WeichiMove::toPosition(line2-1, line3-1)));

	return handicapStones ;
}

MoveLocation WeichiGtpUtil::WeichiMoveToLocation( WeichiMove wMove) {
	uint x=0, y=0 ;
	WeichiMove::toCoordinate( wMove.getPosition(), x, y );
	MoveLocation location(x, y) ;
	return location ;
}

WeichiMove WeichiGtpUtil::dbMoveToWeichiMove( Color c ,DbMove dMove ) {
	MoveLocation move( dMove.getValue())  ;
	WeichiMove wm( c, WeichiMove::toPosition(move.x,move.y) );
	return wm ;
}