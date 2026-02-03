#include "StaticGrid.h"
#include "StaticBoard.h"

void StaticGrid::initialize( uint position )
{
    initializeFeatures ( position ) ;
    initializeNeighbors ( position ) ;
    initializeCandidateRange ( position ) ;
    initializeZHashKey ( position ) ;
    initializeDifference ( position ) ;
	initializeRadiusGridRelation ( position ) ;
	initializeLadderPath ( position ) ;
}

int StaticGrid::getNeighbor( uint position, uint direction )
{
    WeichiMove m = WeichiMove ( static_cast<uint>(position) ) ;
    switch ( direction ) {
    case LEFT_IDX:        return ( (m.x()>0) ? WeichiMove::toPosition(m.x()-1, m.y()) : -1 ) ;
    case UPPER_IDX:       return ( (m.y()<WeichiConfigure::BoardSize-1) ? WeichiMove::toPosition(m.x(), m.y()+1) : -1 ) ;
    case RIGHT_IDX:       return ( (m.x()<WeichiConfigure::BoardSize-1) ? WeichiMove::toPosition(m.x()+1, m.y()) : -1 ) ;
    case DOWN_IDX:        return ( (m.y()>0) ? WeichiMove::toPosition(m.x(), m.y()-1) : -1 ) ;

    case LEFT_DOWN_IDX:   return ( (m.x()>0&&m.y()>0) ? WeichiMove::toPosition(m.x()-1, m.y()-1) : -1 ) ;
    case LEFT_UPPER_IDX:  return ( (m.x()>0&&m.y()<WeichiConfigure::BoardSize-1) ? WeichiMove::toPosition(m.x()-1, m.y()+1) : -1 ) ;
    case RIGHT_UPPER_IDX: return ( (m.x()<WeichiConfigure::BoardSize-1&&m.y()<WeichiConfigure::BoardSize-1) ? WeichiMove::toPosition(m.x()+1, m.y()+1) : -1 ) ;
    case RIGHT_DOWN_IDX:  return ( (m.x()<WeichiConfigure::BoardSize-1&&m.y()>0) ? WeichiMove::toPosition(m.x()+1, m.y()-1) : -1 ) ;

    case LEFT2_IDX:       return ( (m.x()>1) ? WeichiMove::toPosition(m.x()-2, m.y()) : -1 ) ;
    case UPPER2_IDX:      return ( (m.y()<WeichiConfigure::BoardSize-2) ? WeichiMove::toPosition(m.x(), m.y()+2) : -1 ) ;
    case RIGHT2_IDX:      return ( (m.x()<WeichiConfigure::BoardSize-2) ? WeichiMove::toPosition(m.x()+2, m.y()) : -1 ) ;
    case DOWN2_IDX:       return ( (m.y()>1) ? WeichiMove::toPosition(m.x(), m.y()-2) : -1 ) ;
    default: return -1; 
    }
}

int StaticGrid::getReverseNeighbor( uint position, uint direction )
{
	WeichiMove m = WeichiMove ( static_cast<uint>(position) ) ;
	switch ( direction ) {
	case LEFT_IDX:        return getNeighbor(position, RIGHT_IDX) ;
	case UPPER_IDX:       return getNeighbor(position, DOWN_IDX) ;
	case RIGHT_IDX:       return getNeighbor(position, LEFT_IDX) ;
	case DOWN_IDX:        return getNeighbor(position, UPPER_IDX) ;

	case LEFT_DOWN_IDX:   return getNeighbor(position, RIGHT_UPPER_IDX) ;
	case LEFT_UPPER_IDX:  return getNeighbor(position, RIGHT_DOWN_IDX) ;
	case RIGHT_UPPER_IDX: return getNeighbor(position, LEFT_DOWN_IDX) ;
	case RIGHT_DOWN_IDX:  return getNeighbor(position, LEFT_UPPER_IDX) ;

	case LEFT2_IDX:       return getNeighbor(position, RIGHT2_IDX) ;
	case UPPER2_IDX:      return getNeighbor(position, DOWN2_IDX) ;
	case RIGHT2_IDX:      return getNeighbor(position, LEFT2_IDX) ;
	case DOWN2_IDX:       return getNeighbor(position, UPPER2_IDX) ;
	default: return -1; 
	}
}

