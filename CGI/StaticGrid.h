#ifndef STATICGRID_H
#define STATICGRID_H

#include "BasicType.h"
#include "BitBoard.h"
#include "WeichiMove.h"
#include "Pattern33Table.h"
#include "WeichiRadiusPatternTable.h"
class StaticBoard ;

class StaticGrid
{
public:
    struct GridRelation
    {
        bool adjacent ;
        bool diagonal ;
        bool in3x3 ;
        GridRelation() { memset (this, 0, sizeof(GridRelation)); }
    };

	struct RadiusGridRelation
	{
		uint m_iRadius;
		uint m_neighborPos;
		uint m_iOffsetIndex;
		uint m_relativeIndex;
	};

private:
    uint m_LongLineNo, m_LineNo, m_xLineNo, m_yLineNo ; // 1(line 1) 2(line 2) 3(line 3) ~ 10(line 10)
    uint m_pos ;
    int m_x, m_y ; // 0~18, 19~31(unused) 20(pass) 30(koLimited)

    WeichiBitBoard m_bmStoneNbrs;
	WeichiBitBoard m_bm3x3StoneNbrs;

	// direction & relation with other grid
	uint m_grid_direction[MAX_NUM_GRIDS];
    GridRelation m_grid_relation[MAX_NUM_GRIDS];

    /// pre-calculated neighbors position
    int m_adjpos[5] ;
    int m_adjrel[5] ;
    int m_diagpos[5] ;
    int m_diagrel[5] ;
	int m_diamondendpos[5] ;
	int m_diamondendrel[5] ;
	int m_3x3pos[9] ;
    int m_3x3rel[9] ;
    int m_diamondpos[13] ; 
    int m_diamondrel[13] ; 
    int m_neighbors[12] ;
	RadiusGridRelation m_radiusGridRelation[RADIUS_PATTERN_TABLE_SIZE];	// last 1 is -1

    uint m_nCandidates ;
    uint m_candidates[MAX_NEW_CAND] ;
    WeichiBitBoard m_bmCandidates;

    // ZHashKeys, use 3 location for black/ white and reserve [0] for speed up
    HashKey64 m_hash[3] ;

	// radius pattern index
	uint m_InitPatternIndex[MAX_RADIUS_SIZE];

    // pre-stored values to be added to a need-to-be-changed pattern index
    int m_playWhiteIndexDifference[8] ;
    int m_playBlackIndexDifference[8] ;

	// pre-calculated ladder path
	WeichiBitBoard m_bmLadderPath[NUM_LADDER_TYPE] ;

public:
    /*!
        @brief  default constructor
        @author T.F. Liao
    */
    StaticGrid() {}
    /*!
        @brief  initialization, should be invoked by StaticBoard foreach grid 
        @author T.F. Liao
        @param  position    [in] position of this grid
    */
    void initialize ( uint position ) ;

    /************************************************************************/
    /* neighbor maps                                                        */
    /************************************************************************/
    // stored as -1 terminated array
    //         UU           
    //     LU   U   RU      
    // LL   L   .   R   RR  
    //     LB   B   RB      
    //         BB           
    // adjacent: L, U, R, B
    // diagonal: LU, RU, RB, LB
    // 3x3: L, LU, U, RU, R, RB, B, LB
    // Diamond: L, LU, U, RU, R, RB, B, LB, LL, UU, RR, BB

    inline const int* getAdjacentNeighbors () const { return m_adjpos; }
    inline const int* getDiagonalNeighbors () const { return m_diagpos; }
	inline const int* getDiamondEndNeighbors () const { return m_diamondendpos; }
    inline const int* get3x3Neighbors () const { return m_3x3pos; }
    inline const int* getDiamondNeighbors () const { return m_diamondpos; }
    
    inline const int* getAdjacentRelations () const { return m_adjrel; }
    inline const int* getDiagonalRelations () const { return m_diagrel; }
	inline const int* getDiamondEndRelations () const { return m_diamondendrel; }
    inline const int* get3x3Relations () const { return m_3x3rel; }
    inline const int* getDiamondRelations() const { return m_diamondrel; }
    
	inline const RadiusGridRelation* getRadiusGridRelations() const { return m_radiusGridRelation; }

    /*!
        @brief  position of particular neighbor of this grid
        @author T.F. Liao
        @param  direction   [in]    indicate which neighbor
        @return position of the neighbor for this grid
    */
    inline int getNeighbor ( uint direction ) const { assert ( direction < NUM_DIRECTIONS ) ; return m_neighbors[direction] ; }

	/*!
        @brief  get reverse neighbor of position & direction
        @author C.C. Shih
        @param  position    [in] position of this grid
        @param  direction   [in] direction of neighbor to find
        @return the reverse neighbor position of given position & direction, -1 indicate no such neighbor
    */
    static int getReverseNeighbor ( uint position, uint direction ) ;

