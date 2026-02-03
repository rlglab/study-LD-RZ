#ifndef WEICHIMOVE_H
#define WEICHIMOVE_H

#include "BasicType.h"

/**
coordinate:
y
^
n|
.|
.|
.|
0|123...BS
+-------> x
0 ...  n

position:
board: 0~BORAD_SIZE*BOARD_SIZE-1
pass: BORAD_SIZE*BOARD_SIZE
*/

#include "Color.h"
#include "Point.h"

class WeichiMove
{
public:
	static const uint PASS_POSITION = MAX_NUM_GRIDS-1;
private:
	Color m_color ;
	uint m_position ;
public:
	/*!
	@brief  default constructor with color = NONE, Position = pass
	@author T.F. Liao
	*/
	inline WeichiMove ()
		: m_color (COLOR_NONE), m_position ( PASS_POSITION )
	{
	}
	/*!
	@brief  constructor with initial color and Position
	@author T.F. Liao
	@param  c [in] initial color
	@param  p [in] initial position
	*/
	inline WeichiMove ( Color c, uint p = PASS_POSITION )
		: m_color ( c ), m_position ( p )
	{
	}

	/*!
	@brief  constructor with color = NONE and initial Position
	@author T.F. Liao
	@param  p [in] initial position
	*/
	inline WeichiMove ( uint p )
		: m_color (COLOR_NONE), m_position( p )
	{
	}

	/*!
	@brief  constructor with color and position string
	@author T.F. Liao
	@param  c [in] initial color
	@param  pos_str [in] initial position
	*/
	inline WeichiMove ( Color c, const string& pos_str, const int boardsize=WeichiConfigure::BoardSize )
		: m_color (c)
	{
		if ( pos_str.length() < 2 ) {
			m_color = COLOR_NONE ;
			return ;
		}

		if ( pos_str == "tt" || pos_str == "@@" || pos_str.length() > 3 ) { // || pos_str == "PASS" || pos_str == "pass" ) {
			m_position = PASS_POSITION ;
			return ;
		}

		char cx = toupper(pos_str[0]),
			cy = toupper(pos_str[1]);

		int x = cx - 'A' ;
		int y = (boardsize-1) - (cy - 'A') ;
		if ( isdigit(cy) ) {
			x = x - (x>8?1:0);
			y = atoi(pos_str.substr(1).c_str())-1 ;
		}

		if ( x<0 || y<0 || x>=boardsize || y>=boardsize ) {
			m_color = COLOR_NONE ; // INVALID POSITION
		} else {
			m_position = toPosition ( (uint)x, (uint)y ) ;
		}
	}

	/*!
	@brief  get position of move
	@author T.F. Liao
	@return position of this move
	*/
	inline uint getPosition ( ) const {
		return m_position ;
	}

	/*!
	@brief  get compact position of move
	@author kwchen
	@return position of this move (ignore border)
	*/
	inline uint getCompactPosition() const {
		assert ( isPass() || y()*WeichiConfigure::BoardSize + x() < WeichiConfigure::BoardSize*WeichiConfigure::BoardSize );
		return isPass() ? WeichiConfigure::TotalGrids : y()*WeichiConfigure::BoardSize + x();
	}

	inline static uint rotateSoftPosition(uint softPos, SymmetryType type) {
		if (softPos == PASS_POSITION || type == SYM_NORMAL) { return softPos; }
		const int center = MAX_BOARD_SIZE / 2;
		int x = softPos % MAX_BOARD_SIZE;
		int y = softPos / MAX_BOARD_SIZE;
		Point point(x - center, y - center);
		Point rotatePoint = point.getSymmetryOf(type);
		return (rotatePoint.getY() + center) * MAX_BOARD_SIZE + (rotatePoint.getX() + center);
	}