void StaticGrid::initializeFeatures( uint position )
{
    WeichiMove m = WeichiMove ( static_cast<uint>(position) ) ;
    m_pos = position ;
    m_x = m.x();
    m_y = m.y();

    m_xLineNo = std::min(m_x, (int)WeichiConfigure::BoardSize-m_x-1) + 1 ; 
    m_yLineNo = std::min(m_y, (int)WeichiConfigure::BoardSize-m_y-1) + 1 ; 
    m_LineNo = std::min(m_xLineNo, m_yLineNo) ;
	m_LongLineNo = std::max(m_xLineNo, m_yLineNo) ;
}


void StaticGrid::initializeNeighbors ( uint position )
{
    // neighbor
	for( uint i=0; i<MAX_NUM_GRIDS; i++ ) {
		m_grid_direction[i] = -1;
	}
    for ( uint i=0;i<NUM_DIRECTIONS;++i ) {
        m_neighbors[i] = getNeighbor(position, i) ;
		if( m_neighbors[i]!=-1 ) { m_grid_direction[m_neighbors[i]] = i; }
    }
    {
        // adjacent
        uint len = 0 ;
        for ( uint i=0;i<4;++i ) {
            if ( m_neighbors[i] != -1 ) {
                m_adjrel[len] = i ;
                m_adjpos[len++] = m_neighbors[i];
                m_grid_relation[m_neighbors[i]].adjacent = true; 
            }
        } m_adjpos[len] = m_adjrel[len] = -1 ;
    }
    {
        // diagonal
        uint len = 0 ;
        for ( uint i=4;i<8;++i ) {
            if ( m_neighbors[i] != -1 ) {
                m_diagrel[len] = i ;
                m_diagpos[len++] = m_neighbors[i];
                m_grid_relation[m_neighbors[i]].diagonal = true; 
            }
        } m_diagpos[len] = m_diagrel[len] = -1 ;
    }
	{
		// diamond end point
		uint len = 0 ;
		for ( uint i=8;i<12;++i ) {
			if ( m_neighbors[i] != -1 ) {
				m_diamondendrel[len] = i ;
				m_diamondendpos[len++] = m_neighbors[i];
			}
		} m_diamondendpos[len] = m_diamondendrel[len] = -1 ;
	}
    {
        // 3x3
        uint len = 0 ;
        for ( uint i=0;i<8;++i ) {
            if ( m_neighbors[i] != -1 ) {
                m_3x3rel[len] = i ;
                m_3x3pos[len++] = m_neighbors[i];
                m_grid_relation[m_neighbors[i]].in3x3 = true; 
            }
        } m_3x3pos[len] = m_3x3rel[len] = -1 ;
    }
    {
        // diamond
        uint len = 0 ;
        for ( uint i=0;i<12;++i ) {
            if ( m_neighbors[i] != -1 ) {
                m_diamondrel[len] = i ;
                m_diamondpos[len++] = m_neighbors[i];
            }
        } m_diamondpos[len] = m_diamondrel[len] = -1 ;
    }

	{
		m_bmStoneNbrs.Reset();
		m_bmStoneNbrs.SetBitOn(position);
		const int * nbr = m_adjpos;
		for ( ; *nbr!=-1 ; ++ nbr ) {
			m_bmStoneNbrs.SetBitOn(*nbr) ;
		}
	}

	{
		m_bm3x3StoneNbrs.Reset();
		m_bm3x3StoneNbrs.SetBitOn(position);
		const int * nbr = m_3x3pos;
		for ( ; *nbr!=-1 ; ++ nbr ) {
			m_bm3x3StoneNbrs.SetBitOn(*nbr) ;
		}
	}	
}

void StaticGrid::initializeCandidateRange( uint position )
{
	m_nCandidates = 0;
	m_bmCandidates.Reset();
	WeichiMove move(position);

	if( move.x()==1 ) {
		for( int d=-3; d<=3; d++ ) {
			if( move.y()+d<0 || move.y()+d>=WeichiConfigure::BoardSize ) { continue; }
			m_bmCandidates.SetBitOn(WeichiMove::toPosition(0,move.y()+d));
		}
	} else if( move.x()==WeichiConfigure::BoardSize-2 ) {
		for( int d=-3; d<=3; d++ ) {
			if( move.y()+d<0 || move.y()+d>=WeichiConfigure::BoardSize ) { continue; }
			m_bmCandidates.SetBitOn(WeichiMove::toPosition(WeichiConfigure::BoardSize-1,move.y()+d));
		}
	}

	if( move.y()==1 ) {
		for( int d=-3; d<=3; d++ ) {
			if( move.x()+d<0 || move.x()+d>=WeichiConfigure::BoardSize ) { continue; }
			m_bmCandidates.SetBitOn(WeichiMove::toPosition(move.x()+d,0));
		}
	} else if( move.y()==WeichiConfigure::BoardSize-2 ) {
		for( int d=-3; d<=3; d++ ) {
			if( move.x()+d<0 || move.x()+d>=WeichiConfigure::BoardSize ) { continue; }
			m_bmCandidates.SetBitOn(WeichiMove::toPosition(move.x()+d,WeichiConfigure::BoardSize-1));
		}
	}

	m_bmCandidates -= StaticBoard::getInitCandidate();
	Vector<uint,MAX_NUM_GRIDS> vCandidates;
	m_bmCandidates.bitScanAll(vCandidates);
	m_nCandidates = vCandidates.size();
	for( uint i=0; i<m_nCandidates; i++ ) {
		m_candidates[i] = vCandidates[i];
	}
}

