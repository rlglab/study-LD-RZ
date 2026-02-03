#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "BasicType.h"
#include "StaticBoard.h"

class BoundingBox{
private:
	uint m_right:5, m_left:5, m_up:5, m_down:5; //以第幾線表示，可能值是1到BOARD_SIZE
	uint m_lowestPosition:9;  //最小的點位置
public:
	BoundingBox():m_right(0), m_left(0x1f), m_up(0), m_down(0x1f), m_lowestPosition(MAX_NUM_GRIDS) {}
	BoundingBox& operator= ( const BoundingBox& rhs ){
		m_right = rhs.m_right;
		m_left = rhs.m_left;
		m_up = rhs.m_up;
		m_down = rhs.m_down;
		m_lowestPosition = rhs.m_lowestPosition;
		return *this;
	}
	inline bool isInitialized()     const { return m_lowestPosition != MAX_NUM_GRIDS; }
	inline uint getRightBound()     const { assert( isInitialized() ); return m_right; }
	inline uint getLeftBound()      const { assert( isInitialized() ); return m_left; }
	inline uint getUpBound()        const { assert( isInitialized() ); return m_up; }
	inline uint getDownBound()      const { assert( isInitialized() ); return m_down; }
	inline uint getLowestPosition() const { assert( isInitialized() ); return m_lowestPosition; }
	inline void setRightBound( uint rightBound ) {
// 		assert( rightBound<=BOARD_SIZE+1 && rightBound>=1 );
// 		if( rightBound==BOARD_SIZE ) { m_right=BOARD_SIZE+1; }
// 		else { m_right = rightBound; }
		//assert( rightBound<=WeichiConfigure::BoardSize && rightBound>=1 );
		m_right = rightBound;
	}
	inline void setLeftBound( uint leftBound ) {
// 		assert( leftBound<=BOARD_SIZE && leftBound>=0 );
// 		if( leftBound==1 ) { m_left=0; }
// 		else { m_left = leftBound; }
		//assert( leftBound<=WeichiConfigure::BoardSize && leftBound>=1 );
		m_left = leftBound;
	}
	inline void setUpBound( uint upBound ) {
// 		assert( upBound<=BOARD_SIZE+1 && upBound>=1 );
// 		if( upBound==BOARD_SIZE ) { m_up=BOARD_SIZE+1; }
// 		else { m_up = upBound; }
		//assert( upBound<=WeichiConfigure::BoardSize && upBound>=1 );
		m_up = upBound;
	}
	inline void setDownBound( uint downBound ) {
// 		assert( downBound<=BOARD_SIZE && downBound>=0 );
// 		if( downBound==1 ) { m_down=0; }
// 		else { m_down = downBound; }
		//assert( downBound<=WeichiConfigure::BoardSize && downBound>=1 );
		m_down = downBound;
	}
	inline void setLowestPosition(uint position){ assert( position<MAX_NUM_GRIDS && position>=0 ); m_lowestPosition = position; }

	inline uint getLineNo() const
	{
		/*uint line1 = lowestPosition / BOARD_SIZE + 1;
		uint line2 = BOARD_SIZE - lowestPosition / BOARD_SIZE + 1;
		uint line3 = lowestPosition % BOARD_SIZE + 1;
		uint line4 = BOARD_SIZE - lowestPosition % BOARD_SIZE + 1;
		uint min = line1;
		if( min > line2 ) min = line2;
		if( min > line3 ) min = line3;
		if( min > line4 ) min = line4;*/
		assert( isInitialized() );
		return StaticBoard::getGrid(m_lowestPosition).getLineNo();
	}

	inline WeichiBitBoard getBitBoard() const
	{
		WeichiBitBoard bmBorder;

		/*if( getUpBound()!=BOARD_SIZE ) { StaticBoard::getHorizontalBorder(getUpBound()-1); }
		if( getDownBound()!=1 ) { StaticBoard::getHorizontalBorder(getDownBound()-1); }
		if( getLeftBound()!=1 ) { StaticBoard::getVerticalBorder(getLeftBound()-1); }
		if( getRightBound()!=BOARD_SIZE ) { StaticBoard::getVerticalBorder(getRightBound()-1); }
		bmBorder |= StaticBoard::getVerticalBorder(EXTEND_BOARD_SIZE-1);*/

		return bmBorder;
	}

	inline uint getCenterPosition() const
	{
		uint x = (getRightBound() + getLeftBound()) / 2 - 1;
		uint y = (getUpBound() + getDownBound()) / 2 - 1;
		// if x or y is on Line1, change to Line2
		if ( x==0 && (y==1 || y==WeichiConfigure::BoardSize-2) ) { x = 1; }
		if ( y==0 && (x==1 || x==WeichiConfigure::BoardSize-2) ) { y = 1; }
		return WeichiMove::toPosition(x, y);
	}

	inline void combine( const BoundingBox& boundingBox )
	{
		if( boundingBox.getUpBound() > m_up ) { setUpBound(boundingBox.getUpBound()); }
		if( boundingBox.getDownBound() < m_down ) { setDownBound(boundingBox.getDownBound()); }
		if( boundingBox.getLeftBound() < m_left ) { setLeftBound(boundingBox.getLeftBound()); }
		if( boundingBox.getRightBound() > m_right ) { setRightBound(boundingBox.getRightBound()); }
		if( boundingBox.getLowestPosition() < m_lowestPosition ) { setLowestPosition(boundingBox.getLowestPosition()); }
	}

	inline void combine( const uint& pos )
	{
		const StaticGrid& sgrid = StaticBoard::getGrid(pos);
		const uint x = static_cast<uint>( sgrid.x() + 1 );
		const uint y = static_cast<uint>( sgrid.y() + 1 );
		if( y > m_up ) { setUpBound( y ); }
		if( y < m_down ) { setDownBound( y ); }
		if( x < m_left ) { setLeftBound( x ); }
		if( x > m_right ) { setRightBound( x ); }
		if( pos < m_lowestPosition ) { setLowestPosition(pos); }
	}

	inline bool isInBoundingBox( const uint& pos ) const
	{
		const StaticGrid& sgrid = StaticBoard::getGrid(pos);
		const uint x = static_cast<uint>( sgrid.x() + 1 );
		const uint y = static_cast<uint>( sgrid.y() + 1 );

		if( getUpBound()<WeichiConfigure::BoardSize && y>=getUpBound() ) { return false; }
		if( getDownBound()>1 && y<=getDownBound() ) { return false; }
		if( getLeftBound()>1 && x<=getLeftBound() ) { return false; }
		if( getRightBound()<WeichiConfigure::BoardSize && x>=getRightBound() ) { return false; }

		return true;
	}

	std::string toString()
	{
		std::stringstream oss;
		oss << "up: " << m_up << endl;
		oss << "down: " << m_down << endl;
		oss << "left: " << m_left << endl;
		oss << "right: " << m_right << endl;
		oss << "lowestPosition: " << m_lowestPosition << endl;

		return oss.str();
	}
};
#endif