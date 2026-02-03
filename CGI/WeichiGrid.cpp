#include "WeichiGrid.h"

WeichiGrid::WeichiGrid()
    : m_block ( NULL ), m_color ( COLOR_NONE ), m_played ( COLOR_NONE )
{
}

void WeichiGrid::initialize( uint pos )
{
    m_sgrid = &StaticBoard::getGrid(pos) ;
	m_sedge = &StaticBoard::getEdge(pos) ;

    m_color = COLOR_NONE ;
    m_played = COLOR_NONE ;
    m_position = pos ;
    const StaticGrid& sgrid = getStaticGrid() ;
    m_liberty = 4 ;
    if ( sgrid.getXLineNo() == 1 ) --m_liberty ;
    if ( sgrid.getYLineNo() == 1 ) --m_liberty ;

    m_block = NULL ;
	m_edge = NULL ;
	m_closedArea.get(COLOR_BLACK) = NULL;
	m_closedArea.get(COLOR_WHITE) = NULL;

	m_nbrLibIndex = 0;
	m_newPatternIndex = 0 ;

	WeichiMove m = WeichiMove ( static_cast<uint>(pos) ) ;
	uint m_x = m.x();
	uint m_y = m.y();

	if( m_x==0 && m_y==0 ) { setPatternIndex(pattern33::START_ADDR_THREE_LD); }
	else if( m_x==WeichiConfigure::BoardSize-1 && m_y==0 ) { setPatternIndex(pattern33::START_ADDR_THREE_RD); }
	else if( m_x==0 && m_y==WeichiConfigure::BoardSize-1 ) { setPatternIndex(pattern33::START_ADDR_THREE_LU); }
	else if( m_x==WeichiConfigure::BoardSize-1 && m_y==WeichiConfigure::BoardSize-1 ) { setPatternIndex(pattern33::START_ADDR_THREE_RU); }
	else if( m_x==0 ) { setPatternIndex(pattern33::START_ADDR_FIVE_LEFT); }
	else if( m_x==WeichiConfigure::BoardSize-1 ) { setPatternIndex(pattern33::START_ADDR_FIVE_RIGHT); }
	else if( m_y==0 ) { setPatternIndex(pattern33::START_ADDR_FIVE_DOWN); }
	else if( m_y==WeichiConfigure::BoardSize-1 ) { setPatternIndex(pattern33::START_ADDR_FIVE_UPPER); }
	else { setPatternIndex(pattern33::START_ADDR_EIGHT) ; }


	// initialize radius pattern index
	m_vPatternIndex.resize(MAX_RADIUS_SIZE);
	for( uint iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		m_vPatternIndex[iRadius] = sgrid.getInitialRadiusPattern(iRadius);
	}
}

std::string WeichiGrid::getEdgeLinesString() const
{
	if( !m_edge ) { return ""; }

	uint pos;
	ostringstream oss;
	WeichiMove ownMove(m_position);
	WeichiBitBoard bmEndStone = m_edge->getEndStone();
	while( (pos=bmEndStone.bitScanForward())!=-1 ) {
		if( pos==ownMove.getPosition() ) { continue; }
		oss << ownMove.toGtpString() << ' ' << WeichiMove(pos).toGtpString() << " 8 OCEANBLUE ";
	}

	if( (WeichiConfigure::DrawBlack && m_color==COLOR_BLACK) ||
		(WeichiConfigure::DrawWhite && m_color==COLOR_WHITE) )
	{
		return oss.str();
	}

	return "";
/*
	uint pos;
	ostringstream oss;
	WeichiMove m(m_position);
	WeichiBitBoard bmStone = board.getBitBoard();
	WeichiBitBoard bmEndStone;

	//draw strong cut
	if( WeichiConfigure::DrawStrong && WeichiConfigure::DrawCut ) {
		if( !getBlock() || getBlock()->getLiberty()>1 ) {
			bmEndStone = getStrongCutEndStone();
			while( (pos=bmEndStone.bitScanForward())!=-1 ) {
				WeichiMove endMove(pos);
				const WeichiGrid& endGrid = board.getGrid(pos);

				if( !endGrid.getBlock() || endGrid.getBlock()->getLiberty()>1 ) {
					oss << m.toGtpString() << " " << endMove.toGtpString() << " 8 OCEANBLUE ";
				}
			}
		}
	}

	//draw weak cut
	if( WeichiConfigure::DrawWeak && WeichiConfigure::DrawCut ) {
		bmEndStone = getStrongCutEndStone();
		while( (pos=bmEndStone.bitScanForward())!=-1 ) {
			WeichiMove endMove(pos);
			const WeichiGrid& endGrid = board.getGrid(pos);

			if( (getBlock() && getBlock()->getLiberty()==1) || (endGrid.getBlock() && endGrid.getBlock()->getLiberty()==1) ) {
				oss << m.toGtpString() << " " << endMove.toGtpString() << " 3 OCEANBLUE ";
			}
		}
	}

	//draw strong connector
	if( WeichiConfigure::DrawStrong && WeichiConfigure::DrawConnector ) {
		if( !getBlock() || getBlock()->getLiberty()>1 ) {
			bmEndStone = getAnotherEndPosByStrongConnector(board);
			while( (pos=bmEndStone.bitScanForward())!=-1 ) {
				WeichiMove endMove(pos);
				
				oss << m.toGtpString() << " " << endMove.toGtpString() << " 8 RED ";

				/ *const WeichiGrid& endGrid = board.getGrid(pos);

				if( !endGrid.getBlock() || endGrid.getBlock()->getLiberty()>1 ) {
					oss << m.toGtpString() << " " << endMove.toGtpString() << " 8 RED ";
				}* /
			}
		}
	}*/

	//draw weak connector
	/*if( WeichiConfigure::DrawWeak && WeichiConfigure::DrawConnector ) {
		bmEndStone = getConnectorEndStone();
		while( (pos=bmEndStone.bitScanForward())!=-1 ) {
			WeichiMove endMove(pos);
			const WeichiGrid& endGrid = board.getGrid(pos);

			if( (getBlock() && getBlock()->getLiberty()==1) || (endGrid.getBlock() && endGrid.getBlock()->getLiberty()==1) ) {
				oss << m.toGtpString() << " " << endMove.toGtpString() << " 3 RED ";
			}
		}
	}*/

	//return oss.str();
}