void StaticGrid::initializeZHashKey( uint position )
{
    m_hash[COLOR_BLACK] = StaticBoard::getHashGenerator().getZHashKeyOf(COLOR_BLACK, position) ;
    m_hash[COLOR_WHITE] = StaticBoard::getHashGenerator().getZHashKeyOf(COLOR_WHITE, position) ;
}
/*
    4   1   5
    0   .   2
    7   3   6
*/
static const int indexDifferenceTable[9][8] = 
{ //    0   1   2   3   4   5   6   7
    {   0,  1,  9,  0,  0,  3,  0,  0},       //m_x==0 && m_y==0
    {   1,  9,  0,  0,  3,  0,  0,  0},       //m_x==BOARD_SIZE-1 && m_y==0
    {   0,  0,  1,  9,  0,  0,  3,  0},       //m_x==0 && m_y==BOARD_SIZE-1
    {   1,  0,  0,  3,  0,  0,  0,  9},       //m_x==BOARD_SIZE-1 && m_y==BOARD_SIZE-1
    {   0,  1,  9, 81,  0,  3, 27,  0},       //m_x==0
    {   1,  9,  0, 27,  3,  0,  0, 81},       //m_x==BOARD_SIZE-1
    {   1,  9, 81,  0,  3, 27,  0,  0},       //m_y==0
    {   1,  0,  3, 27,  0,  0,  9, 81},       //m_y==BOARD_SIZE-1
    {   1,  9, 81,729,  3, 27,243, 2187}      //neither of above
};

int StaticGrid::getOffset ( int nbr, int rel ) 
{
    int reverse_dir = StaticBoard::getRevDirect(rel);

    WeichiMove wm = WeichiMove(nbr);

	if( wm.x()==0 && wm.y()==0 )															{ return indexDifferenceTable[0][reverse_dir]; }
	else if( wm.x()==WeichiConfigure::BoardSize-1 && wm.y()==0 )							{ return indexDifferenceTable[1][reverse_dir]; }
	else if( wm.x()==0 && wm.y()==WeichiConfigure::BoardSize-1 )							{ return indexDifferenceTable[2][reverse_dir]; }
    else if( wm.x()==WeichiConfigure::BoardSize-1 && wm.y()==WeichiConfigure::BoardSize-1 )	{ return indexDifferenceTable[3][reverse_dir]; }
	else if( wm.x()==0 )																	{ return indexDifferenceTable[4][reverse_dir]; }
	else if( wm.x()==WeichiConfigure::BoardSize-1 )											{ return indexDifferenceTable[5][reverse_dir]; }
	else if( wm.y()==0 )																	{ return indexDifferenceTable[6][reverse_dir]; }
	else if( wm.y()==WeichiConfigure::BoardSize-1 )											{ return indexDifferenceTable[7][reverse_dir]; }
    else																					{ return indexDifferenceTable[8][reverse_dir]; }
}

void StaticGrid::initializeDifference( uint position )
{
    const int* nbr = get3x3Neighbors();
    const int* rel = get3x3Relations();

    for ( ; *nbr != -1 ; ++nbr, ++rel ) {
        const int offset = getOffset(*nbr, *rel);
        m_playBlackIndexDifference[*rel] = COLOR_BLACK * offset ; 
        m_playWhiteIndexDifference[*rel] = COLOR_WHITE * offset ;
    }
}

