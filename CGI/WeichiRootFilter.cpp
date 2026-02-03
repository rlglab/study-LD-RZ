#include "WeichiRootFilter.h"

void WeichiRootFilter::startFilter()
{
	if( WeichiConfigure::use_closed_area ) {
		m_board.m_closedAreaHandler.removeAllClosedArea();
		m_board.m_closedAreaHandler.findFullBoardUCTClosedArea();
	}
}