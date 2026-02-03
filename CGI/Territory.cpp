#include "Territory.h"
#include "StaticBoard.h"

/*void Territory::clear()
{
    m_count = 0 ;
    for ( uint i=0;i<NUM_GRIDS_NO;++i ) {
		m_vCount[i] = 0;
		m_territory[i].reset();
	}
}

void Territory::addTerritory( const Vector<Color, NUM_GRIDS_NO>& territory )
{
	++m_count;
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		if( territory[*it]==COLOR_NONE ) { continue; }
		assert(territory[*it]==COLOR_BLACK || territory[*it]==COLOR_WHITE) ;
		++m_territory[*it].get(territory[*it]);
		++m_vCount[*it];
	}
}

float Territory::getTerritory( uint pos ) const 
{
	assert( pos<NUM_GRIDS_NO || pos%EXTEND_BOARD_SIZE!=BOARD_SIZE );
	if( m_vCount[pos]==0 ) { return 0; }
	else { return (m_territory[pos].black-m_territory[pos].white)/(float)m_vCount[pos]; }
}*/