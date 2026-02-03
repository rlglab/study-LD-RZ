#ifndef STATICBOARD_H
#define STATICBOARD_H

#include "BasicType.h"
#include "StaticGrid.h"
#include "StaticEdge.h"
#include "WeichiHashGenerator.h"
#include "Dual.h"

typedef WeichiHashGenerator<MAX_BOARD_SIZE> HashGenerator;

class StaticBoard
{
private:
    static StaticGrid s_grids[MAX_NUM_GRIDS] ;
	static StaticEdge s_edges[MAX_NUM_GRIDS] ;
    static uint s_revDir[NUM_DIRECTIONS] ;
    static HashGenerator s_hashGenerator ;

    static WeichiBitBoard s_bmCandidate ;
	static WeichiBitBoard s_maskBorder ;
    static WeichiBitBoard s_verticalBorder[MAX_BOARD_SIZE] ;
    static WeichiBitBoard s_horizontalBorder[MAX_BOARD_SIZE] ;
    static WeichiBitBoard s_emptyBoard ;
	static WeichiBitBoard s_cornerBitBoard ;
	static WeichiBitBoard s_edgeBitBoard ;
	static WeichiBitBoard s_centralBitBoard ;

    static Vector<uint, 4> s_vPatternAdjDir[512] ;
    static Vector<uint, 4> s_vPatternDiagDir[512] ;
    static Vector<uint, 8> s_vPattern3x3Dir[512] ;
    static Vector<uint, 4> s_vPatternDirV8Adj[512] ;
    static Vector<uint, 4> s_vPatternDirV8Diag[512] ;

	static pattern33::Pattern33 s_Pattern33[pattern33::TABLE_SIZE] ;
	static Vector<uint,8> s_vEdgeDirections[pattern33::TABLE_SIZE][3] ;			// 3 for center color (B,W,E)
	static Vector<WeichiEdgeBits, 8> s_vEdgeBits[pattern33::TABLE_SIZE][3] ;

	static double s_outerWideningWinRateThreshold;
	static double s_wideningIndex[MAX_EXPAND_CHILD_NODE];

	static uint s_boardPos[MAX_NUM_GRIDS] ;	// last one is -1
	static uint s_goguiPos[MAX_NUM_GRIDS] ;

#ifndef NDEBUG
    static bool isInitialized ;
#endif

public:
	class iterator
	{
		friend class StaticBoard;
	private:
		uint m_current;
		uint *m_position;
	public:
		iterator( uint* position ) : m_current(0), m_position(position) {}
		iterator& operator++() { m_current++; return *this; }
		uint& operator*() { return m_position[m_current]; }
		operator bool() const { return (m_position[m_current]!=-1); }
	};

    static void initialize() ;
	static void initializeBoardSizeArgument( uint boardSize ) ;
    inline static const StaticGrid& getGrid ( uint position ) ;
	inline static const StaticEdge& getEdge ( uint position ) ;
    /*!
        @brief  get reversed direction, e.g. for DIR_UP, reversed is DIR_DOWN
        @author T.F. Liao
        @param  direct  [in]    direct to get its reversed direction
        @return uint as reversed direction
    */
    inline static const uint getRevDirect ( uint direct ) ;

    inline static const HashGenerator& getHashGenerator () { return s_hashGenerator ; }

    //inline static HashKey64 getHashKey ( Color c, uint position ) ;

    inline static const WeichiBitBoard& getInitCandidate () { return s_bmCandidate ; }
	inline static const WeichiBitBoard& getMaskBorder() { return s_maskBorder; }
    inline static const WeichiBitBoard& getVerticalBorder ( const uint value )
		{ if ( 0<=value && value<WeichiConfigure::BoardSize ) return s_verticalBorder[value]; return StaticBoard::s_emptyBoard; }
    inline static const WeichiBitBoard& getHorizontalBorder ( const uint value )
		{ if ( 0<=value && value<WeichiConfigure::BoardSize ) return s_horizontalBorder[value]; return StaticBoard::s_emptyBoard; }
	inline static const WeichiBitBoard& getCornerBitBoard() { return s_cornerBitBoard; }
	inline static const WeichiBitBoard& getEdgeBitBoard() { return s_edgeBitBoard; }
	inline static const WeichiBitBoard& getCentralBitBoard() { return s_centralBitBoard; }

	inline static const double getOuterWideningWinRateThreshold() { return s_outerWideningWinRateThreshold; }
	inline static const double getWideningIndex( int iIndex ) { assert( iIndex<MAX_EXPAND_CHILD_NODE ); return s_wideningIndex[iIndex]; }

	inline static const iterator getIterator() { return iterator(s_boardPos); }
	inline static const iterator getGoguiIterator() { return iterator(s_goguiPos); }

    inline static const Vector<uint, 4>& getPatternAdjacentDirection(uint value)  { assert(value<512); return s_vPatternAdjDir[value]; }
    inline static const Vector<uint, 4>& getPatternDiagonalDirection(uint value) { assert(value<512); return s_vPatternDiagDir[value]; }
    inline static const Vector<uint, 8>& getPattern3x3Direction(uint value)  { assert(value<512); return s_vPattern3x3Dir[value]; }
    inline static const Vector<uint, 4>& getPatternAdjacentIn3x3(uint value)   { assert(value<512); return s_vPatternDirV8Adj[value]; }
    inline static const Vector<uint, 4>& getPatternDiagonalIn3x3(uint value)  { assert(value<512); return s_vPatternDirV8Diag[value]; }

	inline static const Vector<WeichiEdgeBits, 8>& getEdgeBits( uint index, Color centerColor )
		{ assert(index<pattern33::TABLE_SIZE); return s_vEdgeBits[index][centerColor]; }
	inline static const Vector<uint, 8>& getEdgeDirection( uint index, Color centerColor )
		{ assert(index<pattern33::TABLE_SIZE); return s_vEdgeDirections[index][centerColor]; }

	inline static const pattern33::Pattern33& getPattern33(uint index) { assert(index<pattern33::TABLE_SIZE); return s_Pattern33[index]; }

    static bool acuteAngle ( uint v0pos, uint v1pos, uint v2pos );
    static double distance2 ( uint x1, uint y1, uint x2, uint y2);
private:
    static void initializePatternDirectTable() ;
	static void initializeCandidates();

    static void initializeBorder() ;
	static void initializeAttributeBitBoard() ;
	static void initializeBoardPosition() ;
	static void initializeWideningIndex() ;
	static void initializePattern33Table() ;
	static void initializeEdgeDirectionBits() ;
	static void calculateEdgeBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits ) ;
	static void calculateDiagonalBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits ) ;
	static void calculateJumpBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits ) ;
	static void calculateKnightBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits ) ;
	static void calculateBambooBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits ) ;
	static void calculatehBambooBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits ) ;
	static void calculateLHalfBambooBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits ) ;
	static void calculateTHalfBambooBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits ) ;
	static void calculateZHalfBambooBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits ) ;
	static void calculateWallJumpBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits ) ;
	static void calculateWallBambooBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits ) ;
};

inline const StaticGrid& StaticBoard::getGrid( uint position )
{
    assert ( isInitialized && WeichiMove::isValidPosition(position) ) ;
    return s_grids[position] ;
}

inline const StaticEdge& StaticBoard::getEdge( uint position )
{
	assert ( isInitialized && WeichiMove::isValidPosition(position) ) ;
	return s_edges[position] ;
}

inline const uint StaticBoard::getRevDirect( uint direct )
{
    assert ( direct < NUM_DIRECTIONS ) ;
    return s_revDir[direct] ;
}

#endif