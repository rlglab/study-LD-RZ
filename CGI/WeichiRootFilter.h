#ifndef WEICHIROOTFILTER_H
#define WEICHIROOTFILTER_H

#include "WeichiBoard.h"

class WeichiRootFilter
{
private:
	WeichiBoard& m_board;

public:
	WeichiRootFilter( WeichiBoard& board )
		: m_board(board) {}
	void startFilter();

};

#endif