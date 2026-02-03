#include "WeichiBlock.h"

void WeichiBlock::merge( const WeichiBlock* block, const WeichiBitBoard& bmBoard )
{
    m_bmStone |= block->m_bmStone ;
    m_bmStonenNbrs |= block->m_bmStonenNbrs ;
    m_hash ^= block->m_hash ;

    m_numStone += block->getNumStone();
    m_liberty = getLibertyExclude(bmBoard) ;
}

void WeichiBlock::combineBlockWithoutUpdateLib( const WeichiBlock* block )
{
	m_bmStone |= block->m_bmStone;
	m_bmStonenNbrs |= block->m_bmStonenNbrs;
	m_hash ^= block->m_hash;
	m_numStone += block->getNumStone();
}

void WeichiBlock::removeBlockWithoutUpdateLib( const WeichiBlock* block )
{
	m_bmStone -= block->m_bmStone;
	m_hash ^= block->m_hash;
	m_numStone -= block->getNumStone();
}

void WeichiBlock::addGrid ( const WeichiGrid& grid, const WeichiBitBoard& bmBoard )
{
    m_bmStone.SetBitOn(grid.getPosition()) ;
    m_bmStonenNbrs |= grid.getStonenNbrMap() ;
    m_hash ^= grid.getStaticGrid().getHash(m_color) ;
    ++ m_numStone ;
    m_liberty = getLibertyExclude(bmBoard) ;
}

void WeichiBlock::removeGrid( const WeichiGrid& grid, const WeichiBitBoard& bmBoard )
{
	m_bmStone.SetBitOff(grid.getPosition());
	m_hash ^= grid.getStaticGrid().getHash(m_color);
	--m_numStone;
	m_liberty = getLibertyExclude(bmBoard);
}

void WeichiBlock::Clear()
{
    // TODO: implement this function
    m_id = static_cast<uint>(-1) ;
}

void WeichiBlock::init( WeichiGrid & grid )
{
    uint gridPosition = grid.getPosition();
    m_liberty = grid.getLiberty();
    m_color = grid.getColor();
    m_numStone = 1 ;
	m_modifyMoveNumber = -1;
	m_saveSequence = m_killSequence = NULL;

    // TODO: check this m_status = ?? ;
	m_bIsUsed = true;
    m_bmStone.Reset();
    m_bmStone.SetBitOn(gridPosition) ;
    m_bmStonenNbrs = grid.getStonenNbrMap() ;
    
    m_hash = grid.getStaticGrid().getHash(m_color);

    m_iFirstGrid = gridPosition ;

    m_closedAreas.clear();
	m_LADStatus = LAD_NOT_EYE;

	WeichiMove m(gridPosition);
	m_boundingBox.setUpBound(m.y()+1);
	m_boundingBox.setDownBound(m.y()+1);
	m_boundingBox.setLeftBound(m.x()+1);
	m_boundingBox.setRightBound(m.x()+1);
    m_boundingBox.setLowestPosition(gridPosition);

	m_dragonID = -1 ;
	m_snakeID = -1 ;
}

std::string WeichiBlock::getBlockStatusString() const
{
    ostringstream oss ;
    oss << "ID(" << setw(3) << m_id << ")"
		<< ", liberty(" << setw(2) << m_liberty << ")"
		<< ", grids: ";

	uint pos;
	WeichiBitBoard bmStone = m_bmStone;
	while( (pos=bmStone.bitScanForward())!=-1 ) {
		oss << " " << WeichiMove(pos).toGtpString();
	}

    return oss.str();
}

uint WeichiBlock::getLastLiberty(const WeichiBitBoard& bmBoard) const
{
    assert ( getLiberty() == 1 ) ;
    WeichiBitBoard bmLib = getStonenNbrMap() ;
    bmLib -= bmBoard ;
    uint lib = bmLib.bitScanForward();
    assert ( lib != -1 ) ;
    return lib ;
}


bool WeichiBlock::isNeighbor( const WeichiBlock* block ) const
{
    // 鄰居: 不是自己 且 對方棋子在我的棋子周圍
    return (this != block) && m_bmStonenNbrs.hasIntersection(block->getStoneMap());
}

////

WeichiBitBoard WeichiBlock::getLibertyBitBoard(const WeichiBitBoard& bmBoard) const
{
    WeichiBitBoard bLibertyBoard;

    bLibertyBoard = getStonenNbrMap();
    bLibertyBoard -= bmBoard;

    return bLibertyBoard;
} 