    /*!
        @brief  get bitboard contains this grid and neighbors
        @author T.F. Liao
        @return bitboard contains this grid and neighbors
    */
    inline const WeichiBitBoard& getStoneNbrsMap () const { return m_bmStoneNbrs; }
	inline const WeichiBitBoard& get3x3StoneNbrsMap() const { return m_bm3x3StoneNbrs; }

    /*!
        @brief  get HashKey for Color c in this grid
        @author T.F. Liao
        @return HashKey64 of hash key
    */
    inline HashKey64 getHash ( Color c ) const { assert(c==COLOR_WHITE ||c==COLOR_BLACK); return m_hash[c] ; }

	inline uint getInitialRadiusPattern ( int radius ) const { return m_InitPatternIndex[radius]; }

    inline uint getXLineNo () const { return m_xLineNo ; }
    inline uint getYLineNo () const { return m_yLineNo ; }
    inline uint getLineNo () const { return m_LineNo ; }
	inline uint getLongLineNo () const { return m_LongLineNo ; }

    inline int x() const { return m_x ; }
    inline int y() const { return m_y ; }

    inline bool isCorner () const { 
        if( getXLineNo()==1 && getYLineNo()==1 ) return true;
        else return false;
    }

	inline bool isEdge () const { 
		if( getXLineNo()==1 && getYLineNo()!=1 ) return true;
		else if( getXLineNo()!=1 && getYLineNo()==1 ) return true ;
		else return false;
	}

    /*!
        @brief  get number of candidates that should add after this grid is put
        @author T.F. Liao
        @return number of candidates
    */
    inline uint getNumCandidate () const { return m_nCandidates ; }
    /*!
        @brief  get candidates that should add after this grid is put
        @author T.F. Liao
        @return pointer to array of candidates
    */
    inline const uint* getCandidates () const { return m_candidates; }
    inline const WeichiBitBoard& getCandidatesMap () const { return m_bmCandidates; }

	inline const WeichiBitBoard& getLadderPath ( uint ladderType ) const { assert( ladderType<NUM_LADDER_TYPE ); return m_bmLadderPath[ladderType]; }

    /*
        @brief  get the number to be added when a stone with Color = c is put
        @author T.C. Ho, T.F. Liao
        @return pointer to array of differences
    */
    inline const int* getPatternIndexDifference(Color c) const ;

	inline uint getDirectionWith( uint pos ) const { assert( WeichiMove::isValidPosition(pos) ); return m_grid_direction[pos]; }
    inline GridRelation getRelationWith ( uint pos ) const { assert( WeichiMove::isValidPosition(pos) ); return m_grid_relation[pos]; }

private:
    /*!
        @brief  initialize basic features
        @author T.F. Liao
        @param  position    [in] position of this grid
    */
    void initializeFeatures ( uint position ) ;
    /*!
        @brief  initialize all pre-calculated neighbor position with -1 terminated
        @author T.F. Liao
        @param  position    [in] position of this grid
    */
    void initializeNeighbors ( uint position ) ;

    /*!
        @brief  initialize all pre-calculated candidate range (bitboard)
        @author T.F. Liao
        @param  position    [in] position of this grid
    */
    void initializeCandidateRange ( uint position ) ;
    /*!
        @brief  initialize HashKey for black or white put in this grid
        @author T.F. Liao
        @param  position    [in] position of this grid
    */
    void initializeZHashKey ( uint position ) ;

    /*!
        @brief  get neighbor of position & direction
        @author T.F. Liao
        @param  position    [in] position of this grid
        @param  direction   [in] direction of neighbor to find
        @return neighbor position of given position & direction, -1 indicate no such neighbor
    */
    static int getNeighbor ( uint position, uint direction ) ;

    /*!
        @brief  initialize the number to be added when making a move
        @author T.C. Ho
        @param  position    [in] position of this grid
    */
    void initializeDifference ( uint position ) ;

    /*!
        @brief  get the number to be stored in m_playBlackIndexDifference and m_playWhiteIndexDifference, the numbers may be 3^0, 3^1, ..., 3^8
        @author T.C. Ho
        @param  nbr the neighbor
        @param  rel the direction(relation) of nbr
        @return the number corresponding to nbr
    */
    int getOffset ( int nbr, int rel )  ;

	void initializeRadiusGridRelation ( uint position ) ;
	void initializeLadderPath ( uint position ) ;
	void setLadderPath ( WeichiBitBoard& bmPath, uint startPos, int dirX, int dirY );

    /// no implement of copy constructor
    StaticGrid(const StaticGrid& ) ;
    /// no implement of copy assignment
    StaticGrid operator=(const StaticGrid&) const ;
};

const int* StaticGrid::getPatternIndexDifference(Color c) const
{
    assert(ColorNotEmpty(c)) ;
    return (c==COLOR_BLACK?m_playBlackIndexDifference:m_playWhiteIndexDifference) ;
}

#endif 