void StaticGrid::initializeRadiusGridRelation( uint position )
{
	int iCounter = 0;
	int iCenter = (RADIUS_PATTERN_DIAMETER-1)/2;
	WeichiMove m(position);

	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		m_InitPatternIndex[iRadius] = 0;

		const vector<Point>& vOrdershiftPoint = WeichiRadiusPatternTable::vRadiusTableShiftOrder[iRadius];
		for( uint iNum=0; iNum<vOrdershiftPoint.size(); iNum++ ) {
			const Point& shiftPoint = vOrdershiftPoint[iNum];
			uint nbrPos = m.getShiftPosition(shiftPoint.getX(),shiftPoint.getY());

			if( nbrPos==-1 ) {
				m_InitPatternIndex[iRadius] += (COLOR_BORDER<<(iNum*2));
				continue;
			}
			
			assert( WeichiMove::isValidPosition(nbrPos) );
			m_radiusGridRelation[iCounter].m_iRadius = iRadius;
			m_radiusGridRelation[iCounter].m_neighborPos = nbrPos;
			m_radiusGridRelation[iCounter].m_iOffsetIndex = WeichiRadiusPatternTable::vRadiusShiftIndex[iCenter+shiftPoint.getY()][iCenter+shiftPoint.getX()];
			Point relativePoint = shiftPoint.getSymmetryOf(SYM_ROTATE_180);
			m_radiusGridRelation[iCounter].m_relativeIndex = WeichiRadiusPatternTable::vRadiusShiftIndex[iCenter+relativePoint.getY()][iCenter+relativePoint.getX()];
			iCounter++;
		}
	}

	m_radiusGridRelation[iCounter].m_iOffsetIndex = -1;
	m_radiusGridRelation[iCounter].m_iRadius = -1;
	m_radiusGridRelation[iCounter].m_neighborPos = -1;
	m_radiusGridRelation[iCounter].m_relativeIndex = -1;
	assert( iCounter<RADIUS_PATTERN_TABLE_SIZE );
}

void StaticGrid::initializeLadderPath( uint position )
{
	/*
		8 ladder type:
			LADDER_UP_RIGHT, LADDER_UP_LEFT, LADDER_DOWN_RIGHT, LADDER_DOWN_LEFT,
			LADDER_RIGHT_UP, LADDER_RIGHT_DOWN, LADDER_LEFT_UP, LADDER_LEFT_DOWN,
	*/
	WeichiMove move(position);
	const int pathDirX[NUM_LADDER_TYPE] = {1,-1,1,-1,1,1,-1,-1};
	const int pathDirY[NUM_LADDER_TYPE] = {1,1,-1,-1,1,-1,1,-1};

	if( m_LineNo==1 ) {
		for( uint ladderType=0; ladderType<NUM_LADDER_TYPE; ladderType++ ) {
			m_bmLadderPath[ladderType].Reset();

			int dirX = pathDirX[ladderType];
			int dirY = pathDirY[ladderType];
			if( ladderType<4 ) {
				setLadderPath(m_bmLadderPath[ladderType],position,0,dirY);
				setLadderPath(m_bmLadderPath[ladderType],move.getShiftPosition(dirX,0),0,dirY);
			} else {
				setLadderPath(m_bmLadderPath[ladderType],position,dirX,0);
				setLadderPath(m_bmLadderPath[ladderType],move.getShiftPosition(0,dirY),dirX,0);
			}

			m_bmLadderPath[ladderType].SetBitOff(position);
		}
	} else {
		for( uint ladderType=0; ladderType<NUM_LADDER_TYPE; ladderType++ ) {
			m_bmLadderPath[ladderType].Reset();

			int dirX = pathDirX[ladderType];
			int dirY = pathDirY[ladderType];
			setLadderPath(m_bmLadderPath[ladderType],position,dirX,dirY);
			setLadderPath(m_bmLadderPath[ladderType],move.getShiftPosition(dirX,0),dirX,dirY);
			setLadderPath(m_bmLadderPath[ladderType],move.getShiftPosition(0,dirY),dirX,dirY);
			setLadderPath(m_bmLadderPath[ladderType],move.getShiftPosition(dirX,-dirY),dirX,dirY);
			setLadderPath(m_bmLadderPath[ladderType],move.getShiftPosition(-dirX,dirY),dirX,dirY);
			if( ladderType<4 ) { setLadderPath(m_bmLadderPath[ladderType],move.getShiftPosition(-2*dirX,dirY),dirX,dirY); }
			else { setLadderPath(m_bmLadderPath[ladderType],move.getShiftPosition(dirX,-2*dirY),dirX,dirY); }

			m_bmLadderPath[ladderType].SetBitOff(position);
		}
	}
}

void StaticGrid::setLadderPath( WeichiBitBoard& bmPath, uint startPos, int dirX, int dirY )
{
	WeichiMove move(startPos);
	uint ladderPathPos = startPos;

	for( uint i=0; ladderPathPos!=-1; i++ ) {
		bmPath.SetBitOn(ladderPathPos);
		ladderPathPos = move.getShiftPosition(i*dirX,i*dirY);
	}
}