	inline static uint rotateCompactPosition(uint compactPos, SymmetryType type) {
		if (compactPos == WeichiConfigure::TotalGrids || type == SYM_NORMAL) { return compactPos; }
		const int center = WeichiConfigure::BoardSize / 2;
		int x = compactPos % WeichiConfigure::BoardSize;
		int y = compactPos / WeichiConfigure::BoardSize;
		Point point(x - center, y - center);
		Point rotatePoint = point.getSymmetryOf(type);
		return (rotatePoint.getY() + center) * WeichiConfigure::BoardSize + (rotatePoint.getX() + center);
	}

	inline static uint toSoftPosition(uint compactPos) {
		if (compactPos == WeichiConfigure::TotalGrids) { return PASS_POSITION; }
		int x = compactPos % WeichiConfigure::BoardSize;
		int y = compactPos / WeichiConfigure::BoardSize;
		return y*MAX_BOARD_SIZE + x;
	}

	inline static uint toCompactPosition(uint softPos) {
		if (softPos == PASS_POSITION) { return WeichiConfigure::TotalGrids; }
		int x = softPos % MAX_BOARD_SIZE;
		int y = softPos / MAX_BOARD_SIZE;
		return (y * WeichiConfigure::BoardSize + x);
	}

	/*!
	@brief  get color of move
	@author T.F. Liao
	@return color of this move
	*/
	inline Color getColor ( ) const {
		return m_color ;
	}
	/*!
	@brief  set color of move
	@author T.F. Liao
	@param  color of this move
	*/
	inline void setColor ( Color c ) {
		m_color = c ;
	}
	/*!
	@brief  get x coordinate of move
	@author T.F. Liao
	@author 5D
	@date 2014/8/11
	@return x-coordinate of this move
	*/
	inline uint x () const {
		assert ( m_position != PASS_POSITION ) ;
		return m_position%MAX_BOARD_SIZE;
	}
	/*!
	@brief  get y coordinate of move
	@author T.F. Liao
	@author 5D
	@date 2014/8/11
	@return y-coordinate of this move
	*/
	inline uint y () const {
		assert ( m_position != PASS_POSITION ) ;
		return m_position/MAX_BOARD_SIZE;
	}

	/*!
	@brief  overloading compare operator
	@author T.F. Liao
	@param  [in] move to be compared
	@return true if this is the same with move
	*/
	inline bool operator== ( const WeichiMove& move ) const {
		return this->m_color == move.m_color &&
			this->m_position == move.m_position ;
	}

	/*!
	@brief  overloading not equal operator
	@author 5D
	@date 2014/11/8
	@param  [in] move to be compared
	@return true if this is the same with move
	*/
	inline bool operator!= ( const WeichiMove& move ) const {
		return this->m_color != move.m_color ||
			this->m_position != move.m_position ;
	}

	/*!
	@brief  translate (x,y) to position
	@author T.F. Liao
	@author 5D
	@date 2014/8/11
	@param  x [in] x-coordinate
	@param  y [in] y-coordinate
	@return position of (x,y)
	*/
	inline static uint toPosition ( uint x, uint y ) {
		assert ( x>=0 && y>=0 ) ;
		assert ( x<WeichiConfigure::BoardSize && y<WeichiConfigure::BoardSize ) ;
		return y*MAX_BOARD_SIZE + x;
	}

	inline WeichiMove rotateMove ( SymmetryType type ) const {
		if( type==SYM_NORMAL ) { return *this; }

		const int center = MAX_BOARD_SIZE/2;
		int x = m_position % MAX_BOARD_SIZE;
		int y = m_position / MAX_BOARD_SIZE;
		Point point(x-center,y-center);
		Point rotatePoint = point.getSymmetryOf(type);

		uint rotatePosition = ((rotatePoint.getY()+center)*MAX_BOARD_SIZE + (rotatePoint.getX()+center));
		return WeichiMove(m_color,rotatePosition);
	}

	/*!
	@brief  translate position to (x,y)
	@author T.F. Liao
	@author 5D
	@date 2014/8/11
	@param  position [in] position in board
	@param  x [out] x-coordinate
	@param  y [out] y-coordinate
	*/
	inline static void toCoordinate ( uint position, uint& x, uint& y ) {
		assert ( position <= PASS_POSITION ) ; //  form 0 to BOARD_SZIE*BOARD_SZIE
		x = position%MAX_BOARD_SIZE ;
		y = position/MAX_BOARD_SIZE ;
	}

	inline static bool isValidPosition ( uint position ) {
		uint x,y;
		toCoordinate(position,x,y);
		return (x>=0 && x<WeichiConfigure::BoardSize && y>=0 && y<WeichiConfigure::BoardSize);
	}

	inline uint getShiftPosition( int x, int y ) {
		uint newX = m_position%MAX_BOARD_SIZE + x;
		uint newY = m_position/MAX_BOARD_SIZE + y;

		if( newX<0 || newY<0 || newX>=WeichiConfigure::BoardSize || newY>=WeichiConfigure::BoardSize ) { return -1; }
		else { return newY*MAX_BOARD_SIZE + newX; }
	}

	/*!
	@brief  translate to GTP string
	@author T.F. Liao
	@return string in GTP format
	*/
	string toGtpString () const
	{
		if ( m_position >= PASS_POSITION ) return "PASS" ;
		else {
			ostringstream oss ;
			oss << (char)(x()+'A'+(x()>=8)) << y()+1 ;
			return oss.str();
		}
	}

	string toEditorMoveJLMCTSString() const
	{
		ostringstream oss;		
		if (m_position >= PASS_POSITION) { oss << "@@"; }
		else {
			// y() 從 0 到 1，代表 y 軸從下到上 + 'A' 即可轉換成 數字
			// 0 => A, 1=> B 
			oss << (char)(x() + 'A') << (char)(y() + 'A');
		}

		return oss.str();
	}

	/*!
	@brief  translate to Editor coordinate string
	@author C.C. Shih
	@return string in GTP format
	*/
	string toEditorCoorString () const
	{
		if (m_position >= PASS_POSITION) { return "@@"; }
		
		ostringstream oss;
		oss << (char)(x() + 'A') << (char)(y() + 'A');
		return oss.str();
	}

	string toEditorGtpString() const
	{
		if (m_position >= PASS_POSITION) { return "PASS"; }
		
		ostringstream oss;
		oss << (char)(x() + 'A' + (x() >= 8)) << y() + 1;
		return oss.str();
	}

	/*!
	@brief  translate to SGF string
	@author T.F. Liao
	@param  withColor [in] flag that indicate output with Color info or not
	@return string in SGF format
	*/
	string toSgfString ( bool withColor = true ) const
	{
		ostringstream oss ;
		if ( withColor ) {
			oss << toChar(m_color) << '[';
		}
		if ( m_position == PASS_POSITION ) {
			oss << "tt" ;
		} else {
			oss << (char)(x()+'a') << (char)(((WeichiConfigure::BoardSize-1)-y())+'a') ;
		}

		if ( withColor ) oss << ']' ;
		return oss.str();
	}

	string toSgfStringItem ( ) const
	{
		ostringstream oss ;
		oss << '[' ;		
		if ( m_position == PASS_POSITION ) {
			oss << "tt" ;
		} else {
			oss << (char)(x()+'a') << (char)(((WeichiConfigure::BoardSize-1)-y())+'a') ;
		}
		oss << ']' ;
		return oss.str();
	}

	string toString() const { return toSgfString() ; }

	/*!
	@brief  check if this move is pass move
	@author T.F. Liao
	@return true if this move is pass move
	*/
	bool isPass () const {
		return m_position==PASS_POSITION ;
	}

	bool operator<(const WeichiMove& rhs) const
	{
		return this->m_position < rhs.m_position ;
	}
};

/// pre-defined PASS-MOVE
const WeichiMove PASS_MOVE = WeichiMove() ;

#endif 
